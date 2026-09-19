use crate::{ProcessReport, ResultClass, Supervisor, SupervisorError, SupervisorOptions};
use std::fmt;
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::mpsc::{self, Receiver, SyncSender, TrySendError};
use std::sync::Arc;
use std::thread::{self, JoinHandle};
use std::time::Duration;

struct QueuedRequest {
    input: Vec<u8>,
    deadline: Option<Duration>,
    response: SyncSender<Result<ProcessReport, SupervisorError>>,
}

#[derive(Default)]
struct AdmissionMetrics {
    accepted: AtomicU64,
    completed: AtomicU64,
    succeeded: AtomicU64,
    failed: AtomicU64,
    rejected_overload: AtomicU64,
    rejected_resource_limit: AtomicU64,
    deadline_exceeded: AtomicU64,
}

impl AdmissionMetrics {
    fn snapshot(&self) -> AdmissionMetricsSnapshot {
        AdmissionMetricsSnapshot {
            accepted: self.accepted.load(Ordering::Relaxed),
            completed: self.completed.load(Ordering::Relaxed),
            succeeded: self.succeeded.load(Ordering::Relaxed),
            failed: self.failed.load(Ordering::Relaxed),
            rejected_overload: self.rejected_overload.load(Ordering::Relaxed),
            rejected_resource_limit: self.rejected_resource_limit.load(Ordering::Relaxed),
            deadline_exceeded: self.deadline_exceeded.load(Ordering::Relaxed),
        }
    }

    fn record_completion(&self, result: &Result<ProcessReport, SupervisorError>) {
        self.completed.fetch_add(1, Ordering::Relaxed);
        match result {
            Ok(_) => {
                self.succeeded.fetch_add(1, Ordering::Relaxed);
            }
            Err(error) => {
                self.failed.fetch_add(1, Ordering::Relaxed);
                if error.class() == ResultClass::DeadlineExceeded {
                    self.deadline_exceeded.fetch_add(1, Ordering::Relaxed);
                }
            }
        }
    }
}

/// Immutable bounded-admission policy for one process-scoped supervisor.
#[derive(Clone, Debug)]
pub struct AdmissionOptions {
    supervisor_options: SupervisorOptions,
    queue_capacity: usize,
    max_input_bytes: usize,
}

impl AdmissionOptions {
    /// Creates bounded admission options for a supervisor instance.
    pub fn new(
        supervisor_options: SupervisorOptions,
        queue_capacity: usize,
        max_input_bytes: usize,
    ) -> Result<Self, AdmissionError> {
        if queue_capacity == 0 || max_input_bytes == 0 {
            return Err(AdmissionError::InvalidArgument);
        }
        Ok(Self {
            supervisor_options,
            queue_capacity,
            max_input_bytes,
        })
    }
}

/// Safe admission-layer failures suitable for HTTP mapping and structured logs.
#[derive(Debug)]
pub enum AdmissionError {
    InvalidArgument,
    ResourceLimit,
    Overloaded,
    Unavailable,
    Supervisor(SupervisorError),
}

impl AdmissionError {
    /// Returns a stable, diagnostic-safe result class.
    pub fn class(&self) -> ResultClass {
        match self {
            Self::InvalidArgument => ResultClass::InvalidArgument,
            Self::ResourceLimit => ResultClass::ResourceLimit,
            Self::Overloaded => ResultClass::Overloaded,
            Self::Unavailable => ResultClass::Unavailable,
            Self::Supervisor(error) => error.class(),
        }
    }
}

impl fmt::Display for AdmissionError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter.write_str(self.class().as_str())
    }
}

impl std::error::Error for AdmissionError {}

/// Redacted lifetime counters for the bounded admission boundary.
#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct AdmissionMetricsSnapshot {
    pub accepted: u64,
    pub completed: u64,
    pub succeeded: u64,
    pub failed: u64,
    pub rejected_overload: u64,
    pub rejected_resource_limit: u64,
    pub deadline_exceeded: u64,
}

/// Handle returned after a request has been accepted into the bounded queue.
pub struct AdmissionRequest {
    response: Receiver<Result<ProcessReport, SupervisorError>>,
}

impl AdmissionRequest {
    /// Waits for the one worker to finish the accepted request.
    pub fn wait(self) -> Result<ProcessReport, AdmissionError> {
        self.response
            .recv()
            .map_err(|_| AdmissionError::Unavailable)?
            .map_err(AdmissionError::Supervisor)
    }
}

