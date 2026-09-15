//! Narrow, authenticated Axum transport adapter for `RequestLifecycleService`.
//!
//! This crate deliberately contains HTTP concerns only. It neither creates a
//! native engine per request nor reimplements admission, workspace lifecycle,
//! native execution, or cancellation policy.

use axum::body::Body;
use axum::extract::multipart::MultipartError;
use axum::extract::{FromRequest, Multipart, Request, State};
use axum::http::{header, HeaderMap, HeaderValue, StatusCode};
use axum::response::{IntoResponse, Response};
use axum::routing::post;
use axum::{Json, Router};
use imgengine_supervisor::{
    AdmissionController, AdmissionOptions, ApplicationError, ApplicationFailure, IncomingRequest,
    RequestLifecycleService, RequestPolicy, SupervisorOptions,
};
use std::env;
use std::path::PathBuf;
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::Arc;
use std::time::Duration;

/// Provisional integration limit, not a production capacity commitment.
pub const DEFAULT_MAX_REQUEST_BYTES: usize = 1_048_576;
/// Provisional integration deadline, evaluated by the existing lifecycle.
pub const DEFAULT_EXECUTION_TIMEOUT: Duration = Duration::from_secs(10);
/// Environment variable holding active server-side API keys.
pub const API_KEYS_ENV: &str = "IMGENGINE_API_KEYS";

static TRANSPORT_REQUEST_SEQUENCE: AtomicU64 = AtomicU64::new(1);

/// Fail-closed configuration errors that never include credential values.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ConfigError {
    MissingApiKeys,
    InvalidApiKeys,
    InvalidLimits,
    LifecycleUnavailable,
}

/// Process configuration for the narrow P5 adapter.
#[derive(Clone, Debug)]
pub struct ApiConfig {
    api_keys: Vec<String>,
    workspace_root: PathBuf,
    max_request_bytes: usize,
    queue_capacity: usize,
    execution_timeout: Option<Duration>,
}

impl ApiConfig {
    /// Loads API keys from process configuration and fails closed on bad input.
    pub fn from_env(workspace_root: impl Into<PathBuf>) -> Result<Self, ConfigError> {
        let value = env::var(API_KEYS_ENV).map_err(|_| ConfigError::MissingApiKeys)?;
        Self::new(value, workspace_root)
    }

    /// Creates configuration from an explicit value, primarily for controlled tests.
    pub fn new(
        api_keys: impl AsRef<str>,
        workspace_root: impl Into<PathBuf>,
    ) -> Result<Self, ConfigError> {
        let api_keys = parse_api_keys(api_keys.as_ref())?;
        Ok(Self {
            api_keys,
            workspace_root: workspace_root.into(),
            max_request_bytes: DEFAULT_MAX_REQUEST_BYTES,
            queue_capacity: 1,
            execution_timeout: Some(DEFAULT_EXECUTION_TIMEOUT),
        })
    }

    /// Sets explicitly provisional limits for an integration environment.
    pub fn with_limits(
        mut self,
        max_request_bytes: usize,
        queue_capacity: usize,
        execution_timeout: Option<Duration>,
    ) -> Result<Self, ConfigError> {
        if max_request_bytes == 0
            || queue_capacity == 0
            || matches!(execution_timeout, Some(timeout) if timeout.is_zero())
        {
            return Err(ConfigError::InvalidLimits);
        }
        self.max_request_bytes = max_request_bytes;
        self.queue_capacity = queue_capacity;
        self.execution_timeout = execution_timeout;
        Ok(self)
    }
}

/// Running API state. Dropping it drains the existing admission worker.
#[derive(Clone)]
pub struct ApiState {
    lifecycle: Arc<RequestLifecycleService>,
    api_keys: Arc<Vec<String>>,
    max_request_bytes: usize,
}

impl ApiState {
    /// Starts the one existing supervisor/admission path for this process.
    pub fn start(config: ApiConfig) -> Result<Self, ConfigError> {
        let supervisor = SupervisorOptions::new(config.workspace_root);
        let admission =
            AdmissionOptions::new(supervisor, config.queue_capacity, config.max_request_bytes)
                .map_err(|_| ConfigError::InvalidLimits)?;
        let admission = Arc::new(
            AdmissionController::start(admission).map_err(|_| ConfigError::LifecycleUnavailable)?,
        );
        let policy = RequestPolicy::new(config.max_request_bytes, config.execution_timeout)
            .map_err(|_| ConfigError::InvalidLimits)?;
        Ok(Self {
            lifecycle: Arc::new(RequestLifecycleService::new(admission, policy)),
            api_keys: Arc::new(config.api_keys),
            max_request_bytes: config.max_request_bytes,
        })
    }
}

