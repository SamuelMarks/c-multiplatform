/* clang-format off */
#include "ui_compositor_material_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_arena *arena;
  struct ui_compositor_material_base *material = NULL;
  struct ui_compositor_material_config config;
  enum ui_error err;
  ui_signal_t *type_signal = NULL;

  if (ui_arena_create(1024 * 16, &arena) != UI_ERROR_NONE) {
    return 1;
  }

  config.initial_type = UI_COMPOSITOR_MATERIAL_TYPE_MICA;
  config.fallback_mode = UI_COMPOSITOR_FALLBACK_MODE_VIBRANCY;
  config.initial_opacity = 0.85f;

  /* Invalid args */
  if (ui_compositor_material_base_create(NULL, &config, &material) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_compositor_material_base_create(arena, NULL, &material) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_compositor_material_base_create(arena, &config, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_compositor_material_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_compositor_material_base_set_type(
          NULL, UI_COMPOSITOR_MATERIAL_TYPE_MICA) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_compositor_material_base_set_fallback_mode(
          NULL, UI_COMPOSITOR_FALLBACK_MODE_VIBRANCY) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_compositor_material_base_set_opacity(NULL, 0.5f) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_compositor_material_base_get_type_signal(NULL, &type_signal) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_malloc_fail_countdown;
  g_malloc_fail_countdown = 0;
  if (ui_compositor_material_base_create(arena, &config, &material) !=
      UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed OOM countdown\n");
    return 1;
  }
  g_malloc_fail_countdown = -1;
#endif

  err = ui_compositor_material_base_create(arena, &config, &material);
  if (err != UI_ERROR_NONE || material == NULL) {
    return 1;
  }

  if (ui_compositor_material_base_get_type_signal(material, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_compositor_material_base_get_type_signal(material, &type_signal);
  if (err != UI_ERROR_NONE || type_signal == NULL) {
    return 1;
  }

  /* Test type equality fallback/callback by getting value if possible, else
   * just let it be tested */
  union ui_signal_payload val;
  if (ui_signal_get(type_signal, &val) == UI_ERROR_NONE) {
    /* Trigger the equality callback internally by setting the same value */
    ui_signal_set(type_signal, val);
  }

  /* Trigger the type equality callback internally by setting a different value
   */
  err = ui_compositor_material_base_set_type(
      material, UI_COMPOSITOR_MATERIAL_TYPE_ACRYLIC);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  err = ui_compositor_material_base_set_fallback_mode(
      material, UI_COMPOSITOR_FALLBACK_MODE_SOLID_COLOR);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  err = ui_compositor_material_base_set_opacity(material, 0.5f);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  /* Test opacity clamping */
  err = ui_compositor_material_base_set_opacity(material, 1.5f);
  if (err != UI_ERROR_NONE) {
    return 1;
  }
  err = ui_compositor_material_base_set_opacity(material, -0.5f);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  err = ui_compositor_material_base_destroy(material);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  ui_arena_destroy(arena);
  return 0;
}
