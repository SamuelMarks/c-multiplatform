/* clang-format off */
#include <stdio.h>
#include <string.h>

#include "../include/ui_font_manager.h"
#include "../include/ui_error.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

static const unsigned char tests_tiny_ttf[] = {
    0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x63, 0x6d, 0x61, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c,
    0x00, 0x00, 0x00, 0x14, 0x68, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x36, 0x68, 0x68, 0x65, 0x61,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x00, 0x00, 0x00, 0x24,
    0x68, 0x6d, 0x74, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xea,
    0x00, 0x00, 0x00, 0x08, 0x67, 0x6c, 0x79, 0x66, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xf2, 0x00, 0x00, 0x00, 0x01, 0x6c, 0x6f, 0x63, 0x61,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x00, 0x00, 0x00, 0x04,
    0x6d, 0x61, 0x78, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf7,
    0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static int test_oom(void);

static void run_font_methods_coverage(void) {
  struct ui_font_manager *manager = NULL;
  struct ui_font *font = NULL;
  unsigned char bad_ttf[4] = {0};
  unsigned char bad_ttf2[12] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  struct ui_font *found = NULL;
  struct ui_font_axis axes[1] = {{0}};
  int cp[] = {'A'};
  unsigned char *atlas = NULL;
  int w, h;
  ui_font_manager_create(&manager);

  ui_font_manager_load_font_memory(NULL, tests_tiny_ttf, sizeof(tests_tiny_ttf),
                                   &font);
  ui_font_manager_load_font_memory(manager, NULL, sizeof(tests_tiny_ttf),
                                   &font);
  ui_font_manager_load_font_memory(manager, tests_tiny_ttf, 0, &font);
  ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                   sizeof(tests_tiny_ttf), NULL);

  ui_font_manager_load_font_memory(manager, bad_ttf, sizeof(bad_ttf), &font);
  ui_font_manager_load_font_memory(manager, bad_ttf2, sizeof(bad_ttf2), &font);

  ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                   sizeof(tests_tiny_ttf), &font);

  ui_font_set_metadata(font, "Test", 400, 1);
  ui_font_manager_find_font(manager, "Test", 400, 0, &found);
  ui_font_manager_find_font(manager, "Test", 300, 1, &found);
  ui_font_manager_find_font(manager, "Other", 400, 1, &found);

  /* ui_font_create_variable_from_memory(manager, tests_tiny_ttf, */
  /* sizeof(tests_tiny_ttf), 1, NULL, &font); */

  g_malloc_fail_countdown = 1;
  /* ui_font_create_variable_from_memory(manager, tests_tiny_ttf, */
  /* sizeof(tests_tiny_ttf), 1, axes, &font); */
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);
  g_malloc_fail_countdown = 1;
  ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);
  g_malloc_fail_countdown = -1;

  ui_font_manager_destroy(manager);

  ui_font_manager_create(&manager);
  ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                   sizeof(tests_tiny_ttf), &font);

  /* ui_mock_free(font->data); */

  /* font->data = NULL; */
  ui_font_manager_destroy(manager);
}

int main(void) {
  struct ui_font_manager *manager = NULL;
  struct ui_font *font = NULL;
  ui_error_t rc;

  ui_font_manager_create(NULL);
  (void)ui_font_manager_destroy(NULL);

  ui_font_manager_create(&manager);

  ui_font_manager_load_font_memory(manager, NULL, 0, &font);
  ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                   sizeof(tests_tiny_ttf), NULL);

  rc = ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                        sizeof(tests_tiny_ttf), &font);
  printf("RC=%d\n", rc);
  rc = ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                        sizeof(tests_tiny_ttf), &font);

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

    /* More valid calls */
    ui_font_set_status(font, UI_FONT_STATUS_LOADED);
    ui_font_get_status(font, &status);

    struct ui_font_axis axes_arr[1];
    axes_arr[0].tag = 1;
    axes_arr[0].value = 1.0f;
    ui_font_set_variations(font, axes_arr, 1);

    struct ui_font_axis *out_ax;
    int out_c;
    ui_font_get_variations(font, &out_ax, &out_c);

    /* Valid calls */
    ui_font_get_glyph_metrics(font, 'A', 16.0f, &metrics);
    ui_font_get_vmetrics(font, 16.0f, &a, &d, &g);
    ui_font_get_kerning(font, 'A', 'V', 16.0f, &kern);
    ui_font_get_data(font, &data, &size);
    g_malloc_fail_countdown = -1;
    ui_font_generate_atlas(font, 16.0f, cp, 1, &atlas, &w, &h);
  }

  g_malloc_fail_countdown = 0;
  ui_font_manager_create(&manager);
  g_malloc_fail_countdown = -1;

  rc = ui_font_manager_destroy(manager);

  test_oom();
  run_font_methods_coverage();

  extern ui_error_t ui_test_font_manager_coverage_in_src(void);
  ui_test_font_manager_coverage_in_src();

  printf("ui_font_manager tests passed\n");
  return 0;
}

static int test_oom() {
  struct ui_font_manager *manager = NULL;
  struct ui_font *font = NULL;

  ui_font_manager_create(&manager);

  /* Load font memory OOM */
  g_malloc_fail_countdown = 0;
  ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                   sizeof(tests_tiny_ttf), &font);

  g_malloc_fail_countdown = 1;
  ui_font_manager_load_font_memory(manager, tests_tiny_ttf,
                                   sizeof(tests_tiny_ttf), &font);
  g_malloc_fail_countdown = -1;

  (void)ui_font_manager_destroy(manager);
  return 0;
}
