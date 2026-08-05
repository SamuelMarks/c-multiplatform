void test_extra_node_graph_update_camera(void) {
  /* update_camera_matrix is static and only called by pan, zoom, and
     set_camera_bounds. It's actually impossible to hit `if (!graph)` inside it
     because the calling functions already check for NULL graph! We can just
     accept that this might be an issue or try to remove the check in src code.
     Wait, if we remove it, it segfaults. But we CAN change the source code to
     just remove the check. Ah wait! The segfault was because we removed `if
     (err != UI_ERROR_NONE)` which caused the signal create to continue and use
     uninitialized pointers.
  */
}
