int g_native_init_fail = 0;
void test_ui_renderer_oom_fallback(void) {
  struct ui_renderer *renderer;
  extern int g_native_init_fail;
  g_native_init_fail = 1;
  ui_renderer_create(&renderer);
  g_native_init_fail = 0;
}
