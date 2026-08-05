/* clang-format off */
#include "../include/ui_css_scroll_snap.h"
#include "../include/ui_cssom.h"
#include <string.h>
#include <ctype.h>
#include "ui_internal_mem.h"
#include "strtok_posix.h"
/* clang-format on */

#if defined(_MSC_VER)
#define UI_STRTOK(str, delim, ctx) strtok_s((str), (delim), (ctx))
#else
#define UI_STRTOK(str, delim, ctx) strtok_r((str), (delim), (ctx))
#endif

static ui_error_t parse_snap_type(const char *str,
                                  struct ui_css_scroll_snap_type *out_type) {
  char token_buf[128];
  char *token;
  char *next_token = NULL;
  size_t len;

  out_type->axis = UI_CSS_SCROLL_SNAP_AXIS_NONE;
  out_type->strictness = UI_CSS_SCROLL_SNAP_STRICTNESS_NONE;

  if (strcmp(str, "none") == 0)
    return UI_ERROR_NONE;

  len = strlen(str);
  if (len >= sizeof(token_buf))
    len = sizeof(token_buf) - 1;
  memcpy(token_buf, str, len);
  token_buf[len] = '\0';

  token = UI_STRTOK(token_buf, " ", &next_token);
  if (token) {
    if (strcmp(token, "x") == 0)
      out_type->axis = UI_CSS_SCROLL_SNAP_AXIS_X;
    else if (strcmp(token, "y") == 0)
      out_type->axis = UI_CSS_SCROLL_SNAP_AXIS_Y;
    else if (strcmp(token, "block") == 0)
      out_type->axis = UI_CSS_SCROLL_SNAP_AXIS_BLOCK;
    else if (strcmp(token, "inline") == 0)
      out_type->axis = UI_CSS_SCROLL_SNAP_AXIS_INLINE;
    else if (strcmp(token, "both") == 0)
      out_type->axis = UI_CSS_SCROLL_SNAP_AXIS_BOTH;
  }

  token = UI_STRTOK(NULL, " ", &next_token);
  if (token) {
    if (strcmp(token, "mandatory") == 0)
      out_type->strictness = UI_CSS_SCROLL_SNAP_STRICTNESS_MANDATORY;
    else if (strcmp(token, "proximity") == 0)
      out_type->strictness = UI_CSS_SCROLL_SNAP_STRICTNESS_PROXIMITY;
  } else {
    /* Default strictness if missing */
    if (out_type->axis != UI_CSS_SCROLL_SNAP_AXIS_NONE) {
      out_type->strictness = UI_CSS_SCROLL_SNAP_STRICTNESS_PROXIMITY;
    }
    return UI_ERROR_NONE;
  }
  return UI_ERROR_NONE;
}

static ui_error_t
parse_align_keyword(const char *str,
                    enum ui_css_scroll_snap_align_keyword *out_keyword) {
  *out_keyword = UI_CSS_SCROLL_SNAP_ALIGN_NONE;
  if (strcmp(str, "start") == 0)
    *out_keyword = UI_CSS_SCROLL_SNAP_ALIGN_START;
  else if (strcmp(str, "end") == 0)
    *out_keyword = UI_CSS_SCROLL_SNAP_ALIGN_END;
  else if (strcmp(str, "center") == 0)
    *out_keyword = UI_CSS_SCROLL_SNAP_ALIGN_CENTER;
  return UI_ERROR_NONE;
}

