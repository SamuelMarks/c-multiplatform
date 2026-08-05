void test_extra_node_graph_error(void) {
  /* Call update_camera_matrix with NULL by simulating a call from pan/zoom with
   * graph=NULL */
  ui_node_graph_base_pan(NULL, 0.0f, 0.0f);
  ui_node_graph_base_zoom(NULL, 0.0f, NULL);
  ui_node_graph_base_screen_to_graph(NULL, NULL, NULL);
  ui_node_graph_base_set_marquee_selection(NULL, NULL);
}
