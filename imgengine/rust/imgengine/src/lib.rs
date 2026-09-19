//! Safe ownership boundary for the stable `libimgengine` ABI v1.
//!
//! `Engine` deliberately is neither `Send` nor `Sync`: ABI v1 has one active
//! engine per process and serializes operations internally. It also has no
//! mid-operation cancellation; callers must enforce request deadlines outside
//! this crate.

mod sys;

use std::marker::PhantomData;
use std::ptr::NonNull;
use std::rc::Rc;

/// The ABI version required by this crate.
pub const ABI_VERSION: u32 = sys::ABI_VERSION;

/// A stable `libimgengine` failure category.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Error {
    InvalidArgument,
    InvalidImage,
    ResourceLimit,
    OutOfMemory,
    Unsupported,
    Unavailable,
    Internal,
    AbiMismatch { expected: u32, actual: u32 },
    InvalidNativeHandle,
    UnexpectedStatus(u32),
}

impl Error {
    fn from_status(status: u32) -> Self {
        match status {
            1 => Self::InvalidArgument,
            2 => Self::InvalidImage,
            3 => Self::ResourceLimit,
            4 => Self::OutOfMemory,
            5 => Self::Unsupported,
            6 => Self::Unavailable,
            7 => Self::Internal,
            other => Self::UnexpectedStatus(other),
        }
    }
}

/// Construction options for the single ABI-v1 process-scoped engine.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct EngineOptions {
    worker_count: u32,
}

impl EngineOptions {
    /// Creates options with the requested native worker count.
    pub fn new(worker_count: u32) -> Result<Self, Error> {
        if !(1..=64).contains(&worker_count) {
            return Err(Error::InvalidArgument);
        }
        Ok(Self { worker_count })
    }
}

impl Default for EngineOptions {
    fn default() -> Self {
        Self { worker_count: 1 }
    }
}

/// Process-scoped image engine.
///
/// The `Rc` marker deliberately prevents `Send` and `Sync` implementations.
pub struct Engine {
    raw: NonNull<sys::Engine>,
    _not_send_or_sync: PhantomData<Rc<()>>,
}

impl Engine {
    /// Creates the one ABI-v1 engine allowed in this process.
    pub fn new(options: EngineOptions) -> Result<Self, Error> {
        let actual = sys::abi_version();
        if actual != ABI_VERSION {
            return Err(Error::AbiMismatch {
                expected: ABI_VERSION,
                actual,
            });
        }

        let native_options = sys::EngineOptions {
            struct_size: std::mem::size_of::<sys::EngineOptions>() as u32,
            worker_count: options.worker_count,
            reserved: [0; 4],
        };
        let (status, raw) = sys::engine_create(&native_options);
        if status != sys::STATUS_OK {
            return Err(Error::from_status(status));
        }
        let raw = NonNull::new(raw).ok_or(Error::InvalidNativeHandle)?;
        Ok(Self {
            raw,
            _not_send_or_sync: PhantomData,
        })
    }

    /// Returns whether `image.jpeg_encode` is available at runtime.
    pub fn jpeg_encode_supported() -> Result<bool, Error> {
        let identifier = b"image.jpeg_encode\0";
        let (status, supported) = sys::capability_supported(identifier);
        if status != sys::STATUS_OK {
            return Err(Error::from_status(status));
        }
        Ok(supported != 0)
    }

    /// Encodes a JPEG or PNG byte slice to an owned JPEG byte vector.
    ///
    /// The input is borrowed only for this call. The returned vector contains
    /// a Rust-owned copy, so it remains valid after the native output is
    /// released and after this engine is dropped.
    pub fn encode_jpeg(&mut self, input: &[u8]) -> Result<Vec<u8>, Error> {
        let mut output = NativeOutput::default();
        let status = sys::process_encoded_image_to_jpeg(self.raw.as_ptr(), input, &mut output.raw);
        if status != sys::STATUS_OK {
            return Err(Error::from_status(status));
        }
        output.copy_to_vec()
    }
}

impl Drop for Engine {
    fn drop(&mut self) {
        sys::engine_destroy(self.raw.as_ptr());
    }
}

#[derive(Default)]
struct NativeOutput {
    raw: sys::Output,
}

impl NativeOutput {
    fn copy_to_vec(&self) -> Result<Vec<u8>, Error> {
        sys::output_copy(&self.raw).ok_or(Error::Internal)
    }
}

impl Drop for NativeOutput {
    fn drop(&mut self) {
        sys::output_release(&mut self.raw);
    }
}

#[cfg(test)]
mod tests {
    use super::{EngineOptions, Error};

    #[test]
    fn rejects_invalid_worker_counts_without_native_state() {
        assert_eq!(EngineOptions::new(0), Err(Error::InvalidArgument));
        assert_eq!(EngineOptions::new(65), Err(Error::InvalidArgument));
    }
}
