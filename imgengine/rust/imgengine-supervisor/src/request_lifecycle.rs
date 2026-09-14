use crate::{
    size_bucket, AdmissionController, AdmissionError, AdmissionRequest, ProcessReport, ResultClass,
};
use std::fmt;
use std::fs;
use std::io::{self, Read};
use std::sync::Arc;
use std::time::Duration;

const REQUEST_ID_BYTES: usize = 16;

/// Transport-neutral accepted image media types.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ContentType {
    Jpeg,
    Png,
}

impl ContentType {
    /// Parses the media type portion of a Content-Type value.
    pub fn parse(value: &str) -> Option<Self> {
        match value
            .split(';')
            .next()?
            .trim()
            .to_ascii_lowercase()
            .as_str()
        {
            "image/jpeg" => Some(Self::Jpeg),
            "image/png" => Some(Self::Png),
            _ => None,
        }
    }

    /// Returns the canonical media type for logs and response metadata.
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Jpeg => "image/jpeg",
            Self::Png => "image/png",
        }
    }
}

/// Server-generated opaque request correlation identifier.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct RequestId(String);

impl RequestId {
    fn generate() -> io::Result<Self> {
        let mut random = [0u8; REQUEST_ID_BYTES];
        fs::File::open("/dev/urandom")?.read_exact(&mut random)?;
        let mut value = String::with_capacity(REQUEST_ID_BYTES * 2);
        for byte in random {
            use std::fmt::Write;
            write!(&mut value, "{byte:02x}").expect("writing to String cannot fail");
        }
        Ok(Self(value))
    }

    /// Returns the correlation-safe identifier intended for clients and logs.
    pub fn as_str(&self) -> &str {
        &self.0
    }
}

/// Owned body and metadata from a future HTTP adapter.
pub struct IncomingRequest {
    content_type: Option<String>,
    body: Vec<u8>,
}

impl IncomingRequest {
    /// Creates an owned request. The caller must not use a client filename.
    pub fn new(content_type: Option<String>, body: Vec<u8>) -> Self {
        Self { content_type, body }
    }
}

/// Immutable request-boundary limits; native decode limits remain enforced in C.
#[derive(Clone, Debug)]
pub struct RequestPolicy {
    max_input_bytes: usize,
    execution_timeout: Option<Duration>,
}

impl RequestPolicy {
    /// Creates policy with an optional post-operation native execution deadline.
    pub fn new(
        max_input_bytes: usize,
        execution_timeout: Option<Duration>,
    ) -> Result<Self, ApplicationError> {
        if max_input_bytes == 0 || matches!(execution_timeout, Some(limit) if limit.is_zero()) {
            return Err(ApplicationError::InvalidRequest);
        }
        Ok(Self {
            max_input_bytes,
            execution_timeout,
        })
    }
}

/// Stable, transport-safe failures for the application boundary.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ApplicationError {
    InvalidRequest,
    UnsupportedMediaType,
    PayloadTooLarge,
    InvalidImage,
    ResourceLimit,
    Overloaded,
    Unavailable,
    DeadlineExceeded,
    Internal,
}

impl ApplicationError {
    /// Returns the existing redacted result class for logs and metrics.
    pub fn class(self) -> ResultClass {
        match self {
            Self::InvalidRequest => ResultClass::InvalidArgument,
            Self::UnsupportedMediaType | Self::InvalidImage => ResultClass::InvalidImage,
            Self::PayloadTooLarge | Self::ResourceLimit => ResultClass::ResourceLimit,
            Self::Overloaded => ResultClass::Overloaded,
            Self::Unavailable => ResultClass::Unavailable,
            Self::DeadlineExceeded => ResultClass::DeadlineExceeded,
            Self::Internal => ResultClass::Internal,
        }
    }

