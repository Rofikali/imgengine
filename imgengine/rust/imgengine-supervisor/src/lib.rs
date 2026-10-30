//! Single-process request supervision around the safe `libimgengine` wrapper.
//!
//! This crate deliberately owns one non-`Send`/non-`Sync` engine and exposes
//! only synchronous processing. It does not replace the native scheduler and
//! cannot cancel an operation already executing in ABI v1.

mod admission;
mod request_lifecycle;

pub use admission::{
    AdmissionController, AdmissionError, AdmissionExecutionObserver, AdmissionMetricsSnapshot,
    AdmissionOptions, AdmissionRequest,
};
pub use request_lifecycle::{
    AdmittedRequest, AdmittedWaitOutcome, ApplicationError, ApplicationFailure,
    ApplicationResponse, ContentType, IncomingRequest, LifecycleMetricsSnapshot, RequestEvent,
    RequestId, RequestLifecycleService, RequestPolicy, RequestState, ResponseAbandonmentObserver,
    ResponseAbandonmentSignal, TransportStatus,
};

use imgengine::{
    Engine, EngineOptions, Error as EngineError, LayoutOutput, LayoutRect, LayoutRequest,
};
use std::fmt;
use std::fs;
use std::io::{self, Read};
use std::os::unix::fs::{DirBuilderExt, PermissionsExt};
use std::path::PathBuf;
use std::time::{Duration, Instant, SystemTime};

const REQUEST_PREFIX: &str = "request-";
const RANDOM_BYTES: usize = 16;
const CREATE_ATTEMPTS: usize = 8;
const DEFAULT_STALE_AFTER: Duration = Duration::from_secs(5 * 60);

#[cfg(test)]
static TEST_ENGINE_LOCK: std::sync::Mutex<()> = std::sync::Mutex::new(());

/// The safe result classes intended for logs and metrics.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ResultClass {
    Success,
    InvalidArgument,
    InvalidImage,
    ResourceLimit,
    Overloaded,
    Unavailable,
    Internal,
    DeadlineExceeded,
    WorkspaceFailure,
}

impl ResultClass {
    /// Returns a stable, diagnostic-safe label.
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Success => "success",
            Self::InvalidArgument => "invalid_argument",
            Self::InvalidImage => "invalid_image",
            Self::ResourceLimit => "resource_limit",
            Self::Overloaded => "overloaded",
            Self::Unavailable => "unavailable",
            Self::Internal => "internal",
            Self::DeadlineExceeded => "deadline_exceeded",
            Self::WorkspaceFailure => "workspace_failure",
        }
    }
}

/// A supervisor failure that never contains client paths or image bytes.
#[derive(Debug)]
pub enum SupervisorError {
    Engine(EngineError),
    DeadlineExceeded,
    Workspace(io::Error),
}

impl SupervisorError {
    /// Returns a stable, diagnostic-safe error class.
    pub fn class(&self) -> ResultClass {
        match self {
            Self::Engine(EngineError::InvalidArgument) => ResultClass::InvalidArgument,
            Self::Engine(EngineError::InvalidImage) => ResultClass::InvalidImage,
            Self::Engine(EngineError::ResourceLimit) => ResultClass::ResourceLimit,
            Self::Engine(EngineError::Unavailable) => ResultClass::Unavailable,
            Self::DeadlineExceeded => ResultClass::DeadlineExceeded,
            Self::Engine(_) => ResultClass::Internal,
            Self::Workspace(_) => ResultClass::WorkspaceFailure,
        }
    }
}

impl fmt::Display for SupervisorError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter.write_str(self.class().as_str())
    }
}

impl std::error::Error for SupervisorError {}

/// Immutable construction policy for one supervisor instance.
#[derive(Clone, Debug)]
pub struct SupervisorOptions {
    workspace_root: PathBuf,
    stale_after: Duration,
    engine_options: EngineOptions,
    layout_max_output_bytes: usize,
}

impl SupervisorOptions {
    /// Uses a private workspace root and the ABI-v1 default native worker count.
    pub fn new(workspace_root: impl Into<PathBuf>) -> Self {
        Self {
            workspace_root: workspace_root.into(),
            stale_after: DEFAULT_STALE_AFTER,
            engine_options: EngineOptions::default(),
            layout_max_output_bytes: 16 * 1024 * 1024,
        }
    }

