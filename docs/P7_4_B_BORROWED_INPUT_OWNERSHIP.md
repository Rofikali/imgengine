# P7.4-B Borrowed-Input Ownership Evidence

## Invariant

**A borrowed input buffer MUST NOT be unmapped, freed, retained, modified, or
otherwise ownership-released by native code.** The caller keeps it valid for
the synchronous decode call. If native work needs compressed bytes beyond that
borrow lifetime, it must first explicitly copy those bytes. Queued work must
receive native-owned decoded data, never the encoded borrow.

The invariant is executable at `img_decode_fn` in `include/io/io_vtable.h`,
at `img_api_decode_prepared` in `include/api/api_internal.h`, and in the
public ABI-v1 header. The legacy file-job boundary is deliberately different:
`img_api_load_input` transfers ownership of an mmap to its caller, and only
`img_api_decode_input` consumes that owned mapping through
`img_api_release_input`.

## Source trace

| Boundary | Path | Ownership | Release / lifetime |
| --- | --- | --- | --- |
| HTTP/Rust input | Rust safe wrapper -> `imgengine_process_encoded_image_to_jpeg` | caller-owned borrowed bytes | ABI call is synchronous; ABI never releases input |
| ABI raw processing | `imgengine_v1.c` -> `img_api_process_raw` -> `img_api_process_raw_decode` | borrowed | decoder vtable receives `const uint8_t *`; decoded slab buffer is released after encode |
| P7.2.8 private render | `img_layout_render_v1` -> `img_layout_render_decode_nonowning` -> `img_api_decode_prepared` | borrowed until decode returns | state retains only decoded slab-owned `img_buffer_t`; worker releases it in state cleanup |
| Decoder vtable | `g_io_vtable.decode` -> JPEG bulk/stream or PNG decoder | borrowed input | JPEG reads input; stream path copies compressed bytes to slab before later decode; PNG copies decoded pixels to slab |
| Legacy file job | `img_api_load_input` -> `load_file_mmap` -> `img_api_decode_input` | mmap-owned | `img_api_decode_input` calls `img_api_release_input`/`munmap` on every decode result; this path is quarantined from borrowed callers |
| Scheduler | decoded `img_buffer_t` is placed in render/layout task state | decoded slab-owned data | worker completes synchronously; caller/state cleanup releases only slab-owned decoded buffers |

No current Rust/HTTP/P7.2.8 caller routes borrowed bytes through
`img_api_decode_input`. The historical mmap wrapper remains present and
explicitly quarantined; it is not deleted or repurposed.

## Regression design

`src/tests/test_borrowed_input_ownership.c` creates a one-page anonymous mmap
owned by the test. `mincore` establishes whether that mapping remains live
without dereferencing a released address; byte comparison then proves the
borrow remained unchanged. The test covers:

- successful `img_api_decode_prepared` decoder-vtable use with a deterministic PNG;
- malformed decode with no decoded output;
- the public ABI-v1 decode/encode path used by Rust/HTTP, plus the internal raw path it calls;
- P7.2.8 queued private render, which succeeds after decode while the encoded
  source mapping stays live and unchanged;
- failed P7.2.8 private-render state cleanup after malformed decode, with the
  encoded source mapping still live and unchanged;
- a negative control: the legacy release helper changes the guard to non-live.

Thus an accidental `munmap` at a borrowed decode boundary makes the relevant
case fail before any post-release dereference. An accidental `free` of the
non-heap mapping causes a deterministic test-process failure under the normal
or sanitizer run; production code is not deliberately driven into undefined
behavior.

## Principal evidence table

| ID | Ownership boundary | Contract | Test/Evidence | Result | Remaining Risk |
| --- | --- | --- | --- | --- | --- |
| OWN-R01 | borrowed decoder input | decoder neither releases nor mutates borrow | guarded valid decode | PASS | only deterministic PNG fixture in focused test |
| OWN-R02 | decoder-vtable ownership | vtable receives a synchronous `const` borrow | explicit function-pointer contract and source trace | PASS | future vtable implementations must follow the contract |
| OWN-R03 | decode failure | malformed input is not released | guarded malformed decode | PASS | broader malformed corpus remains fuzz/verifier coverage |
| OWN-R04 | ABI/API | ABI caller retains encoded input; output has explicit release | guarded public ABI-v1 test plus internal raw-path test | PASS | ABI v1 public shape unchanged |
| OWN-R05 | queued task lifetime | queue carries decoded slab-owned data, not encoded borrow | guarded P7.2.8 render completes with mapping live | PASS | no asynchronous encoded-borrow API is introduced |
| OWN-R06 | mmap-owning legacy input | legacy owner alone releases mmap | guarded negative control invokes `img_api_release_input` | PASS | legacy file route remains quarantined, not modernized |
| OWN-R07 | cleanup | decoded/output resources are released without touching borrow | successful and failed private-render plus decoder cases | PASS | allocator stress is separate P7.4-C scope |
| OWN-R08 | sanitizer coverage | no ownership memory finding | ASAN/UBSAN/leak focused run and full verifier ASAN CTest | PASS | none observed |
| OWN-R09 | Ubuntu release gate | all canonical verification stages | fresh `IMGENGINE_VERIFY_JOBS=2 IMGENGINE_FUZZ_RUNS=5000 bash scripts/verify_linux.sh` | PASS | bounded fuzzing remains smoke evidence, not proof of absence |

## Current decision

The authoritative Ubuntu 24.04 verifier exited `0`: format, normal and
ASAN/UBSAN/leak CTest (including both ownership tests), native regressions,
ABI/FFI/Rust/API, real-image verification, sandbox, and two 5,000-run fuzz
targets passed. The only observed compiler diagnostic was the existing C23
unnamed-parameter warning in `plugin_grayscale.c`; it is unrelated to this
gate and did not fail verification.

**Final decision: P7.4-B CLOSED.** This document records the historical mmap
path as quarantined; it does not claim that mmap-backed callers are borrowed
callers.