    /// Returns the future-HTTP status without coupling this crate to an HTTP framework.
    pub fn transport_status(self) -> TransportStatus {
        match self {
            Self::InvalidRequest => TransportStatus::BadRequest,
            Self::UnsupportedMediaType => TransportStatus::UnsupportedMediaType,
            Self::PayloadTooLarge | Self::ResourceLimit => TransportStatus::PayloadTooLarge,
            Self::InvalidImage => TransportStatus::UnprocessableContent,
            Self::Overloaded => TransportStatus::TooManyRequests,
            Self::Unavailable => TransportStatus::ServiceUnavailable,
            Self::DeadlineExceeded => TransportStatus::GatewayTimeout,
            Self::Internal => TransportStatus::InternalServerError,
        }
    }

    fn from_admission(error: AdmissionError) -> Self {
        match error.class() {
            ResultClass::InvalidArgument => Self::InvalidRequest,
            ResultClass::InvalidImage => Self::InvalidImage,
            ResultClass::ResourceLimit => Self::ResourceLimit,
            ResultClass::Overloaded => Self::Overloaded,
            ResultClass::Unavailable => Self::Unavailable,
            ResultClass::DeadlineExceeded => Self::DeadlineExceeded,
            ResultClass::WorkspaceFailure | ResultClass::Internal | ResultClass::Success => {
                Self::Internal
            }
        }
    }
}

impl fmt::Display for ApplicationError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter.write_str(self.class().as_str())
    }
}

impl std::error::Error for ApplicationError {}

/// Minimal HTTP-independent status vocabulary for a later adapter.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum TransportStatus {
    BadRequest,
    UnsupportedMediaType,
    UnprocessableContent,
    PayloadTooLarge,
    TooManyRequests,
    ServiceUnavailable,
    GatewayTimeout,
    InternalServerError,
}

impl TransportStatus {
    /// Returns the corresponding HTTP status code.
    pub fn as_u16(self) -> u16 {
        match self {
            Self::BadRequest => 400,
            Self::UnsupportedMediaType => 415,
            Self::UnprocessableContent => 422,
            Self::PayloadTooLarge => 413,
            Self::TooManyRequests => 429,
            Self::ServiceUnavailable => 503,
            Self::GatewayTimeout => 504,
            Self::InternalServerError => 500,
        }
    }
}

/// Request lifecycle states visible to the application boundary.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum RequestState {
    Received,
    Rejected,
    Accepted,
    Completed,
    Failed,
    ResponseAbandoned,
}

/// Redacted state transition suitable for structured logging and metrics.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct RequestEvent {
    pub request_id: RequestId,
    pub state: RequestState,
    pub result: Option<ResultClass>,
    pub input_content_type: Option<ContentType>,
    pub input_size_bucket: Option<&'static str>,
    pub output_size_bucket: Option<&'static str>,
}

/// A rejected application request correlated without exposing body data.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct ApplicationFailure {
    pub request_id: RequestId,
    pub error: ApplicationError,
    pub event: RequestEvent,
}

/// A completed application response with Rust-owned JPEG bytes.
pub struct ApplicationResponse {
    pub request_id: RequestId,
    pub content_type: &'static str,
    pub body: Vec<u8>,
    pub event: RequestEvent,
}

/// Transport-independent application boundary over bounded Rust admission.
pub struct RequestLifecycleService {
    admission: Arc<AdmissionController>,
    policy: RequestPolicy,
    effective_max_input_bytes: usize,
}

impl RequestLifecycleService {
    /// Wraps an existing bounded admission controller; it does not own a scheduler.
    pub fn new(admission: Arc<AdmissionController>, policy: RequestPolicy) -> Self {
        let effective_max_input_bytes = policy.max_input_bytes.min(admission.max_input_bytes());
        Self {
            admission,
            policy,
            effective_max_input_bytes,
        }
    }

    /// Validates and transfers an owned body into bounded admission.
    pub fn admit(&self, request: IncomingRequest) -> Result<AdmittedRequest, ApplicationFailure> {
        let request_id = self.request_id_or_unavailable()?;
        let content_type = self.validate(&request, &request_id)?;
        let input_size_bucket = size_bucket(request.body.len());
        let admission = self
            .admission
            .submit(request.body, self.policy.execution_timeout)
            .map_err(|error| {
                self.failure(
                    request_id.clone(),
                    RequestState::Rejected,
                    Some(content_type),
                    Some(input_size_bucket),
                    ApplicationError::from_admission(error),
                )
            })?;
        Ok(AdmittedRequest {
            request_id,
            content_type,
            input_size_bucket,
            admission: Some(admission),
        })
    }

