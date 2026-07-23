/* clang-format off */
#include "ui_focus_ring.h"
/* clang-format on */

enum ui_error ui_focus_ring_get_standard(ui_color_t theme_primary_color,
                                         struct ui_focus_ring *out_ring) {
  if (!out_ring) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_ring->offset = 2.0f; /* 2px outside the element */
  out_ring->width = 2.0f;
  out_ring->color = theme_primary_color;

  /* High visibility inner ring */
  out_ring->inner_width = 1.0f;
  out_ring->inner_color =
      UI_COLOR_ARGB(255, 255, 255, 255); /* White inner border for contrast */

  return UI_ERROR_NONE;
}
