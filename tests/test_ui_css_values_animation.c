int test_css_values_animation(void) {

  /* Animation Tests */
  {
    struct ui_css_animation *anim = NULL;

    if (anim) {
      ui_css_animation_destroy(anim);
      anim = NULL;
    }
    rc = ui_css_parse_animation(NULL, &anim);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = ui_css_parse_animation("slidein", NULL);
    EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    EXPECT_EQ(UI_ERROR_NONE, ui_css_animation_destroy(NULL));

    if (anim) {
      ui_css_animation_destroy(anim);
      anim = NULL;
    }
    rc = ui_css_parse_animation("none", &anim);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(NULL, anim);

    if (anim) {
      ui_css_animation_destroy(anim);
      anim = NULL;
    }
    rc = ui_css_parse_animation(
        "slidein 3s ease-in 1s infinite reverse both running", &anim);
    if (rc != UI_ERROR_NONE || !anim || strcmp(anim->name, "slidein") != 0 ||
        anim->duration.value != 3.0f ||
        anim->timing_function.type != UI_CSS_EASING_EASE_IN ||
        anim->delay.value != 1.0f || anim->iteration_count != -1.0f ||
        anim->direction != UI_CSS_ANIMATION_DIRECTION_REVERSE ||
        anim->fill_mode != UI_CSS_ANIMATION_FILL_MODE_BOTH ||
        anim->play_state != UI_CSS_ANIMATION_PLAY_STATE_RUNNING) {
      printf("Failed to parse complex animation\n");
      return 1;
    }
    ui_css_animation_destroy(anim);
    anim = NULL;

    if (anim) {
      ui_css_animation_destroy(anim);
      anim = NULL;
    }
    rc = ui_css_parse_animation("slidein 3s, fadeout 2s linear", &anim);
    if (rc != UI_ERROR_NONE || !anim || !anim->next ||
        strcmp(anim->name, "slidein") != 0 || anim->duration.value != 3.0f ||
        strcmp(anim->next->name, "fadeout") != 0 ||
        anim->next->duration.value != 2.0f ||
        anim->next->timing_function.type != UI_CSS_EASING_LINEAR) {
      printf("Failed to parse animation list\n");
      return 1;
    }
    ui_css_animation_destroy(anim);
    anim = NULL;

    if (anim) {
      ui_css_animation_destroy(anim);
      anim = NULL;
    }
    rc = ui_css_parse_animation(
        "name normal alternate alternate-reverse forwards backwards paused",
        &anim);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_ANIMATION_DIRECTION_ALTERNATE_REVERSE, anim->direction);
    EXPECT_EQ(UI_CSS_ANIMATION_FILL_MODE_BACKWARDS, anim->fill_mode);
    EXPECT_EQ(UI_CSS_ANIMATION_PLAY_STATE_PAUSED, anim->play_state);
    ui_css_animation_destroy(anim);
    anim = NULL;

    if (anim) {
      ui_css_animation_destroy(anim);
      anim = NULL;
    }
    rc = ui_css_parse_animation("cubic-bezier(0,0,1,1) steps(1, start) 1",
                                &anim);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_EASING_STEPS, anim->timing_function.type);
    if (!float_eq(1.0f, anim->iteration_count)) {
      printf("Failed to parse animation iteration_count\n");
      return 1;
    }
    ui_css_animation_destroy(anim);
    anim = NULL;

    if (anim) {
      ui_css_animation_destroy(anim);
      anim = NULL;
    }
    rc = ui_css_parse_animation("cubic-bezier(invalid)", &anim);
    EXPECT_EQ(UI_ERROR_NONE,
              rc); /* fails to parse easing but recovers to name */
    ui_css_animation_destroy(anim);
    anim = NULL;

    char long_name[256];
    memset(long_name, 'A', 200);
    long_name[200] = '\0';
    if (anim) {
      ui_css_animation_destroy(anim);
      anim = NULL;
    }
    rc = ui_css_parse_animation(long_name, &anim);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    ui_css_animation_destroy(anim);
    anim = NULL;

    if (anim) {
      ui_css_animation_destroy(anim);
      anim = NULL;
    }
    rc = ui_css_parse_animation("slidein 3s,", &anim);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    ui_css_animation_destroy(anim);
    anim = NULL;

    g_malloc_fail_countdown = 0;
    if (anim) {
      ui_css_animation_destroy(anim);
      anim = NULL;
    }
    rc = ui_css_parse_animation("slidein 3s", &anim);
    EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_malloc_fail_countdown = -1;
  }

  test_missing_branches();
  printf("All css_values tests passed.\n");
  return 0;

  return 0;
}
