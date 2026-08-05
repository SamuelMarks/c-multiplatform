/* clang-format off */
#include "ui_elevation.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

static void test_elevation(void) {
  ui_color_t surface = UI_COLOR_ARGB(255, 255, 255, 255); /* White */
  ui_color_t tint = UI_COLOR_ARGB(255, 0, 0, 255);        /* Blue */
  ui_color_t result;
  struct ui_drop_shadow s1, s2;
  ui_error_t err;

  /* Test ui_elevation_get_tinted_surface null pointers */
  err = ui_elevation_get_tinted_surface(surface, tint, UI_ELEVATION_LEVEL_0,
                                        NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_elevation_get_tinted_surface NULL out_color failed\n");
    exit(1);
  }

  /* Test ui_elevation_get_tinted_surface invalid level */
  err = ui_elevation_get_tinted_surface(surface, tint,
                                        (enum ui_elevation_level) - 1, &result);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_elevation_get_tinted_surface negative level failed\n");
    exit(1);
  }

  err = ui_elevation_get_tinted_surface(surface, tint,
                                        (enum ui_elevation_level)6, &result);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_elevation_get_tinted_surface high level failed\n");
    exit(1);
  }

  /* Test ui_elevation_get_tinted_surface all valid levels */
  {
    int i;
    for (i = 0; i <= 5; i++) {
      err = ui_elevation_get_tinted_surface(
          surface, tint, (enum ui_elevation_level)i, &result);
      if (err != UI_ERROR_NONE) {
        fprintf(stderr, "ui_elevation_get_tinted_surface level %d failed\n", i);
        exit(1);
      }
    }
  }

  /* Test ui_elevation_get_shadows null pointers */
  err = ui_elevation_get_shadows(UI_ELEVATION_LEVEL_0,
                                 UI_COLOR_ARGB(255, 0, 0, 0), NULL, &s2);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_elevation_get_shadows NULL s1 failed\n");
    exit(1);
  }

  err = ui_elevation_get_shadows(UI_ELEVATION_LEVEL_0,
                                 UI_COLOR_ARGB(255, 0, 0, 0), &s1, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_elevation_get_shadows NULL s2 failed\n");
    exit(1);
  }

  err = ui_elevation_get_shadows(UI_ELEVATION_LEVEL_0,
                                 UI_COLOR_ARGB(255, 0, 0, 0), NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_elevation_get_shadows NULL s1,s2 failed\n");
    exit(1);
  }

  /* Test ui_elevation_get_shadows invalid level */
  err = ui_elevation_get_shadows((enum ui_elevation_level) - 1,
                                 UI_COLOR_ARGB(255, 0, 0, 0), &s1, &s2);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_elevation_get_shadows negative level failed\n");
    exit(1);
  }

  err = ui_elevation_get_shadows((enum ui_elevation_level)6,
                                 UI_COLOR_ARGB(255, 0, 0, 0), &s1, &s2);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "ui_elevation_get_shadows high level failed\n");
    exit(1);
  }

  /* Test ui_elevation_get_shadows all valid levels */
  {
    int i;
    for (i = 0; i <= 5; i++) {
      err = ui_elevation_get_shadows((enum ui_elevation_level)i,
                                     UI_COLOR_ARGB(255, 0, 0, 0), &s1, &s2);
      if (err != UI_ERROR_NONE) {
        fprintf(stderr, "ui_elevation_get_shadows level %d failed\n", i);
        exit(1);
      }
    }
  }
}

int main(void) {
  test_elevation();
  printf("test_ui_elevation passed\n");
  return 0;
}
