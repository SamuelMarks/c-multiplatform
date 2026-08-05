#ifndef UI_COMPOSITOR_MATERIAL_BASE_H
#define UI_COMPOSITOR_MATERIAL_BASE_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_component.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Types of OS-level compositor materials.
 */
enum ui_compositor_material_type {
  UI_COMPOSITOR_MATERIAL_TYPE_NONE = 0,
  UI_COMPOSITOR_MATERIAL_TYPE_MICA =
      1, /**< Windows 11 Mica (desktop wallpaper bleed) */
  UI_COMPOSITOR_MATERIAL_TYPE_MICA_ALT =
      2, /**< Windows 11 Mica Alt (heavier tint) */
  UI_COMPOSITOR_MATERIAL_TYPE_ACRYLIC =
      3, /**< Windows 10/11 Acrylic (behind-window blur) */
  UI_COMPOSITOR_MATERIAL_TYPE_MAC_BEHIND_WINDOW =
      4, /**< macOS NSVisualEffectView behind-window */
  UI_COMPOSITOR_MATERIAL_TYPE_MAC_IN_WINDOW =
      5 /**< macOS NSVisualEffectView within-window */
};

/**
 * @brief Fallback rendering paths when native compositor effects are
 * unavailable.
 */
enum ui_compositor_fallback_mode {
  UI_COMPOSITOR_FALLBACK_MODE_SOLID_COLOR = 0,
  UI_COMPOSITOR_FALLBACK_MODE_VIBRANCY =
      1 /**< Fallback to engine-rendered vibrancy/blur */
};

/**
 * @brief Opaque handle for the compositor material component.
 */
struct ui_compositor_material_base;

/**
 * @brief Configuration for a compositor material.
 */
struct ui_compositor_material_config {
  enum ui_compositor_material_type initial_type;
  enum ui_compositor_fallback_mode fallback_mode;
  float initial_opacity;
};

/**
 * @brief Creates a compositor material component.
 *
 * @param arena The memory arena to use for allocation.
 * @param config Configuration for the material.
 * @param out_material Pointer to receive the created component handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_compositor_material_base_create(
    struct ui_arena *arena, const struct ui_compositor_material_config *config,
    struct ui_compositor_material_base **out_material);

/**
 * @brief Destroys a compositor material component.
 *
 * @param material The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_compositor_material_base_destroy(
    struct ui_compositor_material_base *material);

/**
 * @brief Sets the material type, attempting to hook into the OS compositor.
 *
 * @param material The component.
 * @param type The desired material type.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_compositor_material_base_set_type(
    struct ui_compositor_material_base *material,
    enum ui_compositor_material_type type);

/**
 * @brief Sets the fallback rendering mode for when the OS effect is unavailable
 * or unsupported.
 *
 * @param material The component.
 * @param fallback_mode The fallback mode.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_compositor_material_base_set_fallback_mode(
    struct ui_compositor_material_base *material,
    enum ui_compositor_fallback_mode fallback_mode);

/**
 * @brief Sets the opacity channel of the material independent of the canvas
 * draw commands.
 *
 * @param material The component.
 * @param opacity Opacity value between 0.0f and 1.0f.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_compositor_material_base_set_opacity(
    struct ui_compositor_material_base *material, float opacity);

/**
 * @brief Retrieves the signal for material type changes.
 * The payload contains the int value of the ui_compositor_material_type.
 *
 * @param material The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_compositor_material_base_get_type_signal(
    struct ui_compositor_material_base *material, ui_signal_t **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_COMPOSITOR_MATERIAL_BASE_H */
