use imgengine_supervisor::{Supervisor, SupervisorOptions};
use std::path::PathBuf;
use std::time::Duration;

fn main() -> Result<(), String> {
    let arguments: Vec<String> = std::env::args().skip(1).collect();
    let [input_path, output_path, workspace_root, remaining @ ..] = arguments.as_slice() else {
        return Err(
            "usage: imgengine-supervisor INPUT OUTPUT WORKSPACE_ROOT [DEADLINE_MS]".to_owned(),
        );
    };
    let deadline = match remaining {
        [] => None,
        [milliseconds] => Some(Duration::from_millis(
            milliseconds
                .parse::<u64>()
                .map_err(|error| format!("invalid deadline: {error}"))?,
        )),
        _ => {
            return Err(
                "usage: imgengine-supervisor INPUT OUTPUT WORKSPACE_ROOT [DEADLINE_MS]".to_owned(),
            )
        }
    };

    let input =
        std::fs::read(input_path).map_err(|error| format!("unable to read input: {error}"))?;
    let mut supervisor = Supervisor::new(SupervisorOptions::new(PathBuf::from(workspace_root)))
        .map_err(|error| format!("supervisor initialization failed: {error}"))?;
    let report = supervisor
        .process(&input, deadline)
        .map_err(|error| format!("image processing failed: {error}"))?;
    std::fs::write(output_path, &report.output)
        .map_err(|error| format!("unable to write output: {error}"))?;

    let event = report.event();
    println!(
        "{{\"event\":\"image.jpeg_encode.completed\",\"result\":\"{}\",\"duration_ms\":{},\"input_size_bucket\":\"{}\",\"output_size_bucket\":\"{}\"}}",
        event.result.as_str(),
        event.duration.as_millis(),
        event.input_size_bucket,
        event.output_size_bucket.expect("success events have output buckets"),
    );
    Ok(())
}
