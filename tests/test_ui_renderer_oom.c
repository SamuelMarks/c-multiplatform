extern int g_malloc_fail_countdown;
void test_ui_renderer_oom_real(void) {
  struct ui_renderer *renderer;
  g_malloc_fail_countdown = 0;
  ui_renderer_create(&renderer);
  g_malloc_fail_countdown = -1;
}
