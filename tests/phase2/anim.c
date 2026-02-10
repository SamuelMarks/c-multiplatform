#include "m3/m3_anim.h"
#include "test_utils.h"

static int m3_anim_near(M3Scalar a, M3Scalar b, M3Scalar tol) {
  M3Scalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

int main(void) {
  M3AnimTiming timing;
  M3Spring spring;
  M3AnimController controller;
  M3Scalar value;
  M3Bool finished;
  M3Bool running;
  int i;

  M3_TEST_EXPECT(
      m3_anim_timing_init(NULL, 0.0f, 1.0f, 1.0f, M3_ANIM_EASE_LINEAR),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_anim_timing_init(&timing, 0.0f, 1.0f, -1.0f, M3_ANIM_EASE_LINEAR),
      M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_anim_timing_init(&timing, 0.0f, 1.0f, 1.0f, 99),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(
      m3_anim_timing_init(&timing, 0.0f, 10.0f, 1.0f, M3_ANIM_EASE_LINEAR));

  M3_TEST_EXPECT(m3_anim_timing_step(NULL, 0.1f, &value, &finished),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_anim_timing_step(&timing, 0.1f, NULL, &finished),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_anim_timing_step(&timing, 0.1f, &value, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_anim_timing_step(&timing, -0.1f, &value, &finished),
                 M3_ERR_RANGE);

  timing.duration = -1.0f;
  M3_TEST_EXPECT(m3_anim_timing_step(&timing, 0.1f, &value, &finished),
                 M3_ERR_RANGE);
  timing.duration = 1.0f;
  timing.elapsed = -0.25f;
  M3_TEST_EXPECT(m3_anim_timing_step(&timing, 0.1f, &value, &finished),
                 M3_ERR_RANGE);
  timing.elapsed = 0.0f;

  timing.easing = 99;
  M3_TEST_EXPECT(m3_anim_timing_step(&timing, 0.1f, &value, &finished),
                 M3_ERR_INVALID_ARGUMENT);
  timing.easing = M3_ANIM_EASE_LINEAR;

  M3_TEST_OK(
      m3_anim_timing_init(&timing, 5.0f, 7.0f, 0.0f, M3_ANIM_EASE_LINEAR));
  M3_TEST_OK(m3_anim_timing_step(&timing, 0.5f, &value, &finished));
  M3_TEST_ASSERT(finished == M3_TRUE);
  M3_TEST_ASSERT(value == 7.0f);

  M3_TEST_OK(
      m3_anim_timing_init(&timing, 0.0f, 10.0f, 1.0f, M3_ANIM_EASE_LINEAR));
  M3_TEST_OK(m3_anim_timing_step(&timing, 0.5f, &value, &finished));
  M3_TEST_ASSERT(finished == M3_FALSE);
  M3_TEST_ASSERT(m3_anim_near(value, 5.0f, 0.001f));
  M3_TEST_OK(m3_anim_timing_step(&timing, 0.6f, &value, &finished));
  M3_TEST_ASSERT(finished == M3_TRUE);
  M3_TEST_ASSERT(m3_anim_near(value, 10.0f, 0.001f));

  M3_TEST_EXPECT(m3_anim_test_apply_ease(99, 0.5f, &value),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_anim_test_apply_ease(M3_ANIM_EASE_LINEAR, 0.5f, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_anim_test_apply_ease(M3_ANIM_EASE_LINEAR, -1.0f, &value));
  M3_TEST_ASSERT(value == 0.0f);
  M3_TEST_OK(m3_anim_test_apply_ease(M3_ANIM_EASE_LINEAR, 2.0f, &value));
  M3_TEST_ASSERT(value == 1.0f);
  M3_TEST_OK(m3_anim_test_apply_ease(M3_ANIM_EASE_IN, 0.5f, &value));
  M3_TEST_ASSERT(m3_anim_near(value, 0.25f, 0.0001f));
  M3_TEST_OK(m3_anim_test_apply_ease(M3_ANIM_EASE_OUT, 0.5f, &value));
  M3_TEST_ASSERT(m3_anim_near(value, 0.75f, 0.0001f));
  M3_TEST_OK(m3_anim_test_apply_ease(M3_ANIM_EASE_IN_OUT, 0.25f, &value));
  M3_TEST_ASSERT(m3_anim_near(value, 0.125f, 0.0001f));
  M3_TEST_OK(m3_anim_test_apply_ease(M3_ANIM_EASE_IN_OUT, 0.75f, &value));
  M3_TEST_ASSERT(m3_anim_near(value, 0.875f, 0.0001f));

  M3_TEST_EXPECT(m3_spring_init(NULL, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_spring_init(&spring, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_spring_init(&spring, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_spring_init(&spring, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_spring_init(&spring, 0.0f, 1.0f, 60.0f, 8.0f, 1.0f));

  M3_TEST_EXPECT(m3_spring_set_target(NULL, 1.0f), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_spring_set_target(&spring, 2.0f));

  M3_TEST_EXPECT(m3_spring_set_tolerance(NULL, 0.1f, 0.1f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_spring_set_tolerance(&spring, -0.1f, 0.1f), M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_spring_set_tolerance(&spring, 0.1f, -0.1f), M3_ERR_RANGE);
  M3_TEST_OK(m3_spring_set_tolerance(&spring, 0.001f, 0.001f));

  M3_TEST_EXPECT(m3_spring_step(NULL, 0.1f, &finished),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_spring_step(&spring, 0.1f, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_spring_step(&spring, -0.1f, &finished), M3_ERR_RANGE);

  spring.mass = 0.0f;
  M3_TEST_EXPECT(m3_spring_step(&spring, 0.1f, &finished), M3_ERR_RANGE);
  spring.mass = 1.0f;
  spring.stiffness = 0.0f;
  M3_TEST_EXPECT(m3_spring_step(&spring, 0.1f, &finished), M3_ERR_RANGE);
  spring.stiffness = 60.0f;
  spring.damping = -1.0f;
  M3_TEST_EXPECT(m3_spring_step(&spring, 0.1f, &finished), M3_ERR_RANGE);
  spring.damping = 8.0f;
  spring.tolerance = -1.0f;
  M3_TEST_EXPECT(m3_spring_step(&spring, 0.1f, &finished), M3_ERR_RANGE);
  spring.tolerance = 0.001f;
  spring.rest_velocity = -1.0f;
  M3_TEST_EXPECT(m3_spring_step(&spring, 0.1f, &finished), M3_ERR_RANGE);
  spring.rest_velocity = 0.001f;

  M3_TEST_OK(m3_spring_init(&spring, 1.0f, 1.0f, 60.0f, 8.0f, 1.0f));
  M3_TEST_OK(m3_spring_set_tolerance(&spring, 0.001f, 0.001f));
  spring.velocity = 0.0f;
  M3_TEST_OK(m3_spring_step(&spring, 0.0f, &finished));
  M3_TEST_ASSERT(finished == M3_TRUE);
  M3_TEST_ASSERT(spring.position == 1.0f);

  M3_TEST_OK(m3_spring_init(&spring, 0.0f, 1.0f, 60.0f, 8.0f, 1.0f));
  M3_TEST_OK(m3_spring_set_tolerance(&spring, 0.001f, 0.001f));
  finished = M3_FALSE;
  for (i = 0; i < 2000 && finished == M3_FALSE; ++i) {
    M3_TEST_OK(m3_spring_step(&spring, 0.016f, &finished));
  }
  M3_TEST_ASSERT(finished == M3_TRUE);
  M3_TEST_ASSERT(m3_anim_near(spring.position, 1.0f, 0.01f));

  M3_TEST_EXPECT(m3_anim_controller_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_anim_controller_init(&controller));
  M3_TEST_EXPECT(m3_anim_controller_get_value(NULL, &value),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_anim_controller_get_value(&controller, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_anim_controller_get_value(&controller, &value));
  M3_TEST_ASSERT(value == 0.0f);
  M3_TEST_EXPECT(m3_anim_controller_is_running(NULL, &running),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_anim_controller_is_running(&controller, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_anim_controller_is_running(&controller, &running));
  M3_TEST_ASSERT(running == M3_FALSE);
  M3_TEST_EXPECT(m3_anim_controller_stop(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_anim_controller_stop(&controller));

  M3_TEST_EXPECT(m3_anim_controller_step(NULL, 0.1f, &value, &finished),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_anim_controller_step(&controller, 0.1f, NULL, &finished),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_anim_controller_step(&controller, 0.1f, &value, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_anim_controller_step(&controller, 0.1f, &value, &finished),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(m3_anim_controller_step(&controller, -0.1f, &value, &finished),
                 M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_anim_controller_start_timing(NULL, 0.0f, 1.0f, 1.0f,
                                                 M3_ANIM_EASE_LINEAR),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_anim_controller_start_timing(&controller, 0.0f, 1.0f, 1.0f, 99),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_anim_controller_start_timing(&controller, 0.0f, 1.0f, 0.0f,
                                             M3_ANIM_EASE_LINEAR));
  M3_TEST_OK(m3_anim_controller_is_running(&controller, &running));
  M3_TEST_ASSERT(running == M3_FALSE);
  M3_TEST_OK(m3_anim_controller_step(&controller, 0.1f, &value, &finished));
  M3_TEST_ASSERT(finished == M3_TRUE);
  M3_TEST_ASSERT(value == 1.0f);

  M3_TEST_OK(m3_anim_controller_start_timing(&controller, 0.0f, 1.0f, 1.0f,
                                             M3_ANIM_EASE_LINEAR));
  M3_TEST_OK(m3_anim_controller_is_running(&controller, &running));
  M3_TEST_ASSERT(running == M3_TRUE);
  M3_TEST_OK(m3_anim_controller_step(&controller, 0.5f, &value, &finished));
  M3_TEST_ASSERT(finished == M3_FALSE);
  M3_TEST_OK(m3_anim_controller_step(&controller, 0.6f, &value, &finished));
  M3_TEST_ASSERT(finished == M3_TRUE);
  M3_TEST_OK(m3_anim_controller_is_running(&controller, &running));
  M3_TEST_ASSERT(running == M3_FALSE);

  M3_TEST_EXPECT(
      m3_anim_controller_start_spring(NULL, 0.0f, 1.0f, 60.0f, 8.0f, 1.0f),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_anim_controller_start_spring(&controller, 0.0f, 1.0f, 0.0f,
                                                 8.0f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_anim_controller_start_spring(&controller, 0.0f, 1.0f, 60.0f,
                                             8.0f, 1.0f));
  M3_TEST_EXPECT(m3_anim_controller_set_spring_target(NULL, 2.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_anim_controller_set_spring_target(&controller, 2.0f));
  M3_TEST_OK(m3_anim_controller_step(&controller, 0.016f, &value, &finished));
  M3_TEST_ASSERT(controller.mode == M3_ANIM_MODE_SPRING);

  M3_TEST_OK(m3_anim_controller_start_timing(&controller, 0.0f, 1.0f, 1.0f,
                                             M3_ANIM_EASE_LINEAR));
  M3_TEST_EXPECT(m3_anim_controller_set_spring_target(&controller, 3.0f),
                 M3_ERR_STATE);

  controller.mode = 99;
  controller.running = M3_TRUE;
  M3_TEST_EXPECT(m3_anim_controller_step(&controller, 0.1f, &value, &finished),
                 M3_ERR_INVALID_ARGUMENT);

  controller.mode = M3_ANIM_MODE_TIMING;
  controller.running = M3_TRUE;
  controller.timing.duration = -1.0f;
  controller.timing.elapsed = 0.0f;
  controller.timing.easing = M3_ANIM_EASE_LINEAR;
  M3_TEST_EXPECT(m3_anim_controller_step(&controller, 0.1f, &value, &finished),
                 M3_ERR_RANGE);

  controller.mode = M3_ANIM_MODE_SPRING;
  controller.running = M3_TRUE;
  controller.spring.mass = 0.0f;
  controller.spring.stiffness = 1.0f;
  controller.spring.damping = 0.0f;
  controller.spring.tolerance = 0.001f;
  controller.spring.rest_velocity = 0.001f;
  M3_TEST_EXPECT(m3_anim_controller_step(&controller, 0.1f, &value, &finished),
                 M3_ERR_RANGE);

  M3_TEST_OK(m3_spring_init(&controller.spring, 1.0f, 1.0f, 60.0f, 8.0f, 1.0f));
  controller.running = M3_TRUE;
  M3_TEST_OK(m3_anim_controller_step(&controller, 0.0f, &value, &finished));
  M3_TEST_ASSERT(finished == M3_TRUE);
  M3_TEST_ASSERT(controller.running == M3_FALSE);

  controller.mode = M3_ANIM_MODE_TIMING;
  controller.running = M3_FALSE;
  M3_TEST_OK(m3_anim_controller_step(&controller, 0.1f, &value, &finished));
  M3_TEST_ASSERT(finished == M3_TRUE);

  return 0;
}
