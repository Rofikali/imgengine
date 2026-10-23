//! Safe ownership boundary for the stable `libimgengine` ABI v1 and additive
//! `IMGENGINE_2.0` layout capability.
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

/// One validated placement in a server-generated render plan.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct LayoutRect {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
}

/// Typed input for the additive native FIT-only layout capability.
///
/// The input image is supplied separately and remains borrowed only for the
/// synchronous native call. Placement metadata is copied before the native
/// scheduler accepts work.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct LayoutRequest {
    pub canvas_width: u32,
    pub canvas_height: u32,
    pub background: [u8; 3],
    pub placements: Vec<LayoutRect>,
    pub expected_count: u32,
}

/// Rust-owned layout output with native plan accounting preserved for audit.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct LayoutOutput {
    pub jpeg: Vec<u8>,
    pub canvas_width: u32,
    pub canvas_height: u32,
    pub expected_count: u32,
    pub planned_count: u32,
    pub executed_count: u32,
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

    /// Renders a validated FIT-only layout and returns Rust-owned JPEG bytes.
    ///
    /// This calls the additive `IMGENGINE_2.0` capability through the same
    /// ABI-v1 opaque engine; it does not create a second scheduler or engine.
    pub fn render_layout_jpeg(
        &mut self,
        input: &[u8],
        request: &LayoutRequest,
    ) -> Result<LayoutOutput, Error> {
        if input.is_empty()
            || request.canvas_width == 0
            || request.canvas_height == 0
            || request.placements.is_empty()
        {
            return Err(Error::InvalidArgument);
        }
        let placement_count =
            u32::try_from(request.placements.len()).map_err(|_| Error::ResourceLimit)?;
        if request.expected_count != placement_count {
            return Err(Error::InvalidArgument);
        }
        let native_placements: Vec<sys::LayoutRectV2> = request
            .placements
            .iter()
            .map(|rect| sys::LayoutRectV2 {
                x: rect.x,
                y: rect.y,
                width: rect.width,
                height: rect.height,
            })
            .collect();
        let native_request = sys::LayoutRequestV2 {
            struct_size: std::mem::size_of::<sys::LayoutRequestV2>() as u32,
            version: 1,
            input: input.as_ptr(),
            input_size: input.len(),
            canvas_width: request.canvas_width,
            canvas_height: request.canvas_height,
            background_r: request.background[0],
            background_g: request.background[1],
            background_b: request.background[2],
            reserved0: 0,
            placements: native_placements.as_ptr(),
            placement_count,
            expected_count: request.expected_count,
        };
        let mut output = NativeLayoutOutput::default();
        let status = sys::layout_render_v2(self.raw.as_ptr(), &native_request, &mut output.raw);
        if status != sys::STATUS_OK {
            return Err(Error::from_status(status));
        }
        output.copy_to_owned()
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

#[derive(Default)]
struct NativeLayoutOutput {
    raw: sys::LayoutOutputV2,
}

impl NativeLayoutOutput {
    fn copy_to_owned(&self) -> Result<LayoutOutput, Error> {
        let jpeg = sys::layout_output_copy(&self.raw).ok_or(Error::Internal)?;
        if self.raw.expected_count == 0
            || self.raw.expected_count != self.raw.planned_count
            || self.raw.planned_count != self.raw.executed_count
        {
            return Err(Error::Internal);
        }
        Ok(LayoutOutput {
            jpeg,
            canvas_width: self.raw.canvas_width,
            canvas_height: self.raw.canvas_height,
            expected_count: self.raw.expected_count,
            planned_count: self.raw.planned_count,
            executed_count: self.raw.executed_count,
        })
    }
}

impl Drop for NativeLayoutOutput {
    fn drop(&mut self) {
        sys::layout_output_release(&mut self.raw);
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
