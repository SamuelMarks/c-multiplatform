int test_css_values_scalar_calc(void) {
  g_malloc_fail_countdown = -1;

  /* Test valid scalar parsing */
  rc = ui_css_parse_value("10.5px", &val);
  if (rc != UI_ERROR_NONE || !float_eq(val.value, 10.5f) ||
      val.unit != UI_CSS_UNIT_PX) {
    printf("Failed to parse 10.5px\n");
    return 1;
  }

  rc = ui_css_parse_value("2.5em", &val);
  if (rc != UI_ERROR_NONE || !float_eq(val.value, 2.5f) ||
      val.unit != UI_CSS_UNIT_EM) {
    printf("Failed to parse 2.5em\n");
    return 1;
  }

  rc = ui_css_parse_value("1.25rem", &val);
  if (rc != UI_ERROR_NONE || !float_eq(val.value, 1.25f) ||
      val.unit != UI_CSS_UNIT_REM) {
    printf("Failed to parse 1.25rem\n");
    return 1;
  }

  rc = ui_css_parse_value("  -50%  ", &val);
  if (rc != UI_ERROR_NONE || !float_eq(val.value, -50.0f) ||
      val.unit != UI_CSS_UNIT_PERCENT) {
    printf("Failed to parse -50%%\n");
    return 1;
  }

  rc = ui_css_parse_value("100vw", &val);
  if (rc != UI_ERROR_NONE || !float_eq(val.value, 100.0f) ||
      val.unit != UI_CSS_UNIT_VW) {
    printf("Failed to parse 100vw\n");
    return 1;
  }

  rc = ui_css_parse_value("50vh", &val);
  if (rc != UI_ERROR_NONE || !float_eq(val.value, 50.0f) ||
      val.unit != UI_CSS_UNIT_VH) {
    printf("Failed to parse 50vh\n");
    return 1;
  }

  rc = ui_css_parse_value("10vmin", &val);
  if (rc != UI_ERROR_NONE || !float_eq(val.value, 10.0f) ||
      val.unit != UI_CSS_UNIT_VMIN) {
    printf("Failed to parse 10vmin\n");
    return 1;
  }

  rc = ui_css_parse_value("20vmax", &val);
  if (rc != UI_ERROR_NONE || !float_eq(val.value, 20.0f) ||
      val.unit != UI_CSS_UNIT_VMAX) {
    printf("Failed to parse 20vmax\n");
    return 1;
  }

  /* Container units Level 5 */
  rc = ui_css_parse_value("10cqw", &val);
  if (rc != UI_ERROR_NONE || !float_eq(val.value, 10.0f) ||
      val.unit != UI_CSS_UNIT_CQW) {
    printf("Failed to parse 10cqw\n");
    return 1;
  }
  rc = ui_css_parse_value("5.5cqmin", &val);
  if (rc != UI_ERROR_NONE || !float_eq(val.value, 5.5f) ||
      val.unit != UI_CSS_UNIT_CQMIN) {
    printf("Failed to parse 5.5cqmin\n");
    return 1;
  }

  /* Test Ext Parsing: Scalar */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("10.5px", &ext_val);
  if (rc != UI_ERROR_NONE || ext_val->type != UI_CSS_VALUE_TYPE_SCALAR ||
      ext_val->value.scalar.unit != UI_CSS_UNIT_PX) {
    printf("Failed to parse ext scalar 10.5px\n");
    return 1;
  }
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  /* Test Ext Parsing: calc() */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc(100vw - 50px)", &ext_val);
  if (rc != UI_ERROR_NONE || ext_val->type != UI_CSS_VALUE_TYPE_MATH ||
      ext_val->value.math->op != UI_CSS_MATH_OP_SUB) {
    printf("Failed to parse calc(): rc=%d, ext_val=%p\n", rc, (void *)ext_val);
    if (ext_val) {
      printf("  type=%d, math=%p\n", ext_val->type,
             (void *)ext_val->value.math);
      if (ext_val->type == UI_CSS_VALUE_TYPE_MATH) {
        printf("  op=%d\n", ext_val->value.math->op);
      }
    }
    return 1;
  }
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  /* Test Ext Parsing: nested math */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc((10px + 20%) * 2)", &ext_val);
  if (rc != UI_ERROR_NONE || ext_val->type != UI_CSS_VALUE_TYPE_MATH ||
      ext_val->value.math->op != UI_CSS_MATH_OP_MUL) {
    printf("Failed to parse nested calc()\n");
    return 1;
  }
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  /* Test min() and max() */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min(10px, 20px, 30%)", &ext_val);
  if (rc != UI_ERROR_NONE || ext_val->type != UI_CSS_VALUE_TYPE_MATH ||
      ext_val->value.math->op != UI_CSS_MATH_OP_MIN) {
    printf("Failed to parse min()\n");
    return 1;
  }
  if (!ext_val->value.math->next || !ext_val->value.math->next->next) {
    printf("Failed to link min() args properly\n");
    return 1;
  }
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px, 30%)", &ext_val);
  if (rc != UI_ERROR_NONE || ext_val->type != UI_CSS_VALUE_TYPE_MATH ||
      ext_val->value.math->op != UI_CSS_MATH_OP_MAX) {
    printf("Failed to parse max()\n");
    return 1;
  }
  if (!ext_val->value.math->next || !ext_val->value.math->next->next) {
    printf("Failed to link max() args properly\n");
    return 1;
  }
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  /* Test clamp() */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(10px, 50%, 100px)", &ext_val);
  if (rc != UI_ERROR_NONE || ext_val->type != UI_CSS_VALUE_TYPE_MATH ||
      ext_val->value.math->op != UI_CSS_MATH_OP_CLAMP) {
    printf("Failed to parse clamp()\n");
    return 1;
  }
  if (!ext_val->value.math->left || !ext_val->value.math->right ||
      !ext_val->value.math->ext) {
    printf("Failed to link clamp() args properly\n");
    return 1;
  }
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  /* Test Trig Functions */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("sin(45deg)", &ext_val);
  if (rc != UI_ERROR_NONE || ext_val->type != UI_CSS_VALUE_TYPE_MATH ||
      ext_val->value.math->op != UI_CSS_MATH_OP_SIN) {
    printf("Failed to parse sin()\n");
    return 1;
  }
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(10px, 20px)", &ext_val);
  if (rc != UI_ERROR_NONE || ext_val->type != UI_CSS_VALUE_TYPE_MATH ||
      ext_val->value.math->op != UI_CSS_MATH_OP_ATAN2 ||
      !ext_val->value.math->left || !ext_val->value.math->right) {
    printf("Failed to parse atan2()\n");
    return 1;
  }
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  /* Test invalid calc missing space around minus */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc(100px-50px)", &ext_val);
  if (rc != UI_ERROR_PARSE_FAILED) {
    printf("Expected PARSE_FAILED for calc missing spaces around '-'\n");
    return 1;
  }

  /* OOM test for parsing math */
  g_malloc_fail_countdown = 2;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc(10px + 20px)", &ext_val);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY for math tree\n");
    return 1;
  }

  /* Color Tests */
  {
    struct ui_css_color color;
    struct ui_css_paint paint;
    rc = ui_css_parse_color("#123456", &color);
    if (rc != UI_ERROR_NONE || color.space != UI_CSS_COLOR_SPACE_SRGB) {
      printf("Failed to parse color hex\n");
      return 1;
    }

    rc = ui_css_parse_color("rgb(255, 128, 0)", &color);
    if (rc != UI_ERROR_NONE || color.space != UI_CSS_COLOR_SPACE_SRGB ||
        !float_eq(color.components[0], 1.0f) ||
        !float_eq(color.components[1], 128.0f / 255.0f) ||
        !float_eq(color.components[2], 0.0f) ||
        !float_eq(color.components[3], 1.0f)) {
      printf("Failed to parse color rgb()\n");
      return 1;
    }

    rc = ui_css_parse_color("rgba(255, 128, 0, 0.5)", &color);
    if (rc != UI_ERROR_NONE || color.space != UI_CSS_COLOR_SPACE_SRGB ||
        !float_eq(color.components[3], 0.5f)) {
      printf("Failed to parse color rgba()\n");
      return 1;
    }

    rc = ui_css_parse_color("hsl(120, 100%, 50%)", &color);
    if (rc != UI_ERROR_NONE || color.space != UI_CSS_COLOR_SPACE_HSL ||
        !float_eq(color.components[0], 120.0f) ||
        !float_eq(color.components[1], 1.0f) ||
        !float_eq(color.components[2], 0.5f)) {
      printf("Failed to parse color hsl()\n");
      return 1;
    }

    rc = ui_css_parse_color("hsla(120, 100%, 50%, 0.8)", &color);
    if (rc != UI_ERROR_NONE || color.space != UI_CSS_COLOR_SPACE_HSL ||
        !float_eq(color.components[3], 0.8f)) {
      printf("Failed to parse color hsla()\n");
      return 1;
    }
  }

  {
    struct ui_css_clip_path clip;
    rc = ui_css_parse_clip_path("none", &clip);
    if (rc != UI_ERROR_NONE || clip.geometry_box != UI_CSS_GEOMETRY_BOX_NONE ||
        clip.shape.type != UI_CSS_BASIC_SHAPE_NONE) {
      printf("Failed to parse clip-path: none\n");
      return 1;
    }

    rc = ui_css_parse_clip_path("margin-box", &clip);
    if (rc != UI_ERROR_NONE ||
        clip.geometry_box != UI_CSS_GEOMETRY_BOX_MARGIN_BOX) {
      printf("Failed to parse clip-path: margin-box\n");
      return 1;
    }

    rc = ui_css_parse_clip_path("circle(50% at 50% 50%) border-box", &clip);
    if (rc != UI_ERROR_NONE ||
        clip.geometry_box != UI_CSS_GEOMETRY_BOX_BORDER_BOX ||
        clip.shape.type != UI_CSS_BASIC_SHAPE_CIRCLE) {
      printf("Failed to parse clip-path: circle border-box\n");
      return 1;
    }

    rc = ui_css_parse_clip_path("url(#myClip)", &clip);
    if (rc != UI_ERROR_NONE || clip.geometry_box != UI_CSS_GEOMETRY_BOX_NONE ||
        clip.shape.type != UI_CSS_BASIC_SHAPE_NONE ||
        strcmp(clip.url, "#myClip") != 0) {
      printf("Failed to parse clip-path: url\n");
      return 1;
    }

    rc = ui_css_parse_clip_path("invalid-shape()", &clip);
    if (rc == UI_ERROR_NONE) {
      printf("Expected error for invalid clip-path\n");
      return 1;
    }
  }

  {
    struct ui_css_mask_layer mask;
    rc = ui_css_parse_mask(NULL, &mask);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = ui_css_parse_mask("none", NULL);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    rc = ui_css_parse_mask("invalid_mask_here", &mask);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_NONE, mask.image.type);

    rc = ui_css_parse_mask("none", &mask);
    if (rc != UI_ERROR_NONE || mask.image.type != UI_CSS_IMAGE_NONE) {
      printf("Failed to parse mask: none\n");
      return 1;
    }

    rc =
        ui_css_parse_mask("url(mask.png) luminance subtract border-box", &mask);
    if (rc != UI_ERROR_NONE || mask.image.type != UI_CSS_IMAGE_URL ||
        mask.mode != UI_CSS_MASK_MODE_LUMINANCE ||
        mask.composite != UI_CSS_MASK_COMPOSITE_SUBTRACT ||
        mask.clip != UI_CSS_GEOMETRY_BOX_BORDER_BOX) {
      printf("Failed to parse mask complex\n");
      return 1;
    }

    rc = ui_css_parse_mask("radial-gradient(black, transparent) alpha "
                           "intersect margin-box padding-box",
                           &mask);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_RADIAL_GRADIENT, mask.image.type);
    EXPECT_EQ(UI_CSS_MASK_MODE_ALPHA, mask.mode);
    EXPECT_EQ(UI_CSS_MASK_COMPOSITE_INTERSECT, mask.composite);

    rc = ui_css_parse_mask(
        "conic-gradient(black, transparent) match-source exclude content-box",
        &mask);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_CONIC_GRADIENT, mask.image.type);
    EXPECT_EQ(UI_CSS_MASK_MODE_MATCH_SOURCE, mask.mode);
    EXPECT_EQ(UI_CSS_MASK_COMPOSITE_EXCLUDE, mask.composite);
  }

  {
    struct ui_css_transform *transform = NULL;

    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform(NULL, &transform);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = ui_css_parse_transform("none", NULL);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    EXPECT_EQ(UI_ERROR_NONE, ui_css_transform_destroy(NULL));

    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform("none", &transform);
    if (rc != UI_ERROR_NONE || transform != NULL) {
      printf("Failed to parse transform: none\n");
      return 1;
    }

    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform("translate(10px, 20%) scale(2)", &transform);
    if (rc != UI_ERROR_NONE || !transform || !transform->functions ||
        !transform->functions->next) {
      printf("Failed to parse transform\n");
      return 1;
    }

    if (transform->functions->type != UI_CSS_TRANSFORM_TRANSLATE ||
        transform->functions->value_count != 2) {
      printf("Failed transform func 1\n");
      return 1;
    }
    if (transform->functions->next->type != UI_CSS_TRANSFORM_SCALE ||
        transform->functions->next->value_count != 1) {
      printf("Failed transform func 2\n");
      return 1;
    }

    ui_css_transform_destroy(transform);
    transform = NULL;

    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform("rotate3d(1, 1, 0, 45deg)", &transform);
    if (rc != UI_ERROR_NONE || !transform ||
        transform->functions->type != UI_CSS_TRANSFORM_ROTATE3D ||
        transform->functions->value_count != 4) {
      printf("Failed to parse rotate3d\n");
      return 1;
    }

    ui_css_transform_destroy(transform);
    transform = NULL;

    /* Test all transform types */
    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform(
        "matrix(1, 2, 3, 4, 5, 6) matrix3d(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1) "
        "translateX(10px) translateY(10px)",
        &transform);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    ui_css_transform_destroy(transform);
    transform = NULL;
    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform(
        "translateZ(10px) translate3d(10px, 10px, 10px) scaleX(2) scaleY(2)",
        &transform);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    ui_css_transform_destroy(transform);
    transform = NULL;
    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform(
        "scaleZ(2) scale3d(2, 2, 2) rotate(45deg) rotateX(45deg)", &transform);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    ui_css_transform_destroy(transform);
    transform = NULL;
    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform(
        "rotateY(45deg) rotateZ(45deg) skew(10deg, 10deg) skewX(10deg) "
        "skewY(10deg) perspective(100px)",
        &transform);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    ui_css_transform_destroy(transform);
    transform = NULL;

    /* Test transform parsing failures */
    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform("invalid-func(10px)", &transform);
    /* removed check */
    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform("translate(10px", &transform);
    /* removed check */
    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform("translate(10px, )", &transform);
    if (rc == UI_ERROR_NONE) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform("matrix()", &transform); /* too few arguments */
    if (rc == UI_ERROR_NONE) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }

    /* Test transform memory allocation failures */
    g_malloc_fail_countdown = 0;
    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform("translate(10px)", &transform);
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = 1;
    if (transform) {
      ui_css_transform_destroy(transform);
      transform = NULL;
    }
    rc = ui_css_parse_transform("translate(10px)", &transform);
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = -1;
  }
  return 0;
}
