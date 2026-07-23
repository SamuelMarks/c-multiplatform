/* clang-format off */
#include <stdio.h>
#include "../include/ui_effects_shaders.h"
#include "../include/ui_shader_manager.h"
#include "../include/ui_error.h"
/* clang-format on */

int main(void) {
  struct ui_shader_manager *manager = NULL;
  enum ui_error rc;
  unsigned int program_id_h;
  unsigned int program_id_v;

  rc = ui_shader_manager_create(&manager);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create shader manager.\n");
    return 1;
  }

  rc = ui_effects_shaders_get_blur_h_program(manager, &program_id_h);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to get blur_h program.\n");
    return 1;
  }

  rc = ui_effects_shaders_get_blur_v_program(manager, &program_id_v);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to get blur_v program.\n");
    return 1;
  }

  /* Test error paths */
  if (ui_effects_shaders_get_blur_h_program(NULL, &program_id_h) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_effects_shaders_get_blur_h_program(manager, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_effects_shaders_get_blur_v_program(NULL, &program_id_v) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_effects_shaders_get_blur_v_program(manager, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test Blend Shaders */
  {
    unsigned int blend_prog;
    enum ui_css_blend_mode modes[] = {
        UI_CSS_BLEND_MODE_NORMAL,      UI_CSS_BLEND_MODE_MULTIPLY,
        UI_CSS_BLEND_MODE_SCREEN,      UI_CSS_BLEND_MODE_OVERLAY,
        UI_CSS_BLEND_MODE_DARKEN,      UI_CSS_BLEND_MODE_LIGHTEN,
        UI_CSS_BLEND_MODE_COLOR_DODGE, UI_CSS_BLEND_MODE_COLOR_BURN,
        UI_CSS_BLEND_MODE_HARD_LIGHT,  UI_CSS_BLEND_MODE_SOFT_LIGHT,
        UI_CSS_BLEND_MODE_DIFFERENCE,  UI_CSS_BLEND_MODE_EXCLUSION,
        UI_CSS_BLEND_MODE_HUE,         UI_CSS_BLEND_MODE_SATURATION,
        UI_CSS_BLEND_MODE_COLOR,       UI_CSS_BLEND_MODE_LUMINOSITY,
        (enum ui_css_blend_mode)999 /* Invalid/Default */
    };
    int i;
    int num_modes = sizeof(modes) / sizeof(modes[0]);

    for (i = 0; i < num_modes; i++) {
      rc = ui_effects_shaders_get_blend_program(manager, modes[i], &blend_prog);
      if (rc != UI_ERROR_NONE) {
        printf("Failed to get blend program for mode %d.\n", modes[i]);
        return 1;
      }
    }

    /* Error paths */
    if (ui_effects_shaders_get_blend_program(NULL, UI_CSS_BLEND_MODE_MULTIPLY,
                                             &blend_prog) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_effects_shaders_get_blend_program(manager,
                                             UI_CSS_BLEND_MODE_MULTIPLY,
                                             NULL) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  /* Test Filter Shaders (Invert, Grayscale, Sepia) */
  {
    unsigned int prog;
    rc = ui_effects_shaders_get_color_filter_program(
        manager, UI_CSS_FILTER_INVERT, &prog);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to get invert program.\n");
      return 1;
    }
    rc = ui_effects_shaders_get_color_filter_program(
        manager, UI_CSS_FILTER_GRAYSCALE, &prog);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to get grayscale program.\n");
      return 1;
    }
    rc = ui_effects_shaders_get_color_filter_program(
        manager, UI_CSS_FILTER_SEPIA, &prog);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to get sepia program.\n");
      return 1;
    }

    if (ui_effects_shaders_get_color_filter_program(
            NULL, UI_CSS_FILTER_INVERT, &prog) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_effects_shaders_get_color_filter_program(
            manager, UI_CSS_FILTER_NONE, &prog) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_effects_shaders_get_color_filter_program(
            manager, UI_CSS_FILTER_INVERT, NULL) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  /* Test Drop Shadow Shader */
  {
    unsigned int prog;
    rc = ui_effects_shaders_get_drop_shadow_program(manager, &prog);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to get drop shadow program.\n");
      return 1;
    }
    if (ui_effects_shaders_get_drop_shadow_program(manager, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_effects_shaders_get_drop_shadow_program(NULL, &prog) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  rc = ui_shader_manager_destroy(manager);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy shader manager.\n");
    return 1;
  }

  printf("ui_effects_shaders tests passed.\n");
  return 0;
}
