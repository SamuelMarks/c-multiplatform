/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_compositor_anim_lifecycle(void) {
  cmp_compositor_anim_t *anim = NULL;
  int res = cmp_compositor_anim_create(CMP_COMPOSITOR_PROP_OPACITY, &anim);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, anim);

  res = cmp_compositor_anim_destroy(anim);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_compositor_anim_null_args(void) {
  cmp_compositor_val_t val;
  cmp_compositor_anim_t *anim = NULL;
  int finished;
  int res = cmp_compositor_anim_create(CMP_COMPOSITOR_PROP_OPACITY, NULL);

  memset(&val, 0, sizeof(val));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_compositor_anim_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_compositor_anim_set_range(NULL, &val, &val);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_compositor_anim_create(CMP_COMPOSITOR_PROP_OPACITY, &anim);

  res = cmp_compositor_anim_set_range(anim, NULL, &val);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_compositor_anim_set_range(anim, &val, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_compositor_anim_step(NULL, 16.6, 100.0, &val, &finished);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_compositor_anim_step(anim, 16.6, 100.0, NULL, &finished);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_compositor_anim_step(anim, 16.6, 100.0, &val, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_compositor_anim_destroy(anim);
  PASS();
}

TEST test_compositor_anim_opacity_step(void) {
  cmp_compositor_anim_t *anim = NULL;
  cmp_compositor_val_t start_val;
  cmp_compositor_val_t end_val;
  cmp_compositor_val_t out_val;
  int finished = 0;
  int res;

  cmp_compositor_anim_create(CMP_COMPOSITOR_PROP_OPACITY, &anim);

  memset(&start_val, 0, sizeof(start_val));
  memset(&end_val, 0, sizeof(end_val));
  memset(&out_val, 0, sizeof(out_val));

  start_val.opacity = 0.0f;
  end_val.opacity = 1.0f;

  cmp_compositor_anim_set_range(anim, &start_val, &end_val);

  /* Step 1: 50% */
  res = cmp_compositor_anim_step(anim, 50.0, 100.0, &out_val, &finished);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, finished);
  ASSERT_EQ_FMT(0.5f, out_val.opacity, "%f");

  /* Step 2: 100% */
  res = cmp_compositor_anim_step(anim, 50.0, 100.0, &out_val, &finished);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, finished);
  ASSERT_EQ_FMT(1.0f, out_val.opacity, "%f");

  /* Step 3: Past end */
  res = cmp_compositor_anim_step(anim, 50.0, 100.0, &out_val, &finished);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, finished);
  ASSERT_EQ_FMT(1.0f, out_val.opacity, "%f");

  cmp_compositor_anim_destroy(anim);
  PASS();
}

TEST test_compositor_anim_transform_step(void) {
  cmp_compositor_anim_t *anim = NULL;
  cmp_compositor_val_t start_val;
  cmp_compositor_val_t end_val;
  cmp_compositor_val_t out_val;
  int finished = 0;
  int i;
  int res;

  cmp_compositor_anim_create(CMP_COMPOSITOR_PROP_TRANSFORM, &anim);

  memset(&start_val, 0, sizeof(start_val));
  memset(&end_val, 0, sizeof(end_val));
  memset(&out_val, 0, sizeof(out_val));

  for (i = 0; i < 16; i++) {
    start_val.transform_matrix[i] = 0.0f;
    end_val.transform_matrix[i] = 10.0f;
  }

  cmp_compositor_anim_set_range(anim, &start_val, &end_val);

  /* Step 1: 50% */
  res = cmp_compositor_anim_step(anim, 50.0, 100.0, &out_val, &finished);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, finished);
  for (i = 0; i < 16; i++) {
    ASSERT_EQ_FMT(5.0f, out_val.transform_matrix[i], "%f");
  }

  cmp_compositor_anim_destroy(anim);
  PASS();
}

TEST test_compositor_anim_zero_duration(void) {
  cmp_compositor_anim_t *anim = NULL;
  cmp_compositor_val_t out_val;
  int finished = 0;
  int res;

  cmp_compositor_anim_create(CMP_COMPOSITOR_PROP_OPACITY, &anim);

  memset(&out_val, 0, sizeof(out_val));

  res = cmp_compositor_anim_step(anim, 0.0, 0.0, &out_val, &finished);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, finished);
  /* The default implementation might just return 1.0f or end value, check
   * structure behavior */
  ASSERT_EQ_FMT(1.0f, out_val.opacity, "%f");

  cmp_compositor_anim_destroy(anim);
  PASS();
}

SUITE(compositor_anim_suite) {
  RUN_TEST(test_compositor_anim_lifecycle);
  RUN_TEST(test_compositor_anim_null_args);
  RUN_TEST(test_compositor_anim_opacity_step);
  RUN_TEST(test_compositor_anim_transform_step);
  RUN_TEST(test_compositor_anim_zero_duration);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  (void)argc;
  (void)argv;
  RUN_SUITE(compositor_anim_suite);
  GREATEST_MAIN_END();
}
