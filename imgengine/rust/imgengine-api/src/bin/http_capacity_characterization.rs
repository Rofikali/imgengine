//! Reproducible P6.5 loopback characterization harness.
//!
//! This is deliberately a benchmark executable, not a production server. It
//! exercises the public API runtime over TCP without changing its limits.

use imgengine_api::{ApiConfig, ApiRuntime};
use std::collections::BTreeMap;
use std::env;
use std::fs;
use std::io;
use std::path::PathBuf;
use std::time::Instant;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};

const API_KEY: &str = "p65-characterization-key";
const WARMUP_REQUESTS: usize = 2;
const SEQUENTIAL_REQUESTS: usize = 8;
const CONCURRENT_REQUESTS_PER_LEVEL: usize = 12;

struct ResultSample {
    status: u16,
    latency_us: u128,
    output_bytes: usize,
}

fn percentile(sorted: &[u128], numerator: usize, denominator: usize) -> u128 {
    if sorted.is_empty() {
        return 0;
    }
    let index = (sorted.len() * numerator)
        .div_ceil(denominator)
        .saturating_sub(1);
    sorted[index]
}

fn summary(samples: &[ResultSample]) -> String {
    let mut latency: Vec<_> = samples.iter().map(|sample| sample.latency_us).collect();
    latency.sort_unstable();
    let mut statuses = BTreeMap::<u16, usize>::new();
    let mut output_bytes = 0usize;
    for sample in samples {
        *statuses.entry(sample.status).or_default() += 1;
        output_bytes += sample.output_bytes;
    }
    let elapsed_us: u128 = latency.iter().sum();
    format!(
        "{{\"requests\":{},\"status_counts\":{},\"latency_us\":{{\"p50\":{},\"p95\":{},\"p99\":{},\"mean\":{}}},\"output_bytes_total\":{},\"serial_latency_sum_us\":{}}}",
        samples.len(),
        json_statuses(&statuses),
        percentile(&latency, 50, 100),
        percentile(&latency, 95, 100),
        percentile(&latency, 99, 100),
        if latency.is_empty() { 0 } else { elapsed_us / latency.len() as u128 },
        output_bytes,
        elapsed_us,
    )
}

fn json_statuses(statuses: &BTreeMap<u16, usize>) -> String {
    statuses
        .iter()
        .map(|(status, count)| format!("\"{status}\":{count}"))
        .collect::<Vec<_>>()
        .join(",")
        .pipe(|items| format!("{{{items}}}"))
}

trait Pipe: Sized {
    fn pipe<T>(self, transform: impl FnOnce(Self) -> T) -> T {
        transform(self)
    }
}
impl<T> Pipe for T {}

fn multipart(content_type: &str, bytes: &[u8]) -> (String, Vec<u8>) {
    let boundary = "p65-characterization-boundary";
    let mut body = format!(
        "--{boundary}\r\nContent-Disposition: form-data; name=\"file\"; filename=\"fixture\"\r\nContent-Type: {content_type}\r\n\r\n"
    )
    .into_bytes();
    body.extend_from_slice(bytes);
    body.extend_from_slice(format!("\r\n--{boundary}--\r\n").as_bytes());
    (format!("multipart/form-data; boundary={boundary}"), body)
}

async fn request(
    address: std::net::SocketAddr,
    content_type: &str,
    bytes: &[u8],
) -> io::Result<ResultSample> {
    let (multipart_type, body) = multipart(content_type, bytes);
    let start = Instant::now();
    let mut stream = TcpStream::connect(address).await?;
    let header = format!(
        "POST /api/v1/render HTTP/1.1\r\nHost: loopback\r\nX-API-Key: {API_KEY}\r\nContent-Type: {multipart_type}\r\nContent-Length: {}\r\nConnection: close\r\n\r\n",
        body.len()
    );
    stream.write_all(header.as_bytes()).await?;
    stream.write_all(&body).await?;
    // A TCP half-close before reading the response is a real transport
    // abandonment signal in this adapter. Keep the connection writable until
    // the response is observed so the normal-capacity workload is not a P6.2
    // disconnect workload by accident.
    stream.flush().await?;
    let mut response = Vec::new();
    stream.read_to_end(&mut response).await?;
    let header_end = response
        .windows(4)
        .position(|window| window == b"\r\n\r\n")
        .map(|offset| offset + 4)
        .unwrap_or(response.len());
    let status = std::str::from_utf8(&response[..header_end])
        .ok()
        .and_then(|value| value.split_whitespace().nth(1))
        .and_then(|value| value.parse().ok())
        .unwrap_or(0);
    let output_bytes = response.len().saturating_sub(header_end);
    Ok(ResultSample {
        status,
        latency_us: start.elapsed().as_micros(),
        output_bytes,
    })
}

async fn sequential(
    address: std::net::SocketAddr,
    content_type: &str,
    bytes: &[u8],
) -> io::Result<Vec<ResultSample>> {
    for _ in 0..WARMUP_REQUESTS {
        let _ = request(address, content_type, bytes).await?;
    }
    let mut results = Vec::with_capacity(SEQUENTIAL_REQUESTS);
    for _ in 0..SEQUENTIAL_REQUESTS {
        results.push(request(address, content_type, bytes).await?);
    }
    Ok(results)
}

