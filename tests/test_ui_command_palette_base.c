/* clang-format off */
#include <assert.h>
#include <stddef.h>

#include "ui_command_palette_base.h"
#include "ui_error.h"
/* clang-format on */

struct ui_component {
  int id;
};

static ui_error_t test_command_palette_init(void) {
  struct ui_command_palette_base palette;
  struct ui_component comp;
  ui_error_t err;

  err = ui_command_palette_base_init(NULL, &comp, NULL, NULL, NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_command_palette_base_init(&palette, NULL, NULL, NULL, NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_command_palette_base_init(&palette, &comp, NULL, NULL, NULL);
  assert(err == UI_ERROR_NONE);
  assert(palette.component == &comp);
  assert(palette.is_open == 0);
  return UI_ERROR_NONE;
}

static ui_error_t test_command_palette_operations(void) {
  struct ui_command_palette_base palette;
  struct ui_component comp;

  ui_command_palette_base_init(&palette, &comp, NULL, NULL, NULL);

  ui_command_palette_base_open(NULL);
  ui_command_palette_base_open(&palette);
  assert(palette.is_open == 1);

  ui_command_palette_base_close(NULL);
  ui_command_palette_base_close(&palette);
  assert(palette.is_open == 0);

  if (ui_command_palette_base_bind_open(NULL, (struct ui_signal *)1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_command_palette_base_bind_open(&palette, (struct ui_signal *)1) !=
      UI_ERROR_NONE)
    return 1;

  if (ui_command_palette_base_get_animating_signal(
          NULL, (struct ui_computed **)&palette) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_command_palette_base_get_animating_signal(&palette, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    struct ui_computed *animating;
    if (ui_command_palette_base_get_animating_signal(&palette, &animating) !=
        UI_ERROR_NONE)
      return 1;
  }

  return UI_ERROR_NONE;
}

int main(void) {
  test_command_palette_init();
  test_command_palette_operations();
  return 0;
}
