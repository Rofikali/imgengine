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
    AdmissionController, AdmissionExecutionObserver, AdmissionMetricsSnapshot, AdmissionOptions,
    AdmittedWaitOutcome, ApplicationError, ApplicationFailure, IncomingRequest,
    LifecycleMetricsSnapshot, RequestLifecycleService, RequestPolicy, ResponseAbandonmentObserver,
    ResponseAbandonmentSignal, SupervisorOptions,
};
use std::env;
use std::path::PathBuf;
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::Arc;
use std::sync::Mutex;
use std::time::Duration;
use tokio::net::TcpListener;
use tokio::sync::{watch, Mutex as AsyncMutex, Notify};

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
    runtime: Arc<RuntimeControl>,
    #[cfg(test)]
    response_wait_gate: Option<Arc<TestResponseWaitGate>>,
    #[cfg(test)]
    pre_admission_gate: Option<Arc<TestPreAdmissionGate>>,
    #[cfg(test)]
    admission_observer: Option<Arc<TestAdmissionObserver>>,
}

impl ApiState {
    /// Starts the one existing supervisor/admission path for this process.
    pub fn start(config: ApiConfig) -> Result<Self, ConfigError> {
        Self::start_inner(config, None, None)
    }

    fn start_inner(
        config: ApiConfig,
        response_abandonment_observer: Option<Arc<dyn ResponseAbandonmentObserver>>,
        execution_observer: Option<Arc<dyn AdmissionExecutionObserver>>,
    ) -> Result<Self, ConfigError> {
        let supervisor = SupervisorOptions::new(config.workspace_root);
        let mut admission =
            AdmissionOptions::new(supervisor, config.queue_capacity, config.max_request_bytes)
                .map_err(|_| ConfigError::InvalidLimits)?;
        if let Some(observer) = execution_observer {
            admission = admission.with_execution_observer(observer);
        }
        let admission = Arc::new(
            AdmissionController::start(admission).map_err(|_| ConfigError::LifecycleUnavailable)?,
        );
        let policy = RequestPolicy::new(config.max_request_bytes, config.execution_timeout)
            .map_err(|_| ConfigError::InvalidLimits)?;
        Ok(Self {
            lifecycle: Arc::new(
                RequestLifecycleService::new_with_response_abandonment_observer(
                    admission,
                    policy,
                    response_abandonment_observer,
                ),
            ),
            api_keys: Arc::new(config.api_keys),
            max_request_bytes: config.max_request_bytes,
            runtime: Arc::new(RuntimeControl::default()),
            #[cfg(test)]
            response_wait_gate: None,
            #[cfg(test)]
            pre_admission_gate: None,
            #[cfg(test)]
            admission_observer: None,
        })
    }

    /// Returns redacted lifecycle counters suitable for transport metrics.
    pub fn lifecycle_metrics(&self) -> LifecycleMetricsSnapshot {
        self.lifecycle.metrics()
    }

    /// Returns existing redacted bounded-admission counters.
    pub fn admission_metrics(&self) -> AdmissionMetricsSnapshot {
        self.lifecycle.admission_metrics()
    }

    fn is_accepting(&self) -> bool {
        self.runtime.status() == ApiRuntimeStatus::Running
    }

    fn admit_if_running(
        &self,
        request: IncomingRequest,
    ) -> Option<Result<imgengine_supervisor::AdmittedRequest, ApplicationFailure>> {
        self.runtime
            .admit_if_running(|| self.lifecycle.admit(request))
    }
}

/// Observable, process-scoped HTTP lifecycle state.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ApiRuntimeStatus {
    Running,
    Draining,
    Stopped,
}

#[derive(Default)]
struct RuntimeControl {
    status: Mutex<ApiRuntimeStatus>,
    changed: Notify,
}

impl Default for ApiRuntimeStatus {
    fn default() -> Self {
        Self::Running
    }
}

impl RuntimeControl {
    fn status(&self) -> ApiRuntimeStatus {
        *self.status.lock().expect("lock API runtime state")
    }

    fn admit_if_running<T>(&self, admit: impl FnOnce() -> T) -> Option<T> {
        let status = self.status.lock().expect("lock API runtime state");
        if *status != ApiRuntimeStatus::Running {
            return None;
        }
        // This lock spans the existing lifecycle admission call. The state
        // transition to Draining therefore linearizes before any later HTTP
        // admission, without introducing another queue or scheduler.
        Some(admit())
    }

