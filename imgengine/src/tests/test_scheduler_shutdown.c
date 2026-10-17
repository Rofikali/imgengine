#include "runtime/scheduler.h"
#include "runtime/worker.h"
#include "runtime/worker_internal.h"

#include "core/context_internal.h"

#include <limits.h>
#include <pthread.h>
#include <stdio.h>

typedef struct shutdown_race_context {
    img_scheduler_t *scheduler;
    img_task_t *task;
    atomic_int contenders_ready;
    int submit_result;
} shutdown_race_context_t;

static void *submit_during_close(void *opaque) {
    shutdown_race_context_t *race = opaque;

    atomic_fetch_add_explicit(&race->contenders_ready, 1, memory_order_release);
    race->submit_result = img_scheduler_submit(race->scheduler, race->task);
    return NULL;
}

static void *close_during_submit(void *opaque) {
    shutdown_race_context_t *race = opaque;

    atomic_fetch_add_explicit(&race->contenders_ready, 1, memory_order_release);
    img_scheduler_close(race->scheduler);
    return NULL;
}

static int test_unstarted_scheduler_rejects_accepted_work(void) {
    img_scheduler_t scheduler = {0};
    img_task_t tasks[3] = {0};

    if (img_scheduler_init(&scheduler, 1) != 0) {
        fprintf(stderr, "scheduler initialization failed\n");
        return 1;
    }

    for (size_t index = 0; index < 3; index++) {
        tasks[index].state = IMG_TASK_READY;
        tasks[index].status = INT_MIN;
        if (img_scheduler_submit(&scheduler, &tasks[index]) != 0) {
            fprintf(stderr, "task %zu was not accepted\n", index);
            img_scheduler_destroy(&scheduler);
            return 1;
        }
    }

    img_scheduler_close(&scheduler);
    if (img_scheduler_submit(&scheduler, &tasks[0]) == 0) {
        fprintf(stderr, "closed scheduler accepted new work\n");
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    img_scheduler_destroy(&scheduler);

    for (size_t index = 0; index < 3; index++) {
        if (atomic_load_explicit(&tasks[index].state, memory_order_acquire) != IMG_TASK_REJECTED ||
            tasks[index].status != IMG_ERR_INTERNAL ||
            img_runtime_wait_task(&tasks[index]) != IMG_ERR_INTERNAL) {
            fprintf(stderr, "queued task %zu did not receive terminal rejection\n", index);
            return 1;
        }
    }

    if (img_scheduler_submit(&scheduler, &tasks[0]) == 0) {
        fprintf(stderr, "destroyed scheduler accepted new work\n");
        return 1;
    }
    img_scheduler_destroy(&scheduler);
    return 0;
}

static int test_started_worker_drains_accepted_work(void) {
    img_scheduler_t scheduler = {0};
    img_ctx_t context = {0};
    img_task_t tasks[64] = {0};

    if (img_scheduler_init(&scheduler, 1) != 0)
        return 1;
    img_worker_t *worker = &scheduler.workers[0];
    worker->ctx = &context;
    atomic_store_explicit(&worker->running, true, memory_order_release);
    if (pthread_create(&worker->thread, NULL, img_worker_loop, worker) != 0) {
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    worker->thread_started = 1;

    for (size_t index = 0; index < 64; index++) {
        tasks[index].status = INT_MIN;
        if (img_scheduler_submit(&scheduler, &tasks[index]) != 0) {
            fprintf(stderr, "drain task %zu was not accepted\n", index);
            img_scheduler_destroy(&scheduler);
            return 1;
        }
    }
    img_scheduler_destroy(&scheduler);

    for (size_t index = 0; index < 64; index++) {
        if (atomic_load_explicit(&tasks[index].state, memory_order_acquire) != IMG_TASK_DONE ||
            tasks[index].status != IMG_ERR_SECURITY ||
            img_runtime_wait_task(&tasks[index]) != IMG_ERR_SECURITY) {
            fprintf(stderr, "accepted drain task %zu did not complete\n", index);
            return 1;
        }
    }
    return 0;
}

static int test_submit_close_linearization(void) {
    img_scheduler_t scheduler = {0};
    img_task_t task = {0};
    shutdown_race_context_t race = {
        .scheduler = &scheduler,
        .task = &task,
        .contenders_ready = 0,
        .submit_result = INT_MIN,
    };
    pthread_t submitter;
    pthread_t closer;

    if (img_scheduler_init(&scheduler, 1) != 0)
        return 1;

    /* Hold the common linearization lock until both operations are contending. */
    if (pthread_mutex_lock(&scheduler.admission_lock) != 0) {
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    if (pthread_create(&submitter, NULL, submit_during_close, &race) != 0) {
        pthread_mutex_unlock(&scheduler.admission_lock);
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    if (pthread_create(&closer, NULL, close_during_submit, &race) != 0) {
        pthread_mutex_unlock(&scheduler.admission_lock);
        pthread_join(submitter, NULL);
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    while (atomic_load_explicit(&race.contenders_ready, memory_order_acquire) != 2)
        ;
    pthread_mutex_unlock(&scheduler.admission_lock);
    pthread_join(submitter, NULL);
    pthread_join(closer, NULL);

    if (img_scheduler_is_accepting(&scheduler)) {
        fprintf(stderr, "scheduler stayed open after close won its linearization\n");
        img_scheduler_destroy(&scheduler);
        return 1;
    }
    img_scheduler_destroy(&scheduler);

    if (race.submit_result == 0) {
        if (atomic_load_explicit(&task.state, memory_order_acquire) != IMG_TASK_REJECTED ||
            img_runtime_wait_task(&task) != IMG_ERR_INTERNAL) {
            fprintf(stderr, "accepted racing submission did not become terminal\n");
            return 1;
        }
    } else if (atomic_load_explicit(&task.state, memory_order_acquire) != IMG_TASK_READY) {
        fprintf(stderr, "rejected racing submission changed task ownership\n");
        return 1;
    }
    return 0;
}

int main(void) {
    if (test_unstarted_scheduler_rejects_accepted_work() ||
        test_started_worker_drains_accepted_work() || test_submit_close_linearization())
        return 1;
    puts("[scheduler-shutdown] accepted work reaches terminal state before teardown");
    return 0;
}
