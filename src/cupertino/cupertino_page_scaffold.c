/**
 * @file cupertino_page_scaffold.c
 * @brief Cupertino Page Scaffold implementation conforming to Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_page_scaffold.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_page_scaffold_mock_create_fail = 0;
int g_cupertino_page_scaffold_mock_set_top_bar_fail = 0;
int g_cupertino_page_scaffold_mock_set_main_fail = 0;
int g_cupertino_page_scaffold_mock_destroy_fail = 0;

static ui_error_t
mock_scaffold_base_create(struct ui_scaffold_base **out_base) {
  if (g_cupertino_page_scaffold_mock_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_scaffold_base_create(out_base);
}
#undef ui_scaffold_base_create
/** @cond */
#define ui_scaffold_base_create mock_scaffold_base_create
/** @endcond */

static ui_error_t mock_scaffold_base_set_top_bar(struct ui_scaffold_base *base,
                                                 struct ui_component *top_bar) {
  if (g_cupertino_page_scaffold_mock_set_top_bar_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_scaffold_base_set_top_bar(base, top_bar);
}
#undef ui_scaffold_base_set_top_bar
/** @cond */
#define ui_scaffold_base_set_top_bar mock_scaffold_base_set_top_bar
/** @endcond */

static ui_error_t
mock_scaffold_base_set_main_content(struct ui_scaffold_base *base,
                                    struct ui_component *content) {
  if (g_cupertino_page_scaffold_mock_set_main_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_scaffold_base_set_main_content(base, content);
}
#undef ui_scaffold_base_set_main_content
/** @cond */
#define ui_scaffold_base_set_main_content mock_scaffold_base_set_main_content
/** @endcond */

static ui_error_t mock_scaffold_component_destroy(struct ui_component *comp) {
  if (g_cupertino_page_scaffold_mock_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_component_destroy(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_scaffold_component_destroy
/** @endcond */
#endif

ui_error_t cupertino_page_scaffold_create(
    struct ui_engine *engine,
    const struct cupertino_page_scaffold_descriptor *desc,
    struct cupertino_page_scaffold **out_scaffold) {
  struct cupertino_page_scaffold *scaffold;
  ui_error_t rc;

  if (!engine || !desc || !out_scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if ((int)desc->background_style < 0 || (int)desc->background_style > 1) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold = (struct cupertino_page_scaffold *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_page_scaffold));
  if (!scaffold) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(scaffold, 0, sizeof(*scaffold));
  scaffold->background_style = desc->background_style;
  scaffold->resize_to_avoid_bottom_inset =
      desc->resize_to_avoid_bottom_inset ? 1 : 0;
  scaffold->safe_area_insets = desc->safe_area_insets;
  scaffold->bottom_inset =
      (desc->bottom_inset >= 0.0f) ? desc->bottom_inset : 0.0f;
  scaffold->is_dark = desc->is_dark ? 1 : 0;

  rc = ui_scaffold_base_create(&scaffold->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(scaffold);
    return rc;
  }

  *out_scaffold = scaffold;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_page_scaffold_destroy(struct cupertino_page_scaffold *scaffold) {
  ui_error_t rc;

  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (scaffold->base) {
    rc = ui_component_destroy(&scaffold->base->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    scaffold->base = NULL;
  }

  C_MULTIPLATFORM_FREE(scaffold);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_page_scaffold_set_nav_bar(struct cupertino_page_scaffold *scaffold,
                                    struct ui_component *nav_bar) {
  ui_error_t rc;

  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->nav_bar = nav_bar;
  if (scaffold->base && nav_bar && nav_bar->shadow_root) {
    rc = ui_scaffold_base_set_top_bar(scaffold->base, nav_bar);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_get_nav_bar(
    const struct cupertino_page_scaffold *scaffold,
    struct ui_component **out_nav_bar) {
  if (!scaffold || !out_nav_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_nav_bar = scaffold->nav_bar;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_page_scaffold_set_body(struct cupertino_page_scaffold *scaffold,
                                 struct ui_component *body) {
  ui_error_t rc;

  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  scaffold->body = body;
  if (scaffold->base && body && body->shadow_root) {
    rc = ui_scaffold_base_set_main_content(scaffold->base, body);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_page_scaffold_get_body(const struct cupertino_page_scaffold *scaffold,
                                 struct ui_component **out_body) {
  if (!scaffold || !out_body) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_body = scaffold->body;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_set_background_style(
    struct cupertino_page_scaffold *scaffold,
    enum cupertino_scaffold_background background_style) {
  if (!scaffold || ((int)background_style < 0 || (int)background_style > 1)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  scaffold->background_style = background_style;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_get_background_style(
    const struct cupertino_page_scaffold *scaffold,
    enum cupertino_scaffold_background *out_style) {
  if (!scaffold || !out_style) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_style = scaffold->background_style;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_set_resize_to_avoid_bottom_inset(
    struct cupertino_page_scaffold *scaffold, int resize) {
  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  scaffold->resize_to_avoid_bottom_inset = resize ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_get_resize_to_avoid_bottom_inset(
    const struct cupertino_page_scaffold *scaffold, int *out_resize) {
  if (!scaffold || !out_resize) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_resize = scaffold->resize_to_avoid_bottom_inset;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_set_safe_area_insets(
    struct cupertino_page_scaffold *scaffold,
    const struct cupertino_safe_area_insets *insets) {
  if (!scaffold || !insets) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  scaffold->safe_area_insets = *insets;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_get_safe_area_insets(
    const struct cupertino_page_scaffold *scaffold,
    struct cupertino_safe_area_insets *out_insets) {
  if (!scaffold || !out_insets) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_insets = scaffold->safe_area_insets;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_set_bottom_inset(
    struct cupertino_page_scaffold *scaffold, float bottom_inset) {
  if (!scaffold || bottom_inset < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  scaffold->bottom_inset = bottom_inset;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_get_bottom_inset(
    const struct cupertino_page_scaffold *scaffold, float *out_bottom_inset) {
  if (!scaffold || !out_bottom_inset) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_bottom_inset = scaffold->bottom_inset;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_get_effective_padding(
    const struct cupertino_page_scaffold *scaffold, float *out_top_padding,
    float *out_bottom_padding) {
  float top;
  float bottom;

  if (!scaffold || !out_top_padding || !out_bottom_padding) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  top = scaffold->safe_area_insets.top;
  if (scaffold->nav_bar) {
    top += 44.0f;
  }

  if (scaffold->resize_to_avoid_bottom_inset &&
      scaffold->bottom_inset > scaffold->safe_area_insets.bottom) {
    bottom = scaffold->bottom_inset;
  } else {
    bottom = scaffold->safe_area_insets.bottom;
  }

  *out_top_padding = top;
  *out_bottom_padding = bottom;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_page_scaffold_set_dark_mode(struct cupertino_page_scaffold *scaffold,
                                      int is_dark) {
  if (!scaffold) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  scaffold->is_dark = is_dark ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_page_scaffold_get_dark_mode(
    const struct cupertino_page_scaffold *scaffold, int *out_is_dark) {
  if (!scaffold || !out_is_dark) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_dark = scaffold->is_dark;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_page_scaffold_get_base(struct cupertino_page_scaffold *scaffold,
                                 struct ui_scaffold_base **out_base) {
  if (!scaffold || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = scaffold->base;
  return UI_ERROR_NONE;
}
