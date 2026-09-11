use std::ptr::NonNull;

mod ffi {
    use std::ffi::c_char;

    pub const ABI_VERSION: u32 = 1;
    pub const STATUS_OK: u32 = 0;

    #[repr(C)]
    pub struct EngineOptions {
        pub struct_size: u32,
        pub worker_count: u32,
        pub reserved: [u64; 4],
    }

    #[repr(C)]
    pub struct Output {
        pub data: *mut u8,
        pub size: usize,
    }

    pub enum Engine {}

    #[link(name = "imgengine")]
    extern "C" {
        pub fn imgengine_abi_version() -> u32;
        pub fn imgengine_engine_create(
            options: *const EngineOptions,
            out_engine: *mut *mut Engine,
        ) -> u32;
        pub fn imgengine_engine_destroy(engine: *mut Engine);
        pub fn imgengine_capability_supported(identifier: *const c_char, out_supported: *mut u32) -> u32;
        pub fn imgengine_process_encoded_image_to_jpeg(
            engine: *mut Engine,
            input: *const u8,
            input_size: usize,
            output: *mut Output,
        ) -> u32;
        pub fn imgengine_output_release(output: *mut Output);
    }
}

struct Engine(NonNull<ffi::Engine>);

impl Engine {
    fn create() -> Result<Self, u32> {
        let options = ffi::EngineOptions {
            struct_size: std::mem::size_of::<ffi::EngineOptions>() as u32,
            worker_count: 1,
            reserved: [0; 4],
        };
        let mut raw = std::ptr::null_mut();
        let status = unsafe { ffi::imgengine_engine_create(&options, &mut raw) };
        if status != ffi::STATUS_OK {
            return Err(status);
        }
        NonNull::new(raw).map(Self).ok_or(u32::MAX)
    }

    fn encode_jpeg(&mut self, input: &[u8]) -> Result<Output, u32> {
        let mut output = ffi::Output {
            data: std::ptr::null_mut(),
            size: 0,
        };
        let status = unsafe {
            ffi::imgengine_process_encoded_image_to_jpeg(
                self.0.as_ptr(),
                input.as_ptr(),
                input.len(),
                &mut output,
            )
        };
        if status != ffi::STATUS_OK {
            return Err(status);
        }
        Ok(Output(output))
    }
}

impl Drop for Engine {
    fn drop(&mut self) {
        unsafe { ffi::imgengine_engine_destroy(self.0.as_ptr()) };
    }
}

struct Output(ffi::Output);

impl Output {
    fn as_bytes(&self) -> &[u8] {
        unsafe { std::slice::from_raw_parts(self.0.data, self.0.size) }
    }
}

impl Drop for Output {
    fn drop(&mut self) {
        unsafe { ffi::imgengine_output_release(&mut self.0) };
    }
}

const PNG: &[u8] = &[
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48,
    0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00,
    0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63,
    0xf8, 0xcf, 0xc0, 0xf0, 0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99, 0x3d, 0x1d, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
];

fn main() -> Result<(), String> {
    if unsafe { ffi::imgengine_abi_version() } != ffi::ABI_VERSION {
        return Err("unexpected ABI version".to_owned());
    }
    let mut supported = 0;
    let capability = b"image.jpeg_encode\0";
    let status = unsafe {
        ffi::imgengine_capability_supported(capability.as_ptr().cast(), &mut supported)
    };
    if status != ffi::STATUS_OK || supported != 1 {
        return Err("capability discovery failed".to_owned());
    }

    let mut engine = Engine::create().map_err(|status| format!("engine creation failed: {status}"))?;
    let output = engine
        .encode_jpeg(PNG)
        .map_err(|status| format!("JPEG encoding failed: {status}"))?;
    if output.as_bytes().get(..2) != Some(&[0xff, 0xd8]) {
        return Err("engine did not return JPEG output".to_owned());
    }
    println!("[abi] Rust FFI smoke passed");
    Ok(())
}
