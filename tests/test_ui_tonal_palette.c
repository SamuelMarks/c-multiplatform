/* clang-format off */
#include "ui_tonal_palette.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

static int test_tonal_palette(void) {
  ui_color_t key_color = UI_COLOR_ARGB(255, 0, 0, 255); /* Blue */
  struct ui_tonal_palette palette;
  ui_color_t tone_0, tone_100, tone_50;
  ui_error_t err;
  int failed = 0;

  err = ui_tonal_palette_from_color(key_color, &palette);
  failed |= (err != UI_ERROR_NONE);

  err = ui_tonal_palette_get_tone(&palette, 0.0f, &tone_0);
  failed |= (err != UI_ERROR_NONE);

  err = ui_tonal_palette_get_tone(&palette, 100.0f, &tone_100);
  failed |= (err != UI_ERROR_NONE);

  err = ui_tonal_palette_get_tone(&palette, 50.0f, &tone_50);
  failed |= (err != UI_ERROR_NONE);

  /* Tone 0 should be black */
  failed |= (UI_COLOR_RED(tone_0) > 0 || UI_COLOR_GREEN(tone_0) > 0 ||
             UI_COLOR_BLUE(tone_0) > 0);

  /* Tone 100 should be white */
  failed |= (UI_COLOR_RED(tone_100) < 255 || UI_COLOR_GREEN(tone_100) < 255 ||
             UI_COLOR_BLUE(tone_100) < 255);

  err = ui_tonal_palette_from_color(key_color, NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_tonal_palette_get_tone(NULL, 50.0f, &tone_50);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_tonal_palette_get_tone(&palette, 50.0f, NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_tonal_palette();
  if (!failed) {
    printf("test_ui_tonal_palette passed\n");
  }
  return failed;
}
