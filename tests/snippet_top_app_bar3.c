#include <string.h>

static int test_top_app_bar_edge_cases3(void) {
  struct ui_top_app_bar_base *bar = NULL;
  struct ui_component *comp;
  ui_error_t rc;
  int i;

  /* test get_component invalid args */
  assert(UI_ERROR_INVALID_ARGUMENT ==
         ui_top_app_bar_base_get_component(NULL, &comp));

#ifdef UI_TEST_MOCK_ALLOC
  for (i = 0; i < 10; i++) {
    struct ui_arena *arena;
    struct ui_top_app_bar_config config;
    memset(&config, 0, sizeof(config));

    rc = ui_arena_create(2048, &arena);
    assert(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = i;
    rc = ui_top_app_bar_base_create(arena, &config, &bar);
    if (rc != UI_ERROR_NONE && bar) {
      ui_top_app_bar_base_destroy(bar);
    }
    g_malloc_fail_countdown = -1;

    ui_arena_destroy(arena);
  }
#endif

  return 0;
}
