/* clang-format off */
#include "ui_icon_base.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

/* Dummy font struct for testing pointer pass-through */
struct ui_font {
  int dummy;
};

static int run_normal_tests(void) {
  struct ui_icon_base *icon = NULL;
  ui_error_t rc;
  enum ui_icon_type type;
  const char *data = NULL;
  struct ui_font dummy_font;

  printf("Testing ui_icon_base_create...\n");
  if (ui_icon_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_icon_base_create(&icon);
  if (rc != UI_ERROR_NONE || !icon) {
    printf("Failed to create icon base.\n");
    return 1;
  }

  printf("Testing getters and setters...\n");
  if (ui_icon_base_get_type(NULL, &type) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_icon_base_get_type(icon, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_icon_base_get_type(icon, &type);
  if (type != UI_ICON_TYPE_UNSET)
    return 1;

  if (ui_icon_base_get_data(NULL, &data) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_icon_base_get_data(icon, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_icon_base_get_data(icon, &data);
  if (data != NULL)
    return 1;

  /* Font Glyph */
  if (ui_icon_base_set_font_glyph(NULL, &dummy_font, "home") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_icon_base_set_font_glyph(icon, NULL, "home") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_icon_base_set_font_glyph(icon, &dummy_font, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_icon_base_set_font_glyph(icon, &dummy_font, "home");
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_icon_base_get_type(icon, &type);
  if (type != UI_ICON_TYPE_FONT)
    return 1;

  ui_icon_base_get_data(icon, &data);
  if (!data || strcmp(data, "home") != 0)
    return 1;

  /* Override with SVG path */
  if (ui_icon_base_set_svg_path(NULL, "M0 0 L10 10 Z") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_icon_base_set_svg_path(icon, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_icon_base_set_svg_path(icon, "M0 0 L10 10 Z");
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_icon_base_get_type(icon, &type);
  if (type != UI_ICON_TYPE_SVG_PATH)
    return 1;

  ui_icon_base_get_data(icon, &data);
  if (!data || strcmp(data, "M0 0 L10 10 Z") != 0)
    return 1;

  /* Free logic when overriding */
  rc = ui_icon_base_set_svg_path(icon, "M0 0");
  if (rc != UI_ERROR_NONE)
    return 1;
  ui_icon_base_get_data(icon, &data);
  if (!data || strcmp(data, "M0 0") != 0)
    return 1;

  if (ui_icon_base_bind_name(NULL, (struct ui_signal *)1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_icon_base_bind_name(icon, (struct ui_signal *)1) != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_icon_base_destroy(icon);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_icon_base_destroy(NULL); /* Should be safe */

  /* Test destroy with NULL data */
  {
    struct ui_icon_base *fresh_icon = NULL;
    ui_icon_base_create(&fresh_icon);
    {
      ui_error_t rc_cleanup = ui_icon_base_destroy(fresh_icon);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  /* Test set_svg_path with NULL data */
  {
    struct ui_icon_base *fresh_icon2 = NULL;
    ui_icon_base_create(&fresh_icon2);
    ui_icon_base_set_svg_path(fresh_icon2, "M10 10");
    {
      ui_error_t rc_cleanup = ui_icon_base_destroy(fresh_icon2);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  return 0;
}

static int run_oom_tests(void) {
  struct ui_icon_base *icon = NULL;
  ui_error_t rc;
  struct ui_font dummy_font;

  printf("Testing OOM on create...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_icon_base_create(&icon);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  rc = ui_icon_base_create(&icon);
  if (rc != UI_ERROR_NONE)
    return 1;

  printf("Testing OOM on set_font_glyph...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_icon_base_set_font_glyph(icon, &dummy_font, "star");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    {
      ui_error_t rc_cleanup = ui_icon_base_destroy(icon);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    return 1;
  }

  /* Trigger UI_ERROR_UNKNOWN via mocked null copy */
#if defined(UI_TEST_MOCK_ALLOC)
  /* We can't easily mock strncpy right here without exposing internals or
     adding macros, but we can test the `icon->data` true path which is missing
   */
#endif
  ui_icon_base_set_font_glyph(icon, &dummy_font, "icon1");
  ui_icon_base_set_font_glyph(icon, &dummy_font,
                              "icon2"); /* Hits icon->data true path */

  ui_icon_base_set_svg_path(icon, "M0 0");
  ui_icon_base_set_svg_path(icon, "M1 1"); /* Hits icon->data true path */
  ui_icon_base_set_svg_path(icon, "M2 2"); /* Extra hit */

  printf("Testing OOM on set_svg_path...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_icon_base_set_svg_path(icon, "M1 1");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    {
      ui_error_t rc_cleanup = ui_icon_base_destroy(icon);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    return 1;
  }

  {
    ui_error_t rc_cleanup = ui_icon_base_destroy(icon);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All test_ui_icon_base passed.\n");
  return 0;
}
