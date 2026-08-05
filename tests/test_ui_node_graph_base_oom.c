void test_node_graph_oom(void) {
  struct ui_arena *arena;
  struct ui_node_graph_base *graph = NULL;
  struct ui_node_graph_camera_config config;
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

  /* We need to use ui_test_mock_mem to simulate failure, wait arena alloc
   * doesn't use standard malloc */
}
