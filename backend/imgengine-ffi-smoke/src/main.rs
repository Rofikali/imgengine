use imgengine::{Engine, EngineOptions, Error};

const PNG: &[u8] = &[
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4,
    0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0xf8, 0xcf, 0xc0, 0xf0,
    0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99, 0x3d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
];

fn encode_input(input: &[u8], repetitions: usize) -> Result<Vec<u8>, String> {
    if !Engine::jpeg_encode_supported().map_err(format_error)? {
        return Err("JPEG encoding capability is unavailable".to_owned());
    }
    let mut engine = Engine::new(EngineOptions::default()).map_err(format_error)?;
    let mut result = Vec::new();
    for _ in 0..repetitions {
        result = engine.encode_jpeg(input).map_err(format_error)?;
        if result.get(..2) != Some(&[0xff, 0xd8]) {
            return Err("engine did not return JPEG output".to_owned());
        }
    }
    Ok(result)
}

fn format_error(error: Error) -> String {
    format!("libimgengine operation failed: {error:?}")
}

fn main() -> Result<(), String> {
    let arguments: Vec<String> = std::env::args().skip(1).collect();
    let (input, output_path, repetitions) = match arguments.as_slice() {
        [] => (PNG.to_vec(), None, 1),
        [input_path, output_path, repetitions] => {
            let input = std::fs::read(input_path)
                .map_err(|error| format!("unable to read {input_path}: {error}"))?;
            let repetitions = repetitions
                .parse::<usize>()
                .map_err(|error| format!("invalid repetition count: {error}"))?;
            if repetitions == 0 {
                return Err("repetition count must be positive".to_owned());
            }
            (input, Some(output_path), repetitions)
        }
        _ => return Err("usage: imgengine-ffi-smoke [INPUT OUTPUT REPETITIONS]".to_owned()),
    };

    let started = std::time::Instant::now();
    let output = encode_input(&input, repetitions)?;
    let elapsed_ns = started.elapsed().as_nanos();
    if let Some(output_path) = output_path {
        std::fs::write(output_path, &output)
            .map_err(|error| format!("unable to write {output_path}: {error}"))?;
    }
    println!(
        "[abi] Rust FFI smoke passed input_bytes={} output_bytes={} repetitions={} elapsed_ns={}",
        input.len(),
        output.len(),
        repetitions,
        elapsed_ns
    );
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::{encode_input, Engine, EngineOptions, Error, PNG};

    #[test]
    fn safe_wrapper_handles_valid_and_malformed_input() {
        let output = encode_input(PNG, 1).expect("valid PNG should encode");
        assert_eq!(&output[..2], &[0xff, 0xd8]);
        let mut engine = Engine::new(EngineOptions::default()).expect("engine should initialize");
        assert_eq!(engine.encode_jpeg(&[0, 0xff, 1]), Err(Error::InvalidImage));
    }
}
