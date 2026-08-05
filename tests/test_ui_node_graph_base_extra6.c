void test_extra_node_graph_errors(void) {
  (void)ui_node_graph_base_destroy(NULL);
  ui_node_graph_base_pan(NULL, 0.0f, 0.0f);
  ui_node_graph_base_zoom(NULL, 0.0f, NULL);
  ui_node_graph_base_get_camera_signal(NULL, NULL);
  ui_node_graph_base_screen_to_graph(NULL, NULL, NULL);
  ui_node_graph_base_add_connection(NULL, NULL);
  ui_node_graph_base_set_marquee_selection(NULL, NULL);
  ui_node_graph_base_get_topology_signal(NULL, NULL);
}
