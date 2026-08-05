void test_extra_node_graph_errors2(void) {
  /* To hit graph != NULL error paths, we can call things directly */
  ui_node_graph_base_screen_to_graph(NULL, NULL, NULL);
  ui_node_graph_base_add_connection(NULL, NULL);
  ui_node_graph_base_get_camera_signal(NULL, NULL);
  ui_node_graph_base_get_topology_signal(NULL, NULL);
}
