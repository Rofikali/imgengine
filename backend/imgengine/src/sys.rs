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

#[repr(C)]
pub struct LayoutRectV2 {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
}

#[repr(C)]
pub struct LayoutRequestV2 {
    pub struct_size: u32,
    pub version: u32,
    pub input: *const u8,
    pub input_size: usize,
    pub canvas_width: u32,
    pub canvas_height: u32,
    pub background_r: u8,
    pub background_g: u8,
    pub background_b: u8,
    pub reserved0: u8,
    pub placements: *const LayoutRectV2,
    pub placement_count: u32,
    pub expected_count: u32,
}

#[repr(C)]
pub struct SheetRequestV1 {
    pub struct_size: u32,
    pub version: u32,
    pub input: *const u8,
    pub input_size: usize,
    pub canvas_width: u32,
    pub canvas_height: u32,
    pub background_r: u8,
    pub background_g: u8,
    pub background_b: u8,
    pub reserved0: u8,
    pub crop_mode: u32,
    pub reserved1: [u32; 4],
    pub placements: *const LayoutRectV2,
    pub placement_count: u32,
    pub expected_count: u32,
}

#[cfg(target_pointer_width = "64")]
const _: [(); 72] = [(); std::mem::size_of::<SheetRequestV1>()];

#[repr(C)]
pub struct LayoutOutputV2 {
    pub data: *mut u8,
    pub size: usize,
    pub canvas_width: u32,
    pub canvas_height: u32,
    pub expected_count: u32,
    pub planned_count: u32,
    pub executed_count: u32,
}

impl Default for LayoutOutputV2 {
    fn default() -> Self {
        Self {
            data: std::ptr::null_mut(),
            size: 0,
            canvas_width: 0,
            canvas_height: 0,
            expected_count: 0,
            planned_count: 0,
            executed_count: 0,
        }
    }
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
    pub fn imgengine_layout_render_v2(
        engine: *mut Engine,
        request: *const LayoutRequestV2,
        output: *mut LayoutOutputV2,
    ) -> u32;
    pub fn imgengine_layout_render_v2_bounded(
        engine: *mut Engine,
        request: *const LayoutRequestV2,
        max_output_bytes: usize,
        output: *mut LayoutOutputV2,
    ) -> u32;
    pub fn imgengine_layout_output_release(output: *mut LayoutOutputV2);
    pub fn imgengine_sheet_render_v1_bounded(
        engine: *mut Engine,
        request: *const SheetRequestV1,
        max_output_bytes: usize,
        output: *mut LayoutOutputV2,
    ) -> u32;
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

pub fn layout_render_v2(
    engine: *mut Engine,
    request: &LayoutRequestV2,
    output: &mut LayoutOutputV2,
) -> u32 {
    unsafe { imgengine_layout_render_v2(engine, request, output) }
}

pub fn layout_render_v2_bounded(
    engine: *mut Engine,
    request: &LayoutRequestV2,
    max_output_bytes: usize,
    output: &mut LayoutOutputV2,
) -> u32 {
    unsafe { imgengine_layout_render_v2_bounded(engine, request, max_output_bytes, output) }
}

pub fn sheet_render_v1_bounded(
    engine: *mut Engine,
    request: &SheetRequestV1,
    max_output_bytes: usize,
    output: &mut LayoutOutputV2,
) -> u32 {
    unsafe { imgengine_sheet_render_v1_bounded(engine, request, max_output_bytes, output) }
}

pub fn layout_output_copy(output: &LayoutOutputV2) -> Option<Vec<u8>> {
    if output.data.is_null() || output.size == 0 {
        return None;
    }
    Some(unsafe { std::slice::from_raw_parts(output.data, output.size) }.to_vec())
}

pub fn layout_output_release(output: &mut LayoutOutputV2) {
    unsafe { imgengine_layout_output_release(output) };
}
