# FILE 1: bindings/python/imgengine.py
# CFFI binding to libimgengine.so

import cffi
import ctypes
import os
from pathlib import Path
_ffi = cffi.FFI()
_ffi.cdef("""
typedef struct img_engine img_engine_t;
typedef enum {
    IMG_SUCCESS      = 0,
    IMG_ERR_NOMEM    = 1,
    IMG_ERR_FORMAT   = 2,
    IMG_ERR_SECURITY = 3,
    IMG_ERR_IO       = 4,
    IMG_ERR_HW_UNSUP = 5,
    IMG_ERR_INTERNAL = 6,
} img_result_t;

img_engine_t *img_api_init(uint32_t workers);
void          img_api_shutdown(img_engine_t *engine);

img_result_t img_api_process_raw(
    img_engine_t  *engine,
    uint8_t       *input,
    size_t         input_size,
    uint8_t      **output,
    size_t        *output_size
);

void img_encoded_free(uint8_t *ptr);
""")

One engine per process — share across FastAPI workers.
The underlying C engine is thread-safe for process_raw().
"""

def __init__(self, workers: int = 4):
    self._engine = _lib.img_api_init(workers)
    if self._engine == _ffi.NULL:
        raise RuntimeError("img_api_init() returned NULL — engine init failed")

def process_raw(self, data: bytes) -> bytes:
    """
    Decode → process → encode.

    Args:
        data: JPEG or PNG bytes (input image)

    Returns:
        JPEG bytes (output image)

    Raises:
        ValueError: IMG_ERR_FORMAT or IMG_ERR_SECURITY
        MemoryError: IMG_ERR_NOMEM
        RuntimeError: other engine errors
    """
    # Pin input buffer — CFFI keeps it alive for the C call
    input_buf = _ffi.from_buffer(data)

    out_ptr  = _ffi.new("uint8_t **")
    out_size = _ffi.new("size_t *")

    result = _lib.img_api_process_raw(
        self._engine,
        input_buf,
        len(data),
        out_ptr,
        out_size,
    )

    if result != 0:  # IMG_SUCCESS = 0
        _raise_for_result(result)

    # Copy output into Python bytes, then free the C buffer
    size = out_size[0]
    output = bytes(_ffi.buffer(out_ptr[0], size))
    _lib.img_encoded_free(out_ptr[0])

    return output

def shutdown(self):
    if self._engine != _ffi.NULL:
        _lib.img_api_shutdown(self._engine)
        self._engine = _ffi.NULL

def __enter__(self):
    return self

def __exit__(self, *_):
    self.shutdown()

def __del__(self):
    self.shutdown()

def _raise_for_result(code: int):
errors = {
1: (MemoryError,   "IMG_ERR_NOMEM: slab exhausted — image too large"),
2: (ValueError,    "IMG_ERR_FORMAT: unsupported or corrupt image format"),
3: (ValueError,    "IMG_ERR_SECURITY: image failed security validation"),
4: (IOError,       "IMG_ERR_IO: I/O failure"),
5: (RuntimeError,  "IMG_ERR_HW_UNSUP: required SIMD not available"),
6: (RuntimeError,  "IMG_ERR_INTERNAL: internal engine error"),
}
exc_cls, msg = errors.get(code, (RuntimeError, f"unknown error code {code}"))
raise exc_cls(msg)