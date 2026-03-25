/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_window_lifecycle(void) {
  int res;
  cmp_window_t *window = NULL;
  cmp_window_config_t config;

  res = cmp_window_system_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  config.title = "Test Window";
  config.width = 800;
  config.height = 600;
  config.x = -1;
  config.y = -1;
  config.hidden = 0;
  config.frameless = 0;
  config.use_legacy_backend = 0;

  res = cmp_window_create(&config, &window);
#if defined(_WIN32)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(window != NULL);

  res = cmp_window_show(window);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_window_render_test_frame(window);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Spin briefly to let the paint message dispatch */
  cmp_window_poll_events(window);

  res = cmp_window_destroy(window);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  /* UNIX not yet implemented in Phase 6 */
#endif

  res = cmp_window_system_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

static void dummy_drop_callback(const char *path, void *user_data) {
  int *triggered = (int *)user_data;
  *triggered = 1;
  (void)path;
}

TEST test_window_drop_callback(void) {
  int res;
  cmp_window_t *window = NULL;
  cmp_window_config_t config;
  int triggered = 0;

  res = cmp_window_system_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  config.title = "Drop Test Window";
  config.width = 400;
  config.height = 300;
  config.x = -1;
  config.y = -1;
  config.hidden = 0;
  config.frameless = 0;
  config.use_legacy_backend = 0;

  res = cmp_window_create(&config, &window);
#if defined(_WIN32)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(window != NULL);

  res = cmp_window_set_drop_callback(window, dummy_drop_callback, &triggered);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_window_destroy(window);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  /* UNIX not yet implemented in Phase 6 */
#endif

  cmp_window_system_shutdown();
  PASS();
}

static void dummy_menu_callback(void) {}

TEST test_mac_menu_bar(void) {
  int res;
  res = cmp_window_mac_init_menu_bar();
#if defined(__APPLE__)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  res = cmp_window_mac_add_menu_item("Test", "t", dummy_menu_callback);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
  res = cmp_window_mac_add_menu_item("Test", "t", dummy_menu_callback);
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif
  PASS();
}

TEST test_apple_display_link(void) {
  int res;
  cmp_window_t *window = NULL;
  cmp_window_config_t config;

  cmp_window_system_init();
  config.title = "Test";
  config.width = 100;
  config.height = 100;
  config.x = 0;
  config.y = 0;
  config.hidden = 1;
  config.frameless = 0;
  config.use_legacy_backend = 0;
  cmp_window_create(&config, &window);

  res = cmp_window_apple_init_display_link(window, 60);
#if defined(__APPLE__)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  cmp_window_destroy(window);
  cmp_window_system_shutdown();
  PASS();
}

TEST test_apple_gestures(void) {
  int res;
  cmp_window_t *window = NULL;
  cmp_window_config_t config;

  cmp_window_system_init();
  config.title = "Test";
  config.width = 100;
  config.height = 100;
  config.x = 0;
  config.y = 0;
  config.hidden = 1;
  config.frameless = 0;
  config.use_legacy_backend = 0;
  cmp_window_create(&config, &window);

  res = cmp_window_apple_enable_gestures(window, 1, 1, 1);
#if defined(__APPLE__)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  cmp_window_destroy(window);
  cmp_window_system_shutdown();
  PASS();
}

TEST test_linux_apis(void) {
  int res;
  cmp_window_t *window = NULL;
  cmp_window_config_t config;

  cmp_window_system_init();
  config.title = "Test";
  config.width = 100;
  config.height = 100;
  config.x = 0;
  config.y = 0;
  config.hidden = 1;
  config.frameless = 0;
  config.use_legacy_backend = 0;
  cmp_window_create(&config, &window);

  res = cmp_window_os_notify("Title", "Body");
#if defined(__linux__) && !defined(__ANDROID__)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  res = cmp_window_set_clipboard_text(window, CMP_CLIPBOARD_PRIMARY, "test");
#if defined(__linux__) && !defined(__ANDROID__)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  {
    cmp_string_t str;
    res = cmp_window_get_clipboard_text(window, CMP_CLIPBOARD_PRIMARY, &str);
#if defined(__linux__) && !defined(__ANDROID__)
    ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
    cmp_string_destroy(&str);
#else
    ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif
  }

  res = cmp_window_linux_init_evdev("/dev/input/event0");
#if defined(__linux__) && !defined(__ANDROID__)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  cmp_window_destroy(window);
  cmp_window_system_shutdown();
  PASS();
}

