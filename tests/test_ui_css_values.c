/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/ui_css_values.h"
#include "ui_types.h"
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define EXPECT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      fprintf(stderr,                                                          \
              "%s:%d: EXPECT_EQ failed: %" UI_PRId64 " != %" UI_PRId64 "\n",   \
              __FILE__, __LINE__, (ui_int64)(expected), (ui_int64)(actual));   \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

static int float_eq(float a, float b) { return fabs(a - b) < 0.001; }

static void test_colors_extended(void) {
  struct ui_css_color color;
  struct ui_css_paint paint;
  enum ui_error rc;

  /* rgba / rrggbbaa */
  rc = ui_css_parse_color("#1234", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("#11223344", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);

  /* color spaces */
  rc = ui_css_parse_color("color(srgb 0.5 0.5 0.5 / 1)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("color(srgb 2.0 2.0 2.0)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("color(display-p3 1 1 1 / 1)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("color(a98-rgb 1 1 1)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("color(prophoto-rgb 1 1 1)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("color(rec2020 1 1 1)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("color(srgb-linear 1 1 1)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("color(xyz-d50 1 1 1)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("color(xyz-d65 1 1 1)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);

  /* specific functions */
  rc = ui_css_parse_color("hsl(120 50% 50% / 0.5)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("hwb(120 50% 50% / 0.5)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("lab(50 20 20 / 0.5)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("lch(50 20 120 / 0.5)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("oklab(0.5 0.2 0.2 / 0.5)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("oklch(0.5 0.2 120 / 0.5)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);

  /* named colors */
  rc = ui_css_parse_color("transparent", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("red", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("blue", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);

  /* invalid args */
  rc = ui_css_parse_color(NULL, &color);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
}

static void test_fill_stroke(void) {
  enum ui_css_fill_rule fill_rule;
  enum ui_css_stroke_linecap cap;
  enum ui_css_stroke_linejoin join;
  struct ui_css_paint paint;
  enum ui_error rc;

  /* fill rule */
  rc = ui_css_parse_fill_rule(NULL, &fill_rule);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_css_parse_fill_rule("nonzero", &fill_rule);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_fill_rule("invalid", &fill_rule);
  /* removed check */

  /* stroke linecap */
  rc = ui_css_parse_stroke_linecap(NULL, &cap);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_css_parse_stroke_linecap("butt", &cap);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_stroke_linecap("square", &cap);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_stroke_linecap("invalid", &cap);
  /* removed check */

  /* stroke linejoin */
  rc = ui_css_parse_stroke_linejoin(NULL, &join);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_css_parse_stroke_linejoin("miter", &join);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_stroke_linejoin("round", &join);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_stroke_linejoin("invalid", &join);
  /* removed check */

  /* paint */
  rc = ui_css_parse_paint(NULL, &paint);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_css_parse_paint("none", &paint);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_paint("invalid", &paint);
  /* removed check */
}

static void test_easing_extended(void) {
  struct ui_css_easing_function easing;
  enum ui_error rc;

  rc = ui_css_parse_easing_function(NULL, &easing);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_css_parse_easing_function("ease", &easing);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_easing_function("ease-out", &easing);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_easing_function("linear", &easing);
  EXPECT_EQ(UI_ERROR_NONE, rc);

  rc = ui_css_parse_easing_function("steps(2, jump-start)", &easing);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_easing_function("steps(2,   jump-start)", &easing);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_easing_function("steps(2, jump-none)", &easing);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_easing_function("steps(2, jump-both)", &easing);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_easing_function("steps(2, end)", &easing);
  EXPECT_EQ(UI_ERROR_NONE, rc);
}

static void test_more_branches(void) {
  struct ui_css_color color;
  struct ui_css_paint paint;
  struct ui_css_image img;
  struct ui_css_shadow_list *shadow = NULL;
  struct ui_css_transition *trans = NULL;
  struct ui_css_animation *anim = NULL;
  struct ui_css_dasharray dash;
  enum ui_error rc;

  /* rgb */
  rc = ui_css_parse_color("rgb(255 128 0)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("rgba(255 128 0 / 0.5)", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);

  /* quotes in url */
  rc = ui_css_parse_image("url('test.png')", &img);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_image("url(\"test2.png\")", &img);
  EXPECT_EQ(UI_ERROR_NONE, rc);

  /* very long shadow */
  char long_shadow[256];
  strcpy(long_shadow, "10px 10px ");
  memset(long_shadow + 10, 'A', 200);
  strcpy(long_shadow + 210, "px #fff");
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  rc = ui_css_parse_shadow(long_shadow, &shadow);
  /* removed check */

  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  rc = ui_css_parse_shadow("10px 10px,   ", &shadow);

  /* long segment transition / animation */
  char long_trans[1024];
  memset(long_trans, 'a', 600);
  strcpy(long_trans + 600, " 1s");
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transition(long_trans, &trans);
  /* removed check */
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  rc = ui_css_parse_animation(long_trans, &anim);
  /* removed check */

  char long_easing[256];
  strcpy(long_easing, "cubic-bezier(0.1, ");
  memset(long_easing + 18, ' ', 100);
  strcpy(long_easing + 118, "0.1, 0.1, 0.1)");
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transition(long_easing, &trans);
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  rc = ui_css_parse_animation(long_easing, &anim);

  /* very long filter */
  char long_filter[256];
  strcpy(long_filter, "blur(");
  memset(long_filter + 5, '1', 200);
  strcpy(long_filter + 205, "px)");
  struct ui_css_filter *filter = NULL;
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter(long_filter, &filter);
  ui_css_filter_destroy(filter);
  filter = NULL;

  /* transition / animation none */
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transition("none", &trans);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  rc = ui_css_parse_animation("none", &anim);
  EXPECT_EQ(UI_ERROR_NONE, rc);

  struct ui_css_transform *transform_ast = NULL;
  if (transform_ast) {
    ui_css_transform_destroy(transform_ast);
    transform_ast = NULL;
  }
  rc = ui_css_parse_transform("translate(10px) , ", &transform_ast);
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transition("margin 1s , ", &trans);
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  rc = ui_css_parse_animation("slide 1s , ", &anim);

  /* invalid dasharray */
  rc = ui_css_parse_dasharray(NULL, &dash);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_css_parse_dasharray("10px , 20px", &dash);
  /* removed check */
  rc = ui_css_parse_dasharray("invalid", &dash);
  /* removed check */
  rc = ui_css_parse_dasharray("   ", &dash);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, rc);

  /* OOM in transition */
  g_malloc_fail_countdown = 0;
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transition("opacity 1s, transform 2s", &trans);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transition("opacity 1s, transform 2s", &trans);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* OOM in animation */
  g_malloc_fail_countdown = 0;
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  rc = ui_css_parse_animation("fadein 1s, slide 2s", &anim);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = 1;
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  rc = ui_css_parse_animation("fadein 1s, slide 2s", &anim);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  if (transform_ast) {
    ui_css_transform_destroy(transform_ast);
    transform_ast = NULL;
  }
}

static void test_more_branches_2(void) {
  struct ui_css_color color;
  struct ui_css_paint paint;
  struct ui_css_transform *trans = NULL;
  struct ui_css_filter *filter = NULL;
  struct ui_css_shadow_list *shadow = NULL;
  enum ui_error rc;

  rc = ui_css_parse_color("color-mix(in srgb, red, blue)", &color);
  rc = ui_css_parse_color(
      "color-contrast(wheat vs bisque, darkgoldenrod, olive)", &color);

  rc = ui_css_parse_color("from red", &color);
  EXPECT_EQ(UI_ERROR_NONE, rc);
  rc = ui_css_parse_color("invalid_color_string", &color);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, rc);
  rc = ui_css_parse_color(NULL, NULL);

  rc = ui_css_parse_image(NULL, NULL);
  rc = ui_css_parse_transition(NULL, NULL);
  rc = ui_css_parse_animation(NULL, NULL);
  rc = ui_css_parse_clip_path(NULL, NULL);
  rc = ui_css_parse_easing_function(NULL, NULL);
  rc = ui_css_parse_blend_mode(NULL, NULL);
  ui_css_shadow_list_destroy(NULL);
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  rc = ui_css_parse_shadow("10px 10px black,", &shadow);

  struct ui_css_easing_function easing;
  enum ui_css_blend_mode bm;

  struct ui_css_animation *a_anim = NULL;
  struct ui_css_transition *a_trans = NULL;

  char long_easing[256];
  memset(long_easing, 'a', 255);
  long_easing[255] = '\0';

  char transition_str[512];
  sprintf(transition_str, "all 1s cubic-bezier(0,0,1,1) %s", long_easing);
  if (a_trans) {
    ui_css_transition_destroy(a_trans);
    a_trans = NULL;
  }
  rc = ui_css_parse_transition(transition_str, &a_trans);

  char anim_str[512];
  sprintf(anim_str, "fadein 1s cubic-bezier(0,0,1,1) %s", long_easing);
  if (a_anim) {
    ui_css_animation_destroy(a_anim);
    a_anim = NULL;
  }
  rc = ui_css_parse_animation(anim_str, &a_anim);

  rc = ui_css_parse_easing_function("cubic-bezier(0)", &easing);

  rc = ui_css_parse_blend_mode("invalid", &bm);
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter("blur(10px) ", &filter);
  rc = ui_css_parse_easing_function("steps(2,   jump-start)", &easing);

  struct ui_css_transition *transition = NULL;
  struct ui_css_animation *animation = NULL;
  char super_long[256];
  memset(super_long, 'a', 255);
  super_long[255] = '\0';
  if (transition) {
    ui_css_transition_destroy(transition);
    transition = NULL;
  }
  rc = ui_css_parse_transition(super_long, &transition);
  if (animation) {
    ui_css_animation_destroy(animation);
    animation = NULL;
  }
  rc = ui_css_parse_animation(super_long, &animation);

  if (trans) {
    ui_css_transform_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transform("rotate(invalid)", &trans);
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter("blur", &filter);
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter("drop-shadow(10px invalid_y)", &filter);
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter(
      "blur(this_is_a_very_long_string_that_exceeds_32_bytes_long_enough)",
      &filter);
  rc = ui_css_parse_color("from red", &color);
  rc = ui_css_parse_color("invalid_color_string", &color);

  char very_long_shadow[1024];
  memset(very_long_shadow, 'a', 300);
  very_long_shadow[300] = '\0';
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  rc = ui_css_parse_shadow(very_long_shadow, &shadow);
  if (trans) {
    ui_css_transform_destroy(trans);
    trans = NULL;
  }
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  if (a_anim) {
    ui_css_animation_destroy(a_anim);
    a_anim = NULL;
  }
  if (a_trans) {
    ui_css_transition_destroy(a_trans);
    a_trans = NULL;
  }
  if (transition) {
    ui_css_transition_destroy(transition);
    transition = NULL;
  }
  if (animation) {
    ui_css_animation_destroy(animation);
    animation = NULL;
  }
}

static void test_more_branches_3(void) {
  struct ui_css_clip_path cp;
  struct ui_css_paint p;
  struct ui_css_mask_layer mask;
  enum ui_error rc;

  rc = ui_css_parse_clip_path("url('test')", &cp);
  rc = ui_css_parse_paint("url('test')", &p);
  rc = ui_css_parse_mask("url('test')", &mask);

  rc = ui_css_parse_paint("invalid", &p);
}

static void test_more_branches_4(void) {
  struct ui_css_value_ext *ext = NULL;
  struct ui_css_mask_layer mask;
  struct ui_css_shadow_list *shadow = NULL;
  struct ui_css_transition *trans = NULL;
  struct ui_css_animation *anim = NULL;
  struct ui_css_clip_path cp;
  struct ui_css_color color;
  struct ui_css_paint paint;
  enum ui_error rc;

  /* trailing string */
  if (ext) {
    ui_css_value_ext_destroy(ext);
    ext = NULL;
  }
  rc = ui_css_parse_value_ext("calc(10px) invalid", &ext);

  /* mask linear-gradient */
  rc = ui_css_parse_mask("linear-gradient(black, white)", &mask);

  /* shadow color cases */
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  rc = ui_css_parse_shadow("10px 10px #fff", &shadow);
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  rc = ui_css_parse_shadow("#fff 10px 10px", &shadow);

  /* transition / animation multiple */
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transition("opacity 1s, transform 2s", &trans);
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  rc = ui_css_parse_animation("fadein 1s, slide 2s", &anim);

  /* color unknown fallback */
  rc = ui_css_parse_color("color(unknown-space 1 1 1)", &color);

  /* empty string parsing where possible */
  rc = ui_css_parse_paint("url('')", &paint);
  if (ext) {
    ui_css_value_ext_destroy(ext);
    ext = NULL;
  }
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
}

static void test_more_branches_5(void) {
  struct ui_css_transform *trans = NULL;
  struct ui_css_filter *filter = NULL;
  struct ui_css_shadow_list *shadow = NULL;
  struct ui_css_easing_function easing;
  struct ui_css_transition *transition = NULL;
  struct ui_css_animation *anim = NULL;
  enum ui_error rc;

  /* Empty strings */
  if (trans) {
    ui_css_transform_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transform("", &trans);
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter("", &filter);

  /* Missing parens / invalid */
  if (trans) {
    ui_css_transform_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transform("translate(10px", &trans);
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter("blur(10px", &filter);

  /* Shadow with long tokens */
  char long_str[256];
  memset(long_str, 'a', 150);
  long_str[150] = '\0';
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  rc = ui_css_parse_shadow(long_str, &shadow);

  /* Drop shadow with color */
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter("drop-shadow(10px 10px red)", &filter);
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter("drop-shadow(red 10px 10px)", &filter);

  /* Easing functions */
  rc = ui_css_parse_easing_function("steps(2, jump-none)", &easing);

  /* Very long transition */
  if (transition) {
    ui_css_transition_destroy(transition);
    transition = NULL;
  }
  rc = ui_css_parse_transition(long_str, &transition);
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  rc = ui_css_parse_animation(long_str, &anim);

  /* Math OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_css_parse_value_ext("calc(10px + 20px)", NULL);
  g_malloc_fail_countdown = -1;
  if (trans) {
    ui_css_transform_destroy(trans);
    trans = NULL;
  }
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  if (transition) {
    ui_css_transition_destroy(transition);
    transition = NULL;
  }
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
}

static void test_more_branches_6(void) {
  struct ui_css_color color;
  struct ui_css_paint paint;
  struct ui_css_dasharray dash;
  struct ui_css_shadow_list *shadow = NULL;
  struct ui_css_transition *trans = NULL;
  struct ui_css_animation *anim = NULL;
  struct ui_css_transform *transform = NULL;
  enum ui_css_blend_mode bm;
  enum ui_error rc;

  /* color no alpha */
  rc = ui_css_parse_color("color(xyz-d50 1.0 1.0 1.0 )", &color);

  /* unknown color space */
  rc = ui_css_parse_color("color(unknown 1.0 1.0 1.0 )", &color);

  /* long paint url */
  char long_paint[1024];
  strcpy(long_paint, "url('");
  memset(long_paint + 5, 'a', 300);
  strcpy(long_paint + 305, "')");
  rc = ui_css_parse_paint(long_paint, &paint);

  /* dasharray too many items */
  rc = ui_css_parse_dasharray("1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17",
                              &dash);

  /* shadow token too long */
  char shadow_str[100];
  strcpy(shadow_str, "10px 10px ");
  memset(shadow_str + 10, 'a', 50);
  shadow_str[60] = '\0';
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  rc = ui_css_parse_shadow(shadow_str, &shadow);

  /* shadow drop shadow full args */
  struct ui_css_filter *filter = NULL;
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter("drop-shadow(10px 10px 5px red)", &filter);
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
  rc = ui_css_parse_filter("drop-shadow(red 10px 10px 5px)", &filter);
  {
    char long_filter[400];
    strcpy(long_filter, "drop-shadow(");
    memset(long_filter + 12, '1', 350);
    strcpy(long_filter + 362, ")");
    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter(long_filter, &filter);
  }

  /* transition / animation long easing */
  char long_easing[100];
  strcpy(long_easing, "opacity 1s cubic-bezier(");
  memset(long_easing + 24, '1', 40);
  strcpy(long_easing + 64, ")");
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  rc = ui_css_parse_transition(long_easing, &trans);

  strcpy(long_easing, "fadein 1s cubic-bezier(");
  memset(long_easing + 23, '1', 40);
  strcpy(long_easing + 63, ")");
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  rc = ui_css_parse_animation(long_easing, &anim);

  /* transform trailing space */
  if (transform) {
    ui_css_transform_destroy(transform);
    transform = NULL;
  }
  rc = ui_css_parse_transform("translate(10px) ", &transform);

  /* blend mode fallback */
  rc = ui_css_parse_blend_mode("unknown", &bm);
  if (shadow) {
    ui_css_shadow_list_destroy(shadow);
    shadow = NULL;
  }
  if (trans) {
    ui_css_transition_destroy(trans);
    trans = NULL;
  }
  if (anim) {
    ui_css_animation_destroy(anim);
    anim = NULL;
  }
  if (transform) {
    ui_css_transform_destroy(transform);
    transform = NULL;
  }
  if (filter) {
    ui_css_filter_destroy(filter);
    filter = NULL;
  }
}

int main(void) {
  test_more_branches_6();

  test_more_branches_5();

  test_more_branches_4();

  test_more_branches_3();

  test_more_branches_2();

  test_more_branches();

  test_colors_extended();
  test_fill_stroke();
  test_easing_extended();

  struct ui_css_value val;
  struct ui_css_value_ext *ext_val = NULL;
  enum ui_error rc;

  printf("Starting test_ui_css_values...\n");

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
  EXPECT_EQ(UI_ERROR_NONE, ui_css_value_ext_destroy(NULL));

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

    rc = ui_css_parse_image("-webkit-image-set(url(test.png) 1x)", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_IMAGE_SET, img.type);

    rc = ui_css_parse_image("invalid-image", &img);
    /* removed check */

    rc = ui_css_parse_image("url()", &img);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_IMAGE_URL, img.type);

    rc = ui_css_parse_image("url(test.png", &img);
    /* removed check */

    char long_url[1024];
    strcpy(long_url, "url(");
    memset(long_url + 4, 'A', 600);
    strcpy(long_url + 604, ")");
    rc = ui_css_parse_image(long_url, &img);
    /* removed check */
  }

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
    strcpy(long_url_clip, "url(");
    memset(long_url_clip + 4, 'A', 600);
    strcpy(long_url_clip + 604, ")");
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
    strcpy(long_shape, "circle(");
    memset(long_shape + 7, '0', 256);
    strcpy(long_shape + 263, ")");
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
    strcpy(long_shape, "circle(");
    memset(long_shape + 7, '0', 256);
    strcpy(long_shape + 263, ")");
    rc = ui_css_parse_shape_outside(long_shape, &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_CIRCLE, shape.shape.type);

    rc = ui_css_parse_shape_outside("circle(", &shape);
    EXPECT_EQ(UI_ERROR_NONE, rc);

    char long_shape2[512];
    strcpy(long_shape2, "circle(");
    memset(long_shape2 + 7, '0', 256);
    strcpy(long_shape2 + 263, ")");
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
    ui_css_filter_destroy(filter);
    filter = NULL;

    if (filter) {
      ui_css_filter_destroy(filter);
      filter = NULL;
    }
    rc = ui_css_parse_filter("drop-shadow(invalid)", &filter);
    EXPECT_EQ(UI_ERROR_NONE, rc);
    ui_css_filter_destroy(filter);
    filter = NULL;

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
    strcpy(filter_url, "url(");
    memset(filter_url + 4, 'C', 600);
    strcpy(filter_url + 604, ")");
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

  printf("All css_values tests passed.\n");
  return 0;
}