static ui_error_t parse_snap_align(const char *str,
                                   struct ui_css_scroll_snap_align *out_align) {
  ui_error_t rc;
  char token_buf[128];
  char *token;
  char *next_token = NULL;
  size_t len;

  out_align->block = UI_CSS_SCROLL_SNAP_ALIGN_NONE;
  out_align->inline_axis = UI_CSS_SCROLL_SNAP_ALIGN_NONE;

  if (strcmp(str, "none") == 0)
    return UI_ERROR_NONE;

  len = strlen(str);
  if (len >= sizeof(token_buf))
    len = sizeof(token_buf) - 1;
  memcpy(token_buf, str, len);
  token_buf[len] = '\0';

  token = UI_STRTOK(token_buf, " ", &next_token);
  if (token) {
    {
      (void)parse_align_keyword(token, &out_align->block);
    }
    out_align->inline_axis = out_align->block; /* default to 1st value */
  }

  token = UI_STRTOK(NULL, " ", &next_token);
  if (token) {
    {
      (void)parse_align_keyword(token, &out_align->inline_axis);
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t set_quad_default(struct ui_css_value *top,
                                   struct ui_css_value *right,
                                   struct ui_css_value *bottom,
                                   struct ui_css_value *left) {

  top->unit = UI_CSS_UNIT_PX;
  top->value = 0.0f;
  right->unit = UI_CSS_UNIT_PX;
  right->value = 0.0f;
  bottom->unit = UI_CSS_UNIT_PX;
  bottom->value = 0.0f;
  left->unit = UI_CSS_UNIT_PX;
  left->value = 0.0f;
  return UI_ERROR_NONE;
}

static ui_error_t parse_quad_shorthand(const char *str,
                                       struct ui_css_value *top,
                                       struct ui_css_value *right,
                                       struct ui_css_value *bottom,
                                       struct ui_css_value *left) {
  ui_error_t rc;
  char token_buf[128];
  char *token;
  char *next_token = NULL;
  struct ui_css_value values[4];
  int count = 0;
  size_t len;

  len = strlen(str);
  if (len >= sizeof(token_buf))
    len = sizeof(token_buf) - 1;
  memcpy(token_buf, str, len);
  token_buf[len] = '\0';

  token = UI_STRTOK(token_buf, " ", &next_token);
  while (token && count < 4) {
    rc = ui_css_parse_value(token, &values[count]);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    count++;
    token = UI_STRTOK(NULL, " ", &next_token);
  }

  if (count == 1) {
    *top = values[0];
    *right = values[0];
    *bottom = values[0];
    *left = values[0];
  } else if (count == 2) {
    *top = values[0];
    *bottom = values[0];
    *right = values[1];
    *left = values[1];
  } else if (count == 3) {
    *top = values[0];
    *right = values[1];
    *left = values[1];
    *bottom = values[2];
  } else if (count == 4) {
    *top = values[0];
    *right = values[1];
    *bottom = values[2];
    *left = values[3];
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_css_scroll_snap_parse(const struct ui_css_computed_style *style,
                         struct ui_css_scroll_snap_properties *out_props) {
  const char *val_str;

  if (!style || !out_props)
    return UI_ERROR_INVALID_ARGUMENT;

  memset(out_props, 0, sizeof(struct ui_css_scroll_snap_properties));

  /* Defaults */
  out_props->type.axis = UI_CSS_SCROLL_SNAP_AXIS_NONE;
  out_props->type.strictness = UI_CSS_SCROLL_SNAP_STRICTNESS_NONE;
  out_props->align.block = UI_CSS_SCROLL_SNAP_ALIGN_NONE;
  out_props->align.inline_axis = UI_CSS_SCROLL_SNAP_ALIGN_NONE;
  out_props->stop = UI_CSS_SCROLL_SNAP_STOP_NORMAL;

  {
    ui_error_t quad_rc2;
    (void)set_quad_default(&out_props->padding.top, &out_props->padding.right,
                           &out_props->padding.bottom,
                           &out_props->padding.left);
    /* fix below */ (void)set_quad_default(
        &out_props->margin.top, &out_props->margin.right,
        &out_props->margin.bottom, &out_props->margin.left);
  }

  {
    ui_error_t prop_rc =
        ui_css_computed_style_get_property(style, "scroll-snap-type", &val_str);
    if (prop_rc != UI_ERROR_NONE) {
      if (0)
        return prop_rc;
      if (0)
        return prop_rc;
    } else {
      (void)parse_snap_type(val_str, &out_props->type);
    }
  }

  {
    ui_error_t prop_rc = ui_css_computed_style_get_property(
        style, "scroll-snap-align", &val_str);
    if (prop_rc != UI_ERROR_NONE) {
      if (0)
        return prop_rc;
      if (0)
        return prop_rc;
    } else {
      (void)parse_snap_align(val_str, &out_props->align);
    }
  }

  {
    ui_error_t prop_rc =
        ui_css_computed_style_get_property(style, "scroll-snap-stop", &val_str);
    if (prop_rc != UI_ERROR_NONE) {
      if (0)
        return prop_rc;
      if (0)
        return prop_rc;
    } else {
      if (strcmp(val_str, "always") == 0)
        out_props->stop = UI_CSS_SCROLL_SNAP_STOP_ALWAYS;
    }
  }

  {
    ui_error_t prop_rc =
        ui_css_computed_style_get_property(style, "scroll-padding", &val_str);
    if (prop_rc != UI_ERROR_NONE) {
      if (0)
        return prop_rc;

      {
        ui_error_t rc_top = ui_css_computed_style_get_property(
            style, "scroll-padding-top", &val_str);
        if (rc_top != UI_ERROR_NONE) {
          if (0)
            return rc_top;
        }
        if (rc_top == UI_ERROR_NONE) {
          (void)ui_css_parse_value(val_str, &out_props->padding.top);
        }
      }

      {
        ui_error_t rc_right = ui_css_computed_style_get_property(
            style, "scroll-padding-right", &val_str);
        if (rc_right != UI_ERROR_NONE) {
          if (0)
            return rc_right;
        }
        if (rc_right == UI_ERROR_NONE) {
          (void)ui_css_parse_value(val_str, &out_props->padding.right);
        }
      }

      {
        ui_error_t rc_bottom = ui_css_computed_style_get_property(
            style, "scroll-padding-bottom", &val_str);
        if (rc_bottom != UI_ERROR_NONE) {
          if (0)
            return rc_bottom;
        }
        if (rc_bottom == UI_ERROR_NONE) {
          (void)ui_css_parse_value(val_str, &out_props->padding.bottom);
        }
      }

      {
        ui_error_t rc_left = ui_css_computed_style_get_property(
            style, "scroll-padding-left", &val_str);
        if (rc_left != UI_ERROR_NONE) {
          if (0)
            return rc_left;
        }
        if (rc_left == UI_ERROR_NONE) {
          (void)ui_css_parse_value(val_str, &out_props->padding.left);
        }
      }

    } else {
      (void)parse_quad_shorthand(
          val_str, &out_props->padding.top, &out_props->padding.right,
          &out_props->padding.bottom, &out_props->padding.left);
    }
  }

  {
    ui_error_t prop_rc =
        ui_css_computed_style_get_property(style, "scroll-margin", &val_str);
    if (prop_rc != UI_ERROR_NONE) {
      if (0)
        return prop_rc;

      {
        ui_error_t rc_top = ui_css_computed_style_get_property(
            style, "scroll-margin-top", &val_str);
        if (rc_top != UI_ERROR_NONE) {
          if (0)
            return rc_top;
        }
        if (rc_top == UI_ERROR_NONE) {
          (void)ui_css_parse_value(val_str, &out_props->margin.top);
        }
      }

      {
        ui_error_t rc_right = ui_css_computed_style_get_property(
            style, "scroll-margin-right", &val_str);
        if (rc_right != UI_ERROR_NONE) {
          if (0)
            return rc_right;
        }
        if (rc_right == UI_ERROR_NONE) {
          (void)ui_css_parse_value(val_str, &out_props->margin.right);
        }
      }

      {
        ui_error_t rc_bottom = ui_css_computed_style_get_property(
            style, "scroll-margin-bottom", &val_str);
        if (rc_bottom != UI_ERROR_NONE) {
          if (0)
            return rc_bottom;
        }
        if (rc_bottom == UI_ERROR_NONE) {
          (void)ui_css_parse_value(val_str, &out_props->margin.bottom);
        }
      }

      {
        ui_error_t rc_left = ui_css_computed_style_get_property(
            style, "scroll-margin-left", &val_str);
        if (rc_left != UI_ERROR_NONE) {
          if (0)
            return rc_left;
        }
        if (rc_left == UI_ERROR_NONE) {
          (void)ui_css_parse_value(val_str, &out_props->margin.left);
        }
      }

    } else {
      (void)parse_quad_shorthand(
          val_str, &out_props->margin.top, &out_props->margin.right,
          &out_props->margin.bottom, &out_props->margin.left);
    }
  }

  return UI_ERROR_NONE;
}
