/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_spring_animator(void) {
  cmp_spring_animator_t *anim = NULL;
  float val;
  int settled;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_spring_animator_create(1.0f, 100.0f, 10.0f, 0.0f, &anim));

  /* Initial state */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_spring_animator_evaluate(anim, 0.0f, &val, &settled));
  ASSERT_EQ(0.0f, val);
  ASSERT_EQ(0, settled);

  /* Step 1 */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_spring_animator_evaluate(anim, 0.1f, &val, &settled));
  ASSERT_EQ(0, settled);
  ASSERT(val > 0.0f); /* Should be moving towards 1.0 */

  /* Interrupt mid-flight */
  ASSERT_EQ(CMP_SUCCESS, cmp_spring_animator_interrupt(anim, 2.0f));

  /* Fast-forward to settled state (fake dt to force settle logic) */
  int i;
  for (i = 0; i < 50; i++) {
    cmp_spring_animator_evaluate(anim, 0.1f, &val, &settled);
    if (settled)
      break;
  }

  ASSERT_EQ(1, settled);
  ASSERT_EQ(2.0f, val);

  ASSERT_EQ(CMP_SUCCESS, cmp_spring_animator_destroy(anim));
  PASS();
}

TEST test_scrubbing(void) {
  cmp_spring_animator_t *anim = NULL;
  float val;
  int settled;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_spring_animator_create(1.0f, 100.0f, 10.0f, 0.0f, &anim));

  ASSERT_EQ(CMP_SUCCESS, cmp_spring_animator_scrub(anim, 0.5f));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_spring_animator_evaluate(anim, 0.1f, &val, &settled));
  ASSERT_EQ(0.5f, val);
  ASSERT_EQ(0, settled); /* Scrub never physically "settles" */

  /* Reverse scrub */
  ASSERT_EQ(CMP_SUCCESS, cmp_spring_animator_scrub(anim, 0.1f));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_spring_animator_evaluate(anim, 0.1f, &val, &settled));
  ASSERT_EQ(0.1f, val);

  ASSERT_EQ(CMP_SUCCESS, cmp_spring_animator_destroy(anim));
  PASS();
}

TEST test_gesture_velocity(void) {
  float vx, vy;
  ASSERT_EQ(CMP_SUCCESS, cmp_spring_calculate_gesture_velocity(100.0f, -50.0f,
                                                               0.1f, &vx, &vy));
  ASSERT_EQ(1000.0f, vx);
  ASSERT_EQ(-500.0f, vy);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_spring_calculate_gesture_velocity(100.0f, -50.0f, 0.0f, &vx,
                                                  &vy)); /* Prevent zero dt */
  PASS();
}

TEST test_null_args(void) {
  cmp_spring_animator_t *anim = NULL;
  float f;
  int i;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_spring_animator_create(1.0f, 100.0f, 10.0f, 0.0f, NULL));

  cmp_spring_animator_create(1.0f, 100.0f, 10.0f, 0.0f, &anim);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_spring_animator_interrupt(NULL, 1.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_spring_animator_scrub(NULL, 1.0f));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_spring_animator_evaluate(NULL, 0.1f, &f, &i));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_spring_animator_evaluate(anim, 0.1f, NULL, &i));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_spring_animator_evaluate(anim, 0.1f, &f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_spring_calculate_gesture_velocity(1.0f, 1.0f, 0.1f, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_spring_calculate_gesture_velocity(1.0f, 1.0f, 0.1f, &f, NULL));

  cmp_spring_animator_destroy(anim);
  PASS();
}

SUITE(spring_animator_suite) {
  RUN_TEST(test_spring_animator);
  RUN_TEST(test_scrubbing);
  RUN_TEST(test_gesture_velocity);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(spring_animator_suite);
  GREATEST_MAIN_END();
}