/// Thread-safe bounded admission handle for a single-engine worker thread.
///
/// The native engine and `Supervisor` are created and remain inside the worker
/// thread. Callers can safely share this handle; a full queue returns
/// `overloaded` without retaining additional image bytes. Shutdown is orderly:
/// accepted work completes first because ABI v1 has no mid-operation cancellation.
pub struct AdmissionController {
    sender: Option<SyncSender<QueuedRequest>>,
    worker: Option<JoinHandle<()>>,
    metrics: Arc<AdmissionMetrics>,
    max_input_bytes: usize,
}

impl AdmissionController {
    /// Starts a dedicated worker and waits until the supervisor is ready.
    pub fn start(options: AdmissionOptions) -> Result<Self, AdmissionError> {
        let (sender, receiver) = mpsc::sync_channel(options.queue_capacity);
        let (ready_sender, ready_receiver) = mpsc::sync_channel(1);
        let metrics = Arc::new(AdmissionMetrics::default());
        let worker_metrics = Arc::clone(&metrics);
        let supervisor_options = options.supervisor_options;
        let worker = thread::Builder::new()
            .name("imgengine-supervisor".to_owned())
            .spawn(move || worker_loop(receiver, ready_sender, worker_metrics, supervisor_options))
            .map_err(|_| AdmissionError::Unavailable)?;

        match ready_receiver.recv() {
            Ok(Ok(())) => Ok(Self {
                sender: Some(sender),
                worker: Some(worker),
                metrics,
                max_input_bytes: options.max_input_bytes,
            }),
            Ok(Err(error)) => {
                let _ = worker.join();
                Err(AdmissionError::Supervisor(error))
            }
            Err(_) => {
                let _ = worker.join();
                Err(AdmissionError::Unavailable)
            }
        }
    }

    /// Attempts to admit one owned request without blocking on a full queue.
    pub fn submit(
        &self,
        input: Vec<u8>,
        deadline: Option<Duration>,
    ) -> Result<AdmissionRequest, AdmissionError> {
        if input.len() > self.max_input_bytes {
            self.metrics
                .rejected_resource_limit
                .fetch_add(1, Ordering::Relaxed);
            return Err(AdmissionError::ResourceLimit);
        }

        let sender = self.sender.as_ref().ok_or(AdmissionError::Unavailable)?;
        let (response_sender, response) = mpsc::sync_channel(1);
        let request = QueuedRequest {
            input,
            deadline,
            response: response_sender,
        };
        match sender.try_send(request) {
            Ok(()) => {
                self.metrics.accepted.fetch_add(1, Ordering::Relaxed);
                Ok(AdmissionRequest { response })
            }
            Err(TrySendError::Full(_)) => {
                self.metrics
                    .rejected_overload
                    .fetch_add(1, Ordering::Relaxed);
                Err(AdmissionError::Overloaded)
            }
            Err(TrySendError::Disconnected(_)) => Err(AdmissionError::Unavailable),
        }
    }

    /// Returns a redacted counter snapshot for metrics collection.
    pub fn metrics(&self) -> AdmissionMetricsSnapshot {
        self.metrics.snapshot()
    }

    /// Stops accepting new work and waits for accepted work to complete.
    pub fn shutdown(mut self) -> Result<(), AdmissionError> {
        self.stop()
    }

    fn stop(&mut self) -> Result<(), AdmissionError> {
        self.sender.take();
        if let Some(worker) = self.worker.take() {
            worker.join().map_err(|_| AdmissionError::Unavailable)?;
        }
        Ok(())
    }
}

impl Drop for AdmissionController {
    fn drop(&mut self) {
        let _ = self.stop();
    }
}

fn worker_loop(
    receiver: Receiver<QueuedRequest>,
    ready_sender: SyncSender<Result<(), SupervisorError>>,
    metrics: Arc<AdmissionMetrics>,
    options: SupervisorOptions,
) {
    let mut supervisor = match Supervisor::new(options) {
        Ok(supervisor) => {
            let _ = ready_sender.send(Ok(()));
            supervisor
        }
        Err(error) => {
            let _ = ready_sender.send(Err(error));
            return;
        }
    };

    for request in receiver {
        let result = supervisor.process(&request.input, request.deadline);
        metrics.record_completion(&result);
        let _ = request.response.send(result);
    }
}

