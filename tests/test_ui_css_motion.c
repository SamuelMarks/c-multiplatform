/* clang-format off */
#include "ui_css_motion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define EXPECT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      fprintf(stderr, "%s:%d: EXPECT_EQ failed: %d != %d\n", __FILE__,         \
              __LINE__, (int)(expected), (int)(actual));                       \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define EXPECT_FLOAT_EQ(expected, actual)                                      \
  do {                                                                         \
    float diff = (expected) - (actual);                                        \
    if (diff < -0.001f || diff > 0.001f) {                                     \
      fprintf(stderr, "%s:%d: EXPECT_FLOAT_EQ failed: %f != %f\n", __FILE__,   \
              __LINE__, (float)(expected), (float)(actual));                   \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define EXPECT_STR_EQ(expected, actual)                                        \
  do {                                                                         \
    if (strcmp((expected), (actual)) != 0) {                                   \
      fprintf(stderr, "%s:%d: EXPECT_STR_EQ failed: %s != %s\n", __FILE__,     \
              __LINE__, (expected), (actual));                                 \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

static ui_error_t test_parse_offset_path(void) {
  struct ui_css_offset_path path;
  ui_error_t err;

  /* Invalid arguments */
  struct ui_css_value tmp_val;
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_parse_offset_path(NULL, &path));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_parse_offset_path("none", NULL));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_offset_distance(NULL, &tmp_val));

  err = ui_css_parse_offset_path("none", &path);

  EXPECT_EQ(UI_CSS_OFFSET_PATH_NONE, path.type);

  err = ui_css_parse_offset_path("   none", &path);

  EXPECT_EQ(UI_CSS_OFFSET_PATH_NONE, path.type);

  err = ui_css_parse_offset_path("url(invalid", &path);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);
  err = ui_css_parse_offset_path("path(invalid", &path);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);
  err = ui_css_parse_offset_path("ray(invalid", &path);
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, err);

  err = ui_css_parse_offset_path("url(some/path.svg)", &path);

  EXPECT_EQ(UI_CSS_OFFSET_PATH_URL, path.type);
  EXPECT_STR_EQ("some/path.svg", path.url);

  /* long url */
  {
    char long_url[300] = "url(";
    memset(long_url + 4, 'a', 260);
    long_url[264] = ')';
    long_url[265] = '\0';
    err = ui_css_parse_offset_path(long_url, &path);

    EXPECT_EQ(UI_CSS_OFFSET_PATH_URL, path.type);
  }

  /* incomplete url */
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, ui_css_parse_offset_path("url(abc", &path));

  err = ui_css_parse_offset_path("path(M 0 0 L 100 100)", &path);

  EXPECT_EQ(UI_CSS_OFFSET_PATH_BASIC_SHAPE, path.type);
  EXPECT_EQ(UI_CSS_BASIC_SHAPE_PATH, path.shape.type);
  EXPECT_STR_EQ("M 0 0 L 100 100", path.shape.arguments);

  /* long path */
  {
    char long_path[300] = "path(";
    memset(long_path + 5, 'M', 260);
    long_path[265] = ')';
    long_path[266] = '\0';
    err = ui_css_parse_offset_path(long_path, &path);

    EXPECT_EQ(UI_CSS_OFFSET_PATH_BASIC_SHAPE, path.type);
    EXPECT_EQ(UI_CSS_BASIC_SHAPE_PATH, path.shape.type);
  }

  /* incomplete path */

  err = ui_css_parse_offset_path("ray(45deg contain)", &path);

  /* ray without contain */
  err = ui_css_parse_offset_path("ray(45deg)", &path);

  /* long ray */
  {
    char long_ray[300] = "ray(";
    memset(long_ray + 4, 'a', 260);
    long_ray[264] = ')';
    long_ray[265] = '\0';
    err = ui_css_parse_offset_path(long_ray, &path);
  }

  /* incomplete ray */
  EXPECT_EQ(UI_ERROR_PARSE_FAILED,
            ui_css_parse_offset_path("ray(45deg", &path));

  /* invalid */
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, ui_css_parse_offset_path("invalid", &path));

  return UI_ERROR_NONE;
}

