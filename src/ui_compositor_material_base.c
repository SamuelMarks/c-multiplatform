/**
 * @file ui_compositor_material_base.c
 * @brief ui_compositor_material_base.c implementation.
 */
/* clang-format off */
#include "ui_compositor_material_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct ui_compositor_material_base
 * @struct ui_compositor_material_base
 * @brief Internal representation of a compositor material.
 */
struct ui_compositor_material_base {
  /* @brief Arena for internal allocations. */
  struct ui_arena *arena; /**< arena */
  /* @brief Material type. */
  enum ui_compositor_material_type type; /**< type */
  /* @brief Fallback mode. */
  enum ui_compositor_fallback_mode fallback_mode; /**< fallback_mode */
  /* @brief Opacity level. */
  float opacity; /**< opacity */
  /* @brief Signal emitted on type change. */
  ui_signal_t *type_signal; /**< type_signal */
};

/**
 * @brief type_equality.
 * @param a Parameter a.
 * @param b Parameter b.
 * @param out_equal Parameter out_equal.
 * @return Return value.
 */
static ui_error_t type_equality(union ui_signal_payload a,
                                union ui_signal_payload b,
                                ui_bool_t *out_equal) {
  *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_compositor_material_base_create.
 * @param arena Parameter arena.
 * @param config Parameter config.
 * @param out_material Parameter out_material.
 * @return Return value.
 */
ui_error_t ui_compositor_material_base_create(
    struct ui_arena *arena, const struct ui_compositor_material_config *config,
    struct ui_compositor_material_base **out_material) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !config || !out_material) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_compositor_material_base), 8,
                       &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  *out_material = (struct ui_compositor_material_base *)ptr;
  (*out_material)->arena = arena;
  (*out_material)->type = config->initial_type;
  (*out_material)->fallback_mode = config->fallback_mode;
  (*out_material)->opacity = config->initial_opacity;

  initial_payload.int_val = (ui_int32)config->initial_type;
  {
    (void)ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_INT32,
                           type_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                           &(*out_material)->type_signal);
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_compositor_material_base_destroy.
 * @param material Parameter material.
 * @return Return value.
 */
ui_error_t ui_compositor_material_base_destroy(
    struct ui_compositor_material_base *material) {
  if (!material) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_signal_destroy(material->type_signal);

  return UI_ERROR_NONE;
}

/**
 * @brief ui_compositor_material_base_set_type.
 * @param material Parameter material.
 * @param type Parameter type.
 * @return Return value.
 */
ui_error_t ui_compositor_material_base_set_type(
    struct ui_compositor_material_base *material,
    enum ui_compositor_material_type type) {
  union ui_signal_payload payload;

  if (!material) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  material->type = type;
  payload.int_val = (ui_int32)type;

  {
    (void)ui_signal_set(material->type_signal, payload);
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_compositor_material_base_set_fallback_mode.
 * @param material Parameter material.
 * @param fallback_mode Parameter fallback_mode.
 * @return Return value.
 */
ui_error_t ui_compositor_material_base_set_fallback_mode(
    struct ui_compositor_material_base *material,
    enum ui_compositor_fallback_mode fallback_mode) {
  if (!material) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  material->fallback_mode = fallback_mode;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_compositor_material_base_set_opacity.
 * @param material Parameter material.
 * @param opacity Parameter opacity.
 * @return Return value.
 */
ui_error_t ui_compositor_material_base_set_opacity(
    struct ui_compositor_material_base *material, float opacity) {
  if (!material) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (opacity < 0.0f) {
    opacity = 0.0f;
  } else if (opacity > 1.0f) {
    opacity = 1.0f;
  }

  material->opacity = opacity;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_compositor_material_base_get_type_signal.
 * @param material Parameter material.
 * @param out_signal Parameter out_signal.
 * @return Return value.
 */
ui_error_t ui_compositor_material_base_get_type_signal(
    struct ui_compositor_material_base *material, ui_signal_t **out_signal) {
  if (!material || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = material->type_signal;
  return UI_ERROR_NONE;
}