    /// Sets the stale request-directory retention bound.
    pub fn stale_after(mut self, stale_after: Duration) -> Self {
        self.stale_after = stale_after;
        self
    }

    /// Sets validated native engine options.
    pub fn engine_options(mut self, engine_options: EngineOptions) -> Self {
        self.engine_options = engine_options;
        self
    }

    /// Sets the server-owned encoded-JPEG budget for the FIT-only layout path.
    /// A zero value is rejected by the native bounded API before processing.
    pub fn layout_max_output_bytes(mut self, layout_max_output_bytes: usize) -> Self {
        self.layout_max_output_bytes = layout_max_output_bytes;
        self
    }
}

/// A completed in-memory image result and redacted telemetry fields.
#[derive(Debug)]
pub struct ProcessReport {
    /// Rust-owned JPEG bytes. The native allocation has already been released.
    pub output: Vec<u8>,
    /// Wall-clock duration including native execution and workspace cleanup.
    pub duration: Duration,
    /// Coarse bucket intended for logs and metrics, not exact client metadata.
    pub input_size_bucket: &'static str,
    /// Coarse bucket intended for logs and metrics, not exact client metadata.
    pub output_size_bucket: &'static str,
    /// Present only for the typed FIT-only layout operation.
    pub layout: Option<LayoutExecution>,
}

/// Native plan accounting returned only after all placements completed.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct LayoutExecution {
    pub canvas_width: u32,
    pub canvas_height: u32,
    pub expected_count: u32,
    pub planned_count: u32,
    pub executed_count: u32,
}

/// Server-generated FIT-only layout plan. Geometry is in pixels and all
/// rectangles are non-overlapping and inside the declared canvas.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct LayoutPlan {
    pub canvas_width: u32,
    pub canvas_height: u32,
    pub placements: Vec<LayoutRect>,
}

impl LayoutPlan {
    /// Computes deterministic row-major grid placement with checked arithmetic.
    pub fn grid(
        photo_width: u32,
        photo_height: u32,
        copies: u32,
        columns: u32,
        gap: u32,
        margin: u32,
    ) -> Result<Self, SupervisorError> {
        if photo_width == 0 || photo_height == 0 || copies == 0 || columns == 0 {
            return Err(SupervisorError::Engine(EngineError::InvalidArgument));
        }
        let rows = copies
            .checked_add(columns - 1)
            .and_then(|value| value.checked_div(columns))
            .ok_or(SupervisorError::Engine(EngineError::ResourceLimit))?;
        let horizontal_gaps = columns - 1;
        let vertical_gaps = rows - 1;
        let canvas_width = margin
            .checked_mul(2)
            .and_then(|value| value.checked_add(photo_width.checked_mul(columns)?))
            .and_then(|value| value.checked_add(gap.checked_mul(horizontal_gaps)?))
            .ok_or(SupervisorError::Engine(EngineError::ResourceLimit))?;
        let canvas_height = margin
            .checked_mul(2)
            .and_then(|value| value.checked_add(photo_height.checked_mul(rows)?))
            .and_then(|value| value.checked_add(gap.checked_mul(vertical_gaps)?))
            .ok_or(SupervisorError::Engine(EngineError::ResourceLimit))?;
        let mut placements = Vec::with_capacity(copies as usize);
        for index in 0..copies {
            let row = index / columns;
            let column = index % columns;
            let x = margin
                .checked_add(
                    column
                        .checked_mul(photo_width)
                        .ok_or(SupervisorError::Engine(EngineError::ResourceLimit))?,
                )
                .and_then(|value| value.checked_add(column.checked_mul(gap)?))
                .ok_or(SupervisorError::Engine(EngineError::ResourceLimit))?;
            let y = margin
                .checked_add(
                    row.checked_mul(photo_height)
                        .ok_or(SupervisorError::Engine(EngineError::ResourceLimit))?,
                )
                .and_then(|value| value.checked_add(row.checked_mul(gap)?))
                .ok_or(SupervisorError::Engine(EngineError::ResourceLimit))?;
            placements.push(LayoutRect {
                x,
                y,
                width: photo_width,
                height: photo_height,
            });
        }
        Ok(Self {
            canvas_width,
            canvas_height,
            placements,
        })
    }

