#include "cmpc/cmp_anim.h"
#include "test_utils.h"

static int cmp_anim_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

int main(void) {
  CMPAnimTiming timing;
  CMPSpring spring;
  CMPAnimController controller;
  CMPScalar value;
  CMPBool finished;
  CMPBool running;
  int i;

  CMP_TEST_EXPECT(
      cmp_anim_timing_init(NULL, 0.0f, 1.0f, 1.0f, CMP_ANIM_EASE_LINEAR),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_anim_timing_init(&timing, 0.0f, 1.0f, -1.0f, CMP_ANIM_EASE_LINEAR),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_anim_timing_init(&timing, 0.0f, 1.0f, 1.0f, 99),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      cmp_anim_timing_init(&timing, 0.0f, 10.0f, 1.0f, CMP_ANIM_EASE_LINEAR));

  CMP_TEST_EXPECT(cmp_anim_timing_step(NULL, 0.1f, &value, &finished),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_anim_timing_step(&timing, 0.1f, NULL, &finished),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_anim_timing_step(&timing, 0.1f, &value, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_anim_timing_step(&timing, -0.1f, &value, &finished),
                  CMP_ERR_RANGE);

  timing.duration = -1.0f;
  CMP_TEST_EXPECT(cmp_anim_timing_step(&timing, 0.1f, &value, &finished),
                  CMP_ERR_RANGE);
  timing.duration = 1.0f;
  timing.elapsed = -0.25f;
  CMP_TEST_EXPECT(cmp_anim_timing_step(&timing, 0.1f, &value, &finished),
                  CMP_ERR_RANGE);
  timing.elapsed = 0.0f;

  timing.easing = 99;
  CMP_TEST_EXPECT(cmp_anim_timing_step(&timing, 0.1f, &value, &finished),
                  CMP_ERR_INVALID_ARGUMENT);
  timing.easing = CMP_ANIM_EASE_LINEAR;

  CMP_TEST_OK(
      cmp_anim_timing_init(&timing, 5.0f, 7.0f, 0.0f, CMP_ANIM_EASE_LINEAR));
  CMP_TEST_OK(cmp_anim_timing_step(&timing, 0.5f, &value, &finished));
  CMP_TEST_ASSERT(finished == CMP_TRUE);
  CMP_TEST_ASSERT(value == 7.0f);

  CMP_TEST_OK(
      cmp_anim_timing_init(&timing, 0.0f, 10.0f, 1.0f, CMP_ANIM_EASE_LINEAR));
  CMP_TEST_OK(cmp_anim_timing_step(&timing, 0.5f, &value, &finished));
  CMP_TEST_ASSERT(finished == CMP_FALSE);
  CMP_TEST_ASSERT(cmp_anim_near(value, 5.0f, 0.001f));
  CMP_TEST_OK(cmp_anim_timing_step(&timing, 0.6f, &value, &finished));
  CMP_TEST_ASSERT(finished == CMP_TRUE);
  CMP_TEST_ASSERT(cmp_anim_near(value, 10.0f, 0.001f));

  CMP_TEST_EXPECT(cmp_anim_test_apply_ease(99, 0.5f, &value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_anim_test_apply_ease(CMP_ANIM_EASE_LINEAR, 0.5f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_anim_test_apply_ease(CMP_ANIM_EASE_LINEAR, -1.0f, &value));
  CMP_TEST_ASSERT(value == 0.0f);
  CMP_TEST_OK(cmp_anim_test_apply_ease(CMP_ANIM_EASE_LINEAR, 2.0f, &value));
  CMP_TEST_ASSERT(value == 1.0f);
  CMP_TEST_OK(cmp_anim_test_apply_ease(CMP_ANIM_EASE_IN, 0.5f, &value));
  CMP_TEST_ASSERT(cmp_anim_near(value, 0.25f, 0.0001f));
  CMP_TEST_OK(cmp_anim_test_apply_ease(CMP_ANIM_EASE_OUT, 0.5f, &value));
  CMP_TEST_ASSERT(cmp_anim_near(value, 0.75f, 0.0001f));
  CMP_TEST_OK(cmp_anim_test_apply_ease(CMP_ANIM_EASE_IN_OUT, 0.25f, &value));
  CMP_TEST_ASSERT(cmp_anim_near(value, 0.125f, 0.0001f));
  CMP_TEST_OK(cmp_anim_test_apply_ease(CMP_ANIM_EASE_IN_OUT, 0.75f, &value));
  CMP_TEST_ASSERT(cmp_anim_near(value, 0.875f, 0.0001f));

  CMP_TEST_EXPECT(cmp_spring_init(NULL, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_spring_init(&spring, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_spring_init(&spring, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_spring_init(&spring, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_spring_init(&spring, 0.0f, 1.0f, 60.0f, 8.0f, 1.0f));

  CMP_TEST_EXPECT(cmp_spring_set_target(NULL, 1.0f), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_spring_set_target(&spring, 2.0f));

  CMP_TEST_EXPECT(cmp_spring_set_tolerance(NULL, 0.1f, 0.1f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_spring_set_tolerance(&spring, -0.1f, 0.1f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_spring_set_tolerance(&spring, 0.1f, -0.1f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_spring_set_tolerance(&spring, 0.001f, 0.001f));

  CMP_TEST_EXPECT(cmp_spring_step(NULL, 0.1f, &finished),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_spring_step(&spring, 0.1f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_spring_step(&spring, -0.1f, &finished), CMP_ERR_RANGE);

  spring.mass = 0.0f;
  CMP_TEST_EXPECT(cmp_spring_step(&spring, 0.1f, &finished), CMP_ERR_RANGE);
  spring.mass = 1.0f;
  spring.stiffness = 0.0f;
  CMP_TEST_EXPECT(cmp_spring_step(&spring, 0.1f, &finished), CMP_ERR_RANGE);
  spring.stiffness = 60.0f;
  spring.damping = -1.0f;
  CMP_TEST_EXPECT(cmp_spring_step(&spring, 0.1f, &finished), CMP_ERR_RANGE);
  spring.damping = 8.0f;
  spring.tolerance = -1.0f;
  CMP_TEST_EXPECT(cmp_spring_step(&spring, 0.1f, &finished), CMP_ERR_RANGE);
  spring.tolerance = 0.001f;
  spring.rest_velocity = -1.0f;
  CMP_TEST_EXPECT(cmp_spring_step(&spring, 0.1f, &finished), CMP_ERR_RANGE);
  spring.rest_velocity = 0.001f;

  CMP_TEST_OK(cmp_spring_init(&spring, 1.0f, 1.0f, 60.0f, 8.0f, 1.0f));
  CMP_TEST_OK(cmp_spring_set_tolerance(&spring, 0.001f, 0.001f));
  spring.velocity = 0.0f;
  CMP_TEST_OK(cmp_spring_step(&spring, 0.0f, &finished));
  CMP_TEST_ASSERT(finished == CMP_TRUE);
  CMP_TEST_ASSERT(spring.position == 1.0f);

  CMP_TEST_OK(cmp_spring_init(&spring, 0.0f, 1.0f, 60.0f, 8.0f, 1.0f));
  CMP_TEST_OK(cmp_spring_set_tolerance(&spring, 0.001f, 0.001f));
  finished = CMP_FALSE;
  for (i = 0; i < 2000 && finished == CMP_FALSE; ++i) {
    CMP_TEST_OK(cmp_spring_step(&spring, 0.016f, &finished));
  }
  CMP_TEST_ASSERT(finished == CMP_TRUE);
  CMP_TEST_ASSERT(cmp_anim_near(spring.position, 1.0f, 0.01f));

  CMP_TEST_EXPECT(cmp_anim_controller_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_anim_controller_init(&controller));
  CMP_TEST_EXPECT(cmp_anim_controller_get_value(NULL, &value),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_anim_controller_get_value(&controller, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_anim_controller_get_value(&controller, &value));
  CMP_TEST_ASSERT(value == 0.0f);
  CMP_TEST_EXPECT(cmp_anim_controller_is_running(NULL, &running),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_anim_controller_is_running(&controller, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_anim_controller_is_running(&controller, &running));
  CMP_TEST_ASSERT(running == CMP_FALSE);
  CMP_TEST_EXPECT(cmp_anim_controller_stop(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_anim_controller_stop(&controller));

  CMP_TEST_EXPECT(cmp_anim_controller_step(NULL, 0.1f, &value, &finished),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_anim_controller_step(&controller, 0.1f, NULL, &finished),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_anim_controller_step(&controller, 0.1f, &value, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_anim_controller_step(&controller, 0.1f, &value, &finished),
      CMP_ERR_STATE);
  CMP_TEST_EXPECT(
      cmp_anim_controller_step(&controller, -0.1f, &value, &finished),
      CMP_ERR_RANGE);

  CMP_TEST_EXPECT(cmp_anim_controller_start_timing(NULL, 0.0f, 1.0f, 1.0f,
                                                   CMP_ANIM_EASE_LINEAR),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_anim_controller_start_timing(&controller, 0.0f, 1.0f, 1.0f, 99),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_anim_controller_start_timing(&controller, 0.0f, 1.0f, 0.0f,
                                               CMP_ANIM_EASE_LINEAR));
  CMP_TEST_OK(cmp_anim_controller_is_running(&controller, &running));
  CMP_TEST_ASSERT(running == CMP_FALSE);
  CMP_TEST_OK(cmp_anim_controller_step(&controller, 0.1f, &value, &finished));
  CMP_TEST_ASSERT(finished == CMP_TRUE);
  CMP_TEST_ASSERT(value == 1.0f);

  CMP_TEST_OK(cmp_anim_controller_start_timing(&controller, 0.0f, 1.0f, 1.0f,
                                               CMP_ANIM_EASE_LINEAR));
  CMP_TEST_OK(cmp_anim_controller_is_running(&controller, &running));
  CMP_TEST_ASSERT(running == CMP_TRUE);
  CMP_TEST_OK(cmp_anim_controller_step(&controller, 0.5f, &value, &finished));
  CMP_TEST_ASSERT(finished == CMP_FALSE);
  CMP_TEST_OK(cmp_anim_controller_step(&controller, 0.6f, &value, &finished));
  CMP_TEST_ASSERT(finished == CMP_TRUE);
  CMP_TEST_OK(cmp_anim_controller_is_running(&controller, &running));
  CMP_TEST_ASSERT(running == CMP_FALSE);

  CMP_TEST_EXPECT(
      cmp_anim_controller_start_spring(NULL, 0.0f, 1.0f, 60.0f, 8.0f, 1.0f),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_anim_controller_start_spring(&controller, 0.0f, 1.0f,
                                                   0.0f, 8.0f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_anim_controller_start_spring(&controller, 0.0f, 1.0f, 60.0f,
                                               8.0f, 1.0f));
  CMP_TEST_EXPECT(cmp_anim_controller_set_spring_target(NULL, 2.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_anim_controller_set_spring_target(&controller, 2.0f));
  CMP_TEST_OK(cmp_anim_controller_step(&controller, 0.016f, &value, &finished));
  CMP_TEST_ASSERT(controller.mode == CMP_ANIM_MODE_SPRING);

  CMP_TEST_OK(cmp_anim_controller_start_timing(&controller, 0.0f, 1.0f, 1.0f,
                                               CMP_ANIM_EASE_LINEAR));
  CMP_TEST_EXPECT(cmp_anim_controller_set_spring_target(&controller, 3.0f),
                  CMP_ERR_STATE);

  controller.mode = 99;
  controller.running = CMP_TRUE;
  CMP_TEST_EXPECT(
      cmp_anim_controller_step(&controller, 0.1f, &value, &finished),
      CMP_ERR_INVALID_ARGUMENT);

  controller.mode = CMP_ANIM_MODE_TIMING;
  controller.running = CMP_TRUE;
  controller.timing.duration = -1.0f;
  controller.timing.elapsed = 0.0f;
  controller.timing.easing = CMP_ANIM_EASE_LINEAR;
  CMP_TEST_EXPECT(
      cmp_anim_controller_step(&controller, 0.1f, &value, &finished),
      CMP_ERR_RANGE);

  controller.mode = CMP_ANIM_MODE_SPRING;
  controller.running = CMP_TRUE;
  controller.spring.mass = 0.0f;
  controller.spring.stiffness = 1.0f;
  controller.spring.damping = 0.0f;
  controller.spring.tolerance = 0.001f;
  controller.spring.rest_velocity = 0.001f;
  CMP_TEST_EXPECT(
      cmp_anim_controller_step(&controller, 0.1f, &value, &finished),
      CMP_ERR_RANGE);

  CMP_TEST_OK(
      cmp_spring_init(&controller.spring, 1.0f, 1.0f, 60.0f, 8.0f, 1.0f));
  controller.running = CMP_TRUE;
  CMP_TEST_OK(cmp_anim_controller_step(&controller, 0.0f, &value, &finished));
  CMP_TEST_ASSERT(finished == CMP_TRUE);
  CMP_TEST_ASSERT(controller.running == CMP_FALSE);

  controller.mode = CMP_ANIM_MODE_TIMING;
  controller.running = CMP_FALSE;
  CMP_TEST_OK(cmp_anim_controller_step(&controller, 0.1f, &value, &finished));
  CMP_TEST_ASSERT(finished == CMP_TRUE);

  return 0;
}
