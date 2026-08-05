/* clang-format off */
#include "ui_css_round_display.h"
#include <string.h>
#include <ctype.h>
#include "ui_internal_mem.h"
/* clang-format on */

static void skip_whitespace(const char **p_str) {
  while (isspace((unsigned char)**p_str)) {
    (*p_str)++;
  }
}

static void parse_geometry_box(const char *str,
                               enum ui_css_geometry_box *out_box) {
  if (strstr(str, "margin-box"))
    *out_box = UI_CSS_GEOMETRY_BOX_MARGIN_BOX;
  else if (strstr(str, "border-box"))
    *out_box = UI_CSS_GEOMETRY_BOX_BORDER_BOX;
  else if (strstr(str, "padding-box"))
    *out_box = UI_CSS_GEOMETRY_BOX_PADDING_BOX;
  else if (strstr(str, "content-box"))
    *out_box = UI_CSS_GEOMETRY_BOX_CONTENT_BOX;
  else if (strstr(str, "fill-box"))
    *out_box = UI_CSS_GEOMETRY_BOX_FILL_BOX;
  else if (strstr(str, "stroke-box"))
    *out_box = UI_CSS_GEOMETRY_BOX_STROKE_BOX;
  else if (strstr(str, "view-box"))
    *out_box = UI_CSS_GEOMETRY_BOX_VIEW_BOX;
  else
    *out_box = UI_CSS_GEOMETRY_BOX_NONE;
}

ui_error_t ui_css_parse_shape_inside(const char *str,
                                     struct ui_css_shape_inside *out_shape) {
  ui_error_t rc;
  if (!str || !out_shape)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  out_shape->box = UI_CSS_GEOMETRY_BOX_NONE;
  out_shape->shape.type = UI_CSS_BASIC_SHAPE_NONE;
  out_shape->shape.arguments[0] = '\0';
  out_shape->is_display = 0;
  out_shape->is_outside_shape = 0;

  if (strcmp(str, "auto") == 0) {
    return UI_ERROR_NONE; /* Default is auto, meaning none/auto bounds */
  }

  if (strcmp(str, "display") == 0) {
    out_shape->is_display = 1;
    return UI_ERROR_NONE;
  }

  if (strcmp(str, "outside-shape") == 0) {
    out_shape->is_outside_shape = 1;
    return UI_ERROR_NONE;
  }

  { parse_geometry_box(str, &out_shape->box); }

  if (strstr(str, "inset(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_INSET;
  } else if (strstr(str, "circle(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_CIRCLE;
  } else if (strstr(str, "ellipse(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_ELLIPSE;
  } else if (strstr(str, "polygon(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_POLYGON;
  } else if (strstr(str, "path(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_PATH;
  }

  if (out_shape->shape.type != UI_CSS_BASIC_SHAPE_NONE) {
    const char *paren_start = strchr(str, '(');
    const char *paren_end = strrchr(str, ')');
    if (paren_end) {
      size_t len = paren_end - paren_start - 1;
      if (len >= sizeof(out_shape->shape.arguments))
        len = sizeof(out_shape->shape.arguments) - 1;
      memcpy(out_shape->shape.arguments, paren_start + 1, len);
      out_shape->shape.arguments[len] = '\0';
    }

    if (out_shape->box == UI_CSS_GEOMETRY_BOX_NONE) {
      const char *after_paren = paren_end ? (paren_end + 1) : str;
      { parse_geometry_box(after_paren, &out_shape->box); }
    }
  }

  if (out_shape->box == UI_CSS_GEOMETRY_BOX_NONE &&
      out_shape->shape.type == UI_CSS_BASIC_SHAPE_NONE) {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_css_parse_border_boundary(const char *str,
                             enum ui_css_border_boundary *out_boundary) {
  if (!str || !out_boundary)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  if (strcmp(str, "none") == 0) {
    *out_boundary = UI_CSS_BORDER_BOUNDARY_NONE;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "parent") == 0) {
    *out_boundary = UI_CSS_BORDER_BOUNDARY_PARENT;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "display") == 0) {
    *out_boundary = UI_CSS_BORDER_BOUNDARY_DISPLAY;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}
