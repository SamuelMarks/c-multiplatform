/**
 * @file test_screenshot_rasterizer.c
 * @brief Unit tests for screenshot rasterizer anti-aliasing, glyph coverage,
 * text measurement, and PNG headers.
 */

/* clang-format off */
#include "greatest.h"
#include "rasterizer.h"
#include "ui_error.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Tests glyph rasterization coverage calculations and bitmap blending.
 */
TEST test_rasterizer_glyph_coverage(void) {
  struct canvas *c = NULL;
  ui_error_t rc;
  ui_color_t text_col;
  ui_color_t bg_col;
  int w = 80;
  int h = 40;
  int x;
  int y;
  int non_bg_pixels = 0;

  bg_col = UI_COLOR_ARGB(255, 255, 255, 255);
  text_col = UI_COLOR_ARGB(255, 0, 0, 0);

  rc = canvas_create(w, h, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, c);

  rc = canvas_clear(c, bg_col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Draw text and verify anti-aliasing / pixel coverage */
  rc = draw_text(c, 5.0f, 5.0f, "Ag", 1.0f, text_col);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (y = 0; y < h; ++y) {
    for (x = 0; x < w; ++x) {
      ui_color_t pixel = c->pixels[y * w + x];
      if (pixel != bg_col) {
        non_bg_pixels++;
      }
    }
  }

  /* Verify that glyph pixels were drawn */
  ASSERT(non_bg_pixels > 0);

  rc = canvas_destroy(c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

/**
 * @brief Tests text width measurement accuracy against layout expectations.
 */
TEST test_rasterizer_text_measurement(void) {
  float w1 = 0.0f;
  float w2 = 0.0f;
  float w_empty = 0.0f;
  ui_error_t rc;

  /* Invalid argument checks */
  rc = measure_text_width(NULL, 1.0f, &w1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = measure_text_width("Test", 1.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Empty string measurement */
  rc = measure_text_width("", 1.0f, &w_empty);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(0.0f, w_empty, "%f");

  /* Proportional length comparison */
  rc = measure_text_width("A", 1.0f, &w1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w1 > 0.0f);

  rc = measure_text_width("AAAA", 1.0f, &w2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w2 > w1);

  /* Scaling factor check */
  rc = measure_text_width("A", 2.0f, &w2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w2 > w1);

  PASS();
}

/**
 * @brief Tests PNG encoding, file header, dimensions, and color profiles.
 */
TEST test_rasterizer_png_headers_and_dimensions(void) {
  struct canvas *c = NULL;
  ui_error_t rc;
  const char *test_png = "test_rasterizer_dimensions.png";
  FILE *f = NULL;
  unsigned char header[8];
  unsigned char ihdr[25];
  size_t read_bytes;
  int png_w;
  int png_h;
  unsigned char bit_depth;
  unsigned char color_type;

  rc = canvas_create(64, 32, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = canvas_clear(c, UI_COLOR_ARGB(255, 30, 60, 90));
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = save_canvas_png(c, test_png);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  f = fopen(test_png, "rb");
  ASSERT_NEQ(NULL, f);

  /* Read 8-byte PNG magic header */
  read_bytes = fread(header, 1, 8, f);
  ASSERT_EQ((size_t)8, read_bytes);
  ASSERT_EQ((unsigned char)0x89, header[0]);
  ASSERT_EQ((unsigned char)'P', header[1]);
  ASSERT_EQ((unsigned char)'N', header[2]);
  ASSERT_EQ((unsigned char)'G', header[3]);
  ASSERT_EQ((unsigned char)0x0D, header[4]);
  ASSERT_EQ((unsigned char)0x0A, header[5]);
  ASSERT_EQ((unsigned char)0x1A, header[6]);
  ASSERT_EQ((unsigned char)0x0A, header[7]);

  /* Read IHDR chunk */
  read_bytes = fread(ihdr, 1, 25, f);
  ASSERT_EQ((size_t)25, read_bytes);

  /* Chunk type should be "IHDR" */
  ASSERT_EQ((unsigned char)'I', ihdr[4]);
  ASSERT_EQ((unsigned char)'H', ihdr[5]);
  ASSERT_EQ((unsigned char)'D', ihdr[6]);
  ASSERT_EQ((unsigned char)'R', ihdr[7]);

  /* Big-endian width and height */
  png_w = (ihdr[8] << 24) | (ihdr[9] << 16) | (ihdr[10] << 8) | ihdr[11];
  png_h = (ihdr[12] << 24) | (ihdr[13] << 16) | (ihdr[14] << 8) | ihdr[15];
  ASSERT_EQ(64, png_w);
  ASSERT_EQ(32, png_h);

  /* Bit depth = 8, Color type = 6 (RGBA) */
  bit_depth = ihdr[16];
  color_type = ihdr[17];
  ASSERT_EQ((unsigned char)8, bit_depth);
  ASSERT_EQ((unsigned char)6, color_type);

  fclose(f);
  remove(test_png);

  rc = canvas_destroy(c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

/**
 * @brief Tests OOM injection across canvas creation, font loading, and
 * rasterization.
 */
TEST test_rasterizer_oom_injection(void) {
  struct canvas *c = NULL;
  ui_error_t rc;
  ui_color_t col;

  col = UI_COLOR_ARGB(255, 10, 20, 30);

  /* OOM during canvas creation */
  g_malloc_fail_countdown = 0;
  rc = canvas_create(100, 100, &c);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, c);

  g_malloc_fail_countdown = 1;
  rc = canvas_create(100, 100, &c);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, c);
  g_malloc_fail_countdown = -1;

  /* Successful creation */
  rc = canvas_create(100, 100, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, c);

  /* Test font loading OOM if minimal font exists */
  g_malloc_fail_countdown = 0;
  rc = rasterizer_load_font("tests/minimal.ttf");
  if (rc != UI_ERROR_NOT_FOUND) {
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = -1;

  /* If minimal.ttf loads, test glyph rasterization OOM */
  if (rasterizer_load_font("tests/minimal.ttf") == UI_ERROR_NONE) {
    g_malloc_fail_countdown = 0;
    rc = draw_text(c, 10.0f, 10.0f, "A", 1.0f, col);
    /* In rasterizer, if glyph buffer malloc fails, it skips drawing without
     * crashing */
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_malloc_fail_countdown = -1;
  }

  rc = canvas_destroy(c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

/**
 * @brief Greatest test suite definition for screenshot rasterizer.
 */
SUITE(screenshot_rasterizer_suite) {
  RUN_TEST(test_rasterizer_glyph_coverage);
  RUN_TEST(test_rasterizer_text_measurement);
  RUN_TEST(test_rasterizer_png_headers_and_dimensions);
  RUN_TEST(test_rasterizer_oom_injection);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(screenshot_rasterizer_suite);
  GREATEST_MAIN_END();
}