    fn native_request(&self) -> Result<LayoutRequest, SupervisorError> {
        let expected_count = u32::try_from(self.placements.len())
            .map_err(|_| SupervisorError::Engine(EngineError::ResourceLimit))?;
        Ok(LayoutRequest {
            canvas_width: self.canvas_width,
            canvas_height: self.canvas_height,
            background: [255, 255, 255],
            placements: self.placements.clone(),
            expected_count,
        })
    }
}

impl ProcessReport {
    /// Returns a diagnostic-safe completion event for structured logging.
    pub fn event(&self) -> ProcessEvent {
        ProcessEvent {
            result: ResultClass::Success,
            duration: self.duration,
            input_size_bucket: self.input_size_bucket,
            output_size_bucket: Some(self.output_size_bucket),
        }
    }
}

/// Redacted operation telemetry; it never contains a path, filename, or bytes.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct ProcessEvent {
    pub result: ResultClass,
    pub duration: Duration,
    pub input_size_bucket: &'static str,
    pub output_size_bucket: Option<&'static str>,
}

/// Single-engine synchronous control-plane foundation.
///
/// `process` requires `&mut self`, which makes the ABI-v1 concurrency limit of
/// one explicit in Rust. The contained engine is neither `Send` nor `Sync`.
pub struct Supervisor {
    engine: Engine,
    workspace_root: WorkspaceRoot,
    layout_max_output_bytes: usize,
}

impl Supervisor {
    /// Initializes the private workspace root, sweeps stale request folders,
    /// and creates the one native engine permitted by ABI v1.
    pub fn new(options: SupervisorOptions) -> Result<Self, SupervisorError> {
        let workspace_root = WorkspaceRoot::prepare(options.workspace_root, options.stale_after)
            .map_err(SupervisorError::Workspace)?;
        let engine = Engine::new(options.engine_options).map_err(SupervisorError::Engine)?;
        Ok(Self {
            engine,
            workspace_root,
            layout_max_output_bytes: options.layout_max_output_bytes,
        })
    }

    /// Processes one request in an isolated ephemeral workspace.
    ///
    /// A zero or expired deadline is rejected before native execution. If the
    /// call finishes after a positive deadline, output is discarded and a
    /// deadline error is returned. ABI v1 cannot interrupt native work that is
    /// already running.
    pub fn process(
        &mut self,
        input: &[u8],
        deadline: Option<Duration>,
    ) -> Result<ProcessReport, SupervisorError> {
        if matches!(deadline, Some(limit) if limit.is_zero()) {
            return Err(SupervisorError::DeadlineExceeded);
        }

        let started = Instant::now();
        let mut workspace = self
            .workspace_root
            .create_request_workspace()
            .map_err(SupervisorError::Workspace)?;
        let output = self
            .engine
            .encode_jpeg(input)
            .map_err(SupervisorError::Engine);
        let duration = started.elapsed();
        let cleanup = workspace.close();

        if let Err(error) = cleanup {
            return Err(SupervisorError::Workspace(error));
        }
        if matches!(deadline, Some(limit) if duration > limit) {
            return Err(SupervisorError::DeadlineExceeded);
        }
        let output = output?;
        Ok(ProcessReport {
            input_size_bucket: size_bucket(input.len()),
            output_size_bucket: size_bucket(output.len()),
            output,
            duration,
            layout: None,
        })
    }

