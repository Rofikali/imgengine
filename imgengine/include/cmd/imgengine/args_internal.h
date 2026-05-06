// ./include/cmd/imgengine/args_internal.h
#ifndef IMGENGINE_CLI_ARGS_INTERNAL_H
#define IMGENGINE_CLI_ARGS_INTERNAL_H

#include "cmd/imgengine/args.h"
#include "pipeline/job_presets.h"

int img_cli_parse_u32(const char *text, uint32_t *out, const char *name);
int img_cli_parse_float(const char *text, float *out, const char *name);
int img_cli_parse_mode(const char *text, img_scale_mode_t *mode);
void img_cli_set_defaults(img_cli_options_t *opts);
int img_cli_parse_options(int argc, char **argv, img_cli_options_t *opts);
int img_cli_parse_options_io(int opt, const char *optarg, img_cli_options_t *opts);
int img_cli_parse_options_input(int opt, const char *optarg, img_cli_options_t *opts);
int img_cli_parse_options_layout(int opt, const char *optarg, img_cli_options_t *opts);
int img_cli_parse_options_photo(int opt, const char *optarg, img_cli_options_t *opts);
int img_cli_parse_options_print(int opt, const char *optarg, img_cli_options_t *opts);
int img_cli_finalize_options(img_cli_options_t *opts);

void img_cli_print_header(const char *bin);
void img_cli_print_usage_section(const char *bin);
void img_cli_print_input_options(void);
void img_cli_print_layout_options(void);
void img_cli_print_photo_options(void);
void img_cli_print_print_options(void);
void img_cli_print_runtime_options(void);
void img_cli_print_options_section(void);
void img_cli_print_examples_section(const char *bin);

#endif /* IMGENGINE_CLI_ARGS_INTERNAL_H */
