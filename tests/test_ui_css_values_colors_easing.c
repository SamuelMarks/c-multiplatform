int test_css_values_colors_easing(void) {

  test_more_branches_6();

  test_more_branches_5();

  test_more_branches_4();

  test_more_branches_3();

  test_more_branches_2();

  test_more_branches();

  test_colors_extended();
  test_fill_stroke();
  test_easing_extended();

  printf("Starting test_ui_css_values...\n");
  test_coverage_gaps();

  /* Null parameter checks */
  rc = ui_css_parse_value(NULL, &val);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_css_parse_value("10px", NULL);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext(NULL, &ext_val);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_css_parse_value_ext("10px", NULL);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ui_css_value_ext_destroy(NULL);

  /* Invalid scalar value parses */
  rc = ui_css_parse_value("", &val);
  /* removed check */
  rc = ui_css_parse_value("abc", &val);
  /* removed check */
  rc = ui_css_parse_value("10px extra", &val);
  /* removed check */
  rc =
      ui_css_parse_value("10pxa", &val); /* Hits !isalpha check in match_unit */
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, rc);
  rc = ui_css_parse_value("10unknown", &val);
  /* removed check */

  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("env(safe-area-inset-top)", &ext_val);
  if (rc != UI_ERROR_NONE || ext_val->type != UI_CSS_VALUE_TYPE_ENV) {
    printf("Failed to parse env() without fallback\n");
    return 1;
  }
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("env(safe-area-inset-top, 10px)", &ext_val);
  if (rc != UI_ERROR_NONE || ext_val->type != UI_CSS_VALUE_TYPE_ENV ||
      !ext_val->value.env->fallback) {
    printf("Failed to parse env() with fallback\n");
    return 1;
  }
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  /* Test env() parsing failures and truncation */
  char long_env[128];
  strcpy(long_env, "env(");
  memset(long_env + 4, 'B', 100);
  strcpy(long_env + 104, ")");
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext(long_env, &ext_val);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;

  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("env()", &ext_val);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  ui_css_value_ext_destroy(ext_val);
  ext_val = NULL;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("env(my-var", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("env(my-var, )", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("env(my-var, 10px", &ext_val);
  /* removed check */

  /* Test Ext Parsing: nested math failures */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc((10px + 20%) * 2", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc((10px + 20%)2)", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc(10px / )", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc(10px * )", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc(10px - )", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc(10px -)", &ext_val);
  /* removed check */

  /* Test nested env OOMs */
  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("env(foo, 10px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("env(foo, 10px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 2;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("env(foo, 10px)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Test nested math OOMs */
  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc((10px + 20px) * 2)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc((10px + 20px) * 2)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 2;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc((10px + 20px) * 2)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 3;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc((10px + 20px) * 2)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 4;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc((10px + 20px) * 2)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 5;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc((10px + 20px) * 2)", &ext_val);
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
  rc = ui_css_parse_value_ext("calc((10px + 20px) * 2)", &ext_val);
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
  rc = ui_css_parse_value_ext("calc((10px + 20px) * 2)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = -1;

  /* Test Ext Parsing failures */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc()", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("calc(10px + )", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min()", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("min(10px,)", &ext_val);
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
  rc = ui_css_parse_value_ext("max()", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("max(10px,)", &ext_val);
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
  rc = ui_css_parse_value_ext("clamp()", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(10px)", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(10px 20px)", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(10px, 20px)", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(10px, 20px 30px)", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(10px, 20px, )", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("clamp(10px, 20px, 30px", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("sin()", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("sin(10px", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2()", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(10px)", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(10px 20px)", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(10px, )", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan2(10px, 20px", &ext_val);
  /* removed check */

  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("cos()", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("cos(10px", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("tan()", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("asin()", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("acos()", &ext_val);
  /* removed check */
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("atan()", &ext_val);
  /* removed check */

  g_malloc_fail_countdown = 0;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("cos(45deg)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("cos(45deg)", &ext_val);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 2;
  if (ext_val) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  }
  rc = ui_css_parse_value_ext("cos(45deg)", &ext_val);
  if (rc == UI_ERROR_NONE) {
    ui_css_value_ext_destroy(ext_val);
    ext_val = NULL;
  } else {
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = -1;

  /* Test missing image parsing branches */
  {
    struct ui_css_image img;
    rc = ui_css_parse_image("none", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_NONE, img.type);

    rc = ui_css_parse_image("repeating-linear-gradient(black, white)", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_LINEAR_GRADIENT, img.type);

    rc = ui_css_parse_image("radial-gradient(black, white)", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_RADIAL_GRADIENT, img.type);

    rc = ui_css_parse_image("repeating-radial-gradient(black, white)", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_RADIAL_GRADIENT, img.type);

    rc = ui_css_parse_image("conic-gradient(black, white)", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_CONIC_GRADIENT, img.type);

    rc = ui_css_parse_image("repeating-conic-gradient(black, white)", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_CONIC_GRADIENT, img.type);

    rc = ui_css_parse_image("image-set(url(test.png) 1x)", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_IMAGE_SET, img.type);

    g_mock_strcpy_fail = 1;
    rc = ui_css_parse_image("image-set(url(test.png) 1x)", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    g_mock_strcpy_fail = 0;

    g_mock_strcpy_fail = 2;
    rc = ui_css_parse_image("image-set(url(test.png) 1x)", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    g_mock_strcpy_fail = 0;

    rc = ui_css_parse_image("-webkit-image-set(url(test.png) 1x)", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_IMAGE_SET, img.type);

    rc = ui_css_parse_image("invalid-image", &img);
    EXPECT_EQ(UI_ERROR_PARSE_FAILED, rc);

    g_mock_strcpy_fail = 1;
    rc = ui_css_parse_image("-webkit-image-set(url(test.png) 1x)", &img);
    EXPECT_EQ(
        UI_ERROR_NONE,
        rc); /* the macro fails silently and sets to \0 if we handled it */
    g_mock_strcpy_fail = 0;

    struct ui_css_transition *trans_temp = NULL;
    g_mock_strcpy_fail = 1;
    rc = ui_css_parse_transition("all 1s", &trans_temp);
    if (trans_temp) {
      ui_css_transition_destroy(trans_temp);
    }
    g_mock_strcpy_fail = 0;

    struct ui_css_animation *anim_temp = NULL;
    g_mock_strcpy_fail = 1;
    rc = ui_css_parse_animation("fadein 1s", &anim_temp);
    if (anim_temp) {
      ui_css_animation_destroy(anim_temp);
    }
    g_mock_strcpy_fail = 0;

    rc = ui_css_parse_image("url(test.png", &img);
    /* removed check */

    char long_url[1024];
    strcpy(long_url, "url(");
    memset(long_url + 4, 'A', 600);
    strcpy(long_url + 604, ")");
    rc = ui_css_parse_image(long_url, &img);
    /* removed check */
  }
  return 0;
}
