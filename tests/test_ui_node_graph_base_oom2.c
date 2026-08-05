void test_node_graph_oom_3(void) {
  extern int g_malloc_fail_countdown;
  struct ui_arena *arena;
  struct ui_node_graph_base *graph = NULL;
  struct ui_node_graph_camera_config config;
  int i;
  if (ui_arena_create(1024 * 16, &arena) != UI_ERROR_NONE) {
    return;
  }
  config.min_zoom = 0.1f;
  config.max_zoom = 5.0f;
  config.bounds.left = -1000.0f;
  config.bounds.top = -1000.0f;
  config.bounds.right = 1000.0f;
  config.bounds.bottom = 1000.0f;
  config.bounds.width = 2000.0f;
  config.bounds.height = 2000.0f;

  for (i = 0; i < 10; ++i) {
    g_malloc_fail_countdown = i;
    ui_node_graph_base_create(arena, &config, &graph);
  }
  g_malloc_fail_countdown = -1;
  (void)ui_arena_destroy(arena);
}
