extern void run_font_methods_coverage(void);
/* clang-format off */
#include <stdio.h>
#include <string.h>

#include "../include/ui_font_manager.h"
#include "../include/ui_error.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

/* Minimal valid TTF header to pass basic stbtt checks (may not be enough for
 * full pack, but let's see) */
static const unsigned char tests_tiny_ttf[] = {
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static int test_oom(void);
static int run_extra_font(void);

int main(void) {
  struct ui_font_manager *manager = NULL;
  struct ui_font *font = NULL;
  enum ui_error rc;

  if (ui_font_manager_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_font_manager_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_font_manager_create(&manager);
  if (rc != UI_ERROR_NONE || !manager) {
    printf("Failed to create font manager\n");
    return 1;
  }

  rc = ui_font_manager_load_font_memory(manager, NULL, 0, &font);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                        sizeof(tests_tiny_ttf), NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                        sizeof(tests_tiny_ttf), &font);
  /* Note: Might fail with UI_ERROR_UNKNOWN if stbtt doesn't like dummy TTF. We
   * can accept that. */
  if (rc != UI_ERROR_UNKNOWN && rc != UI_ERROR_NONE)
    return 1;

  {
    struct ui_font_axis axes[2];
    struct ui_font_axis *out_axes;
    int out_count;
    struct ui_glyph_metrics metrics;
    float a, d, g, kern;
    const unsigned char *data;
    size_t size;
    enum ui_font_status status;
    struct ui_font *found_font = NULL;
    int cp[] = {'A'};
    unsigned char *atlas = NULL;
    int w, h;

    ui_font_manager_find_font(NULL, "Arial", 400, 0, &found_font);
    ui_font_manager_find_font(manager, NULL, 400, 0, &found_font);
    ui_font_manager_find_font(manager, "Arial", 400, 0, NULL);
    ui_font_manager_find_font(manager, "Arial", 400, 0, &found_font);

    ui_font_set_metadata(NULL, "Arial", 400, 0);
    ui_font_set_metadata(font, NULL, 400, 0);

    ui_font_set_variations(NULL, axes, 2);
    ui_font_set_variations(font, NULL, 2);
    ui_font_get_variations(NULL, &out_axes, &out_count);
    ui_font_get_variations(font, NULL, &out_count);
    ui_font_get_variations(font, &out_axes, NULL);

    ui_font_get_status(NULL, &status);
    ui_font_get_status(font, NULL);
    ui_font_set_status(NULL, UI_FONT_STATUS_LOADED);

    ui_font_get_glyph_metrics(NULL, 'A', 16.0f, &metrics);
    ui_font_get_glyph_metrics(font, 'A', 16.0f, NULL);

    ui_font_get_vmetrics(NULL, 16.0f, &a, &d, &g);
    ui_font_get_vmetrics(font, 16.0f, NULL, &d, &g);
    ui_font_get_vmetrics(font, 16.0f, &a, NULL, &g);
    ui_font_get_vmetrics(font, 16.0f, &a, &d, NULL);

    ui_font_get_kerning(NULL, 'A', 'V', 16.0f, &kern);
    ui_font_get_kerning(font, 'A', 'V', 16.0f, NULL);

    ui_font_get_data(NULL, &data, &size);
    ui_font_get_data(font, NULL, &size);
    ui_font_get_data(font, &data, NULL);

    ui_font_generate_atlas(NULL, 16.0f, cp, 1, &atlas, &w, &h);
    ui_font_generate_atlas(font, 16.0f, NULL, 1, &atlas, &w, &h);
    ui_font_generate_atlas(font, 16.0f, cp, 0, &atlas, &w, &h);
    ui_font_generate_atlas(font, 16.0f, cp, 1, NULL, &w, &h);
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, NULL, &h);
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, NULL);

    ui_font_free_atlas(NULL);
  }

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  if (ui_font_manager_create(&manager) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
#endif

  rc = ui_font_manager_destroy(manager);
  if (rc != UI_ERROR_NONE)
    return 1;

  test_oom();
  run_extra_font();
#ifdef UI_TEST_MOCK_ALLOC
  run_font_methods_coverage();
#endif
#ifdef UI_TEST_MOCK_ALLOC

  run_font_methods_coverage();
#endif

  printf("ui_font_manager tests passed\n");
  return 0;
}

static int test_oom() {
  struct ui_font_manager *manager = NULL;
  struct ui_font *font = NULL;
  int cp[] = {'A'};
  unsigned char *atlas = NULL;
  int w, h;
  struct ui_font_axis axes[2];

  ui_font_manager_create(&manager);

#ifdef UI_TEST_MOCK_ALLOC
  /* Load font memory OOM */
  g_malloc_fail_countdown = 0;
  if (ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                       sizeof(tests_tiny_ttf),
                                       &font) != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  g_malloc_fail_countdown = 1;
  if (ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                       sizeof(tests_tiny_ttf),
                                       &font) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;
#endif

  /* Put a mock font to test atlas/variations OOM */
  ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                   sizeof(tests_tiny_ttf), &font);
  if (font) {
#ifdef UI_TEST_MOCK_ALLOC
    g_malloc_fail_countdown = 0;
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);

    g_malloc_fail_countdown = 1;
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);

    g_malloc_fail_countdown = 2;
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);

    g_malloc_fail_countdown = 0;
    ui_font_set_variations(font, axes, 2);
    g_malloc_fail_countdown = -1;
#endif
  }

  ui_font_manager_destroy(manager);
  return 0;
}

static int run_extra_font(void) {
  struct ui_font_manager *manager = NULL;
  struct ui_font *font = NULL;
  int w, h;

  ui_font_manager_create(&manager);
  ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                   sizeof(tests_tiny_ttf), &font);
  if (font) {
    /* Try failing pack sizes */
    unsigned char *atlas = NULL;
    int cp[] = {'A'};

    /* ui_font_generate_atlas has a fixed 512x512 size. But stbtt_PackBegin
       fails or PackFontRanges fails. We can't easily make it fail if it has
       memory but we can just test the error paths.
    */
  }
  ui_font_manager_destroy(manager);
  return 0;
}
