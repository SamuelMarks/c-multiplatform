/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <math.h>
/* clang-format on */

TEST test_rubber_band_lifecycle(void) {
  cmp_rubber_band_t *band = NULL;
  int res;

  res = cmp_rubber_band_create(&band);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, band);

  res = cmp_rubber_band_destroy(band);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_rubber_band_null_args(void) {
  cmp_rubber_band_t *band = NULL;
  float pos;
  int is_resting;
  int res;

  res = cmp_rubber_band_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_rubber_band_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_rubber_band_create(&band);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_rubber_band_start(NULL, 100.0f, 0.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_rubber_band_step(NULL, 16.0f, 0.0f, &pos, &is_resting);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_rubber_band_step(band, 16.0f, 0.0f, NULL, &is_resting);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_rubber_band_step(band, 16.0f, 0.0f, &pos, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_rubber_band_destroy(band);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_rubber_band_physics(void) {
  cmp_rubber_band_t *band = NULL;
  float pos = 0.0f;
  int is_resting = 0;
  int max_frames = 1000;
  int i;
  int res;

  res = cmp_rubber_band_create(&band);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Start at position 100, moving fast away from target 0 */
  res = cmp_rubber_band_start(band, 500.0f, 100.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Step it a few times to simulate pulling back to 0 */
  res = cmp_rubber_band_step(band, 16.0f, 0.0f, &pos, &is_resting);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, is_resting); /* Should not be resting immediately */
  /* Expect velocity to pull it back down towards 0 */

  /* Simulate until it rests (might take many frames) */
  for (i = 0; i < max_frames; i++) {
    res = cmp_rubber_band_step(band, 16.0f, 0.0f, &pos, &is_resting);
    ASSERT_EQ(CMP_SUCCESS, res);
    if (is_resting) {
      break;
    }
  }

  ASSERT_EQ(1, is_resting);
  /* Should rest at exactly the target */
  ASSERT(fabs(pos) < 0.1f);

  res = cmp_rubber_band_destroy(band);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(rubber_band_suite) {
  RUN_TEST(test_rubber_band_lifecycle);
  RUN_TEST(test_rubber_band_null_args);
  RUN_TEST(test_rubber_band_physics);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(rubber_band_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