TEST test_android_apis(void) {
  int res;
  cmp_window_t *window = NULL;
  cmp_window_config_t config;
  int top, bottom, left, right;

  cmp_window_system_init();
  config.title = "Test";
  config.width = 100;
  config.height = 100;
  config.x = 0;
  config.y = 0;
  config.hidden = 1;
  config.frameless = 0;
  config.use_legacy_backend = 0;
  cmp_window_create(&config, &window);

  res = cmp_window_android_init_hooks(NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_window_android_show_keyboard(window, 1);
#if defined(__ANDROID__)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  res = cmp_window_android_get_safe_area(window, &top, &bottom, &left, &right);
#if defined(__ANDROID__)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  cmp_window_destroy(window);
  cmp_window_system_shutdown();
  PASS();
}

TEST test_wasm_apis(void) {
  int res;
  cmp_modality_t mod;

  cmp_modality_single_init(&mod);

  res = cmp_window_wasm_init("#canvas");
#if defined(__EMSCRIPTEN__)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  res = cmp_window_wasm_resume_audio();
#if defined(__EMSCRIPTEN__)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  res = cmp_window_wasm_set_main_loop(&mod, NULL, NULL);
  /* NULL function pointer should yield INVALID_ARG regardless of platform */
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  cmp_modality_destroy(&mod);
  PASS();
}

TEST test_sdl3_fallback_apis(void) {
  int res;

  res = cmp_sdl3_fallback_init_subsystems();
#if defined(CMP_USE_SDL3)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  res = cmp_sdl3_fallback_bridge_audio();
#if defined(CMP_USE_SDL3)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#else
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
#endif

  PASS();
}

TEST test_scripting_apis(void) {
  int res;

  res = cmp_scripting_lua_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_scripting_lua_execute_file(NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_scripting_napi_init(NULL, NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_scripting_python_generate_bindings(NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_scripting_python_generate_bindings("cmp_bindings.py");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_theme_and_visual_regression_apis(void) {
  int res;
  cmp_window_t *window = NULL;
  cmp_window_config_t config;
  cmp_theme_t theme;
  cmp_palette_t palette;

  res = cmp_window_system_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  config.title = "Visual Test Window";
  config.width = 100;
  config.height = 100;
  config.x = -1;
  config.y = -1;
  config.hidden = 1;
  config.frameless = 0;
  config.use_legacy_backend = 0;

  res = cmp_window_create(&config, &window);
#if defined(_WIN32) || defined(CMP_USE_SDL3)
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_theme_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Test Theme Palette Generation (Material 3 Dynamic Color Stub) */
  {
    cmp_color_t seed = {1.0f, 0.0f, 1.0f, 1.0f};
    res = cmp_theme_generate_palette(seed, &palette);
    ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  }

  /* Test Application of Theme (Material / Fluent / Cupertino) */
  memset(&theme, 0, sizeof(theme));
  theme.light_palette.primary = palette.primary;
  theme.light_palette.background = palette.background;

  res = cmp_window_set_theme(window, &theme);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_theme_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_window_destroy(window);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
#endif

  res = cmp_window_system_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  PASS();
}

SUITE(window_suite) {
  RUN_TEST(test_window_lifecycle);
  RUN_TEST(test_window_drop_callback);
  RUN_TEST(test_mac_menu_bar);
  RUN_TEST(test_apple_display_link);
  RUN_TEST(test_apple_gestures);
  RUN_TEST(test_linux_apis);
  RUN_TEST(test_android_apis);
  RUN_TEST(test_wasm_apis);
  RUN_TEST(test_sdl3_fallback_apis);
  RUN_TEST(test_scripting_apis);
  RUN_TEST(test_theme_and_visual_regression_apis);
}
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(window_suite);
  GREATEST_MAIN_END();
}