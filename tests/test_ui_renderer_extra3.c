int g_gles_init_fail = 0;
void test_ui_renderer_oom_fallback_error(void) {
  struct ui_renderer *renderer;
  extern int g_native_init_fail;
  extern int g_gles_init_fail;
  g_native_init_fail = 1;
  g_gles_init_fail = 1;
  ui_renderer_create(&renderer);
  g_native_init_fail = 0;
  g_gles_init_fail = 0;
}