/// Builds only the P5 route. Legacy routes are deliberately absent.
pub fn router(state: ApiState) -> Router {
    Router::new()
        .route("/api/v1/render", post(render))
        .layer(axum::extract::DefaultBodyLimit::max(
            state.max_request_bytes,
        ))
        .with_state(state)
}

async fn render(State(state): State<ApiState>, request: Request) -> Response {
    let fallback_request_id = next_transport_request_id();
    if !authorized(request.headers(), &state.api_keys) {
        return problem(
            StatusCode::UNAUTHORIZED,
            "authentication_failed",
            "Authentication failed",
            fallback_request_id,
        );
    }

    let mut multipart = match Multipart::from_request(request, &state).await {
        Ok(multipart) => multipart,
        Err(rejection) => {
            let status = rejection.into_response().status();
            return if status == StatusCode::PAYLOAD_TOO_LARGE {
                problem(
                    StatusCode::PAYLOAD_TOO_LARGE,
                    "resource_limit",
                    "Request exceeds the configured limit",
                    fallback_request_id,
                )
            } else {
                problem(
                    StatusCode::BAD_REQUEST,
                    "invalid_request",
                    "Invalid multipart request",
                    fallback_request_id,
                )
            };
        }
    };

    let mut file = None;
    loop {
        let field = match multipart.next_field().await {
            Ok(Some(field)) => field,
            Ok(None) => break,
            Err(error) => return multipart_problem(error, fallback_request_id),
        };
        if field.name() != Some("file") || file.is_some() {
            return problem(
                StatusCode::BAD_REQUEST,
                "invalid_request",
                "Expected exactly one file field",
                fallback_request_id,
            );
        }
        let content_type = field.content_type().map(ToString::to_string);
        let bytes = match field.bytes().await {
            Ok(bytes) => bytes,
            Err(error) => return multipart_problem(error, fallback_request_id),
        };
        file = Some((content_type, bytes.to_vec()));
    }

    let Some((content_type, body)) = file else {
        return problem(
            StatusCode::BAD_REQUEST,
            "invalid_request",
            "Expected exactly one file field",
            fallback_request_id,
        );
    };

    let accepted = match state
        .lifecycle
        .admit(IncomingRequest::new(content_type, body))
    {
        Ok(accepted) => accepted,
        Err(failure) => return lifecycle_problem(failure),
    };
    let result = match tokio::task::spawn_blocking(move || accepted.wait()).await {
        Ok(result) => result,
        Err(_) => {
            return problem(
                StatusCode::INTERNAL_SERVER_ERROR,
                "internal",
                "Internal processing failure",
                fallback_request_id,
            );
        }
    };
    match result {
        Ok(response) => {
            let length = response.body.len();
            let mut http_response = Response::new(Body::from(response.body));
            *http_response.status_mut() = StatusCode::OK;
            let headers = http_response.headers_mut();
            headers.insert(header::CONTENT_TYPE, HeaderValue::from_static("image/jpeg"));
            headers.insert(
                header::CONTENT_LENGTH,
                HeaderValue::from_str(&length.to_string()).expect("decimal content length"),
            );
            headers.insert(
                "x-request-id",
                HeaderValue::from_str(response.request_id.as_str())
                    .expect("lifecycle request IDs are header-safe"),
            );
            http_response
        }
        Err(failure) => lifecycle_problem(failure),
    }
}

fn parse_api_keys(value: &str) -> Result<Vec<String>, ConfigError> {
    if value.is_empty() {
        return Err(ConfigError::InvalidApiKeys);
    }
    let keys: Vec<String> = value
        .split(',')
        .map(|entry| entry.trim().to_owned())
        .collect();
    if keys.is_empty() || keys.iter().any(|key| key.is_empty()) {
        return Err(ConfigError::InvalidApiKeys);
    }
    Ok(keys)
}

fn authorized(headers: &HeaderMap, configured_keys: &[String]) -> bool {
    let Some(value) = headers
        .get("x-api-key")
        .and_then(|value| value.to_str().ok())
    else {
        return false;
    };
    configured_keys.iter().fold(false, |matched, configured| {
        matched | constant_time_equal(value.as_bytes(), configured.as_bytes())
    })
}