    fn request_id_or_unavailable(&self) -> Result<RequestId, ApplicationFailure> {
        RequestId::generate().map_err(|_| ApplicationFailure {
            request_id: RequestId("unavailable".to_owned()),
            error: ApplicationError::Unavailable,
            event: RequestEvent {
                request_id: RequestId("unavailable".to_owned()),
                state: RequestState::Rejected,
                result: Some(ResultClass::Unavailable),
                input_content_type: None,
                input_size_bucket: None,
                output_size_bucket: None,
            },
        })
    }

    fn validate(
        &self,
        request: &IncomingRequest,
        request_id: &RequestId,
    ) -> Result<ContentType, ApplicationFailure> {
        let content_type = request
            .content_type
            .as_deref()
            .and_then(ContentType::parse)
            .ok_or_else(|| {
                self.failure(
                    request_id.clone(),
                    RequestState::Rejected,
                    None,
                    None,
                    ApplicationError::UnsupportedMediaType,
                )
            })?;
        if request.body.is_empty() {
            return Err(self.failure(
                request_id.clone(),
                RequestState::Rejected,
                Some(content_type),
                None,
                ApplicationError::InvalidRequest,
            ));
        }
        let bucket = size_bucket(request.body.len());
        if request.body.len() > self.effective_max_input_bytes {
            return Err(self.failure(
                request_id.clone(),
                RequestState::Rejected,
                Some(content_type),
                Some(bucket),
                ApplicationError::PayloadTooLarge,
            ));
        }
        Ok(content_type)
    }

    fn failure(
        &self,
        request_id: RequestId,
        state: RequestState,
        content_type: Option<ContentType>,
        input_size_bucket: Option<&'static str>,
        error: ApplicationError,
    ) -> ApplicationFailure {
        ApplicationFailure {
            request_id: request_id.clone(),
            error,
            event: RequestEvent {
                request_id,
                state,
                result: Some(error.class()),
                input_content_type: content_type,
                input_size_bucket,
                output_size_bucket: None,
            },
        }
    }
}

/// An accepted request. Dropping or abandoning it never cancels native work.
pub struct AdmittedRequest {
    request_id: RequestId,
    content_type: ContentType,
    input_size_bucket: &'static str,
    admission: Option<AdmissionRequest>,
}

impl AdmittedRequest {
    /// Waits for the accepted request and transfers Rust-owned JPEG bytes to the caller.
    pub fn wait(mut self) -> Result<ApplicationResponse, ApplicationFailure> {
        let admission = self
            .admission
            .take()
            .expect("accepted request is consumed once");
        match admission.wait() {
            Ok(report) => Ok(self.response(report)),
            Err(error) => Err(self.failure(ApplicationError::from_admission(error))),
        }
    }

    /// Marks the client response as abandoned and drops the response receiver.
    ///
    /// The accepted native operation remains queued or runs to completion; ABI v1
    /// has no in-flight cancellation operation.
    pub fn abandon(mut self) -> RequestEvent {
        self.admission.take();
        RequestEvent {
            request_id: self.request_id,
            state: RequestState::ResponseAbandoned,
            result: None,
            input_content_type: Some(self.content_type),
            input_size_bucket: Some(self.input_size_bucket),
            output_size_bucket: None,
        }
    }

    fn response(&self, report: ProcessReport) -> ApplicationResponse {
        let output_size_bucket = report.output_size_bucket;
        ApplicationResponse {
            request_id: self.request_id.clone(),
            content_type: "image/jpeg",
            body: report.output,
            event: RequestEvent {
                request_id: self.request_id.clone(),
                state: RequestState::Completed,
                result: Some(ResultClass::Success),
                input_content_type: Some(self.content_type),
                input_size_bucket: Some(self.input_size_bucket),
                output_size_bucket: Some(output_size_bucket),
            },
        }
    }

