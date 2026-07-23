/* clang-format off */
#include "../include/ui_css_gcpm.h"
#include "../include/ui_cssom.h"
#include <string.h>
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

enum ui_error ui_css_gcpm_parse(const struct ui_css_computed_style *style,
                                struct ui_css_gcpm_properties *out_props) {
  const char *val = NULL;
  enum ui_error rc;
  size_t len;

  if (!style || !out_props) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_props->string_set = NULL;
  out_props->bookmark_label = NULL;
  out_props->bookmark_level = 0;
  out_props->bookmark_state =
      1; /* open by default in spec usually if specified, or auto */

  rc = ui_css_computed_style_get_property(style, "string-set", &val);
  if (rc == UI_ERROR_NONE && strcmp(val, "none") != 0) {
    len = strlen(val);
    out_props->string_set = (char *)UI_MALLOC(len + 1);
    if (out_props->string_set) {
#if defined(_MSC_VER)
      strcpy_s(out_props->string_set, len + 1, val);
#else
      strcpy(out_props->string_set, val);
#endif
    } else {
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  rc = ui_css_computed_style_get_property(style, "bookmark-label", &val);
  if (rc == UI_ERROR_NONE && strcmp(val, "none") != 0) {
    len = strlen(val);
    out_props->bookmark_label = (char *)UI_MALLOC(len + 1);
    if (out_props->bookmark_label) {
#if defined(_MSC_VER)
      strcpy_s(out_props->bookmark_label, len + 1, val);
#else
      strcpy(out_props->bookmark_label, val);
#endif
    } else {
      (void)ui_css_gcpm_properties_cleanup(out_props);
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  rc = ui_css_computed_style_get_property(style, "bookmark-level", &val);
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "none") == 0) {
      out_props->bookmark_level = 0;
    } else {
      out_props->bookmark_level = atoi(val);
    }
  }

  rc = ui_css_computed_style_get_property(style, "bookmark-state", &val);
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "closed") == 0) {
      out_props->bookmark_state = 0;
    } else {
      out_props->bookmark_state = 1;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_css_gcpm_properties_cleanup(struct ui_css_gcpm_properties *props) {
  if (!props)
    return UI_ERROR_INVALID_ARGUMENT;
  if (props->string_set) {
    UI_FREE(props->string_set);
    props->string_set = NULL;
  }
  if (props->bookmark_label) {
    UI_FREE(props->bookmark_label);
    props->bookmark_label = NULL;
  }
  return UI_ERROR_NONE;
}
