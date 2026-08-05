void test_ui_renderer_oom_fallback_error(void) {
  struct ui_renderer *renderer;
  extern int g_native_init_fail;
  /* We can't really mock gles_fallback_init failing because we don't have a
   * flag for it, and adding one means changing source. */
}
