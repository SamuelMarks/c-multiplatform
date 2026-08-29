/**
 * @file ui_css_values_effects.c
 * @brief CSS effects values parsing and memory management implementation.
 */

/* clang-format off */
#include "ui_types.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <stdio.h>

#if defined(_MSC_VER)
#define UI_SSCANF sscanf_s
#endif
/* clang-format on */

/**
 * @brief Parses a CSS transform string.
 * @param[in] str The CSS string to parse.
 * @param[out] out_transform Pointer to the output transform structure pointer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_transform(const char *str,
                                  struct ui_css_transform **out_transform) {
  struct ui_css_transform *transform;
  struct ui_css_transform_function *tail = NULL;

  if (!str || !out_transform)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_transform = NULL;

  {
    skip_whitespace(&str);
  }
  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  transform = (struct ui_css_transform *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_transform));
  if (!transform) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  transform->functions = NULL;

  while (*str) {
    struct ui_css_transform_function *func = NULL;
    enum ui_css_transform_type type = UI_CSS_TRANSFORM_NONE;
    size_t name_len = 0;
    const char *paren_start;
    const char *paren_end;
    const char *arg_str;

    {
      skip_whitespace(&str);
    }
    if (*str == '\0')
      break;

    paren_start = strchr(str, '(');
    if (!paren_start) {
      goto cleanup_fail;
    }

    name_len = (size_t)(paren_start - str);

    if (name_len == 6 && strncmp(str, "matrix", 6) == 0)
      type = UI_CSS_TRANSFORM_MATRIX;
    else if (name_len == 8 && strncmp(str, "matrix3d", 8) == 0)
      type = UI_CSS_TRANSFORM_MATRIX3D;
    else if (name_len == 9 && strncmp(str, "translate", 9) == 0)
      type = UI_CSS_TRANSFORM_TRANSLATE;
    else if (name_len == 10 && strncmp(str, "translateX", 10) == 0)
      type = UI_CSS_TRANSFORM_TRANSLATEX;
    else if (name_len == 10 && strncmp(str, "translateY", 10) == 0)
      type = UI_CSS_TRANSFORM_TRANSLATEY;
    else if (name_len == 10 && strncmp(str, "translateZ", 10) == 0)
      type = UI_CSS_TRANSFORM_TRANSLATEZ;
    else if (name_len == 11 && strncmp(str, "translate3d", 11) == 0)
      type = UI_CSS_TRANSFORM_TRANSLATE3D;
    else if (name_len == 5 && strncmp(str, "scale", 5) == 0)
      type = UI_CSS_TRANSFORM_SCALE;
    else if (name_len == 6 && strncmp(str, "scaleX", 6) == 0)
      type = UI_CSS_TRANSFORM_SCALEX;
    else if (name_len == 6 && strncmp(str, "scaleY", 6) == 0)
      type = UI_CSS_TRANSFORM_SCALEY;
    else if (name_len == 6 && strncmp(str, "scaleZ", 6) == 0)
      type = UI_CSS_TRANSFORM_SCALEZ;
    else if (name_len == 7 && strncmp(str, "scale3d", 7) == 0)
      type = UI_CSS_TRANSFORM_SCALE3D;
    else if (name_len == 6 && strncmp(str, "rotate", 6) == 0)
      type = UI_CSS_TRANSFORM_ROTATE;
    else if (name_len == 7 && strncmp(str, "rotateX", 7) == 0)
      type = UI_CSS_TRANSFORM_ROTATEX;
    else if (name_len == 7 && strncmp(str, "rotateY", 7) == 0)
      type = UI_CSS_TRANSFORM_ROTATEY;
    else if (name_len == 7 && strncmp(str, "rotateZ", 7) == 0)
      type = UI_CSS_TRANSFORM_ROTATEZ;
    else if (name_len == 8 && strncmp(str, "rotate3d", 8) == 0)
      type = UI_CSS_TRANSFORM_ROTATE3D;
    else if (name_len == 4 && strncmp(str, "skew", 4) == 0)
      type = UI_CSS_TRANSFORM_SKEW;
    else if (name_len == 5 && strncmp(str, "skewX", 5) == 0)
      type = UI_CSS_TRANSFORM_SKEWX;
    else if (name_len == 5 && strncmp(str, "skewY", 5) == 0)
      type = UI_CSS_TRANSFORM_SKEWY;
    else if (name_len == 11 && strncmp(str, "perspective", 11) == 0)
      type = UI_CSS_TRANSFORM_PERSPECTIVE;
    else {
      goto cleanup_fail;
    }

    paren_end = strchr(paren_start, ')');
    if (!paren_end) {
      goto cleanup_fail;
    }

    func = (struct ui_css_transform_function *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_css_transform_function));
    if (!func) {
      {
        ui_css_transform_destroy(transform);
      }
      return UI_ERROR_OUT_OF_MEMORY;
    }

    func->type = type;
    func->value_count = 0;
    func->next = NULL;

    arg_str = paren_start + 1;
    while (arg_str < paren_end && func->value_count < 16) {
      ui_error_t rc;
      {
        skip_whitespace(&arg_str);
      }
      if (arg_str >= paren_end)
        break;

      rc = ui_css_parse_value_internal(&arg_str,
                                       &func->values[func->value_count]);
      if (rc != UI_ERROR_NONE) {
        C_MULTIPLATFORM_FREE(func);
        {
          ui_css_transform_destroy(transform);
        }
        return rc;
      }
      func->value_count++;

      {
        skip_whitespace(&arg_str);
      }
      if (*arg_str == ',') {
        arg_str++;
      }
    }

    if (tail) {
      tail->next = func;
    } else {
      transform->functions = func;
    }
    tail = func;

    str = paren_end + 1;
  }

  if (!transform->functions) {
    goto cleanup_fail;
  }

  *out_transform = transform;
  return UI_ERROR_NONE;

cleanup_fail: { ui_css_transform_destroy(transform); }
  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief Parses a CSS shape-outside string.
 * @param[in] str The CSS string to parse.
 * @param[out] out_shape Pointer to the output shape-outside structure.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_shape_outside(const char *str,
                                      struct ui_css_shape_outside *out_shape) {
  const char *p;
  if (!str || !out_shape)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    skip_whitespace(&str);
  }
  out_shape->box = UI_CSS_GEOMETRY_BOX_NONE;
  out_shape->shape.type = UI_CSS_BASIC_SHAPE_NONE;
  out_shape->shape.arguments[0] = '\0';
  out_shape->image.type = UI_CSS_IMAGE_NONE;

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  p = str;

  /* Try parsing an image */
  {
    const char *url_start = strstr(str, "url(");
    const char *lin_grad_start = strstr(str, "linear-gradient(");
    const char *rad_grad_start = strstr(str, "radial-gradient(");
    const char *con_grad_start = strstr(str, "conic-gradient(");
    const char *img_start = NULL;

    if (url_start == str)
      img_start = url_start;
    else if (lin_grad_start == str)
      img_start = lin_grad_start;
    else if (rad_grad_start == str)
      img_start = rad_grad_start;
    else if (con_grad_start == str)
      img_start = con_grad_start;

    if (img_start) {
      return ui_css_parse_image(img_start, &out_shape->image);
    }
  }

  /* Try parsing geometry box or basic shape */
  {
    parse_geometry_box(p, &out_shape->box);
  }

  if (strstr(p, "inset(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_INSET;
  } else if (strstr(p, "circle(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_CIRCLE;
  } else if (strstr(p, "ellipse(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_ELLIPSE;
  } else if (strstr(p, "polygon(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_POLYGON;
  } else if (strstr(p, "path(")) {
    out_shape->shape.type = UI_CSS_BASIC_SHAPE_PATH;
  }

  if (out_shape->shape.type != UI_CSS_BASIC_SHAPE_NONE) {
    const char *paren_start = strchr(p, '(');
    const char *paren_end = strrchr(p, ')');
    if (paren_end) {
      size_t len = (size_t)(paren_end - paren_start) - 1;
      if (len < sizeof(out_shape->shape.arguments)) {
        UI_STRNCPY(out_shape->shape.arguments, len + 1, paren_start + 1, len);
        out_shape->shape.arguments[len] = '\0';
      }
    }

    /* Check if geometry box is appended after shape */
    if (out_shape->box == UI_CSS_GEOMETRY_BOX_NONE && paren_end) {
      const char *after_paren = paren_end + 1;
      {
        parse_geometry_box(after_paren, &out_shape->box);
      }
    }
  }

  if (out_shape->box == UI_CSS_GEOMETRY_BOX_NONE &&
      out_shape->shape.type == UI_CSS_BASIC_SHAPE_NONE) {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a CSS filter list and frees memory.
 * @param[in,out] filter Pointer to the CSS filter to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_filter_destroy(struct ui_css_filter *filter) {
  if (!filter)
    return UI_ERROR_NONE;

  if (filter->functions) {
    struct ui_css_filter_function *current = filter->functions;
    while (current) {
      struct ui_css_filter_function *next = current->next;
      C_MULTIPLATFORM_FREE(current);
      current = next;
    }
  }
  C_MULTIPLATFORM_FREE(filter);
  return UI_ERROR_NONE;
}

/**
 * @brief Parses a CSS filter string.
 * @param[in] str The CSS string to parse.
 * @param[out] out_filter Pointer to the output filter pointer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_filter(const char *str,
                               struct ui_css_filter **out_filter) {
  struct ui_css_filter *filter;
  struct ui_css_filter_function *tail = NULL;

  if (!str || !out_filter)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_filter = NULL;

  {
    skip_whitespace(&str);
  }
  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  filter = (struct ui_css_filter *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_filter));
  if (!filter) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  filter->functions = NULL;

  while (*str) {
    struct ui_css_filter_function *func = NULL;
    enum ui_css_filter_type type = UI_CSS_FILTER_NONE;
    size_t name_len = 0;
    const char *paren_start;
    const char *paren_end;
    const char *arg_str;

    {
      skip_whitespace(&str);
    }
    if (*str == '\0')
      break;

    if (strncmp(str, "url(", 4) == 0) {
      paren_start = str + 3;
      type = UI_CSS_FILTER_URL;
    } else {
      paren_start = strchr(str, '(');
      if (!paren_start)
        goto cleanup_fail;
    }

    if (type != UI_CSS_FILTER_URL) {
      name_len = (size_t)(paren_start - str);
      if (name_len == 4 && strncmp(str, "blur", 4) == 0)
        type = UI_CSS_FILTER_BLUR;
      else if (name_len == 10 && strncmp(str, "brightness", 10) == 0)
        type = UI_CSS_FILTER_BRIGHTNESS;
      else if (name_len == 8 && strncmp(str, "contrast", 8) == 0)
        type = UI_CSS_FILTER_CONTRAST;
      else if (name_len == 11 && strncmp(str, "drop-shadow", 11) == 0)
        type = UI_CSS_FILTER_DROP_SHADOW;
      else if (name_len == 9 && strncmp(str, "grayscale", 9) == 0)
        type = UI_CSS_FILTER_GRAYSCALE;
      else if (name_len == 10 && strncmp(str, "hue-rotate", 10) == 0)
        type = UI_CSS_FILTER_HUE_ROTATE;
      else if (name_len == 6 && strncmp(str, "invert", 6) == 0)
        type = UI_CSS_FILTER_INVERT;
      else if (name_len == 7 && strncmp(str, "opacity", 7) == 0)
        type = UI_CSS_FILTER_OPACITY;
      else if (name_len == 8 && strncmp(str, "saturate", 8) == 0)
        type = UI_CSS_FILTER_SATURATE;
      else if (name_len == 5 && strncmp(str, "sepia", 5) == 0)
        type = UI_CSS_FILTER_SEPIA;
      else
        goto cleanup_fail;
    }

    paren_end = strchr(paren_start, ')');
    if (!paren_end)
      goto cleanup_fail;

    func = (struct ui_css_filter_function *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_css_filter_function));
    if (!func) {
      {
        ui_css_filter_destroy(filter);
      }
      return UI_ERROR_OUT_OF_MEMORY;
    }
    func->type = type;
    func->next = NULL;

    arg_str = paren_start + 1;
    if (type == UI_CSS_FILTER_URL) {
      size_t len = (size_t)(paren_end - arg_str);
      if (len >= sizeof(func->data.url)) {
        len = sizeof(func->data.url) - 1;
      }
      memcpy(func->data.url, arg_str, len);
      func->data.url[len] = '\0';
    } else if (type == UI_CSS_FILTER_DROP_SHADOW) {
      /* Basic parse: offset-x offset-y blur-radius color */
      ui_error_t rc;
      char shadow_str[256];
      size_t len = (size_t)(paren_end - arg_str);
      char *token;
      char *next_token = NULL;
      int part_idx = 0;

      if (len >= sizeof(shadow_str)) {
        len = sizeof(shadow_str) - 1;
      }
      memcpy(shadow_str, arg_str, len);
      shadow_str[len] = '\0';

      func->data.drop_shadow.offset_x.unit = UI_CSS_UNIT_NONE;
      func->data.drop_shadow.offset_y.unit = UI_CSS_UNIT_NONE;
      func->data.drop_shadow.blur_radius.unit = UI_CSS_UNIT_NONE;
      func->data.drop_shadow.has_color = 0;

      /* Parse tokens space-separated */
#if defined(_MSC_VER)
      token = UI_STRTOK(shadow_str, " ", &next_token);
#else
      token = strtok_r(shadow_str, " ", &next_token);
#endif
      while (token && part_idx < 4) {
        if (part_idx == 0) {
          rc = ui_css_parse_value(token, &func->data.drop_shadow.offset_x);
          if (rc != UI_ERROR_NONE) {
            C_MULTIPLATFORM_FREE(func);
            {
              ui_css_filter_destroy(filter);
            }
            return rc;
          }
        } else if (part_idx == 1) {
          rc = ui_css_parse_value(token, &func->data.drop_shadow.offset_y);
          if (rc != UI_ERROR_NONE) {
            C_MULTIPLATFORM_FREE(func);
            {
              ui_css_filter_destroy(filter);
            }
            return rc;
          }
        } else if (part_idx == 2) {
          rc = ui_css_parse_value(token, &func->data.drop_shadow.blur_radius);
          if (rc != UI_ERROR_NONE) {
            if (0)
              return rc;
            /* Maybe it's a color instead of blur_radius */
            {
              ui_error_t color_rc =
                  ui_css_parse_color(token, &func->data.drop_shadow.color);
              (void)color_rc;

              func->data.drop_shadow.has_color = 1;
              break;
            }
          }
        } else { /* part_idx == 3 */
          rc = ui_css_parse_color(token, &func->data.drop_shadow.color);
          if (rc != UI_ERROR_NONE) {
            if (0)
              return rc;
            C_MULTIPLATFORM_FREE(func);
            {
              ui_css_filter_destroy(filter);
            }
            return rc;
          }
          func->data.drop_shadow.has_color = 1;
        }
        part_idx++;
        token = UI_STRTOK(NULL, " ", &next_token);
      }
    } else {
      /* Parse single value */
      char val_str[128];
      size_t len = (size_t)(paren_end - arg_str);
      ui_error_t rc;

      if (len >= sizeof(val_str)) {
        len = sizeof(val_str) - 1;
      }
      memcpy(val_str, arg_str, len);
      val_str[len] = '\0';

      rc = ui_css_parse_value(val_str, &func->data.value);
      if (rc != UI_ERROR_NONE) {
        C_MULTIPLATFORM_FREE(func);
        {
          ui_css_filter_destroy(filter);
        }
        return rc;
      }
    }

    if (!filter->functions) {
      filter->functions = func;
    } else {
      tail->next = func;
    }
    tail = func;

    str = paren_end + 1;
  }

  *out_filter = filter;
  return UI_ERROR_NONE;

cleanup_fail: { ui_css_filter_destroy(filter); }
  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief Parses a CSS blend mode.
 * @param[in] str The CSS string to parse.
 * @param[out] out_blend_mode Pointer to the output blend mode.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_blend_mode(const char *str,
                                   enum ui_css_blend_mode *out_blend_mode) {
  if (!str || !out_blend_mode)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    skip_whitespace(&str);
  }

  if (strcmp(str, "normal") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_NORMAL;
  else if (strcmp(str, "multiply") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_MULTIPLY;
  else if (strcmp(str, "screen") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_SCREEN;
  else if (strcmp(str, "overlay") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_OVERLAY;
  else if (strcmp(str, "darken") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_DARKEN;
  else if (strcmp(str, "lighten") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_LIGHTEN;
  else if (strcmp(str, "color-dodge") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_COLOR_DODGE;
  else if (strcmp(str, "color-burn") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_COLOR_BURN;
  else if (strcmp(str, "hard-light") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_HARD_LIGHT;
  else if (strcmp(str, "soft-light") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_SOFT_LIGHT;
  else if (strcmp(str, "difference") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_DIFFERENCE;
  else if (strcmp(str, "exclusion") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_EXCLUSION;
  else if (strcmp(str, "hue") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_HUE;
  else if (strcmp(str, "saturation") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_SATURATION;
  else if (strcmp(str, "color") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_COLOR;
  else if (strcmp(str, "luminosity") == 0)
    *out_blend_mode = UI_CSS_BLEND_MODE_LUMINOSITY;
  else
    return UI_ERROR_PARSE_FAILED;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a CSS shadow list and frees memory.
 * @param[in,out] list Pointer to the shadow list to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_shadow_list_destroy(struct ui_css_shadow_list *list) {
  if (!list)
    return UI_ERROR_NONE;

  if (list->shadows) {
    struct ui_css_shadow *current = list->shadows;
    while (current) {
      struct ui_css_shadow *next = current->next;
      C_MULTIPLATFORM_FREE(current);
      current = next;
    }
  }
  C_MULTIPLATFORM_FREE(list);
  return UI_ERROR_NONE;
}

/**
 * @brief Parses a CSS shadow string into a list.
 * @param[in] str The CSS string to parse.
 * @param[out] out_shadows Pointer to the output shadow list pointer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_shadow(const char *str,
                               struct ui_css_shadow_list **out_shadows) {
  struct ui_css_shadow_list *list;
  struct ui_css_shadow *tail = NULL;

  if (!str || !out_shadows)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_shadows = NULL;

  {
    skip_whitespace(&str);
  }
  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  list = (struct ui_css_shadow_list *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_shadow_list));
  if (!list) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  list->shadows = NULL;

  while (*str) {
    struct ui_css_shadow *shadow = NULL;
    char token_buf[256];
    char *token;
    char *next_token = NULL;
    int length_idx = 0;
    ui_error_t rc;
    const char *comma_pos;
    size_t part_len;

    {
      skip_whitespace(&str);
    }
    if (*str == '\0')
      break;

    comma_pos = strchr(str, ',');
    if (comma_pos) {
      part_len = (size_t)(comma_pos - str);
    } else {
      part_len = strlen(str);
    }

    if (part_len >= sizeof(token_buf)) {
      part_len = sizeof(token_buf) - 1;
    }
    memcpy(token_buf, str, part_len);
    token_buf[part_len] = '\0';

    shadow = (struct ui_css_shadow *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_css_shadow));
    if (!shadow) {
      {
        ui_css_shadow_list_destroy(list);
      }
      return UI_ERROR_OUT_OF_MEMORY;
    }

    shadow->offset_x.unit = UI_CSS_UNIT_NONE;
    shadow->offset_y.unit = UI_CSS_UNIT_NONE;
    shadow->blur_radius.unit = UI_CSS_UNIT_NONE;
    shadow->blur_radius.value = 0.0f;
    shadow->spread_radius.unit = UI_CSS_UNIT_NONE;
    shadow->spread_radius.value = 0.0f;
    shadow->has_color = 0;
    shadow->is_inset = 0;
    shadow->next = NULL;

    /* Parse tokens space-separated */
    token = UI_STRTOK(token_buf, " ", &next_token);
    while (token) {
      if (strcmp(token, "inset") == 0) {
        shadow->is_inset = 1;
      } else {
        /* Try parsing as a length */
        struct ui_css_value val;
        rc = ui_css_parse_value(token, &val);
        if (rc != UI_ERROR_NONE || val.unit == UI_CSS_UNIT_NONE) {
          ui_error_t color_rc = ui_css_parse_color(token, &shadow->color);
          if (color_rc != UI_ERROR_NONE) {
            C_MULTIPLATFORM_FREE(shadow);
            ui_css_shadow_list_destroy(list);
            if (rc != UI_ERROR_NONE)
              return rc;
            return color_rc;
          }
          shadow->has_color = 1;
        } else {
          if (length_idx == 0) {
            shadow->offset_x = val;
          } else if (length_idx == 1) {
            shadow->offset_y = val;
          } else if (length_idx == 2) {
            shadow->blur_radius = val;
          } else if (length_idx == 3) {
            shadow->spread_radius = val;
          }
          length_idx++;
        }
      }
      token = UI_STRTOK(NULL, " ", &next_token);
    }

    /* Must have at least offset-x and offset-y */
    if (length_idx < 2) {
      C_MULTIPLATFORM_FREE(shadow);
      {
        ui_css_shadow_list_destroy(list);
      }
      return UI_ERROR_PARSE_FAILED;
    }

    if (!list->shadows) {
      list->shadows = shadow;
    } else {
      tail->next = shadow;
    }
    tail = shadow;

    if (comma_pos) {
      str = comma_pos + 1;
    } else {
      str += part_len;
    }
  }

  *out_shadows = list;
  return UI_ERROR_NONE;
}

/**
 * @brief Parses a CSS fill rule string.
 * @param[in] str The CSS string to parse.
 * @param[out] out_rule Pointer to the output fill rule enum.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_fill_rule(const char *str,
                                  enum ui_css_fill_rule *out_rule) {
  if (!str || !out_rule)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    skip_whitespace(&str);
  }
  if (strcmp(str, "nonzero") == 0) {
    *out_rule = UI_CSS_FILL_RULE_NONZERO;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "evenodd") == 0) {
    *out_rule = UI_CSS_FILL_RULE_EVENODD;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief Parses a CSS stroke linecap string.
 * @param[in] str The CSS string to parse.
 * @param[out] out_linecap Pointer to the output stroke linecap enum.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_css_parse_stroke_linecap(const char *str,
                            enum ui_css_stroke_linecap *out_linecap) {
  if (!str || !out_linecap)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    skip_whitespace(&str);
  }
  if (strcmp(str, "butt") == 0) {
    *out_linecap = UI_CSS_STROKE_LINECAP_BUTT;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "round") == 0) {
    *out_linecap = UI_CSS_STROKE_LINECAP_ROUND;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "square") == 0) {
    *out_linecap = UI_CSS_STROKE_LINECAP_SQUARE;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief Parses a CSS stroke linejoin string.
 * @param[in] str The CSS string to parse.
 * @param[out] out_linejoin Pointer to the output stroke linejoin enum.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_css_parse_stroke_linejoin(const char *str,
                             enum ui_css_stroke_linejoin *out_linejoin) {
  if (!str || !out_linejoin)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    skip_whitespace(&str);
  }
  if (strcmp(str, "miter") == 0) {
    *out_linejoin = UI_CSS_STROKE_LINEJOIN_MITER;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "round") == 0) {
    *out_linejoin = UI_CSS_STROKE_LINEJOIN_ROUND;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "bevel") == 0) {
    *out_linejoin = UI_CSS_STROKE_LINEJOIN_BEVEL;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief Parses a CSS paint string (color or url).
 * @param[in] str The CSS string to parse.
 * @param[out] out_paint Pointer to the output paint structure.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_paint(const char *str, struct ui_css_paint *out_paint) {
  if (!str || !out_paint)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    skip_whitespace(&str);
  }

  out_paint->type = UI_CSS_PAINT_NONE;
  out_paint->url[0] = '\0';

  if (strcmp(str, "none") == 0) {
    out_paint->type = UI_CSS_PAINT_NONE;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "currentcolor") == 0 ||
             strcmp(str, "currentColor") == 0) {
    out_paint->type = UI_CSS_PAINT_CURRENT_COLOR;
    return UI_ERROR_NONE;
  } else if (strncmp(str, "url(", 4) == 0) {
    const char *start = str + 4;
    const char *end = strchr(start, ')');
    if (end) {
      size_t len = (size_t)(end - start);
      if (len >= 2) {
        if (start[0] == '"' && start[len - 1] == '"') {
          start++;
          len -= 2;
        } else if (start[0] == '\'' && start[len - 1] == '\'') {
          start++;
          len -= 2;
        }
      }
      if (len < sizeof(out_paint->url)) {
        UI_STRNCPY(out_paint->url, len + 1, start, len);
        out_paint->url[len] = '\0';
        out_paint->type = UI_CSS_PAINT_URL;
        return UI_ERROR_NONE;
      }
    }
    return UI_ERROR_PARSE_FAILED;
  }

  {
    ui_error_t p_rc = ui_css_parse_color(str, &out_paint->color);
    if (p_rc != UI_ERROR_NONE)
      return p_rc;
    out_paint->type = UI_CSS_PAINT_COLOR;
    return UI_ERROR_NONE;
  }
}

/**
 * @brief Parses a CSS dasharray string.
 * @param[in] str The CSS string to parse.
 * @param[out] out_dasharray Pointer to the output dasharray structure.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_dasharray(const char *str,
                                  struct ui_css_dasharray *out_dasharray) {
  char token_buf[512];
  char *token;
  char *next_token = NULL;
  ui_error_t rc;

  if (!str || !out_dasharray)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    skip_whitespace(&str);
  }

  out_dasharray->count = 0;

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  UI_STRNCPY(token_buf, sizeof(token_buf), str, sizeof(token_buf) - 1);
  token_buf[sizeof(token_buf) - 1] = '\0';

  /* Replace commas with spaces to allow strtok to handle either */
  {
    char *p = token_buf;
    while (*p) {
      if (*p == ',')
        *p = ' ';
      p++;
    }
  }

  token = UI_STRTOK(token_buf, " ", &next_token);
  while (token && out_dasharray->count < 16) {
    struct ui_css_value val;
    rc = ui_css_parse_value(token, &val);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    out_dasharray->values[out_dasharray->count++] = val;
    token = UI_STRTOK(NULL, " ", &next_token);
  }

  if (out_dasharray->count == 0) {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Parses a CSS easing function string.
 * @param[in] str The CSS string to parse.
 * @param[out] out_easing Pointer to the output easing function structure.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_css_parse_easing_function(const char *str,
                             struct ui_css_easing_function *out_easing) {
  if (!str || !out_easing)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    skip_whitespace(&str);
  }

  if (strcmp(str, "linear") == 0) {
    out_easing->type = UI_CSS_EASING_LINEAR;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "ease") == 0) {
    out_easing->type = UI_CSS_EASING_EASE;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "ease-in") == 0) {
    out_easing->type = UI_CSS_EASING_EASE_IN;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "ease-out") == 0) {
    out_easing->type = UI_CSS_EASING_EASE_OUT;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "ease-in-out") == 0) {
    out_easing->type = UI_CSS_EASING_EASE_IN_OUT;
    return UI_ERROR_NONE;
  } else if (strncmp(str, "cubic-bezier(", 13) == 0) {
    float x1, y1, x2, y2;
    if (UI_SSCANF(str + 13, "%f , %f , %f , %f", &x1, &y1, &x2, &y2) == 4) {
      out_easing->type = UI_CSS_EASING_CUBIC_BEZIER;
      out_easing->data.cubic_bezier.x1 = x1;
      out_easing->data.cubic_bezier.y1 = y1;
      out_easing->data.cubic_bezier.x2 = x2;
      out_easing->data.cubic_bezier.y2 = y2;
      return UI_ERROR_NONE;
    }
  } else if (strncmp(str, "steps(", 6) == 0) {
    int count = 0;
    char pos_str[32] = {0};
    int matched;
#if defined(_MSC_VER)
    matched = sscanf_s(str + 6, "%d , %31[^)]", &count, pos_str,
                       (unsigned int)sizeof(pos_str));
#else
    matched = UI_SSCANF(str + 6, "%d , %31[^)]", &count, pos_str);
#endif
    if (matched >= 1) {
      out_easing->type = UI_CSS_EASING_STEPS;
      out_easing->data.steps.count = count;
      out_easing->data.steps.position = UI_CSS_STEPS_END; /* Default */

      if (matched == 2) {
        char *p = pos_str;
        if (strstr(p, "jump-start"))
          out_easing->data.steps.position = UI_CSS_STEPS_JUMP_START;
        else if (strstr(p, "jump-end"))
          out_easing->data.steps.position = UI_CSS_STEPS_JUMP_END;
        else if (strstr(p, "jump-none"))
          out_easing->data.steps.position = UI_CSS_STEPS_JUMP_NONE;
        else if (strstr(p, "jump-both"))
          out_easing->data.steps.position = UI_CSS_STEPS_JUMP_BOTH;
        else if (strstr(p, "start"))
          out_easing->data.steps.position = UI_CSS_STEPS_START;
        else if (strstr(p, "end"))
          out_easing->data.steps.position = UI_CSS_STEPS_END;
      }
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief Destroys a CSS transition list.
 * @param[in,out] transitions Pointer to the CSS transitions to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_transition_destroy(struct ui_css_transition *transitions) {
  while (transitions) {
    struct ui_css_transition *next = transitions->next;
    C_MULTIPLATFORM_FREE(transitions);
    transitions = next;
  }
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
