/* clang-format off */
#include "ui_tonal_palette.h"
/* clang-format on */

enum ui_error
ui_tonal_palette_from_color(ui_color_t argb,
                            struct ui_tonal_palette *out_palette) {
  struct ui_color_hct hct;
  enum ui_error err;

  if (!out_palette) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_color_argb_to_hct(argb, &hct);
  (void)err; /* ui_color_argb_to_hct only fails on NULL out_hct, which is
                impossible here */

  out_palette->hue = hct.hue;
  out_palette->chroma = hct.chroma;

  return UI_ERROR_NONE;
}

enum ui_error ui_tonal_palette_get_tone(const struct ui_tonal_palette *palette,
                                        float tone, ui_color_t *out_color) {
  struct ui_color_hct hct;

  if (!palette || !out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tone <= 0.0f) {
    *out_color = UI_COLOR_ARGB(255, 0, 0, 0);
    return UI_ERROR_NONE;
  }

  if (tone >= 100.0f) {
    *out_color = UI_COLOR_ARGB(255, 255, 255, 255);
    return UI_ERROR_NONE;
  }

  hct.hue = palette->hue;
  hct.chroma = palette->chroma;
  hct.tone = tone;

  return ui_color_hct_to_argb(&hct, out_color);
}
