#include <stddef.h>
#include <stdint.h>

#include <imgengine/api/v1/imgengine.h>

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

int main(void) { return imgengine_abi_version() == IMGENGINE_ABI_VERSION ? 0 : 1; }
