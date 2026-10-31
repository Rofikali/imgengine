#include <stddef.h>
#include <stdint.h>

#include <imgengine/api/v1/imgengine.h>
#include <imgengine/api/v2/sheet.h>

_Static_assert(IMGENGINE_ABI_VERSION == 1u, "ABI v1 constant changed");
_Static_assert(sizeof(imgengine_status_t) == sizeof(uint32_t), "status width changed");
_Static_assert(offsetof(imgengine_engine_options_t, struct_size) == 0,
               "options struct_size offset changed");
_Static_assert(offsetof(imgengine_engine_options_t, worker_count) == 4,
               "options worker_count offset changed");
_Static_assert(offsetof(imgengine_engine_options_t, reserved) == 8,
               "options reserved offset changed");
_Static_assert(sizeof(imgengine_engine_options_t) == 40, "options size changed on LP64");
_Static_assert(offsetof(imgengine_output_t, data) == 0, "output data offset changed");
_Static_assert(offsetof(imgengine_output_t, size) == 8, "output size offset changed on LP64");
_Static_assert(sizeof(imgengine_output_t) == 16, "output size changed on LP64");
_Static_assert(IMGENGINE_SHEET_RENDER_V1_VERSION == 1u, "sheet version changed");
_Static_assert(IMGENGINE_SHEET_CROP_CENTERED_FILL == 1u, "sheet crop mode changed");
_Static_assert(IMGENGINE_SHEET_V1_MAX_CANVAS_PIXELS == 10000000u, "sheet pixel limit changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, struct_size) == 0, "sheet struct_size offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, version) == 4, "sheet version offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, input) == 8, "sheet input offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, input_size) == 16, "sheet input_size offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, canvas_width) == 24, "sheet canvas_width offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, background_r) == 32, "sheet background offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, crop_mode) == 36, "sheet crop_mode offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, reserved1) == 40, "sheet reserved1 offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, placements) == 56, "sheet placements offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, placement_count) == 64, "sheet placement_count offset changed");
_Static_assert(offsetof(imgengine_sheet_request_v1_t, expected_count) == 68, "sheet expected_count offset changed");
_Static_assert(sizeof(imgengine_sheet_request_v1_t) == 72, "sheet request size changed on LP64");

int main(void) { return imgengine_abi_version() == IMGENGINE_ABI_VERSION ? 0 : 1; }
