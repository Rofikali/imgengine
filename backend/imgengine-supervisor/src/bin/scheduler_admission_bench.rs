use imgengine_supervisor::{
    AdmissionController, AdmissionError, AdmissionOptions, SupervisorOptions,
};
use std::fs;
use std::path::PathBuf;
use std::sync::{Arc, Barrier};
use std::time::{Duration, Instant};

#[derive(Default)]
struct Arguments {
    input: Option<PathBuf>,
    fixture: Option<String>,
    iterations: Option<usize>,
    warmup: usize,
    concurrency: usize,
    queue_capacity: usize,
}

struct RequestSample {
    wall_ns: u64,
    processing_ns: Option<u64>,
    output_bytes: usize,
    result: Result<(), AdmissionError>,
}

impl Default for RequestSample {
    fn default() -> Self {
        Self {
            wall_ns: 0,
            processing_ns: None,
            output_bytes: 0,
            result: Err(AdmissionError::Unavailable),
        }
    }
}

#[derive(Default)]
struct LatencyStats {
    p50_ns: u64,
    p95_ns: u64,
}

fn main() -> Result<(), String> {
    let arguments = parse_arguments()?;
    let input_path = arguments.input.ok_or("missing --input")?;
    let fixture = arguments.fixture.ok_or("missing --fixture")?;
    let iterations = arguments.iterations.ok_or("missing --iterations")?;
    let input =
        fs::read(input_path).map_err(|error| format!("unable to read benchmark input: {error}"))?;
    let input_size = input.len();
    let workspace_root = std::env::temp_dir().join(format!(
        "imgengine-scheduler-admission-bench-{}",
        std::process::id()
    ));
    let options = AdmissionOptions::new(
        SupervisorOptions::new(workspace_root.clone()),
        arguments.queue_capacity,
        input.len(),
    )
    .map_err(|error| format!("invalid admission benchmark options: {error}"))?;
    let controller = Arc::new(
        AdmissionController::start(options)
            .map_err(|error| format!("unable to start admission controller: {error}"))?,
    );

    for _ in 0..arguments.warmup {
        let request = controller
            .submit(input.clone(), None)
            .map_err(|error| format!("warmup admission failed: {error}"))?;
        let _ = request.wait();
    }

    let cpu_before = process_cpu_ns();
    let measurement_started = Instant::now();
    let samples = run_measurement(
        Arc::clone(&controller),
        Arc::new(input),
        iterations,
        arguments.concurrency,
    );
    let elapsed = measurement_started.elapsed();
    let cpu_after = process_cpu_ns();
    let metrics = controller.metrics();
    controller
        .shutdown()
        .map_err(|error| format!("admission shutdown failed: {error}"))?;
    let cleanup_empty = fs::read_dir(&workspace_root)
        .map_err(|error| format!("unable to inspect workspace root: {error}"))?
        .next()
        .is_none();
    fs::remove_dir_all(&workspace_root)
        .map_err(|error| format!("unable to remove benchmark workspace: {error}"))?;

    let successful: Vec<&RequestSample> = samples
        .iter()
        .filter(|sample| sample.result.is_ok())
        .collect();
    let wall_samples: Vec<u64> = successful.iter().map(|sample| sample.wall_ns).collect();
    let processing_samples: Vec<u64> = successful
        .iter()
        .filter_map(|sample| sample.processing_ns)
        .collect();
    let wall_stats = latency_stats(&wall_samples);
    let processing_stats = latency_stats(&processing_samples);
    let output_bytes: usize = successful.iter().map(|sample| sample.output_bytes).sum();
    let cpu_user_ns = cpu_after.0.saturating_sub(cpu_before.0);
    let cpu_system_ns = cpu_after.1.saturating_sub(cpu_before.1);
    println!(
        concat!(
            "{{\"schema_version\":1,\"path\":\"rust_admission\",\"fixture\":\"{}\",",
            "\"input_bytes\":{},\"iterations\":{},\"warmup\":{},\"concurrency\":{},",
            "\"queue_capacity\":{},\"queue_depth\":{},\"max_queue_depth\":{},",
            "\"queue_rejection_count\":{},",
            "\"successful_completions\":{},\"failed_completions\":{},",
            "\"cleanup_success_count\":{},\"cleanup_failure_count\":{},",
            "\"output_bytes_total\":{},\"wall_latency_p50_ns\":{},\"wall_latency_p95_ns\":{},",
            "\"processing_latency_p50_ns\":{},\"processing_latency_p95_ns\":{},",
            "\"throughput_ops_per_sec\":{:.6},\"cpu_user_ns\":{},\"cpu_system_ns\":{},",
            "\"peak_rss_kib\":{},\"workspace_cleanup_empty\":{}}}"
        ),
        fixture,
        input_size,
        iterations,
        arguments.warmup,
        arguments.concurrency,
        arguments.queue_capacity,
        metrics.queue_depth,
        metrics.max_queue_depth,
        metrics.rejected_overload,
        successful.len(),
        samples.len() - successful.len(),
        successful.len(),
        if cleanup_empty { 0 } else { 1 },
        output_bytes,
        wall_stats.p50_ns,
        wall_stats.p95_ns,
        processing_stats.p50_ns,
        processing_stats.p95_ns,
        if elapsed.is_zero() {
            0.0
        } else {
            samples.len() as f64 / elapsed.as_secs_f64()
        },
        cpu_user_ns,
        cpu_system_ns,
        peak_rss_kib(),
        cleanup_empty
    );
    Ok(())
}

