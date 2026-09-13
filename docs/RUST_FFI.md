# Rust FFI Boundary

**Status:** Implemented and validated in the Ubuntu 24.04 Docker verifier on
2026-09-13. This is Priority 3's safe-wrapper foundation, not a scheduler or
backend migration.

## Scope

`imgengine/rust/imgengine` is the only Rust crate that declares or invokes the
`libimgengine` ABI v1 symbols. Its private `sys` module contains every
`extern "C"` declaration, raw pointer operation, and `unsafe` block. Consumers
use only the safe public API:

```rust
let mut engine = imgengine::Engine::new(imgengine::EngineOptions::default())?;
let jpeg: Vec<u8> = engine.encode_jpeg(&input)?;
```

The returned `Vec<u8>` is a Rust-owned copy. The native allocation is released
by the private RAII guard before the safe call returns, so callers cannot leak,
double-free, or retain a native output pointer.

## Contract Mapping

- `Engine::new` verifies ABI major version `1` and maps stable C status values
  to `imgengine::Error`.
- `EngineOptions::new` rejects worker counts outside the C ABI's `1..=64`
  range before any native state is created.
- `Engine::encode_jpeg` borrows input only for the call and returns owned JPEG
  bytes on success.
- Malformed input returns `Error::InvalidImage`; no native error text, path, or
  decoder diagnostic crosses the safe API.
- `Engine` intentionally carries an `Rc` marker and is neither `Send` nor
  `Sync`. This preserves ABI v1's one-engine process scope and serialized
  operation contract.
- ABI v1 has no mid-operation cancellation. Timeouts, client disconnects, and
  cancellation policies belong to the future Rust orchestration layer rather
  than this wrapper.

## Verification

The existing Ubuntu 24.04 verifier runs the Rust consumer against the built
shared library and now executes its Rust tests as well. Those tests cover a
valid PNG-to-JPEG conversion and typed malformed-input handling. The full
native verifier continues to cover public C consumers, exact exports, ASAN,
UBSAN, sandbox lifecycle, regressions, and fuzzing.

Run the focused Rust FFI tests after producing a normal native build:

```bash
export LD_LIBRARY_PATH="$PWD/build-linux-verify/normal"
export RUSTFLAGS="-L native=$PWD/build-linux-verify/normal"
cargo test --manifest-path rust/imgengine-ffi-smoke/Cargo.toml
```

No scheduler, arena, slab, ownership migration, HTTP server, or Axum code is
included in this phase.
