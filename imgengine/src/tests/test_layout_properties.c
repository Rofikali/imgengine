#include <stdint.h>
#include <stdio.h>

#include "pipeline/canvas_internal.h"

static uint32_t next_value(uint32_t *state) {
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

int main(void) {
    uint32_t state = 0x4d47454eu;
    for (uint32_t case_index = 0; case_index < 1000; case_index++) {
        img_job_t job;
        img_canvas_t canvas = {0};
        img_job_defaults(&job);
        job.dpi = 30u + next_value(&state) % 571u;
        job.cols = 1u + next_value(&state) % 12u;
        job.rows = 1u + next_value(&state) % 12u;
        job.gap = next_value(&state) % 41u;
        job.padding = next_value(&state) % 41u;
        job.photo_w_cm = 1.0f + (float)(next_value(&state) % 90u) / 10.0f;
        job.photo_h_cm = 1.0f + (float)(next_value(&state) % 90u) / 10.0f;

        uint32_t page_width = img_cm_to_px(IMG_A4_W_CM, job.dpi);
        uint32_t page_height = img_cm_to_px(IMG_A4_H_CM, job.dpi);
        uint64_t horizontal_reserved = (uint64_t)job.padding * 2u +
                                       (uint64_t)(job.cols - 1u) * job.gap;
        uint64_t vertical_reserved = (uint64_t)job.padding * 2u +
                                     (uint64_t)(job.rows - 1u) * job.gap;
        int feasible = horizontal_reserved < page_width && vertical_reserved < page_height &&
                       ((uint64_t)page_width - horizontal_reserved) / job.cols > 0u &&
                       ((uint64_t)page_height - vertical_reserved) / job.rows > 0u;
        img_result_t result = img_canvas_compute_geometry(&canvas, &job, page_width, page_height);

        if (!feasible) {
            if (result != IMG_ERR_SECURITY) {
                fprintf(stderr, "infeasible layout accepted at case %u\n", case_index);
                return 1;
            }
            continue;
        }
        if (result != IMG_SUCCESS) {
            fprintf(stderr, "feasible layout rejected at case %u\n", case_index);
            return 1;
        }

        uint64_t grid_width = (uint64_t)job.cols * canvas.photo_w_px +
                              (uint64_t)(job.cols - 1u) * job.gap;
        uint64_t grid_height = (uint64_t)job.rows * canvas.photo_h_px +
                               (uint64_t)(job.rows - 1u) * job.gap;
        if (canvas.photo_w_px == 0 || canvas.photo_h_px == 0 || canvas.start_x > page_width ||
            canvas.start_y > page_height || grid_width > page_width || grid_height > page_height ||
            (uint64_t)canvas.start_x + grid_width > page_width ||
            (uint64_t)canvas.start_y + grid_height > page_height) {
            fprintf(stderr,
                    "layout property failure at case %u: dpi=%u cols=%u rows=%u gap=%u padding=%u "
                    "photo=%.1fx%.1f page=%ux%u cell=%ux%u start=%ux%u grid=%llux%llu\n",
                    case_index, job.dpi, job.cols, job.rows, job.gap, job.padding, job.photo_w_cm,
                    job.photo_h_cm, page_width, page_height, canvas.photo_w_px, canvas.photo_h_px,
                    canvas.start_x, canvas.start_y, (unsigned long long)grid_width,
                    (unsigned long long)grid_height);
            return 1;
        }
    }
    printf("[layout] 1000 deterministic geometry property cases passed\n");
    return 0;
}