    /// Runs the typed native FIT-only layout operation in an isolated workspace.
    pub fn process_layout(
        &mut self,
        input: &[u8],
        plan: &LayoutPlan,
        deadline: Option<Duration>,
    ) -> Result<ProcessReport, SupervisorError> {
        if matches!(deadline, Some(limit) if limit.is_zero()) {
            return Err(SupervisorError::DeadlineExceeded);
        }
        let started = Instant::now();
        let mut workspace = self
            .workspace_root
            .create_request_workspace()
            .map_err(SupervisorError::Workspace)?;
        let request = plan.native_request()?;
        let output = self
            .engine
            .render_layout_jpeg_bounded(input, &request, self.layout_max_output_bytes)
            .map_err(SupervisorError::Engine);
        let duration = started.elapsed();
        let cleanup = workspace.close();
        if let Err(error) = cleanup {
            return Err(SupervisorError::Workspace(error));
        }
        if matches!(deadline, Some(limit) if duration > limit) {
            return Err(SupervisorError::DeadlineExceeded);
        }
        let output: LayoutOutput = output?;
        Ok(ProcessReport {
            input_size_bucket: size_bucket(input.len()),
            output_size_bucket: size_bucket(output.jpeg.len()),
            output: output.jpeg,
            duration,
            layout: Some(LayoutExecution {
                canvas_width: output.canvas_width,
                canvas_height: output.canvas_height,
                expected_count: output.expected_count,
                planned_count: output.planned_count,
                executed_count: output.executed_count,
            }),
        })
    }
}

#[derive(Debug)]
struct WorkspaceRoot {
    path: PathBuf,
}

impl WorkspaceRoot {
    fn prepare(path: PathBuf, stale_after: Duration) -> io::Result<Self> {
        match fs::symlink_metadata(&path) {
            Ok(metadata) if metadata.file_type().is_symlink() => {
                return Err(io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "workspace root must not be a symlink",
                ));
            }
            Ok(metadata) if !metadata.is_dir() => {
                return Err(io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "workspace root must be a directory",
                ));
            }
            Ok(_) => {}
            Err(error) if error.kind() == io::ErrorKind::NotFound => {
                let mut builder = fs::DirBuilder::new();
                builder.recursive(true).mode(0o700);
                builder.create(&path)?;
            }
            Err(error) => return Err(error),
        }
        fs::set_permissions(&path, fs::Permissions::from_mode(0o700))?;
        let root = Self { path };
        root.sweep_stale(stale_after)?;
        Ok(root)
    }

    fn create_request_workspace(&self) -> io::Result<RequestWorkspace> {
        for _ in 0..CREATE_ATTEMPTS {
            let path = self.path.join(request_directory_name()?);
            let mut builder = fs::DirBuilder::new();
            builder.mode(0o700);
            match builder.create(&path) {
                Ok(()) => return Ok(RequestWorkspace { path: Some(path) }),
                Err(error) if error.kind() == io::ErrorKind::AlreadyExists => continue,
                Err(error) => return Err(error),
            }
        }
        Err(io::Error::new(
            io::ErrorKind::AlreadyExists,
            "unable to allocate an isolated request workspace",
        ))
    }

    fn sweep_stale(&self, stale_after: Duration) -> io::Result<()> {
        let now = SystemTime::now();
        for entry in fs::read_dir(&self.path)? {
            let entry = entry?;
            let name = entry.file_name();
            if !name.to_string_lossy().starts_with(REQUEST_PREFIX) {
                continue;
            }
            let path = entry.path();
            let metadata = fs::symlink_metadata(&path)?;
            if metadata.file_type().is_symlink() {
                fs::remove_file(path)?;
                continue;
            }
            if !metadata.is_dir() {
                continue;
            }
            let age = now.duration_since(metadata.modified()?).unwrap_or_default();
            if age > stale_after {
                fs::remove_dir_all(path)?;
            }
        }
        Ok(())
    }
}

#[derive(Debug)]
struct RequestWorkspace {
    path: Option<PathBuf>,
}

impl RequestWorkspace {
    fn close(&mut self) -> io::Result<()> {
        let Some(path) = self.path.take() else {
            return Ok(());
        };
        match fs::remove_dir_all(path) {
            Ok(()) => Ok(()),
            Err(error) if error.kind() == io::ErrorKind::NotFound => Ok(()),
            Err(error) => Err(error),
        }
    }
}

impl Drop for RequestWorkspace {
    fn drop(&mut self) {
        let _ = self.close();
    }
}

