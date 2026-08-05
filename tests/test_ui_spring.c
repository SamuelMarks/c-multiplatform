/* clang-format off */
#include "../include/ui_spring.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_spring_config config;
  struct ui_spring_state state;
  struct ui_spring_state next_state;
  ui_error_t err;

  config.damping = 10.0f;
  config.stiffness = 100.0f;
  config.mass = 1.0f;

  state.value = 0.0f;
  state.velocity = 0.0f;

  err = ui_spring_update(&config, &state, 100.0f, 0.016f, &next_state);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  /* invalid args */
  err = ui_spring_update(NULL, &state, 100.0f, 0.016f, &next_state);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  config.mass = 0.0f;
  err = ui_spring_update(&config, &state, 100.0f, 0.016f, &next_state);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  config.mass = 1.0f;

  err = ui_spring_update(&config, &state, 100.0f, 0.0f, &next_state);
  if (err != UI_ERROR_NONE)
    return 1;

  printf("PASS\n");
  return 0;
}
