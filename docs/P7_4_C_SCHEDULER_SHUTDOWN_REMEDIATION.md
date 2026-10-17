# P7.4-C Scheduler Shutdown Remediation

## Status

**CLOSED — VERIFIED.** This document preserves the original C-001/C-002
findings and records the narrow remediation and executed evidence. It does not
claim production readiness.

## Findings

### C-001 — High: accepted queued task can lose its terminal path

`img_scheduler_submit()` accepts an `img_task_t *` into a per-worker SPSC
queue or the scheduler MPMC overflow queue in
`src/runtime/scheduler_submit.c`. `img_runtime_wait_task()` then waits until
the task reaches `IMG_TASK_DONE`.

Before remediation, `img_scheduler_destroy()` stopped and joined workers, then
destroyed their SPSC queues and scheduler MPMC queue without resolving queued
task pointers. A task accepted before shutdown but not popped by a worker had
no remaining owner able to set a terminal state; a synchronous waiter could
spin indefinitely. API-global `g_task_queue` was also destroyed later by
`src/api/api_shutdown_globals.c` without a task-resolution step.

This is a production correctness issue, not merely missing coverage: task
ownership is lost by teardown and an accepted request may never receive its
required completion result or cleanup path.

### C-002 — Moderate: lifecycle sharing lacks C11 synchronization

Before remediation, `img_task_t.state` in `include/runtime/task.h` was written by
`src/runtime/worker_loop.c` and read by
`src/runtime/scheduler_submit.c`. `img_worker_t.running` in
`include/runtime/worker.h` is written by
`src/runtime/worker_control.c` and read by the worker loop. Neither shared
field is atomic; `volatile` does not establish a C happens-before relation.
Concurrent shutdown/wait observations are therefore not defined by the C11
memory model.

## Current source-backed lifecycle model

```text
READY task (caller-owned task storage)
  -> img_scheduler_submit() accepts pointer into SPSC or scheduler MPMC queue
  -> worker dequeues pointer
  -> RUNNING
  -> worker writes status
  -> DONE
  -> waiter observes terminal state; caller reclaims task-associated resources

Current teardown:
  stop workers -> join workers -> destroy queues -> free scheduler
```

The final arrow is invalid for a task still in any queue: no path changes its
state to `DONE` or another documented terminal state.

## Required invariant

> Once a task is accepted by the scheduler, exactly one ownership path remains
> responsible for reaching a terminal state: executed completion or explicit
> terminal shutdown rejection/completion. Queue destruction must never silently
> discard an accepted task.

Worker join proves only that workers exited. It does not prove that accepted
queues are empty or that their task owners were resolved.

## Implemented minimum protocol

The existing public/internal contract says `img_api_shutdown()` drains queues,
does not provide native cancellation, and must not run concurrently with a
job. The remediation introduces one scheduler-owned admission linearization
point: `admission_lock` protects the acquire-or-close race and `accepting`
closes admission with release publication. Workers continue polling accepted
work after close and leave only when queues are empty. `destroy()` joins those
workers before queue storage is released.

If a queue remains populated because its worker never started or teardown is
otherwise unable to execute it, destruction drains the pointer and publishes
`IMG_TASK_REJECTED` with `IMG_ERR_INTERNAL`. Thus every accepted task obtains
exactly one terminal outcome: worker `DONE` or explicit teardown `REJECTED`.
`img_runtime_wait_task()` acquire-loads terminal state before returning its
status. Task state and worker running state are C11 atomics; status is written
before a release publication of terminal state.

This does not change ABI v1, scheduler topology, native cancellation, Rust,
HTTP, or product resource limits. The scheduler's documented non-concurrent
shutdown precondition remains unchanged.

## Verification workflow

Development uses a focused scheduler test and targeted normal/sanitizer CTest
gates. Subsystem gates cover the full native CTest/regression suite and
affected ABI/FFI, supervisor, and API tests. The unchanged Ubuntu 24.04
`scripts/verify_linux.sh` remains the authoritative release gate and was run
with its full 5,000-run fuzz settings. `imgengine/.dockerignore` only reduces
Docker build context by excluding generated local build artifacts; it does not
remove or weaken any verification stage.

## Required evidence

| ID | Invariant | Source path | Test | Expected result | Actual result | Sanitizer | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| SCH-C01 | accepted queued task reaches terminal state | `scheduler_submit.c`, `scheduler_destroy.c` | `test_scheduler_shutdown` unstarted-queue case | terminal result before queue destruction | PASS: terminal `REJECTED`/`IMG_ERR_INTERNAL` | ASAN/UBSAN/leak + Ubuntu | PASS |
| SCH-C02 | queue is not destroyed with accepted task ownership outstanding | `scheduler_destroy.c`, queue sources | explicit destruction drain/rejection | all accepted tasks resolved | PASS: SPSC, scheduler MPMC, and attached API MPMC resolve before release | ASAN/UBSAN/leak + Ubuntu | PASS |
| SCH-C03 | waiter cannot spin forever | `scheduler_submit.c`, `worker_loop.c` | terminal wait assertions | returns after terminal publication | PASS: accepted unstarted tasks return `IMG_ERR_INTERNAL`; drained tasks return their worker result | ASAN/UBSAN/leak + Ubuntu | PASS |
| SCH-C04 | active task retains intended completion behavior | `worker_loop.c`, `scheduler_destroy.c` | 64 accepted worker-drain tasks | accepted work completes before join | PASS: all reach `DONE` with the expected validation result | ASAN/UBSAN/leak + Ubuntu | PASS |
| SCH-C05 | shutdown is deterministic under the documented contract | scheduler/API shutdown sources | repeated `destroy()` regression | bounded, no deadlock | PASS: repeated destroyed-scheduler call is inert | ASAN/UBSAN/leak + Ubuntu | PASS |
| SCH-C06 | admission closes before teardown | `scheduler_submit.c` | submit-after-close and controlled submit/close race | rejected, not accepted after closure | PASS: post-close submit rejects; racing submission is rejected unchanged or resolved terminally | ASAN/UBSAN/leak + Ubuntu | PASS |
| SCH-C07 | shared lifecycle state has defined synchronization | `task.h`, `worker.h` | source review plus sanitizer regression | C11 acquire/release contract | PASS: atomic task/worker state and release/acquire terminal publication | ASAN/UBSAN/leak + Ubuntu | PASS |