async fn concurrent(
    address: std::net::SocketAddr,
    content_type: &str,
    bytes: &[u8],
    level: usize,
) -> io::Result<(Vec<ResultSample>, u128)> {
    let started = Instant::now();
    let mut all = Vec::new();
    for _ in 0..CONCURRENT_REQUESTS_PER_LEVEL {
        let mut group = Vec::with_capacity(level);
        for _ in 0..level {
            let content_type = content_type.to_owned();
            let bytes = bytes.to_vec();
            group.push(tokio::spawn(async move {
                request(address, &content_type, &bytes).await
            }));
        }
        for result in group {
            all.push(result.await.expect("benchmark task join")?);
        }
    }
    Ok((all, started.elapsed().as_micros()))
}

fn status_file_metric() -> String {
    let status = fs::read_to_string("/proc/self/status").unwrap_or_default();
    let vm_hwm = status
        .lines()
        .find(|line| line.starts_with("VmHWM:"))
        .unwrap_or("VmHWM: unavailable");
    let vm_rss = status
        .lines()
        .find(|line| line.starts_with("VmRSS:"))
        .unwrap_or("VmRSS: unavailable");
    format!("\"{}\",\"{}\"", vm_hwm.trim(), vm_rss.trim())
}

#[tokio::main(flavor = "multi_thread", worker_threads = 2)]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut args = env::args().skip(1);
    let workspace = PathBuf::from(
        args.next()
            .ok_or("usage: <workspace> <small-jpeg> <small-png> <large-jpeg> <report>")?,
    );
    let small_jpeg = fs::read(args.next().ok_or("missing small JPEG")?)?;
    let small_png = fs::read(args.next().ok_or("missing small PNG")?)?;
    let large_jpeg = fs::read(args.next().ok_or("missing large JPEG")?)?;
    let report = PathBuf::from(args.next().ok_or("missing report path")?);
    fs::create_dir_all(&workspace)?;

    let config = ApiConfig::new(API_KEY, workspace.clone()).map_err(|error| {
        io::Error::new(
            io::ErrorKind::Other,
            format!("API configuration: {error:?}"),
        )
    })?;
    let runtime = ApiRuntime::start(config)
        .map_err(|error| io::Error::new(io::ErrorKind::Other, format!("API start: {error:?}")))?;
    let listener = TcpListener::bind(("127.0.0.1", 0)).await?;
    let address = listener.local_addr()?;
    let server = runtime.clone().serve(listener);
    let started = Instant::now();

    let small_jpeg_samples = sequential(address, "image/jpeg", &small_jpeg).await?;
    let small_png_samples = sequential(address, "image/png", &small_png).await?;
    let large_jpeg_samples = sequential(address, "image/jpeg", &large_jpeg).await?;
    let malformed_samples = sequential(address, "image/jpeg", b"not-an-image").await?;
    let unsupported_samples = sequential(address, "application/octet-stream", &small_jpeg).await?;
    let healthy_after_malformed = request(address, "image/jpeg", &small_jpeg).await?;

    let mut concurrent_results = Vec::new();
    for level in [1usize, 2, 3, 4] {
        let (samples, elapsed_us) = concurrent(address, "image/jpeg", &large_jpeg, level).await?;
        concurrent_results.push(format!("\"{level}\":{{\"wall_us\":{elapsed_us},\"throughput_attempts_per_second\":{},\"summary\":{}}}", (samples.len() as f64 * 1_000_000.0 / elapsed_us.max(1) as f64), summary(&samples)));
    }
    let admission = runtime.admission_metrics();
    let lifecycle = runtime.lifecycle_metrics();
    server.shutdown().await.map_err(|error| {
        io::Error::new(io::ErrorKind::Other, format!("API shutdown: {error:?}"))
    })?;
    let clean = fs::read_dir(&workspace)?.next().is_none();
    let report_text = format!(
        "{{\n  \"label\": \"CHARACTERIZATION — Docker/WSL2 loopback only; not production capacity\",\n  \"configuration\": {{\"body_limit_bytes\":1048576,\"queue_capacity\":1,\"rust_workers\":1,\"execution_deadline_seconds\":10}},\n  \"fixtures\": {{\"small_jpeg_bytes\":{},\"small_png_bytes\":{},\"large_jpeg_bytes\":{}}},\n  \"sequential\": {{\"small_jpeg\":{},\"small_png\":{},\"large_jpeg\":{}}},\n  \"malformed\": {{\"invalid_jpeg\":{},\"unsupported_media_type\":{},\"healthy_after_status\":{}}},\n  \"concurrent\": {{{}}},\n  \"admission_metrics\": {{\"accepted\":{},\"completed\":{},\"succeeded\":{},\"failed\":{},\"rejected_overload\":{},\"rejected_resource_limit\":{},\"deadline_exceeded\":{},\"max_queue_depth\":{}}},\n  \"lifecycle_response_abandoned\":{},\n  \"workspace_clean_after_shutdown\":{},\n  \"process_memory\": [ {} ],\n  \"elapsed_us\":{}\n}}\n",
        small_jpeg.len(), small_png.len(), large_jpeg.len(),
        summary(&small_jpeg_samples), summary(&small_png_samples), summary(&large_jpeg_samples),
        summary(&malformed_samples), summary(&unsupported_samples), healthy_after_malformed.status,
        concurrent_results.join(","),
        admission.accepted, admission.completed, admission.succeeded, admission.failed,
        admission.rejected_overload, admission.rejected_resource_limit, admission.deadline_exceeded, admission.max_queue_depth,
        lifecycle.response_abandoned, clean, status_file_metric(), started.elapsed().as_micros(),
    );
    fs::write(report, report_text)?;
    fs::remove_dir_all(workspace)?;
    Ok(())
}
