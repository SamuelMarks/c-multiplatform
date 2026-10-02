/**
 * @file test_cupertino_squircle.c
 * @brief Unit tests for Cupertino squircle geometry and continuous curvature.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_squircle.h"
#include <math.h>
/* clang-format on */

SUITE(cupertino_squircle_suite);

TEST test_squircle_point_at(void) {
  float x = 0.0f;
  float y = 0.0f;
  float a;
  float b;
  float n;
  float theta;
  float eq;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_squircle_point_at(0.0f, 10.0f, 4.7f, 0.0f, &x, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_squircle_point_at(10.0f, 0.0f, 4.7f, 0.0f, &x, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_squircle_point_at(10.0f, 10.0f, 0.0f, 0.0f, &x, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_squircle_point_at(10.0f, 10.0f, -1.0f, 0.0f, &x, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_squircle_point_at(10.0f, 10.0f, 4.7f, 0.0f, NULL, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_squircle_point_at(10.0f, 10.0f, 4.7f, 0.0f, &x, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid points at cardinal angles */
  a = 20.0f;
  b = 10.0f;
  n = 4.7f;

  /* theta = 0 -> (a, 0) */
  rc = cupertino_squircle_point_at(a, b, n, 0.0f, &x, &y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(x - a) < 1e-3f);
  ASSERT(fabs(y) < 1e-3f);

  /* theta = pi/2 -> (0, b) */
  rc = cupertino_squircle_point_at(a, b, n, 1.5707963f, &x, &y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(x) < 1e-3f);
  ASSERT(fabs(y - b) < 1e-3f);

  /* theta = pi -> (-a, 0) */
  rc = cupertino_squircle_point_at(a, b, n, 3.14159265f, &x, &y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(x - (-a)) < 1e-3f);
  ASSERT(fabs(y) < 1e-3f);

  /* theta = 3*pi/2 -> (0, -b) */
  rc = cupertino_squircle_point_at(a, b, n, 4.71238898f, &x, &y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(x) < 1e-3f);
  ASSERT(fabs(y - (-b)) < 1e-3f);

  /* Angle at pi / 4: verify point satisfies equation (|x/a|^n + |y/b|^n = 1) */
  theta = 0.78539816f;
  rc = cupertino_squircle_point_at(a, b, n, theta, &x, &y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  eq = (float)(pow(fabs(x / a), n) + pow(fabs(y / b), n));
  ASSERT(fabs(eq - 1.0f) < 1e-3f);

  /* Numerical underflow branch: denom_term <= 0.0f */
  rc = cupertino_squircle_point_at(0.5f, 0.5f, 500.0f, 0.78539816f, &x, &y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_cupertino_squircle_mock_denom_zero;
    g_cupertino_squircle_mock_denom_zero = 1;
    rc = cupertino_squircle_point_at(10.0f, 10.0f, 4.0f, 0.78539816f, &x, &y);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_squircle_mock_denom_zero = 0;
  }
#endif

  PASS();
}

TEST test_squircle_inset(void) {
  float inner = 0.0f;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_squircle_inset(-1.0f, 1.0f, &inner);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_squircle_inset(10.0f, -1.0f, &inner);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_squircle_inset(10.0f, 1.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Outer > stroke */
  rc = cupertino_squircle_inset(16.0f, 2.0f, &inner);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(inner - 14.0f) < 1e-5f);

  /* Outer <= stroke */
  rc = cupertino_squircle_inset(5.0f, 5.0f, &inner);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(inner - 0.0f) < 1e-5f);

  rc = cupertino_squircle_inset(3.0f, 10.0f, &inner);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(inner - 0.0f) < 1e-5f);

  PASS();
}

TEST test_hairline_and_pixel_snapping(void) {
  float width = 0.0f;
  float snapped = 0.0f;
  ui_error_t rc;

  /* Hairline width invalid args */
  rc = cupertino_hairline_width(0.0f, &width);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_hairline_width(1.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Hairline scales */
  rc = cupertino_hairline_width(1.0f, &width);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(width - 1.0f) < 1e-5f);

  rc = cupertino_hairline_width(2.0f, &width);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(width - 0.5f) < 1e-5f);

  rc = cupertino_hairline_width(3.0f, &width);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(width - (1.0f / 3.0f)) < 1e-5f);

  /* Pixel snapping invalid args */
  rc = cupertino_snap_to_device_pixel(10.2f, 0.0f, 1.0f, &snapped);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_snap_to_device_pixel(10.2f, 2.0f, -1.0f, &snapped);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_snap_to_device_pixel(10.2f, 2.0f, 1.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Even stroke pixels (e.g. stroke = 1.0pt on 2x -> 2 physical pixels) */
  rc = cupertino_snap_to_device_pixel(10.2f, 2.0f, 1.0f, &snapped);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* 10.2 * 2 = 20.4 -> floor(20.4 + 0.5) = 20 -> 20 / 2 = 10.0 */
  ASSERT(fabs(snapped - 10.0f) < 1e-4f);

  /* Odd stroke pixels (e.g. stroke = 0.5pt on 2x -> 1 physical pixel) */
  rc = cupertino_snap_to_device_pixel(10.2f, 2.0f, 0.5f, &snapped);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* 10.2 * 2 = 20.4 -> floor(20.4) + 0.5 = 20.5 -> 20.5 / 2 = 10.25 */
  ASSERT(fabs(snapped - 10.25f) < 1e-4f);

  PASS();
}

TEST test_path_operations_and_squircle_path(void) {
  struct ui_path path;
  ui_error_t rc;

  memset(&path, 0, sizeof(path));

  /* Path init invalid args */
  rc = cupertino_path_init(NULL, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_path_init(&path, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_path_init(&path, -5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid init */
  rc = cupertino_path_init(&path, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, path.cmd_count);
  ASSERT_EQ(2, path.cmd_capacity);

  /* Append commands and verify auto-expansion beyond initial capacity */
  rc = cupertino_path_move_to(NULL, 0.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_path_move_to(&path, 5.0f, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, path.cmd_count);
  ASSERT_EQ(UI_PATH_CMD_MOVE_TO, path.cmds[0].type);
  ASSERT(fabs(path.cmds[0].x1 - 5.0f) < 1e-5f);
  ASSERT(fabs(path.cmds[0].y1 - 10.0f) < 1e-5f);

  rc = cupertino_path_line_to(NULL, 10.0f, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_path_line_to(&path, 10.0f, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, path.cmd_count);

  /* Expansion happens on 3rd command */
  rc = cupertino_path_bezier_to(NULL, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_path_bezier_to(&path, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, path.cmd_count);
  ASSERT(path.cmd_capacity >= 3);

  rc = cupertino_path_close(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_path_close(&path);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, path.cmd_count);
  ASSERT_EQ(UI_PATH_CMD_CLOSE, path.cmds[3].type);

  /* Path destroy */
  rc = cupertino_path_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_path_destroy(&path);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(path.cmds == NULL);
  ASSERT_EQ(0, path.cmd_count);
  ASSERT_EQ(0, path.cmd_capacity);

  /* Squircle generate path invalid args */
  rc = cupertino_path_init(&path, 32);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 0.0f, 100.0f, 10.0f, 1.0f,
                                        &path);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 100.0f, -1.0f, 10.0f, 1.0f,
                                        &path);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 100.0f, 100.0f, -2.0f, 1.0f,
                                        &path);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 100.0f, 100.0f, 10.0f, 1.0f,
                                        NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Generate sharp rectangle (radius = 0) */
  path.cmd_count = 0;
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 100.0f, 50.0f, 0.0f, 0.0f,
                                        &path);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(path.cmd_count >= 5); /* MoveTo, 3 LineTo, Close */

  /* Generate circular rounded rectangle (smoothness = 0.0) */
  path.cmd_count = 0;
  rc = cupertino_squircle_generate_path(10.0f, 20.0f, 100.0f, 80.0f, 16.0f,
                                        0.0f, &path);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(path.cmd_count > 0);

  /* Generate continuous G2 squircle (smoothness = 1.0) */
  path.cmd_count = 0;
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 200.0f, 150.0f, 24.0f, 1.0f,
                                        &path);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(path.cmd_count > 0);

  /* Smoothness clamping checks: negative clamped to 0, >1 clamped to 1 */
  path.cmd_count = 0;
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 100.0f, 100.0f, 10.0f,
                                        -0.5f, &path);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  path.cmd_count = 0;
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 100.0f, 100.0f, 10.0f, 2.5f,
                                        &path);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Corner radius clamped to half-dimension (height < width and width < height)
   */
  path.cmd_count = 0;
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 40.0f, 30.0f, 50.0f, 1.0f,
                                        &path);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  path.cmd_count = 0;
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 30.0f, 40.0f, 50.0f, 1.0f,
                                        &path);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* L > max_r vs L <= max_r */
  path.cmd_count = 0;
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 100.0f, 100.0f, 45.0f, 1.0f,
                                        &path);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  path.cmd_count = 0;
  rc = cupertino_squircle_generate_path(0.0f, 0.0f, 100.0f, 100.0f, 10.0f, 1.0f,
                                        &path);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_path_destroy(&path);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Path destroy when cmds is NULL */
  memset(&path, 0, sizeof(path));
  rc = cupertino_path_destroy(&path);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
extern int g_cupertino_squircle_mock_cmd_fail_countdown;
ui_error_t test_hook_ensure_path_capacity(struct ui_path *path);
#endif

TEST test_squircle_oom_mock(void) {
  struct ui_path path;
  int i;
  ui_error_t rc;

  memset(&path, 0, sizeof(path));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_path_init(&path, 16);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, path.cmds);
  g_malloc_fail_countdown = -1;

  /* test_hook_ensure_path_capacity tests */
  rc = test_hook_ensure_path_capacity(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  memset(&path, 0, sizeof(path));
  rc = cupertino_path_init(&path, 4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = test_hook_ensure_path_capacity(&path);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_path_destroy(&path);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* ensure_path_capacity with cmd_capacity == 0 */
  memset(&path, 0, sizeof(path));
  rc = test_hook_ensure_path_capacity(&path);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(16, path.cmd_capacity);
  rc = cupertino_path_destroy(&path);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* ensure_path_capacity OOM when cmd_capacity == 0 */
  memset(&path, 0, sizeof(path));
  g_malloc_fail_countdown = 0;
  rc = test_hook_ensure_path_capacity(&path);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* ensure_path_capacity OOM during individual commands */
  memset(&path, 0, sizeof(path));
  g_malloc_fail_countdown = 0;
  rc = cupertino_path_move_to(&path, 1.0f, 1.0f);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  memset(&path, 0, sizeof(path));
  g_malloc_fail_countdown = 0;
  rc = cupertino_path_line_to(&path, 1.0f, 1.0f);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  memset(&path, 0, sizeof(path));
  g_malloc_fail_countdown = 0;
  rc = cupertino_path_bezier_to(&path, 1.0f, 1.0f, 2.0f, 2.0f, 3.0f, 3.0f);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  memset(&path, 0, sizeof(path));
  g_malloc_fail_countdown = 0;
  rc = cupertino_path_close(&path);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Step failure loops for cupertino_squircle_generate_path */
  /* Sharp rectangle (5 steps: 0..4) */
  rc = cupertino_path_init(&path, 32);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  for (i = 0; i < 5; i++) {
    path.cmd_count = 0;
    g_cupertino_squircle_mock_cmd_fail_countdown = i;
    rc = cupertino_squircle_generate_path(0.0f, 0.0f, 100.0f, 50.0f, 0.0f, 0.0f,
                                          &path);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  /* Circular rounded rectangle (10 steps: 0..9) */
  for (i = 0; i < 10; i++) {
    path.cmd_count = 0;
    g_cupertino_squircle_mock_cmd_fail_countdown = i;
    rc = cupertino_squircle_generate_path(10.0f, 20.0f, 100.0f, 80.0f, 16.0f,
                                          0.0f, &path);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  /* Continuous G2 squircle (14 steps: 0..13) */
  for (i = 0; i < 14; i++) {
    path.cmd_count = 0;
    g_cupertino_squircle_mock_cmd_fail_countdown = i;
    rc = cupertino_squircle_generate_path(0.0f, 0.0f, 200.0f, 150.0f, 24.0f,
                                          1.0f, &path);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  g_cupertino_squircle_mock_cmd_fail_countdown = -1;
  rc = cupertino_path_destroy(&path);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#endif

  PASS();
}

SUITE(cupertino_squircle_suite) {
  RUN_TEST(test_squircle_point_at);
  RUN_TEST(test_squircle_inset);
  RUN_TEST(test_hairline_and_pixel_snapping);
  RUN_TEST(test_path_operations_and_squircle_path);
  RUN_TEST(test_squircle_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_squircle_suite);
  GREATEST_MAIN_END();
}
