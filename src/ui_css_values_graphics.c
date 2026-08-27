ui_error_t ui_css_parse_image(const char *str, struct ui_css_image *out_image) {
  if (!str || !out_image)
    return UI_ERROR_INVALID_ARGUMENT;

  { skip_whitespace(&str); }
  out_image->type = UI_CSS_IMAGE_NONE;

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  if (strncmp(str, "url(", 4) == 0) {
    const char *start = str + 4;
    const char *end = strchr(start, ')');
    if (end) {
      size_t len = (size_t)(end - start);
      /* Strip quotes if present */
      if (len >= 2) {
        if (start[0] == '"' && start[len - 1] == '"') {
          start++;
          len -= 2;
        } else if (start[0] == '\'' && start[len - 1] == '\'') {
          start++;
          len -= 2;
        }
      }
      if (len < sizeof(out_image->data.url)) {
        UI_STRNCPY(out_image->data.url, len + 1, start, len);
        out_image->data.url[len] = '\0';
        out_image->type = UI_CSS_IMAGE_URL;
        return UI_ERROR_NONE;
      }
    }
  } else if (strncmp(str, "linear-gradient(", 16) == 0 ||
             strncmp(str, "repeating-linear-gradient(", 26) == 0) {
    out_image->type = UI_CSS_IMAGE_LINEAR_GRADIENT;
    out_image->data.linear_gradient.angle = 180.0f; /* default to bottom */
    out_image->data.linear_gradient.stop_count = 2; /* mock 2 stops */
    {
      ui_error_t image_color_rc = ui_css_parse_color(
          "black", &out_image->data.linear_gradient.stops[0].color);
      (void)image_color_rc;
    }
    {
      ui_error_t image_color_rc = ui_css_parse_color(
          "white", &out_image->data.linear_gradient.stops[1].color);
      (void)image_color_rc;
    }
    return UI_ERROR_NONE;
  } else if (strncmp(str, "radial-gradient(", 16) == 0 ||
             strncmp(str, "repeating-radial-gradient(", 26) == 0) {
    out_image->type = UI_CSS_IMAGE_RADIAL_GRADIENT;
    out_image->data.radial_gradient.stop_count = 2;
    {
      ui_error_t image_color_rc = ui_css_parse_color(
          "black", &out_image->data.radial_gradient.stops[0].color);
      (void)image_color_rc;
    }
    {
      ui_error_t image_color_rc = ui_css_parse_color(
          "white", &out_image->data.radial_gradient.stops[1].color);
      (void)image_color_rc;
    }
    return UI_ERROR_NONE;
  } else if (strncmp(str, "conic-gradient(", 15) == 0 ||
             strncmp(str, "repeating-conic-gradient(", 25) == 0) {
    out_image->type = UI_CSS_IMAGE_CONIC_GRADIENT;
    out_image->data.conic_gradient.angle = 0.0f;
    out_image->data.conic_gradient.stop_count = 2;
    {
      ui_error_t image_color_rc = ui_css_parse_color(
          "black", &out_image->data.conic_gradient.stops[0].color);
      (void)image_color_rc;
    }
    {
      ui_error_t image_color_rc = ui_css_parse_color(
          "white", &out_image->data.conic_gradient.stops[1].color);
      (void)image_color_rc;
    }
    return UI_ERROR_NONE;
  } else if (strncmp(str, "image-set(", 10) == 0 ||
             strncmp(str, "-webkit-image-set(", 18) == 0) {
    out_image->type = UI_CSS_IMAGE_IMAGE_SET;
    out_image->data.image_set.image_count = 1;
    UI_STRCPY(out_image->data.image_set.urls[0],
              sizeof(out_image->data.image_set.urls[0]), "mock.png");
    out_image->data.image_set.resolutions[0] = 1;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

/*
 * @brief parse_geometry_box.
 * @param str Parameter str.
 * @param out_box Parameter out_box.
 * @return Return value.
 */
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

/*
 * @brief ui_css_parse_clip_path.
 * @param str Parameter str.
 * @param out_clip_path Parameter out_clip_path.
 * @return Return value.
 */
ui_error_t ui_css_parse_clip_path(const char *str,
                                  struct ui_css_clip_path *out_clip_path) {
  if (!str || !out_clip_path)
    return UI_ERROR_INVALID_ARGUMENT;

  { skip_whitespace(&str); }
  out_clip_path->geometry_box = UI_CSS_GEOMETRY_BOX_NONE;
  out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_NONE;
  out_clip_path->shape.arguments[0] = '\0';
  out_clip_path->url[0] = '\0';

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  if (strncmp(str, "url(", 4) == 0) {
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
      if (len < sizeof(out_clip_path->url)) {
        UI_STRNCPY(out_clip_path->url, len + 1, start, len);
        out_clip_path->url[len] = '\0';
        return UI_ERROR_NONE;
      }
    }
    return UI_ERROR_PARSE_FAILED;
  }

  { parse_geometry_box(str, &out_clip_path->geometry_box); }

  if (strstr(str, "inset(")) {
    out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_INSET;
  } else if (strstr(str, "circle(")) {
    out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_CIRCLE;
  } else if (strstr(str, "ellipse(")) {
    out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_ELLIPSE;
  } else if (strstr(str, "polygon(")) {
    out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_POLYGON;
  } else if (strstr(str, "path(")) {
    out_clip_path->shape.type = UI_CSS_BASIC_SHAPE_PATH;
  }

  if (out_clip_path->shape.type != UI_CSS_BASIC_SHAPE_NONE) {
    const char *paren_start = strchr(str, '(');
    const char *paren_end = strrchr(str, ')');
    if (paren_end) {
      size_t len = (size_t)(paren_end - paren_start) - 1;
      if (len < sizeof(out_clip_path->shape.arguments)) {
        UI_STRNCPY(out_clip_path->shape.arguments, len + 1, paren_start + 1,
                   len);
        out_clip_path->shape.arguments[len] = '\0';
      }
    }
  }

  if (out_clip_path->geometry_box == UI_CSS_GEOMETRY_BOX_NONE &&
      out_clip_path->shape.type == UI_CSS_BASIC_SHAPE_NONE) {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

/*
 * @brief ui_css_parse_mask.
 * @param str Parameter str.
 * @param out_mask Parameter out_mask.
 * @return Return value.
 */
ui_error_t ui_css_parse_mask(const char *str,
                             struct ui_css_mask_layer *out_mask) {
  if (!str || !out_mask)
    return UI_ERROR_INVALID_ARGUMENT;

  { skip_whitespace(&str); }

  /* Initialize with defaults */
  out_mask->image.type = UI_CSS_IMAGE_NONE;
  out_mask->mode = UI_CSS_MASK_MODE_MATCH_SOURCE;
  out_mask->clip = UI_CSS_GEOMETRY_BOX_BORDER_BOX;
  out_mask->origin = UI_CSS_GEOMETRY_BOX_BORDER_BOX;
  out_mask->composite = UI_CSS_MASK_COMPOSITE_ADD;

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  /* Extract components using strstr (mock parsing) */
  if (strstr(str, "luminance"))
    out_mask->mode = UI_CSS_MASK_MODE_LUMINANCE;
  else if (strstr(str, "alpha"))
    out_mask->mode = UI_CSS_MASK_MODE_ALPHA;
  else if (strstr(str, "match-source"))
    out_mask->mode = UI_CSS_MASK_MODE_MATCH_SOURCE;

  if (strstr(str, "subtract"))
    out_mask->composite = UI_CSS_MASK_COMPOSITE_SUBTRACT;
  else if (strstr(str, "intersect"))
    out_mask->composite = UI_CSS_MASK_COMPOSITE_INTERSECT;
  else if (strstr(str, "exclude"))
    out_mask->composite = UI_CSS_MASK_COMPOSITE_EXCLUDE;
  else
    out_mask->composite = UI_CSS_MASK_COMPOSITE_ADD;

  { parse_geometry_box(str, &out_mask->clip); }
  out_mask->origin = out_mask->clip; /* In real parsing they are separate */

  /* Try parsing an image */
  {
    const char *url_start = strstr(str, "url(");
    const char *lin_grad_start = strstr(str, "linear-gradient(");
    const char *rad_grad_start = strstr(str, "radial-gradient(");
    const char *con_grad_start = strstr(str, "conic-gradient(");
    const char *img_start = NULL;

    if (url_start)
      img_start = url_start;
    else if (lin_grad_start)
      img_start = lin_grad_start;
    else if (rad_grad_start)
      img_start = rad_grad_start;
    else if (con_grad_start)
      img_start = con_grad_start;

    if (img_start) {
      /* We will just try parsing the substring */
      {
        ui_error_t mask_img_rc =
            ui_css_parse_image(img_start, &out_mask->image);
        (void)mask_img_rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

/*
 * @brief ui_css_transform_destroy.
 * @param transform Parameter transform.
 * @return Return value.
 */
ui_error_t ui_css_transform_destroy(struct ui_css_transform *transform) {
  if (!transform)
    return UI_ERROR_NONE;

  if (transform->functions) {
    struct ui_css_transform_function *current = transform->functions;
    while (current) {
      struct ui_css_transform_function *next = current->next;
      C_MULTIPLATFORM_FREE(current);
      current = next;
    }
  }
  C_MULTIPLATFORM_FREE(transform);
  return UI_ERROR_NONE;
}

/*
 * @brief ui_css_parse_transform.
 * @param str Parameter str.
 * @param out_transform Parameter out_transform.
 * @return Return value.
 */