fn constant_time_equal(left: &[u8], right: &[u8]) -> bool {
    let mut difference = left.len() ^ right.len();
    let longest = left.len().max(right.len());
    for index in 0..longest {
        let left_byte = left.get(index).copied().unwrap_or(0);
        let right_byte = right.get(index).copied().unwrap_or(0);
        difference |= usize::from(left_byte ^ right_byte);
    }
    difference == 0
}

fn lifecycle_problem(failure: ApplicationFailure) -> Response {
    let status = StatusCode::from_u16(failure.error.transport_status().as_u16())
        .expect("lifecycle transport statuses are valid HTTP statuses");
    let (code, title) = match failure.error {
        ApplicationError::InvalidRequest => ("invalid_request", "Invalid request"),
        ApplicationError::UnsupportedMediaType => {
            ("unsupported_media_type", "Unsupported media type")
        }
        ApplicationError::PayloadTooLarge | ApplicationError::ResourceLimit => {
            ("resource_limit", "Resource limit exceeded")
        }
        ApplicationError::InvalidImage => ("invalid_image", "Invalid image"),
        ApplicationError::Overloaded => ("overloaded", "Service overloaded"),
        ApplicationError::Unavailable => ("unavailable", "Service unavailable"),
        ApplicationError::DeadlineExceeded => ("deadline_exceeded", "Processing deadline exceeded"),
        ApplicationError::Internal => ("internal", "Internal processing failure"),
    };
    problem(status, code, title, failure.request_id.as_str().to_owned())
}

/// Keeps Axum's bounded-body classification at the HTTP boundary. In
/// particular, a `DefaultBodyLimit` error is delivered while reading a
/// multipart field rather than when constructing the extractor.
fn multipart_problem(error: MultipartError, request_id: String) -> Response {
    match error.status() {
        StatusCode::PAYLOAD_TOO_LARGE => problem(
            StatusCode::PAYLOAD_TOO_LARGE,
            "resource_limit",
            "Request exceeds the configured limit",
            request_id,
        ),
        StatusCode::BAD_REQUEST => problem(
            StatusCode::BAD_REQUEST,
            "invalid_request",
            "Invalid multipart request",
            request_id,
        ),
        _ => problem(
            StatusCode::INTERNAL_SERVER_ERROR,
            "internal",
            "Internal processing failure",
            request_id,
        ),
    }
}

fn problem(
    status: StatusCode,
    code: &'static str,
    title: &'static str,
    request_id: String,
) -> Response {
    let mut response = (
        status,
        Json(Problem {
            problem_type: format!("https://imgengine.example/problems/{code}"),
            title,
            status: status.as_u16(),
            code,
            request_id: request_id.clone(),
        }),
    )
        .into_response();
    response.headers_mut().insert(
        header::CONTENT_TYPE,
        HeaderValue::from_static("application/problem+json"),
    );
    response.headers_mut().insert(
        "x-request-id",
        HeaderValue::from_str(&request_id).expect("transport request ID is header-safe"),
    );
    response
}

#[derive(serde::Serialize)]
struct Problem {
    #[serde(rename = "type")]
    problem_type: String,
    title: &'static str,
    status: u16,
    code: &'static str,
    request_id: String,
}

fn next_transport_request_id() -> String {
    format!(
        "http-{:016x}",
        TRANSPORT_REQUEST_SEQUENCE.fetch_add(1, Ordering::Relaxed)
    )
}

#[cfg(test)]
mod tests {
    use super::{router, ApiConfig, ApiState, ConfigError};
    use axum::body::Body;
    use axum::http::{header, Request, StatusCode};
    use http_body_util::BodyExt;
    use std::fs;
    use std::path::PathBuf;
    use std::sync::atomic::{AtomicU64, Ordering};
    use std::sync::Mutex;
    use std::time::Duration;
    use tower::ServiceExt;

    static ENGINE_LOCK: Mutex<()> = Mutex::new(());
    static TEST_SEQUENCE: AtomicU64 = AtomicU64::new(1);
    const REAL_JPEG: &[u8] = include_bytes!("../../../tests/fixtures/cc0_camera_landscape.jpg");
    const REAL_PNG: &[u8] = &[
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44,
        0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f,
        0x15, 0xc4, 0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0xf8,
        0xcf, 0xc0, 0xf0, 0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99, 0x3d, 0x1d, 0x00, 0x00,
        0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
    ];

    fn root(label: &str) -> PathBuf {
        std::env::temp_dir().join(format!(
            "imgengine-api-{label}-{}-{}",
            std::process::id(),
            TEST_SEQUENCE.fetch_add(1, Ordering::Relaxed)
        ))
    }

