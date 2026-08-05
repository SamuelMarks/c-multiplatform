/* clang-format off */
#include "ui_css_motion.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "ui_internal_mem.h"
#include "strtok_posix.h"
/* clang-format on */

#if defined(_MSC_VER)
#define UI_STRTOK(str, delim, ctx) strtok_s((str), (delim), (ctx))
#else
#define UI_STRTOK(str, delim, ctx) strtok_r((str), (delim), (ctx))
#endif

static void skip_whitespace(const char **p_str) {
  while (isspace((unsigned char)**p_str)) {
    (*p_str)++;
  }
}

ui_error_t ui_css_parse_offset_path(const char *str,
                                    struct ui_css_offset_path *out_path) {
  ui_error_t rc;
  if (!str || !out_path)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  out_path->type = UI_CSS_OFFSET_PATH_NONE;
  out_path->box = UI_CSS_GEOMETRY_BOX_BORDER_BOX;
  out_path->url[0] = '\0';

  if (strcmp(str, "none") == 0) {
    out_path->type = UI_CSS_OFFSET_PATH_NONE;
    return UI_ERROR_NONE;
  }

  if (strncmp(str, "url(", 4) == 0) {
    const char *start = str + 4;
    const char *end = strrchr(start, ')');
    if (end) {
      size_t len = end - start;
      if (len >= sizeof(out_path->url))
        len = sizeof(out_path->url) - 1;
      memcpy(out_path->url, start, len);
      out_path->url[len] = '\0';
      out_path->type = UI_CSS_OFFSET_PATH_URL;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "path(", 5) == 0) {
    const char *start = str + 5;
    const char *end = strrchr(start, ')');
    if (end) {
      size_t len = end - start;
      out_path->shape.type = UI_CSS_BASIC_SHAPE_PATH;
      if (len >= sizeof(out_path->shape.arguments))
        len = sizeof(out_path->shape.arguments) - 1;
      memcpy(out_path->shape.arguments, start, len);
      out_path->shape.arguments[len] = '\0';
      out_path->type = UI_CSS_OFFSET_PATH_BASIC_SHAPE;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "ray(", 4) == 0) {
    /* Basic ray() support */
    const char *start = str + 4;
    const char *end = strchr(start, ')');
    if (end) {
      char arg_buf[128];
      size_t len = end - start;
      if (len >= sizeof(arg_buf))
        len = sizeof(arg_buf) - 1;
      memcpy(arg_buf, start, len);
      arg_buf[len] = '\0';

      out_path->type = UI_CSS_OFFSET_PATH_RAY;
      out_path->ray.has_size = 0;
      out_path->ray.is_contain = 0;
      out_path->ray.angle.unit = UI_CSS_UNIT_DEG;
      out_path->ray.angle.value = 0.0f;

      if (strstr(arg_buf, "contain")) {
        out_path->ray.is_contain = 1;
      }

      {
        rc = ui_css_parse_value(arg_buf, &out_path->ray.angle);
        if (rc != UI_ERROR_NONE)
          return rc;
      }
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_PARSE_FAILED;
}

ui_error_t ui_css_parse_offset_distance(const char *str,
                                        struct ui_css_value *out_distance) {
  if (!str || !out_distance)
    return UI_ERROR_INVALID_ARGUMENT;
  skip_whitespace(&str);
  return ui_css_parse_value(str, out_distance);
}

/** \brief ui_error */
ui_error_t
ui_css_parse_offset_position(const char *str,
                             struct ui_css_offset_position *out_position) {
  ui_error_t rc;
  if (!str || !out_position)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  out_position->is_auto = 0;
  out_position->x.unit = UI_CSS_UNIT_PERCENT;
  out_position->x.value = 50.0f;
  out_position->y.unit = UI_CSS_UNIT_PERCENT;
  out_position->y.value = 50.0f;

  if (strcmp(str, "auto") == 0) {
    out_position->is_auto = 1;
    return UI_ERROR_NONE;
  }

  /* Just parse x for now, full bg-position needs a larger parser sharing with
   * backgrounds */
  rc = ui_css_parse_value(str, &out_position->x);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  out_position->y = out_position->x; /* duplicate if y is missing */
  return rc;
}

/** \brief ui_error */
ui_error_t ui_css_parse_offset_anchor(const char *str,
                                      struct ui_css_offset_anchor *out_anchor) {
  ui_error_t rc;
  if (!str || !out_anchor)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  out_anchor->is_auto = 0;
  out_anchor->x.unit = UI_CSS_UNIT_PERCENT;
  out_anchor->x.value = 50.0f;
  out_anchor->y.unit = UI_CSS_UNIT_PERCENT;
  out_anchor->y.value = 50.0f;

  if (strcmp(str, "auto") == 0) {
    out_anchor->is_auto = 1;
    return UI_ERROR_NONE;
  }

  rc = ui_css_parse_value(str, &out_anchor->x);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  out_anchor->y = out_anchor->x;
  return rc;
}

/** \brief ui_error */
ui_error_t ui_css_parse_offset_rotate(const char *str,
                                      struct ui_css_offset_rotate *out_rotate) {
  ui_error_t rc;
  char token_buf[128];
  char *token;
  char *next_token = NULL;

  if (!str || !out_rotate)
    return UI_ERROR_INVALID_ARGUMENT;

  skip_whitespace(&str);

  out_rotate->is_auto = 0;
  out_rotate->is_reverse = 0;
  out_rotate->angle.unit = UI_CSS_UNIT_DEG;
  out_rotate->angle.value = 0.0f;

  UI_STRNCPY(token_buf, sizeof(token_buf), str, sizeof(token_buf) - 1);
  token_buf[sizeof(token_buf) - 1] = '\0';

  token = UI_STRTOK(token_buf, " ", &next_token);
  while (token) {
    if (strcmp(token, "auto") == 0) {
      out_rotate->is_auto = 1;
    } else if (strcmp(token, "reverse") == 0) {
      out_rotate->is_reverse = 1;
      /* reverse implies auto */
      out_rotate->is_auto = 1;
    } else {
      rc = ui_css_parse_value(token, &out_rotate->angle);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
    token = UI_STRTOK(NULL, " ", &next_token);
  }

  return UI_ERROR_NONE;
}
