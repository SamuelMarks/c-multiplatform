/* clang-format off */
#include "ui_icon_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_icon_base {
  enum ui_icon_type type;
  char *data;
  struct ui_font *font;
  struct ui_signal *name_signal;
};

ui_error_t ui_icon_base_create(struct ui_icon_base **out_icon) {
  struct ui_icon_base *icon;
  ui_error_t rc = UI_ERROR_NONE;

  if (!out_icon) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  icon = (struct ui_icon_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_icon_base));
  if (!icon) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  icon->type = UI_ICON_TYPE_UNSET;
  icon->data = NULL;
  icon->font = NULL;

  *out_icon = icon;

cleanup:
  return rc;
}

ui_error_t ui_icon_base_destroy(struct ui_icon_base *icon) {
  if (!icon) {
    return UI_ERROR_NONE;
  }
  if (icon->data) {
    C_MULTIPLATFORM_FREE(icon->data);
  }
  C_MULTIPLATFORM_FREE(icon);
  return UI_ERROR_NONE;
}

ui_error_t ui_icon_base_set_font_glyph(struct ui_icon_base *icon,
                                       struct ui_font *font,
                                       const char *glyph_name_or_code) {
  ui_error_t rc = UI_ERROR_NONE;
  size_t len;
  char *new_data = NULL;

  if (!icon || !font || !glyph_name_or_code) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  len = strlen(glyph_name_or_code);
  new_data = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!new_data) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

#if defined(_MSC_VER)
  if (strcpy_s(new_data, len + 1, glyph_name_or_code) != 0) {
    C_MULTIPLATFORM_FREE(new_data);
    rc = UI_ERROR_UNKNOWN;
    goto cleanup;
  }
#else
  strcpy(new_data, glyph_name_or_code);
#endif

  if (icon->data) {
    C_MULTIPLATFORM_FREE(icon->data);
  }

  icon->data = new_data;
  icon->type = UI_ICON_TYPE_FONT;
  icon->font = font;

cleanup:
  return rc;
}

ui_error_t ui_icon_base_set_svg_path(struct ui_icon_base *icon,
                                     const char *svg_path_data) {
  ui_error_t rc = UI_ERROR_NONE;
  size_t len;
  char *new_data = NULL;

  if (!icon || !svg_path_data) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  len = strlen(svg_path_data);
  new_data = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!new_data) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

#if defined(_MSC_VER)
  if (strcpy_s(new_data, len + 1, svg_path_data) != 0) {
    C_MULTIPLATFORM_FREE(new_data);
    rc = UI_ERROR_UNKNOWN;
    goto cleanup;
  }
#else
  strcpy(new_data, svg_path_data);
#endif

  if (icon->data) {
    C_MULTIPLATFORM_FREE(icon->data);
  }

  icon->data = new_data;
  icon->type = UI_ICON_TYPE_SVG_PATH;
  icon->font = NULL;

cleanup:
  return rc;
}

ui_error_t ui_icon_base_get_type(const struct ui_icon_base *icon,
                                 enum ui_icon_type *out_type) {
  if (!icon || !out_type) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_type = icon->type;
  return UI_ERROR_NONE;
}

ui_error_t ui_icon_base_get_data(const struct ui_icon_base *icon,
                                 const char **out_data) {
  if (!icon || !out_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_data = icon->data;
  return UI_ERROR_NONE;
}

ui_error_t ui_icon_base_bind_name(struct ui_icon_base *widget,
                                  struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->name_signal = signal;
  return UI_ERROR_NONE;
}
