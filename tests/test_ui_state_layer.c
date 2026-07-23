/* clang-format off */
#include "ui_state_layer.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

static enum ui_error test_state_layer(void) {
  ui_color_t surface = UI_COLOR_ARGB(255, 255, 255, 255); /* White */
  ui_color_t on_surface = UI_COLOR_ARGB(255, 0, 0, 0);    /* Black */
  ui_color_t result;
  enum ui_error err;

  err = ui_state_layer_get_color(surface, on_surface, UI_STATE_LAYER_HOVER,
                                 &result);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_state_layer_get_color HOVER failed\n");
    exit(1);
    return UI_ERROR_NONE;
  }

  /* 8% black on white -> roughly rgb(235, 235, 235) */
  if (UI_COLOR_RED(result) > 240 || UI_COLOR_RED(result) < 230) {
    fprintf(stderr, "HOVER color math is wrong: %08X\n", result);
    exit(1);
  }

  err = ui_state_layer_get_color(surface, on_surface, UI_STATE_LAYER_PRESSED,
                                 &result);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_state_layer_get_color PRESSED failed\n");
    exit(1);
  }

  /* 12% black on white -> roughly rgb(224, 224, 224) */
  if (UI_COLOR_RED(result) > 230 || UI_COLOR_RED(result) < 220) {
    fprintf(stderr, "PRESSED color math is wrong: %08X\n", result);
    exit(1);
  }

  err = ui_state_layer_get_color(surface, on_surface, UI_STATE_LAYER_DRAGGED,
                                 &result);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_state_layer_get_color DRAGGED failed\n");
    exit(1);
  }

  err =
      ui_state_layer_get_color(surface, on_surface, UI_STATE_LAYER_HOVER, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "NULL out_color did not fail\n");
    exit(1);
  }

  err = ui_state_layer_get_color(surface, on_surface,
                                 (enum ui_state_layer_type)999, &result);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Invalid state did not fail\n");
    exit(1);
  }
}

int main(void) {
  test_state_layer();
  printf("test_ui_state_layer passed\n");
  return 0;
}
