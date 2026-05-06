.
 |-- CMakeLists-Instructions.md
 |-- CMakeLists.txt
 |-- api
 |--  |  v1
 |--  |   |  img_api.h
 |--  |   |  img_buffer_utils.h
 |--  |   |  img_error.h
 |--  |   |  img_job.h
 |--  |   |  img_pipeline.h
 |--  |   |  img_plugin_api.h
 |-- bindings
 |--  |  go
 |--  |   |  cgo_bridge.go
 |--  |  java
 |--  |   |  jni_adapter.c
 |--  |  python
 |--  |   |  cffi_wrapper.py
 |-- cmake-instructions.md
 |-- folder-structer.md
 |-- include
 |--  |  api
 |--  |   |  api_init_internal.h
 |--  |   |  api_internal.h
 |--  |   |  api_job_finish_output_internal.h
 |--  |   |  api_job_internal.h
 |--  |   |  api_process_fast_internal.h
 |--  |   |  api_process_raw_internal.h
 |--  |   |  api_shutdown_internal.h
 |--  |   |  io_vtable.h
 |--  |  arch
 |--  |   |  arch_interface.h
 |--  |   |  cpu_caps.h
 |--  |   |  resize_params.h
 |--  |  cmd
 |--  |   |  imgengine
 |--  |   |   |  args.h
 |--  |   |   |  args_internal.h
 |--  |   |   |  io_uring_engine.h
 |--  |   |   |  job_builder.h
 |--  |  cold
 |--  |   |  debug.h
 |--  |   |  error.h
 |--  |   |  validation.h
 |--  |  core
 |--  |   |  align.h
 |--  |   |  arch_interface.h
 |--  |   |  batch.h
 |--  |   |  buffer.h
 |--  |   |  buffer_lifecycle.h
 |--  |   |  context_internal.h
 |--  |   |  cpu_caps.h
 |--  |   |  opcodes.h
 |--  |   |  pipeline_types.h
 |--  |   |  result.h
 |--  |   |  time.h
 |--  |  hot
 |--  |   |  batch_exec.h
 |--  |   |  pipeline_exec.h
 |--  |   |  pipeline_exec_inline.h
 |--  |   |  pixel_ops.h
 |--  |  io
 |--  |   |  decoder
 |--  |   |   |  decoder_entry.h
 |--  |   |  encoder
 |--  |   |   |  encoder_entry.h
 |--  |   |   |  pdf_encoder.h
 |--  |   |  io_vfs.h
 |--  |   |  io_vtable.h
 |--  |   |  remote_fetch.h
 |--  |   |  streaming_decoder.h
 |--  |   |  streaming_decoder_internal.h
 |--  |   |  zero_copy_contract.h
 |--  |  memory
 |--  |   |  align.h
 |--  |   |  arena.h
 |--  |   |  hugepage.h
 |--  |   |  memory.h
 |--  |   |  numa.h
 |--  |   |  poison.h
 |--  |   |  slab.h
 |--  |   |  slab_internal.h
 |--  |  observability
 |--  |   |  binlog.h
 |--  |   |  binlog_fast.h
 |--  |   |  events.h
 |--  |   |  logger.h
 |--  |   |  logger_internal.h
 |--  |   |  metrics.h
 |--  |   |  profiler.h
 |--  |   |  tracepoints.h
 |--  |   |  tracing.h
 |--  |  pipeline
 |--  |   |  batch.h
 |--  |   |  batch_exec.h
 |--  |   |  border.h
 |--  |   |  canvas.h
 |--  |   |  canvas_internal.h
 |--  |   |  dispatch.h
 |--  |   |  engine.h
 |--  |   |  fused_kernel.h
 |--  |   |  fused_params.h
 |--  |   |  hardware_registry.h
 |--  |   |  job.h
 |--  |   |  jump_table.h
 |--  |   |  jump_table_init_internal.h
 |--  |   |  kernel_adapter.h
 |--  |   |  layout.h
 |--  |   |  layout_grid_internal.h
 |--  |   |  layout_resize.h
 |--  |   |  layout_resize_internal.h
 |--  |   |  pipeline.h
 |--  |   |  pipeline_compiled.h
 |--  |   |  pipeline_fuse_exec_internal.h
 |--  |   |  pipeline_fuse_internal.h
 |--  |   |  pipeline_fused.h
 |--  |   |  pipeline_signature.h
 |--  |   |  pipeline_types.h
 |--  |   |  plugin_abi.h
 |--  |   |  trace.h
 |--  |  plugins
 |--  |   |  plugin_crop.h
 |--  |   |  plugin_internal_removed.h
 |--  |   |  plugin_registry_internal.h
 |--  |   |  plugin_resize.h
 |--  |  runtime
 |--  |   |  affinity.h
 |--  |   |  backpressure.h
 |--  |   |  buffer_lifecycle.h
 |--  |   |  cluster_registry.h
 |--  |   |  exec_router.h
 |--  |   |  plugin_loader.h
 |--  |   |  plugin_loader_internal.h
 |--  |   |  queue_internal.h
 |--  |   |  queue_mpmc.h
 |--  |   |  queue_spsc.h
 |--  |   |  rpc_client.h
 |--  |   |  rpc_client_internal.h
 |--  |   |  rpc_protocol.h
 |--  |   |  rpc_server_internal.h
 |--  |   |  scheduler.h
 |--  |   |  task.h
 |--  |   |  worker.h
 |--  |   |  worker_internal.h
 |--  |  security
 |--  |   |  bounds_check.h
 |--  |   |  input_validator.h
 |--  |   |  poision.h
 |--  |   |  sandbox.h
 |--  |   |  sandbox_internal.h
 |--  |  startup
 |--  |   |  engine_init_internal.h
 |-- interactive_call_graph.html
 |-- photo.jpg
 |-- project_structure.md
 |-- src
 |--  |  api
 |--  |   |  api.c
 |--  |   |  api_decode_secure.c
 |--  |   |  api_encoded.c
 |--  |   |  api_file_mmap.c
 |--  |   |  api_helpers.c
 |--  |   |  api_init.c
 |--  |   |  api_init_engine.c
 |--  |   |  api_init_reset_engine.c
 |--  |   |  api_init_reset_engine_state.c
 |--  |   |  api_init_reset_task_state.c
 |--  |   |  api_init_reset_worker_state.c
 |--  |   |  api_init_rollback.c
 |--  |   |  api_init_rollback_workers.c
 |--  |   |  api_init_sandbox.c
 |--  |   |  api_init_task_queue.c
 |--  |   |  api_init_worker_prepare.c
 |--  |   |  api_init_workers.c
 |--  |   |  api_job.c
 |--  |   |  api_job_begin.c
 |--  |   |  api_job_decode_input.c
 |--  |   |  api_job_finish_output.c
 |--  |   |  api_job_finish_output_file.c
 |--  |   |  api_job_finish_output_pdf.c
 |--  |   |  api_job_finish_stage.c
 |--  |   |  api_job_impl_legacy.txt
 |--  |   |  api_job_init_ctx.c
 |--  |   |  api_job_load_input.c
 |--  |   |  api_job_output_legacy.txt
 |--  |   |  api_job_postfx.c
 |--  |   |  api_job_prepare_render_stage.c
 |--  |   |  api_job_release_input.c
 |--  |   |  api_job_render_output.c
 |--  |   |  api_job_run.c
 |--  |   |  api_job_run_raw.c
 |--  |   |  api_job_write_buffer.c
 |--  |   |  api_lifecycle.c
 |--  |   |  api_pdf_output.c
 |--  |   |  api_process.c
 |--  |   |  api_process_common.c
 |--  |   |  api_process_fast.c
 |--  |   |  api_process_fast_decode.c
 |--  |   |  api_process_fast_pipeline.c
 |--  |   |  api_process_fast_validate.c
 |--  |   |  api_process_raw.c
 |--  |   |  api_process_raw_decode.c
 |--  |   |  api_process_raw_encode.c
 |--  |   |  api_process_raw_execute.c
 |--  |   |  api_process_raw_free_buffer.c
 |--  |   |  api_process_raw_legacy.txt
 |--  |   |  api_process_raw_prepare.c
 |--  |   |  api_process_raw_validate.c
 |--  |   |  api_shutdown.c
 |--  |   |  api_shutdown_globals.c
 |--  |   |  api_shutdown_workers.c
 |--  |   |  api_state.c
 |--  |   |  api_task_submit.c
 |--  |  arch
 |--  |   |  x86_64
 |--  |   |   |  avx2
 |--  |   |   |   |  blit_avx2.c
 |--  |   |   |   |  color_avx2.c
 |--  |   |   |   |  fused_resize_color_norm_avx2.c
 |--  |   |   |   |  resize_avx2.c
 |--  |   |   |   |  resize_fused_avx2.c
 |--  |   |   |   |  resize_h_avx2.c
 |--  |   |   |   |  resize_v_avx2.c
 |--  |   |   |  avx512
 |--  |   |   |   |  avx512.c
 |--  |   |   |  cpu_detect.c
 |--  |   |   |  scalar
 |--  |   |   |   |  color_scalar.c
 |--  |   |   |   |  resize_scalar.c
 |--  |  cmd
 |--  |   |  bench
 |--  |   |   |  lat_bench.c
 |--  |   |  imgengine
 |--  |   |   |  args.c
 |--  |   |   |  args_parse.c
 |--  |   |   |  args_parse_defaults.c
 |--  |   |   |  args_parse_finalize.c
 |--  |   |   |  args_parse_float.c
 |--  |   |   |  args_parse_mode.c
 |--  |   |   |  args_parse_options.c
 |--  |   |   |  args_parse_options_io.c
 |--  |   |   |  args_parse_options_layout.c
 |--  |   |   |  args_parse_options_photo.c
 |--  |   |   |  args_parse_options_print.c
 |--  |   |   |  args_parse_u32.c
 |--  |   |   |  args_usage.c
 |--  |   |   |  args_usage_examples.c
 |--  |   |   |  args_usage_header.c
 |--  |   |   |  args_usage_layout.c
 |--  |   |   |  args_usage_options.c
 |--  |   |   |  args_usage_options_layout.c
 |--  |   |   |  args_usage_options_photo.c
 |--  |   |   |  args_usage_options_print.c
 |--  |   |   |  args_usage_options_runtime.c
 |--  |   |   |  args_usage_section.c
 |--  |   |   |  io_uring_engine.c
 |--  |   |   |  job_builder.c
 |--  |   |   |  main.c
 |--  |  cold
 |--  |   |  debug.c
 |--  |   |  error.c
 |--  |   |  pipeline_compiled.c
 |--  |   |  validation.c
 |--  |  core
 |--  |   |  batch_builder.c
 |--  |   |  buffer_lifecycle.c
 |--  |   |  config.c
 |--  |   |  context.c
 |--  |   |  context_create.c
 |--  |   |  context_destroy.c
 |--  |   |  img_pipeline copy.c
 |--  |  hot
 |--  |   |  batch_exec.c
 |--  |   |  pipeline_exec.c
 |--  |  io
 |--  |   |  decoder
 |--  |   |   |  decoder_entry.c
 |--  |   |   |  png_decoder.c
 |--  |   |   |  stb_bridge.c
 |--  |   |   |  streaming_decoder.c
 |--  |   |   |  streaming_decoder_core.c
 |--  |   |  encoder
 |--  |   |   |  encoder_entry.c
 |--  |   |   |  jpeg_encoder.c
 |--  |   |   |  pdf_encoder.c
 |--  |   |  io_register.c
 |--  |   |  vfs
 |--  |   |   |  http_stream.c
 |--  |   |   |  memory_stream.c
 |--  |   |   |  posix_io.c
 |--  |   |   |  s3_adapter.c
 |--  |  memory
 |--  |   |  arena.c
 |--  |   |  arena_alloc.c
 |--  |   |  arena_alloc_aligned.c
 |--  |   |  arena_create.c
 |--  |   |  arena_destroy.c
 |--  |   |  arena_reset.c
 |--  |   |  hugepage.c
 |--  |   |  numa.c
 |--  |   |  slab.c
 |--  |   |  slab_block_size.c
 |--  |   |  slab_create.c
 |--  |   |  slab_destroy.c
 |--  |   |  slab_hot.c
 |--  |   |  slab_lifecycle.c
 |--  |  observability
 |--  |   |  binlog.c
 |--  |   |  binlog_lifecycle.c
 |--  |   |  binlog_state.c
 |--  |   |  binlog_write.c
 |--  |   |  logger.c
 |--  |   |  logger_clear_ring.c
 |--  |   |  logger_init_ring.c
 |--  |   |  logger_lifecycle.c
 |--  |   |  logger_loop.c
 |--  |   |  logger_ring.c
 |--  |   |  logger_start_thread.c
 |--  |   |  logger_stop_thread.c
 |--  |   |  logger_write.c
 |--  |   |  metrics.c
 |--  |   |  profiler.c
 |--  |   |  tracing.c
 |--  |  pipeline
 |--  |   |  batch_exec.c
 |--  |   |  bleed.c
 |--  |   |  border.c
 |--  |   |  canvas.c
 |--  |   |  canvas_alloc_buffer.c
 |--  |   |  canvas_fill_background.c
 |--  |   |  canvas_free_buffer.c
 |--  |   |  canvas_geometry.c
 |--  |   |  crop_marks.c
 |--  |   |  dispatch.c
 |--  |   |  engine.c
 |--  |   |  fused_dispatch.c
 |--  |   |  fused_kernels.c
 |--  |   |  fused_kernels.inc
 |--  |   |  fused_registry.c
 |--  |   |  hardware_registry.c
 |--  |   |  jump_table.c
 |--  |   |  jump_table_init.c
 |--  |   |  jump_table_register.c
 |--  |   |  jump_table_reset.c
 |--  |   |  jump_table_select_resize.c
 |--  |   |  jump_table_select_resize_avx2.c
 |--  |   |  jump_table_select_resize_avx512.c
 |--  |   |  jump_table_select_resize_scalar.c
 |--  |   |  layout.c
 |--  |   |  layout_grid.c
 |--  |   |  layout_grid_alloc_cells.c
 |--  |   |  layout_grid_place_cells.c
 |--  |   |  layout_grid_prepare_scaled.c
 |--  |   |  layout_prepare_fill.c
 |--  |   |  layout_prepare_fill_commit.c
 |--  |   |  layout_prepare_fit.c
 |--  |   |  layout_prepare_fit_commit.c
 |--  |   |  layout_resize.c
 |--  |   |  layout_resize_bilinear.c
 |--  |   |  layout_resize_resample_bilinear.c
 |--  |   |  layout_resize_validate_alloc.c
 |--  |   |  pipeline_build.c
 |--  |   |  pipeline_executor.c
 |--  |   |  pipeline_fuse.c
 |--  |   |  pipeline_fuse_exec.c
 |--  |   |  pipeline_fuse_exec_avx2.c
 |--  |   |  pipeline_fuse_exec_scalar.c
 |--  |   |  pipeline_fuse_execute_batch.c
 |--  |   |  pipeline_fuse_select.c
 |--  |   |  pipeline_fuse_state.c
 |--  |  plugins
 |--  |   |  plugin_bleed.c
 |--  |   |  plugin_crop.c
 |--  |   |  plugin_grayscale.c
 |--  |   |  plugin_pdf.c
 |--  |   |  plugin_registry.c
 |--  |   |  plugin_registry_range.c
 |--  |   |  plugin_registry_register.c
 |--  |   |  plugin_registry_validate.c
 |--  |   |  plugin_resize.c
 |--  |  runtime
 |--  |   |  affinity.c
 |--  |   |  cluster_registry.c
 |--  |   |  exec_router.c
 |--  |   |  plugin_loader.c
 |--  |   |  plugin_loader_descriptor.c
 |--  |   |  plugin_loader_range.c
 |--  |   |  queue_create.c
 |--  |   |  queue_destroy.c
 |--  |   |  queue_mpmc.c
 |--  |   |  queue_mpmc_destroy.c
 |--  |   |  queue_mpmc_init.c
 |--  |   |  queue_mpmc_pop.c
 |--  |   |  queue_mpmc_push.c
 |--  |   |  queue_pop.c
 |--  |   |  queue_push.c
 |--  |   |  queue_spsc.c
 |--  |   |  rpc_client.c
 |--  |   |  rpc_client_close.c
 |--  |   |  rpc_client_connect.c
 |--  |   |  rpc_client_open.c
 |--  |   |  rpc_client_recv_response.c
 |--  |   |  rpc_client_send.c
 |--  |   |  rpc_client_send_request.c
 |--  |   |  rpc_server.c
 |--  |   |  rpc_server_accept.c
 |--  |   |  rpc_server_client.c
 |--  |   |  rpc_server_close.c
 |--  |   |  rpc_server_io.c
 |--  |   |  rpc_server_open.c
 |--  |   |  rpc_server_socket.c
 |--  |   |  scheduler.c
 |--  |   |  scheduler_destroy.c
 |--  |   |  scheduler_init.c
 |--  |   |  scheduler_submit.c
 |--  |   |  worker.c
 |--  |   |  worker_control.c
 |--  |   |  worker_init.c
 |--  |   |  worker_loop.c
 |--  |  security
 |--  |   |  fuzz_hooks.c
 |--  |   |  input_validator.c
 |--  |   |  sandbox.c
 |--  |   |  sandbox_apply.c
 |--  |   |  sandbox_build.c
 |--  |  startup
 |--  |   |  engine_init.c
 |--  |   |  engine_init_caps.c
 |--  |   |  engine_init_compute.c
 |--  |   |  engine_init_plugins_and_io.c
 |--  |   |  engine_init_reset.c
 |--  |   |  engine_init_sandbox.c
 |--  |   |  engine_init_subsystems.c
 |--  |  third_party
 |--  |   |  stb
 |--  |   |   |  stb_image.h
 |--  |   |   |  stb_image_impl.c
 |--  |  tools
 |--  |   |  binlog_decode.c
 |-- tests
 |--  |  fuzz
 |--  |   |  target_decoder.c
