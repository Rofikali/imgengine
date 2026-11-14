use imgengine_api::{ApiConfig, ApiRuntime};
use std::env;
use std::io;
use std::path::PathBuf;
use tokio::net::TcpListener;

#[cfg(unix)]
async fn wait_for_shutdown_signal() -> io::Result<()> {
    use tokio::signal::unix::{signal, SignalKind};

    let mut terminate = signal(SignalKind::terminate())?;
    tokio::select! {
        result = tokio::signal::ctrl_c() => result,
        _ = terminate.recv() => Ok(()),
    }
}

#[cfg(not(unix))]
async fn wait_for_shutdown_signal() -> io::Result<()> {
    tokio::signal::ctrl_c().await
}

/// Process entry point for the existing P5/P6 runtime.
///
/// Configuration deliberately stays narrow: authentication is supplied by
/// `IMGENGINE_API_KEYS`, while this binary owns listener binding and forwards
/// SIGINT or Linux SIGTERM into the already-defined graceful runtime drain. It
/// adds no route, policy, or native-execution behavior.
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let bind_address = env::var("IMGENGINE_API_BIND")
        .unwrap_or_else(|_| "127.0.0.1:8000".to_owned());
    let workspace_root = env::var_os("IMGENGINE_WORKSPACE_ROOT")
        .map(PathBuf::from)
        .unwrap_or_else(|| PathBuf::from("/tmp/imgengine-workspaces"));

    let config = ApiConfig::from_env(workspace_root)
        .map_err(|error| io::Error::new(io::ErrorKind::InvalidInput, format!("API configuration: {error:?}")))?;
    let runtime = ApiRuntime::start(config)
        .map_err(|error| io::Error::new(io::ErrorKind::Other, format!("API startup: {error:?}")))?;
    let listener = TcpListener::bind(&bind_address).await?;
    let server = runtime.serve(listener);

    wait_for_shutdown_signal().await?;
    server
        .shutdown()
        .await
        .map_err(|error| io::Error::new(io::ErrorKind::Other, format!("API shutdown: {error:?}")))?;
    Ok(())
}