fn parse_arguments() -> Result<Arguments, String> {
    let mut arguments = Arguments {
        concurrency: 1,
        queue_capacity: 1,
        ..Arguments::default()
    };
    let mut values = std::env::args().skip(1);
    while let Some(argument) = values.next() {
        let value = values
            .next()
            .ok_or_else(|| format!("missing value for {argument}"))?;
        match argument.as_str() {
            "--input" => arguments.input = Some(PathBuf::from(value)),
            "--fixture" => arguments.fixture = Some(value),
            "--iterations" => arguments.iterations = Some(parse_positive("iterations", &value)?),
            "--warmup" => arguments.warmup = parse_nonnegative("warmup", &value)?,
            "--concurrency" => arguments.concurrency = parse_positive("concurrency", &value)?,
            "--queue-capacity" => {
                arguments.queue_capacity = parse_positive("queue-capacity", &value)?
            }
            _ => return Err(format!("unknown argument: {argument}")),
        }
    }
    Ok(arguments)
}

fn parse_positive(name: &str, value: &str) -> Result<usize, String> {
    let parsed = parse_nonnegative(name, value)?;
    if parsed == 0 {
        return Err(format!("{name} must be positive"));
    }
    Ok(parsed)
}

fn parse_nonnegative(name: &str, value: &str) -> Result<usize, String> {
    value
        .parse::<usize>()
        .map_err(|_| format!("{name} must be an unsigned integer"))
}

fn run_measurement(
    controller: Arc<AdmissionController>,
    input: Arc<Vec<u8>>,
    iterations: usize,
    concurrency: usize,
) -> Vec<RequestSample> {
    let mut samples = Vec::with_capacity(iterations);
    while samples.len() < iterations {
        let batch_size = (iterations - samples.len()).min(concurrency);
        if batch_size == 1 {
            samples.push(run_request(Arc::clone(&controller), Arc::clone(&input)));
            continue;
        }
        let barrier = Arc::new(Barrier::new(batch_size));
        let mut workers = Vec::with_capacity(batch_size);
        for _ in 0..batch_size {
            let worker_controller = Arc::clone(&controller);
            let worker_input = Arc::clone(&input);
            let worker_barrier = Arc::clone(&barrier);
            workers.push(std::thread::spawn(move || {
                worker_barrier.wait();
                run_request(worker_controller, worker_input)
            }));
        }
        for worker in workers {
            samples.push(worker.join().unwrap_or_else(|_| RequestSample {
                result: Err(AdmissionError::Unavailable),
                ..RequestSample::default()
            }));
        }
    }
    samples
}

fn run_request(controller: Arc<AdmissionController>, input: Arc<Vec<u8>>) -> RequestSample {
    let started = Instant::now();
    match controller.submit((*input).clone(), None) {
        Ok(request) => match request.wait() {
            Ok(report) => RequestSample {
                wall_ns: duration_ns(started.elapsed()),
                processing_ns: Some(duration_ns(report.duration)),
                output_bytes: report.output.len(),
                result: Ok(()),
            },
            Err(error) => RequestSample {
                wall_ns: duration_ns(started.elapsed()),
                result: Err(error),
                ..RequestSample::default()
            },
        },
        Err(error) => RequestSample {
            wall_ns: duration_ns(started.elapsed()),
            result: Err(error),
            ..RequestSample::default()
        },
    }
}

fn latency_stats(samples: &[u64]) -> LatencyStats {
    if samples.is_empty() {
        return LatencyStats::default();
    }
    let mut sorted = samples.to_vec();
    sorted.sort_unstable();
    let last = sorted.len() - 1;
    LatencyStats {
        p50_ns: sorted[last / 2],
        p95_ns: sorted[(last * 95).div_ceil(100)],
    }
}

fn duration_ns(duration: Duration) -> u64 {
    duration.as_nanos().min(u64::MAX as u128) as u64
}

fn process_cpu_ns() -> (u64, u64) {
    let Some(stat) = fs::read_to_string("/proc/self/stat").ok() else {
        return (0, 0);
    };
    let Some(after_name) = stat.rsplit_once(") ").map(|(_, after_name)| after_name) else {
        return (0, 0);
    };
    let fields: Vec<&str> = after_name.split_whitespace().collect();
    let ticks_per_second = 100u64;
    let user_ticks = fields
        .get(11)
        .and_then(|value| value.parse::<u64>().ok())
        .unwrap_or(0);
    let system_ticks = fields
        .get(12)
        .and_then(|value| value.parse::<u64>().ok())
        .unwrap_or(0);
    (
        user_ticks.saturating_mul(1_000_000_000 / ticks_per_second),
        system_ticks.saturating_mul(1_000_000_000 / ticks_per_second),
    )
}

fn peak_rss_kib() -> u64 {
    fs::read_to_string("/proc/self/status")
        .ok()
        .and_then(|status| {
            status.lines().find_map(|line| {
                line.strip_prefix("VmHWM:")?
                    .split_whitespace()
                    .next()?
                    .parse::<u64>()
                    .ok()
            })
        })
        .unwrap_or(0)
}