    fn begin_drain(&self) -> bool {
        let mut status = self.status.lock().expect("lock API runtime state");
        if *status != ApiRuntimeStatus::Running {
            return false;
        }
        *status = ApiRuntimeStatus::Draining;
        self.changed.notify_waiters();
        true
    }

    fn mark_stopped(&self) {
        let mut status = self.status.lock().expect("lock API runtime state");
        *status = ApiRuntimeStatus::Stopped;
        self.changed.notify_waiters();
    }

    async fn wait_until_stopped(&self) {
        loop {
            let notified = self.changed.notified();
            if self.status() == ApiRuntimeStatus::Stopped {
                return;
            }
            notified.await;
        }
    }

    #[cfg(test)]
    async fn wait_until_draining(&self) {
        loop {
            let notified = self.changed.notified();
            if self.status() != ApiRuntimeStatus::Running {
                return;
            }
            notified.await;
        }
    }
}

/// Process-scoped owner for the narrow API state and its graceful HTTP server.
#[derive(Clone)]
pub struct ApiRuntime {
    state: ApiState,
}

impl ApiRuntime {
    pub fn start(config: ApiConfig) -> Result<Self, ConfigError> {
        Ok(Self {
            state: ApiState::start(config)?,
        })
    }

    pub fn router(&self) -> Router {
        router(self.state.clone())
    }

    pub fn lifecycle_metrics(&self) -> LifecycleMetricsSnapshot {
        self.state.lifecycle_metrics()
    }

    pub fn admission_metrics(&self) -> AdmissionMetricsSnapshot {
        self.state.admission_metrics()
    }

    pub fn status(&self) -> ApiRuntimeStatus {
        self.state.runtime.status()
    }

    /// Starts an Axum listener owned by this runtime. `ApiServer::shutdown`
    /// is the sole graceful-drain path for this listener.
    pub fn serve(self, listener: TcpListener) -> ApiServer {
        let (shutdown_sender, mut shutdown_receiver) = watch::channel(false);
        let app = self.router();
        let server = tokio::spawn(async move {
            axum::serve(listener, app)
                .with_graceful_shutdown(async move {
                    while !*shutdown_receiver.borrow() {
                        if shutdown_receiver.changed().await.is_err() {
                            return;
                        }
                    }
                })
                .await
        });
        ApiServer {
            runtime: self,
            shutdown_sender,
            server: AsyncMutex::new(Some(server)),
        }
    }
}

/// Error category for a graceful runtime shutdown; it never contains native or
/// client diagnostics.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ApiShutdownError {
    AdmissionUnavailable,
    ServerUnavailable,
}

/// Sole owner of one Axum listener and its graceful shutdown sequence.
pub struct ApiServer {
    runtime: ApiRuntime,
    shutdown_sender: watch::Sender<bool>,
    server: AsyncMutex<Option<tokio::task::JoinHandle<std::io::Result<()>>>>,
}

impl ApiServer {
    pub fn runtime(&self) -> &ApiRuntime {
        &self.runtime
    }

    /// Performs an idempotent graceful shutdown: close lifecycle admission,
    /// stop listener acceptance, drain accepted work, join the worker, then
    /// mark the runtime stopped. Process termination budgets remain external.
    pub async fn shutdown(&self) -> Result<(), ApiShutdownError> {
        if !self.runtime.state.runtime.begin_drain() {
            self.runtime.state.runtime.wait_until_stopped().await;
            return Ok(());
        }

        let _ = self.shutdown_sender.send(true);
        let lifecycle = Arc::clone(&self.runtime.state.lifecycle);
        let admission = tokio::task::spawn_blocking(move || lifecycle.shutdown()).await;
        let server = self.server.lock().await.take();
        let server_result = match server {
            Some(server) => server.await,
            None => Ok(Ok(())),
        };
        self.runtime.state.runtime.mark_stopped();

        if !matches!(admission, Ok(Ok(()))) {
            return Err(ApiShutdownError::AdmissionUnavailable);
        }
        match server_result {
            Ok(Ok(())) => Ok(()),
            _ => Err(ApiShutdownError::ServerUnavailable),
        }
    }

