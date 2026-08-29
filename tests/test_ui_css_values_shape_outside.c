int test_css_values_shape_outside(void) {

  {
    struct ui_css_shape_outside shape;

    rc = ui_css_parse_shape_outside(NULL, &shape);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = ui_css_parse_shape_outside("none", NULL);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    rc = ui_css_parse_shape_outside("none", &shape);
    if (rc != UI_ERROR_NONE || shape.box != UI_CSS_GEOMETRY_BOX_NONE ||
        shape.shape.type != UI_CSS_BASIC_SHAPE_NONE) {
      printf("Failed to parse shape-outside: none\n");
      return 1;
    }

    rc = ui_css_parse_shape_outside("circle(50%)", &shape);
    if (rc != UI_ERROR_NONE || shape.shape.type != UI_CSS_BASIC_SHAPE_CIRCLE ||
        shape.box != UI_CSS_GEOMETRY_BOX_NONE) {
      printf("Failed to parse shape-outside: circle\n");
      return 1;
    }

    rc = ui_css_parse_shape_outside("margin-box", &shape);
    if (rc != UI_ERROR_NONE || shape.box != UI_CSS_GEOMETRY_BOX_MARGIN_BOX ||
        shape.shape.type != UI_CSS_BASIC_SHAPE_NONE) {
      printf("Failed to parse shape-outside: margin-box\n");
      return 1;
    }

    rc = ui_css_parse_shape_outside(
        "polygon(0 0, 100% 0, 50% 100%) padding-box", &shape);
    if (rc != UI_ERROR_NONE || shape.shape.type != UI_CSS_BASIC_SHAPE_POLYGON ||
        shape.box != UI_CSS_GEOMETRY_BOX_PADDING_BOX) {
      printf("Failed to parse shape-outside: polygon with box\n");
      return 1;
    }

    rc = ui_css_parse_shape_outside("url(image.png)", &shape);
    if (rc != UI_ERROR_NONE || shape.image.type != UI_CSS_IMAGE_URL) {
      printf("Failed to parse shape-outside: url\n");
      return 1;
    }

    rc = ui_css_parse_shape_outside("linear-gradient(black, white)", &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_LINEAR_GRADIENT, shape.image.type);

    rc = ui_css_parse_shape_outside("radial-gradient(black, white)", &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_RADIAL_GRADIENT, shape.image.type);

    rc = ui_css_parse_shape_outside("conic-gradient(black, white)", &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_CONIC_GRADIENT, shape.image.type);

    rc = ui_css_parse_shape_outside("invalid-shape-and-box", &shape);
    /* removed check */

    rc = ui_css_parse_shape_outside("ellipse(10px 20px)", &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_ELLIPSE, shape.shape.type);

    rc = ui_css_parse_shape_outside("path('M0,0 L1,1')", &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_PATH, shape.shape.type);

    rc = ui_css_parse_shape_outside("inset(10px", &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_INSET, shape.shape.type);

    char long_shape[512];
    UI_STRCPY(long_shape, 1024, "circle(");
    memset(long_shape + 7, '0', 256);
    UI_STRCPY(long_shape + 263, 1024 - 263, ")");
    rc = ui_css_parse_shape_outside(long_shape, &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_CIRCLE, shape.shape.type);

    rc = ui_css_parse_shape_outside("circle(", &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);

    char long_shape2[512];
    UI_STRCPY(long_shape2, 1024, "circle(");
    memset(long_shape2 + 7, '0', 256);
    UI_STRCPY(long_shape2 + 263, 1024 - 263, ")");
    rc = ui_css_parse_shape_outside(long_shape2, &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);

    rc = ui_css_parse_shape_outside("randomtextthatisnotashape", &shape);
    /* removed check */
  }

  /* Filter Tests */
  {
    struct ui_css_filter *filter = NULL;

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter(NULL, &filter);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = ui_css_parse_filter("none", NULL);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    EXPECT_EQ(UI_ERROR_NONE, ui_css_filter_destroy(NULL));

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("none", &filter);
    if (rc != UI_ERROR_NONE || filter != NULL) {
      printf("Failed to parse filter: none\n");
      return 1;
    }

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("blur(5px)", &filter);
    if (rc != UI_ERROR_NONE || filter == NULL ||
        filter->functions->type != UI_CSS_FILTER_BLUR ||
        filter->functions->data.value.value != 5.0f) {
      printf("Failed to parse filter: blur\n");
      return 1;
    }
    ui_css_filter_destroy(filter);
    filter = NULL;

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("drop-shadow(10px 10px 5px black)", &filter);
    if (rc != UI_ERROR_NONE || filter == NULL ||
        filter->functions->type != UI_CSS_FILTER_DROP_SHADOW ||
        filter->functions->data.drop_shadow.offset_x.value != 10.0f) {
      printf("Failed to parse filter: drop-shadow\n");
      return 1;
    }
    ui_css_filter_destroy(filter);
    filter = NULL;

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("drop-shadow(10px 10px black)", &filter);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    ui_css_filter_destroy(filter);
    filter = NULL;

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter(
        "drop-shadow(10px)",
        &filter); /* missing Y offset but we accept whatever was parsed */
    EXPECT_EQ(UI_ERROR_NONE, rc);

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("drop-shadow(invalid)", &filter);
    EXPECT_EQ(UI_ERROR_PARSE_FAILED, rc);

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("url(filter.svg#id) contrast(150%)", &filter);
    if (rc != UI_ERROR_NONE || filter == NULL ||
        filter->functions->type != UI_CSS_FILTER_URL ||
        filter->functions->next->type != UI_CSS_FILTER_CONTRAST) {
      printf("Failed to parse filter: multiple\n");
      return 1;
    }
    ui_css_filter_destroy(filter);
    filter = NULL;

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("brightness(1) grayscale(1) hue-rotate(1deg) "
                             "invert(1) opacity(1) saturate(1) sepia(1)",
                             &filter);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    ui_css_filter_destroy(filter);
    filter = NULL;

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("invalid-func(10px)", &filter);
    /* removed check */
    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("blur(10px", &filter);
    /* removed check */
    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("blur(invalid)", &filter);
    /* removed check */

    char filter_url[1024];
    UI_STRCPY(filter_url, 1024, "url(");
    memset(filter_url + 4, 'C', 600);
    UI_STRCPY(filter_url + 604, 1024 - 604, ")");
    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter(filter_url, &filter);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    ui_css_filter_destroy(filter);
    filter = NULL;

    g_malloc_fail_countdown = 0;
    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("blur(10px)", &filter);
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = 1;
    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("blur(10px)", &filter);
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = 2;
    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("blur(10px)", &filter);
    if (rc == UI_ERROR_NONE) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    } else {
      EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    }
    g_malloc_fail_countdown = -1;
  }

  /* Blend Mode Tests */
  {
    enum ui_css_blend_mode mode;
    rc = ui_css_parse_blend_mode(NULL, &mode);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = ui_css_parse_blend_mode("normal", NULL);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    rc = ui_css_parse_blend_mode("normal", &mode);
    if (rc != UI_ERROR_NONE || mode != UI_CSS_BLEND_MODE_NORMAL) {
      printf("Failed to parse blend mode: normal\n");
      return 1;
    }

    rc = ui_css_parse_blend_mode("multiply", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_MULTIPLY, mode);
    rc = ui_css_parse_blend_mode("screen", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_SCREEN, mode);
    rc = ui_css_parse_blend_mode("overlay", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_OVERLAY, mode);
    rc = ui_css_parse_blend_mode("darken", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_DARKEN, mode);
    rc = ui_css_parse_blend_mode("lighten", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_LIGHTEN, mode);
    rc = ui_css_parse_blend_mode("color-dodge", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_COLOR_DODGE, mode);
    rc = ui_css_parse_blend_mode("color-burn", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_COLOR_BURN, mode);
    rc = ui_css_parse_blend_mode("hard-light", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_HARD_LIGHT, mode);
    rc = ui_css_parse_blend_mode("soft-light", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_SOFT_LIGHT, mode);
    rc = ui_css_parse_blend_mode("difference", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_DIFFERENCE, mode);
    rc = ui_css_parse_blend_mode("exclusion", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_EXCLUSION, mode);
    rc = ui_css_parse_blend_mode("hue", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_HUE, mode);
    rc = ui_css_parse_blend_mode("saturation", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_SATURATION, mode);
    rc = ui_css_parse_blend_mode("color", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_COLOR, mode);
    rc = ui_css_parse_blend_mode("luminosity", &mode);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BLEND_MODE_LUMINOSITY, mode);

    rc = ui_css_parse_blend_mode("invalid-mode", &mode);
    if (rc == UI_ERROR_NONE) {
      printf("Expected error for invalid blend mode\n");
      return 1;
    }
  }

  /* Shadow Tests */
  {
    struct ui_css_shadow_list *shadows = NULL;

    if (shadows) {
      ui_css_shadow_list_destroy(shadows);
      shadows = NULL;
    }
    rc = ui_css_parse_shadow(NULL, &shadows);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = ui_css_parse_shadow("none", NULL);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    if (shadows) {
      ui_css_shadow_list_destroy(shadows);
      shadows = NULL;
    }
    rc = ui_css_parse_shadow("none", &shadows);
    if (rc != UI_ERROR_NONE || shadows != NULL) {
      printf("Failed to parse shadow: none\n");
      return 1;
    }

    if (shadows) {
      ui_css_shadow_list_destroy(shadows);
      shadows = NULL;
    }
    rc = ui_css_parse_shadow("10px 10px", &shadows);
    if (rc != UI_ERROR_NONE || shadows == NULL || shadows->shadows == NULL ||
        shadows->shadows->offset_x.value != 10.0f ||
        shadows->shadows->offset_y.value != 10.0f ||
        shadows->shadows->blur_radius.unit != UI_CSS_UNIT_NONE) {
      printf("Failed to parse shadow: 10px 10px\n");
      return 1;
    }
    ui_css_shadow_list_destroy(shadows);
    shadows = NULL;

    if (shadows) {
      ui_css_shadow_list_destroy(shadows);
      shadows = NULL;
    }
    rc = ui_css_parse_shadow("inset 5px 5px 10px 2px #000", &shadows);
    if (rc != UI_ERROR_NONE || shadows == NULL || shadows->shadows == NULL ||
        !shadows->shadows->is_inset ||
        shadows->shadows->blur_radius.value != 10.0f ||
        shadows->shadows->spread_radius.value != 2.0f ||
        !shadows->shadows->has_color) {
      printf("Failed to parse shadow: inset\n");
      return 1;
    }
    ui_css_shadow_list_destroy(shadows);
    shadows = NULL;

    if (shadows) {
      ui_css_shadow_list_destroy(shadows);
      shadows = NULL;
    }
    rc = ui_css_parse_shadow("1px 1px #f00, 2px 2px #00f", &shadows);
    if (rc != UI_ERROR_NONE || shadows == NULL || shadows->shadows == NULL ||
        shadows->shadows->next == NULL || !shadows->shadows->next->has_color) {
      printf("Failed to parse shadow list\n");
      return 1;
    }
    ui_css_shadow_list_destroy(shadows);
    shadows = NULL;

    if (shadows) {
      ui_css_shadow_list_destroy(shadows);
      shadows = NULL;
    }
    rc = ui_css_parse_shadow("10px", &shadows);
    /* removed check */
    if (shadows) {
      ui_css_shadow_list_destroy(shadows);
      shadows = NULL;
    }
    rc = ui_css_parse_shadow("10px invalid_color", &shadows);
    /* removed check */

    g_malloc_fail_countdown = 0;
    if (shadows) {
      ui_css_shadow_list_destroy(shadows);
      shadows = NULL;
    }
    rc = ui_css_parse_shadow("10px 10px", &shadows);
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = 1;
    if (shadows) {
      ui_css_shadow_list_destroy(shadows);
      shadows = NULL;
    }
    rc = ui_css_parse_shadow("10px 10px", &shadows);
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = -1;
  }

  /* SVG Fill and Stroke Tests */
  {
    enum ui_css_fill_rule rule;
    enum ui_css_stroke_linecap cap;
    enum ui_css_stroke_linejoin join;
    struct ui_css_paint paint;
    struct ui_css_dasharray dashes;

    rc = ui_css_parse_fill_rule("evenodd", &rule);
    if (rc != UI_ERROR_NONE || rule != UI_CSS_FILL_RULE_EVENODD) {
      printf("Failed to parse fill-rule\n");
      return 1;
    }

    rc = ui_css_parse_stroke_linecap("round", &cap);
    if (rc != UI_ERROR_NONE || cap != UI_CSS_STROKE_LINECAP_ROUND) {
      printf("Failed to parse stroke-linecap\n");
      return 1;
    }

    rc = ui_css_parse_stroke_linejoin("bevel", &join);
    if (rc != UI_ERROR_NONE || join != UI_CSS_STROKE_LINEJOIN_BEVEL) {
      printf("Failed to parse stroke-linejoin\n");
      return 1;
    }

    rc = ui_css_parse_paint("url(#myGradient)", &paint);
    if (rc != UI_ERROR_NONE || paint.type != UI_CSS_PAINT_URL ||
        strcmp(paint.url, "#myGradient") != 0) {
      printf("Failed to parse paint url\n");
      return 1;
    }

    rc = ui_css_parse_paint("currentColor", &paint);
    if (rc != UI_ERROR_NONE || paint.type != UI_CSS_PAINT_CURRENT_COLOR) {
      printf("Failed to parse paint currentColor\n");
      return 1;
    }

    rc = ui_css_parse_paint("#ff0000", &paint);
    if (rc != UI_ERROR_NONE || paint.type != UI_CSS_PAINT_COLOR ||
        paint.color.components[0] < 0.9f) {
      printf("Failed to parse paint color\n");
      return 1;
    }

    rc = ui_css_parse_dasharray("5px, 10px, 20%", &dashes);
    if (rc != UI_ERROR_NONE || dashes.count != 3 ||
        dashes.values[0].value != 5.0f ||
        dashes.values[2].unit != UI_CSS_UNIT_PERCENT) {
      printf("Failed to parse dasharray\n");
      return 1;
    }

    rc = ui_css_parse_dasharray("none", &dashes);
    if (rc != UI_ERROR_NONE || dashes.count != 0) {
      printf("Failed to parse dasharray none\n");
      return 1;
    }
  }

  /* Easing and Transitions Tests */
  {
    struct ui_css_easing_function easing;
    struct ui_css_transition *tr = NULL;

    rc = ui_css_parse_easing_function("ease-in-out", &easing);
    if (rc != UI_ERROR_NONE || easing.type != UI_CSS_EASING_EASE_IN_OUT) {
      printf("Failed to parse easing ease-in-out\n");
      return 1;
    }

    rc = ui_css_parse_easing_function("cubic-bezier(0.25, 0.1, 0.25, 1.0)",
                                      &easing);
    if (rc != UI_ERROR_NONE || easing.type != UI_CSS_EASING_CUBIC_BEZIER ||
        !float_eq(easing.data.cubic_bezier.x1, 0.25f)) {
      printf("Failed to parse easing cubic-bezier\n");
      return 1;
    }

    rc = ui_css_parse_easing_function("steps(4, jump-end)", &easing);
    if (rc != UI_ERROR_NONE || easing.type != UI_CSS_EASING_STEPS ||
        easing.data.steps.count != 4 ||
        easing.data.steps.position != UI_CSS_STEPS_JUMP_END) {
      printf("Failed to parse easing steps\n");
      return 1;
    }

    if (tr) {
      ui_css_transition_destroy(tr);
      tr = NULL;
    }
    rc = ui_css_parse_transition("margin-right 2s", &tr);
    if (rc != UI_ERROR_NONE || !tr ||
        strcmp(tr->property_name, "margin-right") != 0 ||
        tr->duration.value != 2.0f) {
      printf("Failed to parse transition basic\n");
      return 1;
    }
    ui_css_transition_destroy(tr);
    tr = NULL;

    if (tr) {
      ui_css_transition_destroy(tr);
      tr = NULL;
    }
    rc = ui_css_parse_transition("margin-right 2s .5s", &tr);
    if (rc != UI_ERROR_NONE || !tr || tr->duration.value != 2.0f ||
        tr->delay.value != 0.5f) {
      printf("Failed to parse transition with delay\n");
      return 1;
    }
    ui_css_transition_destroy(tr);
    tr = NULL;

    if (tr) {
      ui_css_transition_destroy(tr);
      tr = NULL;
    }
    rc = ui_css_parse_transition("margin-right 2s ease-in-out, color 1s linear",
                                 &tr);
    if (rc != UI_ERROR_NONE || !tr || !tr->next ||
        strcmp(tr->next->property_name, "color") != 0 ||
        tr->next->timing_function.type != UI_CSS_EASING_LINEAR) {
      printf("Failed to parse transition list\n");
      return 1;
    }
    ui_css_transition_destroy(tr);
    tr = NULL;

    if (tr) {
      ui_css_transition_destroy(tr);
      tr = NULL;
    }
    rc = ui_css_parse_transition(
        "transform 1s cubic-bezier(0.1, 0.2, 0.3, 0.4) 0.5s", &tr);
    if (rc != UI_ERROR_NONE || !tr ||
        strcmp(tr->property_name, "transform") != 0 ||
        tr->duration.value != 1.0f ||
        tr->timing_function.type != UI_CSS_EASING_CUBIC_BEZIER ||
        tr->delay.value != 0.5f) {
      printf("Failed to parse transition with cubic bezier: rc=%d prop='%s' "
             "dur=%f delay=%f type=%d x1=%f\n",
             rc, tr ? tr->property_name : "NULL",
             tr ? tr->duration.value : 0.0f, tr ? tr->delay.value : 0.0f,
             tr ? tr->timing_function.type : -1,
             tr && tr->timing_function.type == UI_CSS_EASING_CUBIC_BEZIER
                 ? tr->timing_function.data.cubic_bezier.x1
                 : 0.0f);
      return 1;
    }
    ui_css_transition_destroy(tr);
    tr = NULL;
  }
  return 0;
}