    fn failure(&self, error: ApplicationError) -> ApplicationFailure {
        ApplicationFailure {
            request_id: self.request_id.clone(),
            error,
            event: RequestEvent {
                request_id: self.request_id.clone(),
                state: RequestState::Failed,
                result: Some(error.class()),
                input_content_type: Some(self.content_type),
                input_size_bucket: Some(self.input_size_bucket),
                output_size_bucket: None,
            },
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{
        ApplicationError, ContentType, IncomingRequest, RequestLifecycleService, RequestPolicy,
        RequestState, TransportStatus,
    };
    use crate::{
        AdmissionController, AdmissionOptions, ResultClass, SupervisorOptions, TEST_ENGINE_LOCK,
    };
    use std::fs;
    use std::path::PathBuf;
    use std::sync::Arc;
    use std::thread;
    use std::time::Duration;

    const PNG: &[u8] = &[
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44,
        0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f,
        0x15, 0xc4, 0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0xf8,
        0xcf, 0xc0, 0xf0, 0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99, 0x3d, 0x1d, 0x00, 0x00,
        0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
    ];

    fn test_root(label: &str) -> PathBuf {
        std::env::temp_dir().join(format!(
            "imgengine-request-lifecycle-{label}-{}",
            std::process::id()
        ))
    }

    fn service(root: PathBuf) -> (RequestLifecycleService, Arc<AdmissionController>) {
        let options =
            AdmissionOptions::new(SupervisorOptions::new(root), 2, 1024).expect("options");
        let admission = Arc::new(AdmissionController::start(options).expect("admission"));
        let policy = RequestPolicy::new(1024, Some(Duration::from_secs(1))).expect("policy");
        (
            RequestLifecycleService::new(Arc::clone(&admission), policy),
            admission,
        )
    }

    #[test]
    fn validates_content_type_size_and_transport_mapping_without_native_work() {
        assert_eq!(
            ContentType::parse("image/jpeg; charset=binary"),
            Some(ContentType::Jpeg)
        );
        assert_eq!(
            ApplicationError::PayloadTooLarge.transport_status(),
            TransportStatus::PayloadTooLarge
        );
        assert_eq!(
            ApplicationError::PayloadTooLarge
                .transport_status()
                .as_u16(),
            413
        );
        assert!(RequestPolicy::new(0, None).is_err());
        assert!(RequestPolicy::new(1, Some(Duration::ZERO)).is_err());
    }

    #[test]
    fn rejects_invalid_boundary_requests_with_safe_events() {
        let _engine_lock = TEST_ENGINE_LOCK.lock().expect("lock test engine");
        let root = test_root("validation");
        let (service, admission) = service(root.clone());
        let failure = match service.admit(IncomingRequest::new(
            Some("text/plain".to_owned()),
            PNG.to_vec(),
        )) {
            Err(failure) => failure,
            Ok(_) => panic!("content type accepted"),
        };
        assert_eq!(failure.error, ApplicationError::UnsupportedMediaType);
        assert_eq!(failure.event.state, RequestState::Rejected);
        assert_eq!(failure.event.result, Some(ResultClass::InvalidImage));
        let failure = match service.admit(IncomingRequest::new(
            Some("image/png".to_owned()),
            vec![0; 1025],
        )) {
            Err(failure) => failure,
            Ok(_) => panic!("body limit accepted"),
        };
        assert_eq!(failure.error, ApplicationError::PayloadTooLarge);
        admission.shutdown().expect("shutdown");
        drop(admission);
        fs::remove_dir_all(root).expect("remove root");
    }

    #[test]
    fn uses_the_stricter_application_or_admission_body_limit() {
        let _engine_lock = TEST_ENGINE_LOCK.lock().expect("lock test engine");
        let root = test_root("effective-limit");
        let options =
            AdmissionOptions::new(SupervisorOptions::new(root.clone()), 2, 64).expect("options");
        let admission = Arc::new(AdmissionController::start(options).expect("admission"));
        let policy = RequestPolicy::new(1024, Some(Duration::from_secs(1))).expect("policy");
        let service = RequestLifecycleService::new(Arc::clone(&admission), policy);
        let failure = match service.admit(IncomingRequest::new(
            Some("image/png".to_owned()),
            vec![0; 65],
        )) {
            Err(failure) => failure,
            Ok(_) => panic!("admission limit bypassed"),
        };
        assert_eq!(failure.error, ApplicationError::PayloadTooLarge);
        admission.shutdown().expect("shutdown");
        drop(admission);
        fs::remove_dir_all(root).expect("remove root");
    }

    #[test]
    fn maps_native_malformed_input_without_exposing_diagnostics() {
        let _engine_lock = TEST_ENGINE_LOCK.lock().expect("lock test engine");
        let root = test_root("malformed");
        let (service, admission) = service(root.clone());
        let failure = service
            .admit(IncomingRequest::new(
                Some("image/png".to_owned()),
                b"not an image".to_vec(),
            ))
            .expect("admit")
            .wait();
        let failure = match failure {
            Err(failure) => failure,
            Ok(_) => panic!("malformed image completed"),
        };
        assert_eq!(failure.error, ApplicationError::InvalidImage);
        assert_eq!(failure.event.state, RequestState::Failed);
        admission.shutdown().expect("shutdown");
        drop(admission);
        fs::remove_dir_all(root).expect("remove root");
    }

    #[test]
    fn shutdown_rejects_new_requests_without_discarding_accepted_work() {
        let _engine_lock = TEST_ENGINE_LOCK.lock().expect("lock test engine");
        let root = test_root("shutdown");
        let (service, admission) = service(root.clone());
        let accepted = service
            .admit(IncomingRequest::new(
                Some("image/png".to_owned()),
                PNG.to_vec(),
            ))
            .expect("admit before shutdown");
        admission.shutdown().expect("drain shutdown");
        let response = accepted.wait().expect("accepted work completes");
        assert_eq!(response.event.state, RequestState::Completed);
        let failure = match service.admit(IncomingRequest::new(
            Some("image/png".to_owned()),
            PNG.to_vec(),
        )) {
            Err(failure) => failure,
            Ok(_) => panic!("request accepted after shutdown"),
        };
        assert_eq!(failure.error, ApplicationError::Unavailable);
        drop(admission);
        fs::remove_dir_all(root).expect("remove root");
    }

    #[test]
    fn completes_response_with_rust_owned_output_and_cleanup() {
        let _engine_lock = TEST_ENGINE_LOCK.lock().expect("lock test engine");
        let root = test_root("complete");
        let (service, admission) = service(root.clone());
        let response = service
            .admit(IncomingRequest::new(
                Some("image/png".to_owned()),
                PNG.to_vec(),
            ))
            .expect("admit")
            .wait()
            .expect("complete");
        assert_eq!(response.content_type, "image/jpeg");
        assert_eq!(&response.body[..2], &[0xff, 0xd8]);
        assert_eq!(response.event.state, RequestState::Completed);
        assert_eq!(response.event.result, Some(ResultClass::Success));
        admission.shutdown().expect("shutdown");
        assert!(fs::read_dir(&root).expect("list root").next().is_none());
        drop(admission);
        fs::remove_dir_all(root).expect("remove root");
    }

    #[test]
    fn response_abandonment_does_not_cancel_accepted_native_work() {
        let _engine_lock = TEST_ENGINE_LOCK.lock().expect("lock test engine");
        let root = test_root("abandon");
        let (service, admission) = service(root.clone());
        let event = service
            .admit(IncomingRequest::new(
                Some("image/png".to_owned()),
                PNG.to_vec(),
            ))
            .expect("admit")
            .abandon();
        assert_eq!(event.state, RequestState::ResponseAbandoned);
        for _ in 0..20 {
            if admission.metrics().completed == 1 {
                break;
            }
            thread::sleep(Duration::from_millis(5));
        }
        assert_eq!(admission.metrics().completed, 1);
        admission.shutdown().expect("shutdown");
        assert!(fs::read_dir(&root).expect("list root").next().is_none());
        drop(admission);
        fs::remove_dir_all(root).expect("remove root");
    }
}