fn request_directory_name() -> io::Result<String> {
    let mut random = [0u8; RANDOM_BYTES];
    fs::File::open("/dev/urandom")?.read_exact(&mut random)?;
    let mut name = String::from(REQUEST_PREFIX);
    for byte in random {
        use std::fmt::Write;
        write!(&mut name, "{byte:02x}").expect("writing to String cannot fail");
    }
    Ok(name)
}

pub(crate) fn size_bucket(size: usize) -> &'static str {
    match size {
        0..=4_095 => "0-4KiB",
        4_096..=65_535 => "4-64KiB",
        65_536..=1_048_575 => "64KiB-1MiB",
        1_048_576..=8_388_607 => "1-8MiB",
        _ => "8MiB+",
    }
}

#[cfg(test)]
mod tests {
    use super::{
        size_bucket, LayoutPlan, ResultClass, Supervisor, SupervisorOptions, WorkspaceRoot,
    };
    use std::fs;
    use std::path::PathBuf;
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
            "imgengine-supervisor-{label}-{}",
            std::process::id()
        ))
    }

    #[test]
    fn sizes_are_redacted_into_buckets() {
        assert_eq!(size_bucket(0), "0-4KiB");
        assert_eq!(size_bucket(4_096), "4-64KiB");
        assert_eq!(size_bucket(1_048_576), "1-8MiB");
    }

    #[test]
    fn stale_request_directories_are_swept() {
        let root = test_root("sweep");
        let stale = root.join("request-stale");
        fs::create_dir_all(&stale).expect("create stale workspace");
        thread::sleep(Duration::from_millis(20));
        let _workspace_root =
            WorkspaceRoot::prepare(root.clone(), Duration::ZERO).expect("prepare root");
        assert!(!stale.exists());
        fs::remove_dir_all(root).expect("remove test root");
    }

    #[test]
    fn workspace_root_and_request_directory_are_private() {
        use std::os::unix::fs::PermissionsExt;

        let root = test_root("permissions");
        let workspace_root =
            WorkspaceRoot::prepare(root.clone(), Duration::ZERO).expect("prepare root");
        assert_eq!(
            fs::metadata(&root)
                .expect("read root metadata")
                .permissions()
                .mode()
                & 0o777,
            0o700
        );
        let mut workspace = workspace_root
            .create_request_workspace()
            .expect("create workspace");
        let path = workspace.path.as_ref().expect("workspace path");
        assert_eq!(
            fs::metadata(path)
                .expect("read workspace metadata")
                .permissions()
                .mode()
                & 0o777,
            0o700
        );
        workspace.close().expect("close workspace");
        fs::remove_dir_all(root).expect("remove test root");
    }

    #[test]
    fn supervisor_cleans_up_and_preserves_deadline_contract() {
        let _engine_lock = super::TEST_ENGINE_LOCK.lock().expect("lock test engine");
        let root = test_root("process");
        let mut supervisor =
            Supervisor::new(SupervisorOptions::new(root.clone())).expect("create supervisor");
        assert_eq!(
            supervisor
                .process(PNG, Some(Duration::ZERO))
                .unwrap_err()
                .class(),
            ResultClass::DeadlineExceeded
        );
        assert_eq!(
            supervisor.process(&[0, 0xff, 1], None).unwrap_err().class(),
            ResultClass::InvalidImage
        );
        let report = supervisor.process(PNG, None).expect("encode PNG");
        assert_eq!(&report.output[..2], &[0xff, 0xd8]);
        assert_eq!(report.event().result, ResultClass::Success);
        let plan = LayoutPlan::grid(12, 8, 2, 2, 2, 1).expect("checked layout plan");
        assert_eq!((plan.canvas_width, plan.canvas_height), (28, 10));
        let layout = supervisor
            .process_layout(PNG, &plan, None)
            .expect("render native layout");
        assert_eq!(&layout.output[..2], &[0xff, 0xd8]);
        assert_eq!(
            layout.layout.expect("layout accounting"),
            super::LayoutExecution {
                canvas_width: 28,
                canvas_height: 10,
                expected_count: 2,
                planned_count: 2,
                executed_count: 2,
            }
        );
        assert!(fs::read_dir(&root)
            .expect("list workspace root")
            .next()
            .is_none());
        drop(supervisor);
        fs::remove_dir_all(root).expect("remove test root");
    }
}
