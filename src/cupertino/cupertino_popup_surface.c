/**
 * @file cupertino_popup_surface.c
 * @brief Cupertino Popup Surface implementation conforming to Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_popup_surface.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_popup_surface_mock_surface_create_fail = 0;
int g_cupertino_popup_surface_mock_set_elevation_fail = 0;
int g_cupertino_popup_surface_mock_destroy_fail = 0;

static ui_error_t
mock_surface_base_create(struct ui_surface_base **out_surface) {
  if (g_cupertino_popup_surface_mock_surface_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_surface_base_create(out_surface);
}
#undef ui_surface_base_create
/** @cond */
#define ui_surface_base_create mock_surface_base_create
/** @endcond */

static ui_error_t
mock_surface_base_set_elevation(struct ui_surface_base *surface,
                                enum ui_elevation_level elevation) {
  if (g_cupertino_popup_surface_mock_set_elevation_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_surface_base_set_elevation(surface, elevation);
}
#undef ui_surface_base_set_elevation
/** @cond */
#define ui_surface_base_set_elevation mock_surface_base_set_elevation
/** @endcond */

static ui_error_t mock_ui_component_destroy(struct ui_component *comp) {
  if (g_cupertino_popup_surface_mock_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_ui_component_destroy
/** @endcond */
#endif

static void cupertino_popup_surface_compute_params(
    struct cupertino_popup_surface *surface) {
  if (surface->reduce_transparency) {
    surface->vibrancy_material = surface->is_dark
                                     ? UI_VIBRANCY_MATERIAL_CUPERTINO_DARK
                                     : UI_VIBRANCY_MATERIAL_CUPERTINO_LIGHT;
  } else {
    surface->vibrancy_material = surface->is_dark
                                     ? UI_VIBRANCY_MATERIAL_CUPERTINO_DARK
                                     : UI_VIBRANCY_MATERIAL_CUPERTINO_LIGHT;
  }

  if (surface->elevation == UI_ELEVATION_LEVEL_0) {
    surface->shadow_blur = 0.0f;
    surface->shadow_spread = 0.0f;
    surface->shadow_opacity = 0.0f;
    surface->shadow_offset_y = 0.0f;
  } else if (surface->elevation == UI_ELEVATION_LEVEL_1) {
    surface->shadow_blur = 4.0f;
    surface->shadow_spread = 0.0f;
    surface->shadow_opacity = 0.08f;
    surface->shadow_offset_y = 2.0f;
  } else if (surface->elevation == UI_ELEVATION_LEVEL_2) {
    surface->shadow_blur = 8.0f;
    surface->shadow_spread = 0.0f;
    surface->shadow_opacity = 0.12f;
    surface->shadow_offset_y = 4.0f;
  } else if (surface->elevation == UI_ELEVATION_LEVEL_3) {
    surface->shadow_blur = 16.0f;
    surface->shadow_spread = 0.0f;
    surface->shadow_opacity = 0.16f;
    surface->shadow_offset_y = 8.0f;
  } else if (surface->elevation == UI_ELEVATION_LEVEL_4) {
    surface->shadow_blur = 24.0f;
    surface->shadow_spread = 0.0f;
    surface->shadow_opacity = 0.20f;
    surface->shadow_offset_y = 12.0f;
  } else {
    surface->shadow_blur = 32.0f;
    surface->shadow_spread = 0.0f;
    surface->shadow_opacity = 0.24f;
    surface->shadow_offset_y = 16.0f;
  }
}

ui_error_t cupertino_popup_surface_create(
    struct ui_engine *engine,
    const struct cupertino_popup_surface_descriptor *desc,
    struct cupertino_popup_surface **out_surface) {
  struct cupertino_popup_surface *surface;
  ui_error_t rc;
  ui_error_t rc_cleanup;

  if (!engine || !desc || !out_surface) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if ((int)desc->preset < 0 || (int)desc->preset > 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  surface = (struct cupertino_popup_surface *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_popup_surface));
  if (!surface) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(surface, 0, sizeof(*surface));
  surface->preset = desc->preset;
  surface->is_dark = desc->is_dark ? 1 : 0;
  surface->reduce_transparency = desc->reduce_transparency ? 1 : 0;
  surface->width = (desc->width >= 0.0f) ? desc->width : 280.0f;
  surface->height = (desc->height >= 0.0f) ? desc->height : 180.0f;

  if (desc->preset == CUPERTINO_POPUP_SURFACE_PRESET_ALERT) {
    surface->corner_radius = CUPERTINO_POPUP_SURFACE_RADIUS_ALERT;
    surface->elevation = UI_ELEVATION_LEVEL_4;
  } else if (desc->preset == CUPERTINO_POPUP_SURFACE_PRESET_ACTION_SHEET) {
    surface->corner_radius = CUPERTINO_POPUP_SURFACE_RADIUS_ACTION_SHEET;
    surface->elevation = UI_ELEVATION_LEVEL_3;
  } else if (desc->preset == CUPERTINO_POPUP_SURFACE_PRESET_POPOVER) {
    surface->corner_radius = CUPERTINO_POPUP_SURFACE_RADIUS_POPOVER;
    surface->elevation = UI_ELEVATION_LEVEL_2;
  } else {
    surface->corner_radius = (desc->custom_corner_radius >= 0.0f)
                                 ? desc->custom_corner_radius
                                 : CUPERTINO_POPUP_SURFACE_RADIUS_ALERT;
    surface->elevation = UI_ELEVATION_LEVEL_3;
  }

  cupertino_popup_surface_compute_params(surface);

  rc = ui_surface_base_create(&surface->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(surface);
    return rc;
  }

  rc = ui_surface_base_set_elevation(surface->base, surface->elevation);
  if (rc != UI_ERROR_NONE) {
    rc_cleanup = ui_component_destroy(&surface->base->base);
    C_MULTIPLATFORM_FREE(surface);
    return (rc_cleanup != UI_ERROR_NONE) ? rc_cleanup : rc;
  }

  *out_surface = surface;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_popup_surface_destroy(struct cupertino_popup_surface *surface) {
  ui_error_t rc;

  if (!surface) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (surface->base) {
    rc = ui_component_destroy(&surface->base->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    surface->base = NULL;
  }

  C_MULTIPLATFORM_FREE(surface);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popup_surface_set_corner_radius(
    struct cupertino_popup_surface *surface, float radius) {
  if (!surface || radius < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  surface->corner_radius = radius;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popup_surface_get_corner_radius(
    const struct cupertino_popup_surface *surface, float *out_radius) {
  if (!surface || !out_radius) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_radius = surface->corner_radius;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_popup_surface_set_dark_mode(struct cupertino_popup_surface *surface,
                                      int is_dark) {
  if (!surface) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  surface->is_dark = is_dark ? 1 : 0;
  cupertino_popup_surface_compute_params(surface);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popup_surface_get_dark_mode(
    const struct cupertino_popup_surface *surface, int *out_is_dark) {
  if (!surface || !out_is_dark) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_dark = surface->is_dark;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popup_surface_set_reduce_transparency(
    struct cupertino_popup_surface *surface, int reduce) {
  if (!surface) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  surface->reduce_transparency = reduce ? 1 : 0;
  cupertino_popup_surface_compute_params(surface);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popup_surface_get_reduce_transparency(
    const struct cupertino_popup_surface *surface, int *out_reduce) {
  if (!surface || !out_reduce) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_reduce = surface->reduce_transparency;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popup_surface_get_vibrancy_material(
    const struct cupertino_popup_surface *surface,
    enum ui_vibrancy_material *out_material) {
  if (!surface || !out_material) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_material = surface->vibrancy_material;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popup_surface_get_elevation(
    const struct cupertino_popup_surface *surface,
    enum ui_elevation_level *out_elevation) {
  if (!surface || !out_elevation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_elevation = surface->elevation;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_popup_surface_set_elevation(struct cupertino_popup_surface *surface,
                                      enum ui_elevation_level elevation) {
  ui_error_t rc;

  if (!surface || (int)elevation < 0 || (int)elevation > 5) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  surface->elevation = elevation;
  cupertino_popup_surface_compute_params(surface);

  if (surface->base) {
    rc = ui_surface_base_set_elevation(surface->base, elevation);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_popup_surface_get_shadow_params(
    const struct cupertino_popup_surface *surface, float *out_blur,
    float *out_spread, float *out_opacity, float *out_offset_y) {
  if (!surface || !out_blur || !out_spread || !out_opacity || !out_offset_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_blur = surface->shadow_blur;
  *out_spread = surface->shadow_spread;
  *out_opacity = surface->shadow_opacity;
  *out_offset_y = surface->shadow_offset_y;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_popup_surface_set_dimensions(struct cupertino_popup_surface *surface,
                                       float width, float height) {
  if (!surface || width < 0.0f || height < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  surface->width = width;
  surface->height = height;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_popup_surface_get_dimensions(
    const struct cupertino_popup_surface *surface, float *out_width,
    float *out_height) {
  if (!surface || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_width = surface->width;
  *out_height = surface->height;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_popup_surface_get_base(struct cupertino_popup_surface *surface,
                                 struct ui_surface_base **out_base) {
  if (!surface || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = surface->base;
  return UI_ERROR_NONE;
}
