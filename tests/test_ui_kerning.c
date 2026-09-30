/**
 * @file test_ui_kerning.c
 * @brief Unit tests for GPOS kerning and bidirectional text shaping.
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/ui_font_manager.h"
#include "../include/ui_text_layout.h"
#include "../include/ui_error.h"
/* clang-format on */

static const unsigned char test_ttf_bytes[] = {
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

/**
 * @brief Tests kerning query and RTL text layout.
 * @return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t test_kerning_and_rtl_layout(void) {
  FILE *f;
  const char *temp_file = "test_font_temp_kern.ttf";
  struct ui_font_manager *manager = NULL;
  struct ui_font *font = NULL;
  struct ui_text_layout *layout = NULL;
  const struct ui_positioned_glyph *glyphs = NULL;
  size_t count = 0;
  float kern = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
  ui_error_t rc;

  /* Write temp font file */
#if defined(_MSC_VER)
  {
    errno_t err = fopen_s(&f, temp_file, "wb");
    if (err != 0 || !f) {
      return UI_ERROR_IO_FAILED;
    }
  }
#else
  f = fopen(temp_file, "wb");
  if (!f) {
    return UI_ERROR_IO_FAILED;
  }
#endif
  fwrite(test_ttf_bytes, 1, sizeof(test_ttf_bytes), f);
  fclose(f);

  rc = ui_font_manager_create(&manager);
  if (rc != UI_ERROR_NONE) {
    remove(temp_file);
    return rc;
  }

  rc = ui_font_manager_load_font_file(manager, temp_file, &font);
  if (rc != UI_ERROR_NONE || !font) {
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return 1;
  }

  /* 1. Test kerning argument validation */
  rc = ui_font_get_kerning(NULL, 'A', 'V', 16.0f, &kern);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return 2;
  }

  rc = ui_font_get_kerning(font, 'A', 'V', 16.0f, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return 3;
  }

  rc = ui_font_get_kerning(font, 'A', 'V', 16.0f, &kern);
  if (rc != UI_ERROR_NONE) {
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return 4;
  }

  /* 2. Test text layout creation and shaping */
  rc = ui_text_layout_create(&layout);
  if (rc != UI_ERROR_NONE) {
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return rc;
  }

  /* LTR text shaping */
  rc = ui_text_layout_shape(layout, font, 16.0f, "AAA", 500.0f,
                            UI_TEXT_DIRECTION_LTR);
  if (rc != UI_ERROR_NONE) {
    ui_text_layout_destroy(layout);
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return 5;
  }

  rc = ui_text_layout_get_bounds(layout, &width, &height);
  if (rc != UI_ERROR_NONE) {
    ui_text_layout_destroy(layout);
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return 6;
  }

  rc = ui_text_layout_get_glyphs(layout, &glyphs, &count);
  if (rc != UI_ERROR_NONE || count == 0 || !glyphs) {
    ui_text_layout_destroy(layout);
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return 7;
  }

  /* RTL text shaping */
  rc = ui_text_layout_shape(layout, font, 16.0f, "AAA\nAAA", 500.0f,
                            UI_TEXT_DIRECTION_RTL);
  if (rc != UI_ERROR_NONE) {
    ui_text_layout_destroy(layout);
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return 8;
  }

  rc = ui_text_layout_get_glyphs(layout, &glyphs, &count);
  if (rc != UI_ERROR_NONE || count == 0 || !glyphs) {
    ui_text_layout_destroy(layout);
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return 9;
  }

  /* Verify first line first glyph in RTL has positive X and is positioned right
   */
  if (glyphs[0].x < 0.0f) {
    ui_text_layout_destroy(layout);
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return 10;
  }

  rc = ui_text_layout_destroy(layout);
  if (rc != UI_ERROR_NONE) {
    ui_font_manager_destroy(manager);
    remove(temp_file);
    return rc;
  }

  rc = ui_font_manager_destroy(manager);
  remove(temp_file);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc;

  rc = test_kerning_and_rtl_layout();
  if (rc != UI_ERROR_NONE) {
    return (int)rc;
  }

  puts("test_ui_kerning passed.");
  return 0;
}
