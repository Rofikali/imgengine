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

impl Default for Output {
    fn default() -> Self {
        Self {
            data: std::ptr::null_mut(),
            size: 0,
        }
    }
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
    pub fn imgengine_capability_supported(
        identifier: *const c_char,
        out_supported: *mut u32,
    ) -> u32;
    pub fn imgengine_process_encoded_image_to_jpeg(
        engine: *mut Engine,
        input: *const u8,
        input_size: usize,
        output: *mut Output,
    ) -> u32;
    pub fn imgengine_output_release(output: *mut Output);
}

pub fn abi_version() -> u32 {
    unsafe { imgengine_abi_version() }
}

pub fn engine_create(options: &EngineOptions) -> (u32, *mut Engine) {
    let mut engine = std::ptr::null_mut();
    let status = unsafe { imgengine_engine_create(options, &mut engine) };
    (status, engine)
}

pub fn engine_destroy(engine: *mut Engine) {
    unsafe { imgengine_engine_destroy(engine) };
}

pub fn capability_supported(identifier: &[u8]) -> (u32, u32) {
    debug_assert_eq!(identifier.last(), Some(&0));
    let mut supported = 0;
    let status =
        unsafe { imgengine_capability_supported(identifier.as_ptr().cast(), &mut supported) };
    (status, supported)
}

pub fn process_encoded_image_to_jpeg(
    engine: *mut Engine,
    input: &[u8],
    output: &mut Output,
) -> u32 {
    unsafe { imgengine_process_encoded_image_to_jpeg(engine, input.as_ptr(), input.len(), output) }
}

pub fn output_copy(output: &Output) -> Option<Vec<u8>> {
    if output.data.is_null() || output.size == 0 {
        return None;
    }
    Some(unsafe { std::slice::from_raw_parts(output.data, output.size) }.to_vec())
}

pub fn output_release(output: &mut Output) {
    unsafe { imgengine_output_release(output) };
}
