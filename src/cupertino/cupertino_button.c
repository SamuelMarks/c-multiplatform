#include "cupertino/cupertino_button.h"
#include <string.h>

static int cupertino_button_validate_style(const CupertinoButtonStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->variant > CUPERTINO_BUTTON_VARIANT_GRAY) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->size >= CUPERTINO_BUTTON_SIZE_MAX) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->shape > CUPERTINO_BUTTON_SHAPE_CIRCLE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_button_style_init(CupertinoButtonStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));
  style->variant = CUPERTINO_BUTTON_VARIANT_PLAIN;
  style->size = CUPERTINO_BUTTON_SIZE_MEDIUM;
  style->shape = CUPERTINO_BUTTON_SHAPE_CAPSULE;
  cupertino_color_get_system(CUPERTINO_COLOR_BLUE, CMP_FALSE,
                             &style->tint_color);
  style->is_dark_mode = CMP_FALSE;

  /* Initialize typography */
  {
    CupertinoTypographyScale typo;
    cupertino_typography_scale_init(&typo);
    cupertino_typography_get_style(&typo, CUPERTINO_TYPOGRAPHY_BODY,
                                   &style->text_style);
    style->text_style.weight = 600; /* Buttons use semibold */
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_button_init(CupertinoButton *button,
                                           const CMPTextBackend *text_backend) {
  if (button == NULL || text_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (text_backend->vtable == NULL ||
      text_backend->vtable->measure_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  memset(button, 0, sizeof(*button));
  button->text_backend = *text_backend;
  cupertino_button_style_init(&button->style);

  button->widget.ctx = button;
  /* Widget vtable setup will happen later if integrated fully into the widget
   * tree */

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_button_set_label(CupertinoButton *button,
                                                const char *label_utf8,
                                                cmp_usize label_len) {
  if (button == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (label_utf8 == NULL && label_len > 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  button->label_utf8 = label_utf8;
  button->label_len = label_len;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_button_set_style(
    CupertinoButton *button, const CupertinoButtonStyle *style) {
  int rc;
  if (button == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cupertino_button_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  button->style = *style;
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_button_measure(CupertinoButton *button,
                                              CMPMeasureSpec width_spec,
                                              CMPMeasureSpec height_spec,
                                              CMPSize *out_size) {
  CMPScalar padding_x = 0.0f;
  CMPScalar padding_y = 0.0f;
  CMPTextMetrics text_metrics = {0};
  int rc;

  if (button == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* Base padding depending on size */
  switch (button->style.size) {
  case CUPERTINO_BUTTON_SIZE_SMALL:
    padding_x = 10.0f;
    padding_y = 4.0f;
    break;
  case CUPERTINO_BUTTON_SIZE_MEDIUM:
    padding_x = 14.0f;
    padding_y = 8.0f;
    break;
  case CUPERTINO_BUTTON_SIZE_LARGE:
    padding_x = 20.0f;
    padding_y = 14.0f;
    break;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (button->style.variant == CUPERTINO_BUTTON_VARIANT_PLAIN) {
    padding_x = 0.0f;
    padding_y = 0.0f;
  }

  if (button->label_utf8 && button->label_len > 0) {
    CMPHandle font = {0};

    if (cmp_text_font_create(&button->text_backend, &button->style.text_style,
                             &font) == CMP_OK) {
      rc =
          cmp_text_measure_utf8(&button->text_backend, font, button->label_utf8,
                                button->label_len, 0, &text_metrics);
      cmp_text_font_destroy(&button->text_backend, font);
      if (rc != CMP_OK)
        return rc;
    }
  }

  out_size->width = text_metrics.width + (padding_x * 2.0f);
  out_size->height = text_metrics.height + (padding_y * 2.0f);

  if (button->style.size == CUPERTINO_BUTTON_SIZE_MEDIUM &&
      out_size->height < 44.0f) {
    out_size->height = 44.0f; /* iOS minimum hit target */
  }

  /* Respect measure specs */
  if (width_spec.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width_spec.size;
  } else if (width_spec.mode == CMP_MEASURE_AT_MOST &&
             out_size->width > width_spec.size) {
    out_size->width = width_spec.size;
  }

  if (height_spec.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height_spec.size;
  } else if (height_spec.mode == CMP_MEASURE_AT_MOST &&
             out_size->height > height_spec.size) {
    out_size->height = height_spec.size;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_button_layout(CupertinoButton *button,
                                             CMPRect bounds) {
  if (button == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  button->bounds = bounds;
  return CMP_OK;
}

static int cupertino_button_resolve_colors(const CupertinoButton *button,
                                           CMPColor *bg, CMPColor *text) {
  CMPColor tint = button->style.tint_color;
  CMPBool is_dark = button->style.is_dark_mode;

  bg->r = 0;
  bg->g = 0;
  bg->b = 0;
  bg->a = 0;
  text->r = 0;
  text->g = 0;
  text->b = 0;
  text->a = 1;

  switch (button->style.variant) {
  case CUPERTINO_BUTTON_VARIANT_PLAIN:
    text->r = tint.r;
    text->g = tint.g;
    text->b = tint.b;
    text->a = tint.a;
    if (button->is_pressed) {
      text->a *= 0.5f; /* iOS typically halves alpha for plain button press */
    } else if (button->is_hovered) {
      text->a *= 0.8f;
    }
    if (button->is_disabled) {
      CMPColor sys_gray;
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY, is_dark, &sys_gray);
      text->r = sys_gray.r;
      text->g = sys_gray.g;
      text->b = sys_gray.b;
      text->a = sys_gray.a;
    }
    break;
  case CUPERTINO_BUTTON_VARIANT_TINTED:
    bg->r = tint.r;
    bg->g = tint.g;
    bg->b = tint.b;
    bg->a = is_dark ? 0.2f : 0.15f;
    text->r = tint.r;
    text->g = tint.g;
    text->b = tint.b;
    text->a = 1.0f;
    if (button->is_pressed) {
      bg->a = is_dark ? 0.3f : 0.25f;
    } else if (button->is_hovered) {
      bg->a = is_dark ? 0.25f : 0.2f;
    }
    if (button->is_disabled) {
      CMPColor sys_gray;
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY4, is_dark, &sys_gray);
      bg->r = sys_gray.r;
      bg->g = sys_gray.g;
      bg->b = sys_gray.b;
      bg->a = sys_gray.a;
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY, is_dark, &sys_gray);
      text->r = sys_gray.r;
      text->g = sys_gray.g;
      text->b = sys_gray.b;
      text->a = sys_gray.a;
    }
    break;
  case CUPERTINO_BUTTON_VARIANT_FILLED:
    bg->r = tint.r;
    bg->g = tint.g;
    bg->b = tint.b;
    bg->a = 1.0f;
    text->r = 1.0f;
    text->g = 1.0f;
    text->b = 1.0f;
    text->a = 1.0f; /* Always white */
    if (button->is_pressed) {
      bg->r *= 0.8f;
      bg->g *= 0.8f;
      bg->b *= 0.8f; /* Darken */
    } else if (button->is_hovered) {
      bg->r *= 0.9f;
      bg->g *= 0.9f;
      bg->b *= 0.9f;
    }
    if (button->is_disabled) {
      CMPColor sys_gray;
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY5, is_dark, &sys_gray);
      bg->r = sys_gray.r;
      bg->g = sys_gray.g;
      bg->b = sys_gray.b;
      bg->a = sys_gray.a;
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY, is_dark, &sys_gray);
      text->r = sys_gray.r;
      text->g = sys_gray.g;
      text->b = sys_gray.b;
      text->a = sys_gray.a;
    }
    break;
  case CUPERTINO_BUTTON_VARIANT_GRAY: {
    CMPColor sys_gray_bg;
    cupertino_color_get_system(CUPERTINO_COLOR_GRAY5, is_dark, &sys_gray_bg);
    bg->r = sys_gray_bg.r;
    bg->g = sys_gray_bg.g;
    bg->b = sys_gray_bg.b;
    bg->a = sys_gray_bg.a;
    text->r = tint.r;
    text->g = tint.g;
    text->b = tint.b;
    text->a = 1.0f;

    if (button->is_pressed) {
      CMPColor sys_gray4;
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY4, is_dark, &sys_gray4);
      bg->r = sys_gray4.r;
      bg->g = sys_gray4.g;
      bg->b = sys_gray4.b;
      bg->a = sys_gray4.a;
    }
    if (button->is_disabled) {
      CMPColor sys_gray;
      cupertino_color_get_system(CUPERTINO_COLOR_GRAY, is_dark, &sys_gray);
      text->r = sys_gray.r;
      text->g = sys_gray.g;
      text->b = sys_gray.b;
      text->a = sys_gray.a;
    }
  } break;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

CMP_API int CMP_CALL cupertino_button_paint(const CupertinoButton *button,
                                            CMPPaintContext *ctx) {
  CMPColor bg_color, text_color;
  CMPScalar corner_radius = 0.0f;
  int rc;

  if (button == NULL || ctx == NULL || ctx->gfx == NULL ||
      ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (ctx->gfx->vtable->draw_path == NULL ||
      ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = cupertino_button_resolve_colors(button, &bg_color, &text_color);
  if (rc != CMP_OK)
    return rc;

  if (button->style.shape == CUPERTINO_BUTTON_SHAPE_CAPSULE) {
    corner_radius =
        (button->bounds.height > button->bounds.width ? button->bounds.width
                                                      : button->bounds.height) /
        2.0f;
  } else if (button->style.shape == CUPERTINO_BUTTON_SHAPE_ROUNDED_RECTANGLE) {
    corner_radius = 8.0f; /* Standard corner */
  } else if (button->style.shape == CUPERTINO_BUTTON_SHAPE_CIRCLE) {
    corner_radius =
        (button->bounds.height > button->bounds.width ? button->bounds.width
                                                      : button->bounds.height) /
        2.0f;
  }

  if (bg_color.a > 0.0f) {
    if (button->style.shape == CUPERTINO_BUTTON_SHAPE_ROUNDED_RECTANGLE) {
      /* Use continuous curves for rounded rects */
      CMPPath path = {0};
      CMPAllocator alloc;
      rc = cmp_get_default_allocator(&alloc);
      if (rc == CMP_OK) {
        rc = cmp_path_init(&path, &alloc, 32);
        if (rc == CMP_OK) {
          cupertino_shape_append_squircle(&path, button->bounds, corner_radius,
                                          corner_radius, corner_radius,
                                          corner_radius);
          ctx->gfx->vtable->draw_path(ctx->gfx->ctx, &path, bg_color);
          cmp_path_shutdown(&path);
        }
      }
    } else {
      /* Standard arcs for fully circular corners */
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &button->bounds, bg_color,
                                  corner_radius);
    }
  }

  if (button->label_utf8 && button->label_len > 0) {
    CMPTextStyle applied_style = button->style.text_style;
    CMPTextMetrics metrics = {0};
    CMPHandle font = {0};
    CMPScalar text_x, text_y;

    applied_style.color = text_color;

    if (cmp_text_font_create(&button->text_backend, &applied_style, &font) ==
        CMP_OK) {
      cmp_text_measure_utf8(&button->text_backend, font, button->label_utf8,
                            button->label_len, 0, &metrics);

      /* Center text */
      text_x = button->bounds.x + (button->bounds.width - metrics.width) / 2.0f;
      text_y = button->bounds.y +
               (button->bounds.height - metrics.height) / 2.0f +
               metrics.baseline;

      cmp_text_draw_utf8_gfx(ctx->gfx, font, button->label_utf8,
                             button->label_len, 0, text_x, text_y, text_color);

      cmp_text_font_destroy(&button->text_backend, font);
    }
  }

  return CMP_OK;
}
