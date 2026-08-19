/* clang-format off */
#include "../include/ui_css_page.h"
#include "../include/ui_cssom.h"
#include <string.h>
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @brief ui_css_page_parse.
 * @param style Parameter style.
 * @param out_props Parameter out_props.
 * @return Return value.
 */
ui_error_t ui_css_page_parse(const struct ui_css_computed_style *style,
                             struct ui_css_page_properties *out_props) {
  const char *val = NULL;
  ui_error_t rc;

  if (!style || !out_props) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_props->size = UI_CSS_PAGE_SIZE_AUTO;
  out_props->marks = UI_CSS_PAGE_MARKS_NONE;
  out_props->page_name = NULL;
  out_props->bleed_is_auto = 1;
  out_props->bleed_value = 0.0f;

  rc = ui_css_computed_style_get_property(style, "size", &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "portrait") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_PORTRAIT;
    else if (strcmp(val, "landscape") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_LANDSCAPE;
    else if (strcmp(val, "a4") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_A4;
    else if (strcmp(val, "a5") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_A5;
    else if (strcmp(val, "a3") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_A3;
    else if (strcmp(val, "b5") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_B5;
    else if (strcmp(val, "b4") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_B4;
    else if (strcmp(val, "jis-b5") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_JIS_B5;
    else if (strcmp(val, "jis-b4") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_JIS_B4;
    else if (strcmp(val, "letter") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_LETTER;
    else if (strcmp(val, "legal") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_LEGAL;
    else if (strcmp(val, "ledger") == 0)
      out_props->size = UI_CSS_PAGE_SIZE_LEDGER;
    else
      out_props->size = UI_CSS_PAGE_SIZE_AUTO;
  }

  rc = ui_css_computed_style_get_property(style, "marks", &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strstr(val, "crop") && strstr(val, "cross"))
      out_props->marks = UI_CSS_PAGE_MARKS_BOTH;
    else if (strstr(val, "crop"))
      out_props->marks = UI_CSS_PAGE_MARKS_CROP;
    else if (strstr(val, "cross"))
      out_props->marks = UI_CSS_PAGE_MARKS_CROSS;
    else
      out_props->marks = UI_CSS_PAGE_MARKS_NONE;
  }

  rc = ui_css_computed_style_get_property(style, "page", &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "auto") != 0) {
      size_t len = strlen(val);
      out_props->page_name = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
      if (!out_props->page_name) {
        return UI_ERROR_OUT_OF_MEMORY;
      }
#if defined(_MSC_VER)
      strcpy_s(out_props->page_name, len + 1, val);
#else
      strcpy(out_props->page_name, val);
#endif
    }
  }

  rc = ui_css_computed_style_get_property(style, "bleed", &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "auto") == 0) {
      out_props->bleed_is_auto = 1;
    } else {
      out_props->bleed_is_auto = 0;
      out_props->bleed_value =
          (float)atof(val); /* Simple length parsing for now */
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_css_page_properties_cleanup(struct ui_css_page_properties *props) {
  if (props && props->page_name) {
    C_MULTIPLATFORM_FREE(props->page_name);
    props->page_name = NULL;
  }
  return UI_ERROR_NONE;
}
