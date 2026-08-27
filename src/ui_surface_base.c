/* clang-format off */
#include "ui_surface_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

/*
 * @brief ui_surface_base_create.
 * @param out_surface Parameter out_surface.
 * @return Return value.
 */
ui_error_t ui_surface_base_create(struct ui_surface_base **out_surface) {
  struct ui_surface_base *surface;
  struct ui_component *base_comp;
  ui_error_t err;

  if (!out_surface) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  surface = (struct ui_surface_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_surface_base));
  if (!surface) {
    (void)ui_component_destroy(base_comp);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  surface->base = *base_comp;
  C_MULTIPLATFORM_FREE(base_comp);

  surface->elevation = UI_ELEVATION_LEVEL_0;

  err =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &surface->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(surface);
    return err;
  }

  err = ui_dom_node_set_tag_name(surface->base.shadow_root, "ui-surface");
  if (err != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(surface->base.shadow_root);
    C_MULTIPLATFORM_FREE(surface);
    return err;
  }

  *out_surface = surface;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_surface_base_set_elevation.
 * @param surface Parameter surface.
 * @param level Parameter level.
 * @return Return value.
 */
ui_error_t ui_surface_base_set_elevation(struct ui_surface_base *surface,
                                         enum ui_elevation_level level) {
  char level_str[16];

  if (!surface) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  surface->elevation = level;

#if defined(_MSC_VER)
  sprintf_s(level_str, sizeof(level_str), "%d", (int)level);
#else
  sprintf(level_str, "%d", (int)level);
#endif

  return ui_dom_node_set_attribute(surface->base.shadow_root, "data-elevation",
                                   level_str);
}

/*
 * @brief ui_surface_base_bind_data.
 * @param widget Parameter widget.
 * @param signal Parameter signal.
 * @return Return value.
 */
ui_error_t ui_surface_base_bind_data(struct ui_surface_base *widget,
                                     struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