    fn app(label: &str, keys: &str) -> (axum::Router, PathBuf) {
        let root = root(label);
        let config = ApiConfig::new(keys, root.clone()).expect("valid test configuration");
        (
            router(ApiState::start(config).expect("start API state")),
            root,
        )
    }

    fn request(parts: &[(&str, &str, &[u8])], key: Option<&str>) -> Request<Body> {
        let boundary = "imgengine-test-boundary";
        let mut body = Vec::new();
        for (name, content_type, bytes) in parts {
            body.extend_from_slice(format!("--{boundary}\r\n").as_bytes());
            body.extend_from_slice(
                format!(
                    "Content-Disposition: form-data; name=\"{name}\"; filename=\"ignored\"\r\n"
                )
                .as_bytes(),
            );
            body.extend_from_slice(format!("Content-Type: {content_type}\r\n\r\n").as_bytes());
            body.extend_from_slice(bytes);
            body.extend_from_slice(b"\r\n");
        }
        body.extend_from_slice(format!("--{boundary}--\r\n").as_bytes());
        let mut builder = Request::builder()
            .method("POST")
            .uri("/api/v1/render")
            .header(
                header::CONTENT_TYPE,
                format!("multipart/form-data; boundary={boundary}"),
            );
        if let Some(key) = key {
            builder = builder.header("x-api-key", key);
        }
        builder.body(Body::from(body)).expect("build request")
    }

    async fn response_bytes(response: axum::response::Response) -> Vec<u8> {
        response
            .into_body()
            .collect()
            .await
            .expect("collect response")
            .to_bytes()
            .to_vec()
    }

    #[test]
    fn configuration_fails_closed_without_valid_keys() {
        assert_eq!(
            ApiConfig::new("", root("empty")).unwrap_err(),
            ConfigError::InvalidApiKeys
        );
        assert_eq!(
            ApiConfig::new("one,,two", root("empty-entry")).unwrap_err(),
            ConfigError::InvalidApiKeys
        );
        assert_eq!(
            super::parse_api_keys(" ").unwrap_err(),
            ConfigError::InvalidApiKeys
        );
    }

    #[tokio::test]
    async fn rejects_missing_and_invalid_keys_before_processing() {
        let _engine = ENGINE_LOCK
            .lock()
            .unwrap_or_else(|poison| poison.into_inner());
        let (app, root) = app("auth", "first-key,second-key");
        let missing = app
            .clone()
            .oneshot(request(&[("file", "image/jpeg", REAL_JPEG)], None))
            .await
            .expect("response");
        assert_eq!(missing.status(), StatusCode::UNAUTHORIZED);
        let missing_body = String::from_utf8(response_bytes(missing).await).expect("problem JSON");
        assert!(missing_body.contains("authentication_failed"));
        assert!(!missing_body.contains("first-key"));
        let invalid = app
            .oneshot(request(
                &[("file", "image/jpeg", REAL_JPEG)],
                Some("wrong-key"),
            ))
            .await
            .expect("response");
        assert_eq!(invalid.status(), StatusCode::UNAUTHORIZED);
        fs::remove_dir_all(root).expect("remove root");
    }

    #[tokio::test]
    async fn real_jpeg_and_png_traverse_http_to_native_and_cleanup() {
        let _engine = ENGINE_LOCK
            .lock()
            .unwrap_or_else(|poison| poison.into_inner());
        let (app, root) = app("images", "test-key");
        for (content_type, input) in [("image/jpeg", REAL_JPEG), ("image/png", REAL_PNG)] {
            let response = app
                .clone()
                .oneshot(request(&[("file", content_type, input)], Some("test-key")))
                .await
                .expect("response");
            assert_eq!(response.status(), StatusCode::OK);
            assert_eq!(response.headers()[header::CONTENT_TYPE], "image/jpeg");
            assert!(response.headers().contains_key("x-request-id"));
            let output = response_bytes(response).await;
            assert!(output.len() > 4);
            assert_eq!(&output[..2], &[0xff, 0xd8]);
            assert_eq!(&output[output.len() - 2..], &[0xff, 0xd9]);
            let mut decoder = jpeg_decoder::Decoder::new(output.as_slice());
            decoder
                .decode()
                .expect("independently decode returned JPEG");
        }
        drop(app);
        assert!(fs::read_dir(&root)
            .expect("read workspace root")
            .next()
            .is_none());
        fs::remove_dir_all(root).expect("remove root");
    }

