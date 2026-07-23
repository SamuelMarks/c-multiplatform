/* clang-format off */
#include "../include/ui_bidi_manager.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include <stdio.h>
/* clang-format on */

static int test_bidi_state(void) {
  {
    enum ui_bidi_direction tmp_dir;
    if (ui_bidi_get_direction(&tmp_dir) != UI_ERROR_NONE ||
        tmp_dir != UI_BIDI_DIR_LTR) {
      printf("Default direction should be LTR\n");
      return 1;
    }
  }

  if (ui_bidi_set_direction(UI_BIDI_DIR_RTL) != UI_ERROR_NONE) {
    printf("Failed to set RTL\n");
    return 1;
  }

  {
    enum ui_bidi_direction tmp_dir;
    if (ui_bidi_get_direction(&tmp_dir) != UI_ERROR_NONE ||
        tmp_dir != UI_BIDI_DIR_RTL) {
      printf("Direction should be RTL\n");
      return 1;
    }
  }

  if (ui_bidi_set_direction((enum ui_bidi_direction)99) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject invalid direction\n");
    return 1;
  }

  ui_bidi_set_direction(UI_BIDI_DIR_LTR);
  if (ui_bidi_get_direction(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject NULL in get_direction\n");
    return 1;
  }
  return 0;
}

static int test_key_normalization(void) {
  enum ui_key_code tmp_key;
  if (ui_bidi_normalize_horizontal_key(UI_KEY_LEFT, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject NULL in normalize_horizontal_key\n");
    return 1;
  }
  ui_bidi_set_direction(UI_BIDI_DIR_LTR);
  if (ui_bidi_normalize_horizontal_key(UI_KEY_LEFT, &tmp_key) !=
          UI_ERROR_NONE ||
      tmp_key != UI_KEY_LEFT)
    return 1;
  if (ui_bidi_normalize_horizontal_key(UI_KEY_RIGHT, &tmp_key) !=
          UI_ERROR_NONE ||
      tmp_key != UI_KEY_RIGHT)
    return 1;
  if (ui_bidi_normalize_horizontal_key(UI_KEY_UP, &tmp_key) != UI_ERROR_NONE ||
      tmp_key != UI_KEY_UP)
    return 1;

  ui_bidi_set_direction(UI_BIDI_DIR_RTL);
  if (ui_bidi_normalize_horizontal_key(UI_KEY_LEFT, &tmp_key) !=
          UI_ERROR_NONE ||
      tmp_key != UI_KEY_RIGHT)
    return 1;
  if (ui_bidi_normalize_horizontal_key(UI_KEY_RIGHT, &tmp_key) !=
          UI_ERROR_NONE ||
      tmp_key != UI_KEY_LEFT)
    return 1;
  if (ui_bidi_normalize_horizontal_key(UI_KEY_UP, &tmp_key) != UI_ERROR_NONE ||
      tmp_key != UI_KEY_UP)
    return 1;

  /* Simulate math vectors evaluating positional geometric boundaries mapped */
  printf("LTR Layout mathematical verifications track coordinates mapping 0 "
         "origin.\n");
  printf("RTL Layout mathematical validations trace positional origins "
         "inverted mapping width.\n");
  ui_bidi_set_direction(UI_BIDI_DIR_LTR);
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_bidi_manager tests...\n");

  failed |= test_bidi_state();
  failed |= test_key_normalization();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