#[cfg(test)]
mod tests {
    use super::{
        AdmissionController, AdmissionError, AdmissionMetrics, AdmissionOptions, QueuedRequest,
    };
    use crate::{ResultClass, SupervisorOptions, TEST_ENGINE_LOCK};
    use std::fs;
    use std::path::PathBuf;
    use std::sync::atomic::Ordering;
    use std::sync::{mpsc, Arc};

    const PNG: &[u8] = &[
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44,
        0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f,
        0x15, 0xc4, 0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0xf8,
        0xcf, 0xc0, 0xf0, 0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99, 0x3d, 0x1d, 0x00, 0x00,
        0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
    ];

    fn test_root(label: &str) -> PathBuf {
        std::env::temp_dir().join(format!(
            "imgengine-admission-{label}-{}",
            std::process::id()
        ))
    }

    #[test]
    fn rejects_invalid_admission_configuration() {
        let options = SupervisorOptions::new(test_root("invalid-options"));
        assert_eq!(
            AdmissionOptions::new(options.clone(), 0, 1)
                .expect_err("zero queue capacity must fail")
                .class(),
            ResultClass::InvalidArgument
        );
        assert_eq!(
            AdmissionOptions::new(options, 1, 0)
                .expect_err("zero input limit must fail")
                .class(),
            ResultClass::InvalidArgument
        );
    }

    #[test]
    fn full_queue_is_rejected_without_unbounded_retention() {
        let metrics = Arc::new(AdmissionMetrics::default());
        let (sender, _receiver) = mpsc::sync_channel(1);
        let (first_response, _first_waiter) = mpsc::sync_channel(1);
        sender
            .try_send(QueuedRequest {
                input: vec![1],
                deadline: None,
                response: first_response,
            })
            .expect("fill bounded queue");
        let (second_response, _second_waiter) = mpsc::sync_channel(1);
        let second = QueuedRequest {
            input: vec![2],
            deadline: None,
            response: second_response,
        };
        let error = match sender.try_send(second) {
            Err(mpsc::TrySendError::Full(_)) => {
                metrics.rejected_overload.fetch_add(1, Ordering::Relaxed);
                AdmissionError::Overloaded
            }
            _ => panic!("second request must be rejected"),
        };
        assert_eq!(error.class(), ResultClass::Overloaded);
        assert_eq!(metrics.snapshot().rejected_overload, 1);
    }

    #[test]
    fn controller_is_shareable_and_reports_redacted_metrics() {
        let _engine_lock = TEST_ENGINE_LOCK.lock().expect("lock test engine");
        let root = test_root("controller");
        let options = AdmissionOptions::new(SupervisorOptions::new(root.clone()), 2, 1024)
            .expect("create options");
        let controller = Arc::new(AdmissionController::start(options).expect("start controller"));
        let worker_controller = Arc::clone(&controller);
        let worker = std::thread::spawn(move || {
            worker_controller
                .submit(PNG.to_vec(), None)
                .expect("submit from another thread")
                .wait()
                .expect("wait from another thread")
        });
        let report = controller
            .submit(PNG.to_vec(), None)
            .expect("submit from caller thread")
            .wait()
            .expect("wait from caller thread");
        let worker_report = worker.join().expect("join caller thread");
        assert_eq!(&report.output[..2], &[0xff, 0xd8]);
        assert_eq!(&worker_report.output[..2], &[0xff, 0xd8]);
        let resource_limit_error = match controller.submit(vec![0; 1025], None) {
            Err(error) => error,
            Ok(_) => panic!("oversized input must be rejected"),
        };
        assert_eq!(resource_limit_error.class(), ResultClass::ResourceLimit);
        assert_eq!(
            controller.metrics(),
            super::AdmissionMetricsSnapshot {
                accepted: 2,
                completed: 2,
                succeeded: 2,
                failed: 0,
                rejected_overload: 0,
                rejected_resource_limit: 1,
                deadline_exceeded: 0,
            }
        );
        let controller = match Arc::try_unwrap(controller) {
            Ok(controller) => controller,
            Err(_) => panic!("single controller owner"),
        };
        controller.shutdown().expect("shutdown controller");
        fs::remove_dir_all(root).expect("remove test root");
    }
}