    #[cfg(test)]
    async fn wait_until_draining(&self) {
        self.runtime.state.runtime.wait_until_draining().await;
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
    if !state.is_accepting() {
        return unavailable_problem(fallback_request_id);
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

    #[cfg(test)]
    if let Some(gate) = &state.pre_admission_gate {
        gate.wait_until_released();
    }

    let accepted = match state.admit_if_running(IncomingRequest::new(content_type, body)) {
        Some(Ok(accepted)) => accepted,
        Some(Err(failure)) => return lifecycle_problem(failure),
        None => return unavailable_problem(fallback_request_id),
    };
    #[cfg(test)]
    if let Some(observer) = &state.admission_observer {
        observer.admitted();
    }
    let abandonment_signal = ResponseAbandonmentSignal::new();
    let mut abandonment_guard = TransportAbandonmentGuard::new(abandonment_signal.clone());
    #[cfg(test)]
    let response_wait_gate = state.response_wait_gate.clone();
    let result = match tokio::task::spawn_blocking(move || {
        #[cfg(test)]
        if let Some(gate) = response_wait_gate {
            gate.wait_until_released();
        }
        accepted.wait_or_abandon(&abandonment_signal)
    })
    .await
    {
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
        AdmittedWaitOutcome::Completed(Ok(response)) => {
            abandonment_guard.disarm();
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
        AdmittedWaitOutcome::Completed(Err(failure)) => {
            abandonment_guard.disarm();
            lifecycle_problem(failure)
        }
        // This path is reached only when a non-handler caller explicitly
        // signals abandonment before the blocking task completes. A dropped
        // handler never observes this outcome because its future is gone.
        AdmittedWaitOutcome::ResponseAbandoned(_) => problem(
            StatusCode::INTERNAL_SERVER_ERROR,
            "internal",
            "Internal processing failure",
            fallback_request_id,
        ),
    }
}

/// Signals only response-delivery abandonment when Axum drops an in-flight
/// handler future. It does not cancel admission or native execution.
struct TransportAbandonmentGuard {
    signal: ResponseAbandonmentSignal,
    armed: bool,
}

impl TransportAbandonmentGuard {
    fn new(signal: ResponseAbandonmentSignal) -> Self {
        Self {
            signal,
            armed: true,
        }
    }

    fn disarm(&mut self) {
        self.armed = false;
    }
}

impl Drop for TransportAbandonmentGuard {
    fn drop(&mut self) {
        if self.armed {
            self.signal.abandon();
        }
    }
}

#[cfg(test)]
#[derive(Default)]
struct TestResponseWaitGate {
    state: std::sync::Mutex<TestResponseWaitGateState>,
    changed: std::sync::Condvar,
}

#[cfg(test)]
#[derive(Default)]
struct TestResponseWaitGateState {
    entered: bool,
    released: bool,
}

#[cfg(test)]
impl TestResponseWaitGate {
    fn wait_until_released(&self) {
        let mut state = self.state.lock().expect("lock response wait gate");
        state.entered = true;
        self.changed.notify_all();
        while !state.released {
            state = self.changed.wait(state).expect("wait response wait gate");
        }
    }

    fn wait_until_entered(&self, timeout: Duration) -> bool {
        let state = self.state.lock().expect("lock response wait gate");
        let (state, _) = self
            .changed
            .wait_timeout_while(state, timeout, |state| !state.entered)
            .expect("wait response wait gate");
        state.entered
    }

    fn release(&self) {
        let mut state = self.state.lock().expect("lock response wait gate");
        state.released = true;
        self.changed.notify_all();
    }
}

/// Test-only synchronization at the HTTP-to-lifecycle boundary. It keeps a
/// real handler live while a drain closes admission; it is not compiled into
/// production builds.
#[cfg(test)]
struct TestPreAdmissionGate {
    state: std::sync::Mutex<TestPreAdmissionGateState>,
    changed: std::sync::Condvar,
}

#[cfg(test)]
struct TestPreAdmissionGateState {
    entries: u64,
    block_on_entry: u64,
    blocked: bool,
    released: bool,
}

#[cfg(test)]
impl TestPreAdmissionGate {
    fn blocking_entry(block_on_entry: u64) -> Self {
        Self {
            state: std::sync::Mutex::new(TestPreAdmissionGateState {
                entries: 0,
                block_on_entry,
                blocked: false,
                released: false,
            }),
            changed: std::sync::Condvar::new(),
        }
    }

    fn wait_until_released(&self) {
        let mut state = self.state.lock().expect("lock pre-admission gate");
        state.entries += 1;
        if state.entries != state.block_on_entry {
            return;
        }
        state.blocked = true;
        self.changed.notify_all();
        while !state.released {
            state = self.changed.wait(state).expect("wait pre-admission gate");
        }
    }

    fn wait_until_entered(&self, timeout: Duration) -> bool {
        let state = self.state.lock().expect("lock pre-admission gate");
        let (state, _) = self
            .changed
            .wait_timeout_while(state, timeout, |state| !state.blocked)
            .expect("wait pre-admission gate");
        state.blocked
    }

    fn release(&self) {
        let mut state = self.state.lock().expect("lock pre-admission gate");
        state.released = true;
        self.changed.notify_all();
    }
}

/// Test-only admission acknowledgement. It observes only a count, never
/// request data, and avoids timing-based queue assertions.
#[cfg(test)]
#[derive(Default)]
struct TestAdmissionObserver {
    state: std::sync::Mutex<u64>,
    changed: std::sync::Condvar,
}

#[cfg(test)]
impl TestAdmissionObserver {
    fn admitted(&self) {
        let mut count = self.state.lock().expect("lock admission observer");
        *count += 1;
        self.changed.notify_all();
    }

    fn wait_for_count(&self, expected: u64, timeout: Duration) -> bool {
        let count = self.state.lock().expect("lock admission observer");
        let (count, _) = self
            .changed
            .wait_timeout_while(count, timeout, |count| *count < expected)
            .expect("wait admission observer");
        *count >= expected
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

fn unavailable_problem(request_id: String) -> Response {
    problem(
        StatusCode::SERVICE_UNAVAILABLE,
        "unavailable",
        "Service unavailable",
        request_id,
    )
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
    use super::{
        router, ApiConfig, ApiRuntime, ApiRuntimeStatus, ApiServer, ApiState, ConfigError,
        TestAdmissionObserver, TestPreAdmissionGate, TestResponseWaitGate,
    };
    use axum::body::Body;
    use axum::http::{header, Request, StatusCode};
    use http_body_util::BodyExt;
    use imgengine_supervisor::{
        AdmissionExecutionObserver, RequestEvent, RequestState, ResponseAbandonmentObserver,
    };
    use std::fs;
    use std::net::SocketAddr;
    use std::path::PathBuf;
    use std::sync::atomic::{AtomicU64, Ordering};
    use std::sync::{mpsc, Arc, Condvar, Mutex};
    use std::time::Duration;
    use tokio::io::{AsyncReadExt, AsyncWriteExt};
    use tokio::net::{TcpListener, TcpStream};
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
        let (content_type, body) = multipart_body(parts);
        let mut builder = Request::builder()
            .method("POST")
            .uri("/api/v1/render")
            .header(header::CONTENT_TYPE, content_type);
        if let Some(key) = key {
            builder = builder.header("x-api-key", key);
        }
        builder.body(Body::from(body)).expect("build request")
    }

    fn multipart_body(parts: &[(&str, &str, &[u8])]) -> (String, Vec<u8>) {
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
        (format!("multipart/form-data; boundary={boundary}"), body)
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

    #[derive(Default)]
    struct TestExecutionGate {
        state: Mutex<TestExecutionGateState>,
        changed: Condvar,
    }

    #[derive(Default)]
    struct TestExecutionGateState {
        entries: u64,
        released: bool,
    }

    impl TestExecutionGate {
        fn wait_until_entered(&self, timeout: Duration) -> bool {
            self.wait_for_entries(1, timeout)
        }

        fn wait_for_entries(&self, expected: u64, timeout: Duration) -> bool {
            let state = self.state.lock().expect("lock execution gate");
            let (state, _) = self
                .changed
                .wait_timeout_while(state, timeout, |state| state.entries < expected)
                .expect("wait execution gate");
            state.entries >= expected
        }

        fn release(&self) {
            let mut state = self.state.lock().expect("lock execution gate");
            state.released = true;
            self.changed.notify_all();
        }
    }

    impl AdmissionExecutionObserver for TestExecutionGate {
        fn before_native_processing(&self) {
            let mut state = self.state.lock().expect("lock execution gate");
            state.entries += 1;
            self.changed.notify_all();
            while state.entries == 1 && !state.released {
                state = self.changed.wait(state).expect("wait execution gate");
            }
        }
    }

    struct RecordingAbandonmentObserver {
        sender: mpsc::Sender<RequestEvent>,
    }

    impl ResponseAbandonmentObserver for RecordingAbandonmentObserver {
        fn response_abandoned(&self, event: &RequestEvent) {
            self.sender
                .send(event.clone())
                .expect("test abandonment receiver remains available");
        }
    }

    async fn start_loopback(app: axum::Router) -> (SocketAddr, tokio::task::JoinHandle<()>) {
        let listener = TcpListener::bind(("127.0.0.1", 0))
            .await
            .expect("bind loopback listener");
        let address = listener.local_addr().expect("read loopback address");
        let server = tokio::spawn(async move {
            axum::serve(listener, app)
                .await
                .expect("serve loopback application");
        });
        (address, server)
    }

    async fn start_runtime_server(runtime: ApiRuntime) -> (SocketAddr, Arc<ApiServer>) {
        let listener = TcpListener::bind(("127.0.0.1", 0))
            .await
            .expect("bind loopback listener");
        let address = listener.local_addr().expect("read loopback address");
        (address, Arc::new(runtime.serve(listener)))
    }

    async fn write_http_request(
        stream: &mut TcpStream,
        content_type: &str,
        content_length: usize,
        body: &[u8],
    ) {
        let headers = format!(
            "POST /api/v1/render HTTP/1.1\r\nHost: loopback\r\nX-API-Key: test-key\r\nContent-Type: {content_type}\r\nContent-Length: {content_length}\r\nConnection: close\r\n\r\n"
        );
        stream
            .write_all(headers.as_bytes())
            .await
            .expect("write headers");
        stream.write_all(body).await.expect("write body");
        stream.flush().await.expect("flush request");
    }

    async fn successful_loopback_request(address: SocketAddr) -> Vec<u8> {
        let (content_type, body) = multipart_body(&[("file", "image/png", REAL_PNG)]);
        let mut stream = TcpStream::connect(address).await.expect("connect loopback");
        write_http_request(&mut stream, &content_type, body.len(), &body).await;
        let mut response = Vec::new();
        stream
            .read_to_end(&mut response)
            .await
            .expect("read loopback response");
        response
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

    #[tokio::test(flavor = "multi_thread", worker_threads = 2)]
    async fn tcp_disconnect_after_admission_abandons_only_response_delivery() {
        let _engine = ENGINE_LOCK
            .lock()
            .unwrap_or_else(|poison| poison.into_inner());
        let root = root("tcp-disconnect-after-admission");
        let execution_gate = Arc::new(TestExecutionGate::default());
        let response_wait_gate = Arc::new(TestResponseWaitGate::default());
        let (event_sender, event_receiver) = mpsc::channel();
        let observer = Arc::new(RecordingAbandonmentObserver {
            sender: event_sender,
        });
        let config = ApiConfig::new("test-key", root.clone()).expect("configuration");
        let mut state = ApiState::start_inner(config, Some(observer), Some(execution_gate.clone()))
            .expect("start API state");
        state.response_wait_gate = Some(response_wait_gate.clone());
        let runtime = ApiRuntime { state };
        let (address, server) = start_loopback(runtime.router()).await;

        let (content_type, body) = multipart_body(&[("file", "image/png", REAL_PNG)]);
        let mut client = TcpStream::connect(address).await.expect("connect loopback");
        write_http_request(&mut client, &content_type, body.len(), &body).await;
        assert!(
            response_wait_gate.wait_until_entered(Duration::from_secs(2)),
            "accepted request must enter lifecycle-owned response wait"
        );
        assert!(
            execution_gate.wait_until_entered(Duration::from_secs(2)),
            "accepted request must reach the native-processing boundary"
        );
        assert_eq!(runtime.admission_metrics().accepted, 1);
        assert_eq!(runtime.admission_metrics().completed, 0);

        client.shutdown().await.expect("close client write side");
        drop(client);
        response_wait_gate.release();
        let abandoned = tokio::task::spawn_blocking(move || {
            event_receiver
                .recv_timeout(Duration::from_secs(2))
                .expect("real TCP close must produce abandonment event")
        })
        .await
        .expect("join abandonment receiver");
        assert_eq!(abandoned.state, RequestState::ResponseAbandoned);
        assert!(abandoned.result.is_none());
        assert_eq!(runtime.lifecycle_metrics().response_abandoned, 1);

        execution_gate.release();
        let response = successful_loopback_request(address).await;
        assert!(response.starts_with(b"HTTP/1.1 200"));
        assert!(response.ends_with(&[0xff, 0xd9]));
        let admission = runtime.admission_metrics();
        assert_eq!(admission.accepted, 2);
        assert_eq!(admission.completed, 2);
        assert_eq!(runtime.lifecycle_metrics().response_abandoned, 1);

        server.abort();
        let _ = server.await;
        drop(runtime);
        assert!(fs::read_dir(&root)
            .expect("read workspace root")
            .next()
            .is_none());
        fs::remove_dir_all(root).expect("remove root");
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 2)]
    async fn tcp_disconnect_during_upload_never_enters_lifecycle() {
        let _engine = ENGINE_LOCK
            .lock()
            .unwrap_or_else(|poison| poison.into_inner());
        let root = root("tcp-disconnect-during-upload");
        let (event_sender, event_receiver) = mpsc::channel();
        let observer = Arc::new(RecordingAbandonmentObserver {
            sender: event_sender,
        });
        let config = ApiConfig::new("test-key", root.clone()).expect("configuration");
        let state = ApiState::start_inner(config, Some(observer), None).expect("start API state");
        let runtime = ApiRuntime { state };
        let (address, server) = start_loopback(runtime.router()).await;

        let (content_type, body) = multipart_body(&[("file", "image/png", REAL_PNG)]);
        let mut client = TcpStream::connect(address).await.expect("connect loopback");
        write_http_request(
            &mut client,
            &content_type,
            body.len(),
            &body[..body.len() / 2],
        )
        .await;
        client.shutdown().await.expect("close incomplete upload");
        drop(client);

        let response = successful_loopback_request(address).await;
        assert!(response.starts_with(b"HTTP/1.1 200"));
        let admission = runtime.admission_metrics();
        assert_eq!(admission.accepted, 1);
        assert_eq!(admission.completed, 1);
        assert_eq!(runtime.lifecycle_metrics().response_abandoned, 0);
        assert!(event_receiver.try_recv().is_err());

        server.abort();
        let _ = server.await;
        drop(runtime);
        assert!(fs::read_dir(&root)
            .expect("read workspace root")
            .next()
            .is_none());
        fs::remove_dir_all(root).expect("remove root");
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 2)]
    async fn graceful_shutdown_closes_admission_drains_work_and_stops_listener() {
        let _engine = ENGINE_LOCK
            .lock()
            .unwrap_or_else(|poison| poison.into_inner());
        let root = root("graceful-shutdown");
        let execution_gate = Arc::new(TestExecutionGate::default());
        let pre_admission_gate = Arc::new(TestPreAdmissionGate::blocking_entry(3));
        let admission_observer = Arc::new(TestAdmissionObserver::default());
        let config = ApiConfig::new("test-key", root.clone()).expect("configuration");
        let mut state = ApiState::start_inner(config, None, Some(execution_gate.clone()))
            .expect("start API state");
        state.pre_admission_gate = Some(pre_admission_gate.clone());
        state.admission_observer = Some(admission_observer.clone());
        let (address, server) = start_runtime_server(ApiRuntime { state }).await;

        // The first request is active at the native-processing boundary; the
        // second is accepted into the existing bounded queue.
        let first = tokio::spawn(successful_loopback_request(address));
        assert!(
            execution_gate.wait_until_entered(Duration::from_secs(2)),
            "first request must become active"
        );
        let second = tokio::spawn(successful_loopback_request(address));
        assert!(
            admission_observer.wait_for_count(2, Duration::from_secs(2)),
            "second request must be accepted into the existing queue"
        );
        assert_eq!(server.runtime().admission_metrics().accepted, 2);
        assert_eq!(server.runtime().admission_metrics().completed, 0);

        // Keep a real handler at the HTTP-to-lifecycle boundary. It starts
        // while the listener is live, but must receive 503 after drain starts.
        let rejected = tokio::spawn(successful_loopback_request(address));
        assert!(
            pre_admission_gate.wait_until_entered(Duration::from_secs(2)),
            "third request must reach the live handler before admission"
        );
        let shutdown_server = Arc::clone(&server);
        let shutdown = tokio::spawn(async move { shutdown_server.shutdown().await });
        server.wait_until_draining().await;
        assert_eq!(server.runtime().status(), ApiRuntimeStatus::Draining);
        let repeated_shutdown_server = Arc::clone(&server);
        let repeated_shutdown =
            tokio::spawn(async move { repeated_shutdown_server.shutdown().await });
        pre_admission_gate.release();
        let rejected = tokio::time::timeout(Duration::from_secs(2), rejected)
            .await
            .expect("draining handler must return")
            .expect("join rejected handler");
        assert!(rejected.starts_with(b"HTTP/1.1 503"));
        assert_eq!(server.runtime().admission_metrics().accepted, 2);

        // Releasing the active request lets the original FIFO worker finish it
        // before reaching the already accepted queued request.
        execution_gate.release();
        assert!(
            execution_gate.wait_for_entries(2, Duration::from_secs(2)),
            "queued work must begin only after active work completes"
        );
        for response in [first, second] {
            let response = tokio::time::timeout(Duration::from_secs(2), response)
                .await
                .expect("accepted request must drain")
                .expect("join accepted request");
            assert!(response.starts_with(b"HTTP/1.1 200"));
            assert!(response.ends_with(&[0xff, 0xd9]));
        }

        tokio::time::timeout(Duration::from_secs(2), shutdown)
            .await
            .expect("shutdown must finish after accepted work drains")
            .expect("join shutdown")
            .expect("graceful shutdown");
        tokio::time::timeout(Duration::from_secs(2), repeated_shutdown)
            .await
            .expect("repeated shutdown must wait safely")
            .expect("join repeated shutdown")
            .expect("idempotent shutdown");
        assert_eq!(server.runtime().status(), ApiRuntimeStatus::Stopped);
        let admission = server.runtime().admission_metrics();
        assert_eq!(admission.accepted, 2);
        assert_eq!(admission.completed, 2);

        // Listener closure is a connection-level result; it is deliberately
        // distinct from the live-handler 503 asserted above.
        assert!(TcpStream::connect(address).await.is_err());
        drop(server);
        assert!(fs::read_dir(&root)
            .expect("read workspace root")
            .next()
            .is_none());
        fs::remove_dir_all(root).expect("remove root");
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 2)]
    async fn graceful_shutdown_drains_an_accepted_deadline_result_without_cancellation() {
        let _engine = ENGINE_LOCK
            .lock()
            .unwrap_or_else(|poison| poison.into_inner());
        let root = root("graceful-shutdown-deadline");
        let execution_gate = Arc::new(TestExecutionGate::default());
        let config = ApiConfig::new("test-key", root.clone())
            .expect("configuration")
            .with_limits(DEFAULT_LIMIT, 1, Some(Duration::from_nanos(1)))
            .expect("deadline configuration");
        let state = ApiState::start_inner(config, None, Some(execution_gate.clone()))
            .expect("start API state");
        let (address, server) = start_runtime_server(ApiRuntime { state }).await;

        // The request is admitted and held immediately before Supervisor::process.
        // Starting drain here proves shutdown waits for its lifecycle result rather
        // than cancelling the already accepted native operation.
        let request = tokio::spawn(successful_loopback_request(address));
        assert!(
            execution_gate.wait_until_entered(Duration::from_secs(2)),
            "request must reach the execution boundary"
        );
        let shutdown_server = Arc::clone(&server);
        let shutdown = tokio::spawn(async move { shutdown_server.shutdown().await });
        server.wait_until_draining().await;
        execution_gate.release();

        let response = tokio::time::timeout(Duration::from_secs(2), request)
            .await
            .expect("accepted request must drain")
            .expect("join request");
        assert!(response.starts_with(b"HTTP/1.1 504"));
        tokio::time::timeout(Duration::from_secs(2), shutdown)
            .await
            .expect("shutdown must wait for deadline cleanup")
            .expect("join shutdown")
            .expect("graceful shutdown");

        assert_eq!(server.runtime().status(), ApiRuntimeStatus::Stopped);
        let admission = server.runtime().admission_metrics();
        assert_eq!(admission.accepted, 1);
        assert_eq!(admission.completed, 1);
        assert!(fs::read_dir(&root)
            .expect("read workspace root")
            .next()
            .is_none());
        drop(server);
        fs::remove_dir_all(root).expect("remove root");
    }

    const DEFAULT_LIMIT: usize = 1_048_576;
}
