int test_css_values_clip_math(void) {

  /* Test basic clip-path syntax missing geometries */
  {
    struct ui_css_clip_path clip;
    rc = ui_css_parse_clip_path(NULL, &clip);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = ui_css_parse_clip_path("padding-box", NULL);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    rc = ui_css_parse_clip_path("url(unclosed", &clip);
    /* removed check */

    char long_url_clip[1024];
    UI_STRCPY(long_url_clip, 1024, "url(");
    memset(long_url_clip + 4, 'A', 600);
    UI_STRCPY(long_url_clip + 604, 1024 - 604, ")");
    rc = ui_css_parse_clip_path(long_url_clip, &clip);
    /* removed check */

    rc = ui_css_parse_clip_path("padding-box", &clip);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_GEOMETRY_BOX_PADDING_BOX, clip.geometry_box);

    rc = ui_css_parse_clip_path("content-box", &clip);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_GEOMETRY_BOX_CONTENT_BOX, clip.geometry_box);

    rc = ui_css_parse_clip_path("fill-box", &clip);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_GEOMETRY_BOX_FILL_BOX, clip.geometry_box);

    rc = ui_css_parse_clip_path("stroke-box", &clip);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_GEOMETRY_BOX_STROKE_BOX, clip.geometry_box);

    rc = ui_css_parse_clip_path("view-box", &clip);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_GEOMETRY_BOX_VIEW_BOX, clip.geometry_box);

    rc = ui_css_parse_clip_path("inset(10px)", &clip);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_INSET, clip.shape.type);

    rc = ui_css_parse_clip_path("ellipse(10px 20px)", &clip);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_ELLIPSE, clip.shape.type);

    rc = ui_css_parse_clip_path("polygon(0 0, 1 1)", &clip);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_POLYGON, clip.shape.type);

    rc = ui_css_parse_clip_path("path('M0,0 L1,1')", &clip);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_PATH, clip.shape.type);

    rc = ui_css_parse_clip_path("inset(10px",
                                &clip); /* Missing close paren gracefully does
                                           nothing but parses shape */
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_INSET, clip.shape.type);

    char long_shape[512];
    UI_STRCPY(long_shape, 1024, "circle(");
    memset(long_shape + 7, '0', 256);
    UI_STRCPY(long_shape + 263, 1024 - 263, ")");
    rc = ui_css_parse_clip_path(long_shape, &clip);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_CIRCLE, clip.shape.type);

    rc = ui_css_parse_clip_path("randomtextthatisnotashape", &clip);
    /* removed check */
  }

  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("10px + 20px", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("10px + 20px", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 2;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("10px + 20px", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("10px * 20px", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("10px * 20px", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 2;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("10px * 20px", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 3;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("10px * 20px", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = -1;

  /* Test parsing parenthesis failures */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("(10px", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("((10px)", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("( )", &ext_val);
  /* removed check */

  /* Test parsing unclosed parenthesis for functions */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc(10px", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min(10px, 20px", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("sin(10px", &ext_val);
  /* removed check */

  /* Test Math creation OOM */
  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("10px", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc(10px + 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Min / Max / Clamp memory exhaustion and parse failure test coverage */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px, )", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px, )", &ext_val);
  /* removed check */
  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min(10px, 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min(10px, 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 2;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min(10px, 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 3;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min(10px, 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 4;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min(10px, 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 5;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min(10px, 20px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 6;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min(10px, 20px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 2;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 3;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 4;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 5;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 6;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 7;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px, 20px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 2;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 3;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 4;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 5;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 6;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 7;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 8;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 9;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 10;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(1px, 2px, 3px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(1px, 2px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(1px, 2px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 2;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(1px, 2px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 3;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(1px, 2px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 4;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(1px, 2px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = 5;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(1px, 2px)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  return 0;
}