static ui_error_t test_parse_offset_distance(void) {
  struct ui_css_value distance;
  ui_error_t err;

  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_offset_distance(NULL, &distance));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_offset_distance("50%", NULL));

  err = ui_css_parse_offset_distance("50%", &distance);

  EXPECT_EQ(UI_CSS_UNIT_PERCENT, distance.unit);
  EXPECT_FLOAT_EQ(50.0f, distance.value);

  err = ui_css_parse_offset_distance("100px", &distance);

  EXPECT_EQ(UI_CSS_UNIT_PX, distance.unit);
  EXPECT_FLOAT_EQ(100.0f, distance.value);
  return UI_ERROR_NONE;
}

static ui_error_t test_parse_offset_position(void) {
  struct ui_css_offset_position pos;
  ui_error_t err;

  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_offset_position(NULL, &pos));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_offset_position("auto", NULL));

  err = ui_css_parse_offset_position("auto", &pos);

  EXPECT_EQ(1, pos.is_auto);

  err = ui_css_parse_offset_position("20px", &pos);

  EXPECT_EQ(0, pos.is_auto);
  EXPECT_EQ(UI_CSS_UNIT_PX, pos.x.unit);
  EXPECT_FLOAT_EQ(20.0f, pos.x.value);

  /* invalid value */
  EXPECT_EQ(UI_ERROR_PARSE_FAILED,
            ui_css_parse_offset_position("invalid", &pos));

  return UI_ERROR_NONE;
}

static ui_error_t test_parse_offset_anchor(void) {
  struct ui_css_offset_anchor anchor;
  ui_error_t err;

  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_offset_anchor(NULL, &anchor));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_offset_anchor("auto", NULL));

  err = ui_css_parse_offset_anchor("auto", &anchor);

  EXPECT_EQ(1, anchor.is_auto);

  err = ui_css_parse_offset_anchor("50%", &anchor);

  EXPECT_EQ(0, anchor.is_auto);
  EXPECT_EQ(UI_CSS_UNIT_PERCENT, anchor.x.unit);
  EXPECT_FLOAT_EQ(50.0f, anchor.x.value);

  /* invalid value */
  EXPECT_EQ(UI_ERROR_PARSE_FAILED,
            ui_css_parse_offset_anchor("invalid", &anchor));

  return UI_ERROR_NONE;
}

static ui_error_t test_parse_offset_rotate(void) {
  struct ui_css_offset_rotate rot;
  ui_error_t err;

  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_css_parse_offset_rotate(NULL, &rot));
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_css_parse_offset_rotate("auto", NULL));

  err = ui_css_parse_offset_rotate("auto", &rot);

  EXPECT_EQ(1, rot.is_auto);
  EXPECT_EQ(0, rot.is_reverse);

  err = ui_css_parse_offset_rotate("reverse", &rot);

  EXPECT_EQ(1, rot.is_auto);
  EXPECT_EQ(1, rot.is_reverse);

  err = ui_css_parse_offset_rotate("auto 90deg", &rot);

  EXPECT_EQ(1, rot.is_auto);
  EXPECT_EQ(0, rot.is_reverse);
  EXPECT_EQ(UI_CSS_UNIT_DEG, rot.angle.unit);
  EXPECT_FLOAT_EQ(90.0f, rot.angle.value);

  /* parse fail */
  EXPECT_EQ(UI_ERROR_PARSE_FAILED, ui_css_parse_offset_rotate("invalid", &rot));

  return UI_ERROR_NONE;
}

int main(void) {
  test_parse_offset_path();
  test_parse_offset_distance();
  test_parse_offset_position();
  test_parse_offset_anchor();
  test_parse_offset_rotate();
  printf("All CSS Motion Path tests passed.\n");
  return 0;
}
