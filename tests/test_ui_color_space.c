/* clang-format off */
#include "ui_color_space.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

static ui_error_t test_argb_to_hct(void) {
  ui_color_t red = UI_COLOR_ARGB(255, 255, 0, 0);
  struct ui_color_hct hct;
  ui_color_t back;
  ui_error_t err;

  err = ui_color_argb_to_hct(red, &hct);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_color_argb_to_hct failed\n");
    exit(1);
    return UI_ERROR_NONE;
  }

  err = ui_color_hct_to_argb(&hct, &back);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_color_hct_to_argb failed\n");
    exit(1);
  }

  /* Since RGB -> XYZ -> LAB -> RGB can have slight precision loss, just check
   * approximately */
  if (UI_COLOR_RED(back) < 250 || UI_COLOR_GREEN(back) > 5 ||
      UI_COLOR_BLUE(back) > 5) {
    fprintf(stderr, "Color conversion roundtrip failed: %08X vs %08X\n", red,
            back);
    exit(1);
  }

  return UI_ERROR_NONE;
}

static ui_error_t test_argb_to_cam16(void) {
  ui_color_t green = UI_COLOR_ARGB(255, 0, 255, 0);
  struct ui_color_cam16 cam16;
  ui_color_t back;
  ui_error_t err;

  err = ui_color_argb_to_cam16(green, &cam16);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_color_argb_to_cam16 failed\n");
    exit(1);
  }

  err = ui_color_cam16_to_argb(&cam16, &back);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_color_cam16_to_argb failed\n");
    exit(1);
  }

  if (UI_COLOR_GREEN(back) < 250 || UI_COLOR_RED(back) > 5 ||
      UI_COLOR_BLUE(back) > 5) {
    fprintf(stderr,
            "Color conversion roundtrip failed for CAM16: %08X vs %08X\n",
            green, back);
    exit(1);
  }
  return UI_ERROR_NONE;
}

static ui_error_t test_edge_cases(void) {
  ui_color_t black =
      UI_COLOR_ARGB(255, 1, 1, 1); /* Near black to hit low value branches */
  struct ui_color_hct hct;
  struct ui_color_cam16 cam16;
  ui_color_t back;

  /* Hit small float bounds in conversions (t <= 216/24389) and RGB linear
   * bounds */
  ui_color_argb_to_hct(black, &hct);
  ui_color_hct_to_argb(&hct, &back);

  ui_color_argb_to_cam16(black, &cam16);
  ui_color_cam16_to_argb(&cam16, &back);

  /* Force negative hue by manually modifying intermediate structures to hit h <
   * 0.0 wrapping */
  /* And also to hit clamping by exceeding 1.0 bounds */
  hct.hue = -50.0f;
  hct.chroma = 150.0f;
  hct.tone = 150.0f;
  ui_color_hct_to_argb(&hct, &back);

  cam16.hue = -50.0f;
  cam16.chroma = 150.0f;
  cam16.j = 150.0f;
  ui_color_cam16_to_argb(&cam16, &back);

  /* Test with a blue color to hit b > 0.04045 branches */
  ui_color_t blue = UI_COLOR_ARGB(255, 0, 0, 255);
  ui_color_argb_to_hct(blue, &hct);
  ui_color_hct_to_argb(&hct, &back);

  ui_color_argb_to_cam16(blue, &cam16);
  ui_color_cam16_to_argb(&cam16, &back);

  /* Invalid arguments */
  if (ui_color_argb_to_hct(black, NULL) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);
  if (ui_color_hct_to_argb(NULL, &back) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);
  if (ui_color_hct_to_argb(&hct, NULL) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);

  if (ui_color_argb_to_cam16(black, NULL) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);
  if (ui_color_cam16_to_argb(NULL, &back) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);
  if (ui_color_cam16_to_argb(&cam16, NULL) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);

  return UI_ERROR_NONE;
}

int main(void) {
  test_argb_to_hct();
  test_argb_to_cam16();
  test_edge_cases();
  printf("test_color_space passed\n");
  return 0;
}
