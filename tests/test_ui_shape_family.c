/* clang-format off */
#include "ui_shape_family.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

static enum ui_error test_shape_family(void) {
  float radius;
  enum ui_error err;

  err = ui_shape_family_get_radius(UI_SHAPE_FAMILY_NONE, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    exit(1);
  }

  err = ui_shape_family_get_radius((enum ui_shape_family)999, &radius);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    exit(1);
  }

  err = ui_shape_family_get_radius(UI_SHAPE_FAMILY_NONE, &radius);
  if (err != UI_ERROR_NONE || radius != 0.0f) {
    exit(1);
    return UI_ERROR_NONE;
  }

  err = ui_shape_family_get_radius(UI_SHAPE_FAMILY_EXTRA_SMALL, &radius);
  if (err != UI_ERROR_NONE || radius != 4.0f) {
    exit(1);
  }

  err = ui_shape_family_get_radius(UI_SHAPE_FAMILY_SMALL, &radius);
  if (err != UI_ERROR_NONE || radius != 8.0f) {
    exit(1);
  }

  err = ui_shape_family_get_radius(UI_SHAPE_FAMILY_MEDIUM, &radius);
  if (err != UI_ERROR_NONE || radius != 12.0f) {
    exit(1);
  }

  err = ui_shape_family_get_radius(UI_SHAPE_FAMILY_LARGE, &radius);
  if (err != UI_ERROR_NONE || radius != 16.0f) {
    exit(1);
  }

  err = ui_shape_family_get_radius(UI_SHAPE_FAMILY_EXTRA_LARGE, &radius);
  if (err != UI_ERROR_NONE || radius != 28.0f) {
    exit(1);
  }

  err = ui_shape_family_get_radius(UI_SHAPE_FAMILY_PILL, &radius);
  if (err != UI_ERROR_NONE || radius < 9000.0f) {
    exit(1);
  }

  return UI_ERROR_NONE;
}

int main(void) {
  test_shape_family();
  printf("test_ui_shape_family passed\n");
  return 0;
}