    #[tokio::test]
    async fn rejects_missing_duplicate_and_extra_fields() {
        let _engine = ENGINE_LOCK
            .lock()
            .unwrap_or_else(|poison| poison.into_inner());
        let (app, root) = app("multipart", "test-key");
        for parts in [
            vec![],
            vec![
                ("file", "image/png", REAL_PNG),
                ("file", "image/png", REAL_PNG),
            ],
            vec![("other", "text/plain", b"unexpected" as &[u8])],
        ] {
            let response = app
                .clone()
                .oneshot(request(&parts, Some("test-key")))
                .await
                .expect("response");
            assert_eq!(response.status(), StatusCode::BAD_REQUEST);
        }
        drop(app);
        fs::remove_dir_all(root).expect("remove root");
    }

    #[tokio::test]
    async fn rejects_malformed_multipart_without_native_processing() {
        let _engine = ENGINE_LOCK
            .lock()
            .unwrap_or_else(|poison| poison.into_inner());
        let (app, root) = app("malformed-multipart", "test-key");
        let response = app
            .clone()
            .oneshot(
                Request::builder()
                    .method("POST")
                    .uri("/api/v1/render")
                    .header(
                        header::CONTENT_TYPE,
                        "multipart/form-data; boundary=missing",
                    )
                    .header("x-api-key", "test-key")
                    .body(Body::from("not a multipart body"))
                    .expect("build malformed request"),
            )
            .await
            .expect("response");
        assert_eq!(response.status(), StatusCode::BAD_REQUEST);
        drop(app);
        fs::remove_dir_all(root).expect("remove root");
    }

    #[tokio::test]
    async fn maps_unsupported_malformed_and_limited_input_safely() {
        let _engine = ENGINE_LOCK
            .lock()
            .unwrap_or_else(|poison| poison.into_inner());
        let root = root("failures");
        let config = ApiConfig::new("test-key", root.clone())
            .expect("config")
            .with_limits(1_024, 1, Some(Duration::from_secs(1)))
            .expect("limits");
        let app = router(ApiState::start(config).expect("start"));
        let unsupported = app
            .clone()
            .oneshot(request(
                &[("file", "text/plain", b"text")],
                Some("test-key"),
            ))
            .await
            .expect("response");
        assert_eq!(unsupported.status(), StatusCode::UNSUPPORTED_MEDIA_TYPE);
        let malformed = app
            .clone()
            .oneshot(request(
                &[("file", "image/png", b"not a PNG")],
                Some("test-key"),
            ))
            .await
            .expect("response");
        assert_eq!(malformed.status(), StatusCode::UNPROCESSABLE_ENTITY);
        let oversized = app
            .oneshot(request(
                &[("file", "image/png", &[0; 2_048])],
                Some("test-key"),
            ))
            .await
            .expect("response");
        assert_eq!(oversized.status(), StatusCode::PAYLOAD_TOO_LARGE);
        fs::remove_dir_all(root).expect("remove root");
    }

    #[tokio::test]
    async fn deadline_and_overload_are_honestly_mapped() {
        let _engine = ENGINE_LOCK
            .lock()
            .unwrap_or_else(|poison| poison.into_inner());
        let root = root("deadline");
        let deadline_config = ApiConfig::new("test-key", root.clone())
            .expect("config")
            .with_limits(DEFAULT_LIMIT, 1, Some(Duration::from_nanos(1)))
            .expect("limits");
        let deadline_app = router(ApiState::start(deadline_config).expect("start"));
        let deadline = deadline_app
            .oneshot(request(
                &[("file", "image/png", REAL_PNG)],
                Some("test-key"),
            ))
            .await
            .expect("response");
        assert_eq!(deadline.status(), StatusCode::GATEWAY_TIMEOUT);
        fs::remove_dir_all(&root).expect("remove deadline root");

        let (app, overload_root) = app("overload", "test-key");
        let first = app.clone().oneshot(request(
            &[("file", "image/jpeg", REAL_JPEG)],
            Some("test-key"),
        ));
        let second = app.clone().oneshot(request(
            &[("file", "image/jpeg", REAL_JPEG)],
            Some("test-key"),
        ));
        let third = app.clone().oneshot(request(
            &[("file", "image/jpeg", REAL_JPEG)],
            Some("test-key"),
        ));
        let (first, second, third) = tokio::join!(first, second, third);
        let statuses = [
            first.expect("first").status(),
            second.expect("second").status(),
            third.expect("third").status(),
        ];
        assert!(statuses.contains(&StatusCode::TOO_MANY_REQUESTS));
        drop(app);
        fs::remove_dir_all(overload_root).expect("remove overload root");
    }

    const DEFAULT_LIMIT: usize = 1_048_576;
}
