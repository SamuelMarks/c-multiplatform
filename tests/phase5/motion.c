#include "m3/m3_motion.h"
#include "test_utils.h"

static int test_shared_axis(void) {
  M3MotionResult enter = {0}, exit = {0};

  if (m3_motion_shared_axis(M3_SHARED_AXIS_X, CMP_TRUE, 0.5f, NULL, &exit) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_motion_shared_axis(M3_SHARED_AXIS_X, CMP_TRUE, 0.5f, &enter, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  if (m3_motion_shared_axis(999, CMP_TRUE, 0.5f, &enter, &exit) != CMP_ERR_INVALID_ARGUMENT) return 1;
  m3_motion_shared_axis(M3_SHARED_AXIS_X, CMP_TRUE, 0.5f, &enter, &exit);
  if (enter.opacity <= 0.0f || exit.opacity >= 1.0f)
    return 1;

  m3_motion_shared_axis(M3_SHARED_AXIS_Y, CMP_FALSE, -0.5f, &enter, &exit);
  if (enter.opacity != 0.0f)
    return 1;

  m3_motion_shared_axis(M3_SHARED_AXIS_Z, CMP_TRUE, 1.5f, &enter, &exit);
  if (enter.opacity != 1.0f)
    return 1;

  return 0;
}

static int test_fade_through(void) {
  M3MotionResult enter = {0}, exit = {0};

  if (m3_motion_fade_through(0.5f, NULL, &exit) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  m3_motion_fade_through(0.2f, &enter, &exit);
  if (enter.opacity != 0.0f || exit.opacity > 0.5f)
    return 1;

  m3_motion_fade_through(0.8f, &enter, &exit);
  if (enter.opacity < 0.5f || exit.opacity != 0.0f)
    return 1;

  m3_motion_fade_through(-1.0f, &enter, &exit);
  if (enter.opacity != 0.0f || exit.opacity != 1.0f)
    return 1;

  m3_motion_fade_through(2.0f, &enter, &exit);
  if (enter.opacity != 1.0f || exit.opacity != 0.0f)
    return 1;

  return 0;
}

static int test_container_transform(void) {
  CMPRect start = {0.0f, 0.0f, 100.0f, 100.0f};
  CMPRect end = {100.0f, 100.0f, 200.0f, 200.0f};
  CMPRect out = {0};
  M3MotionResult enter = {0}, exit = {0};

  if (m3_motion_container_transform(start, end, 0.5f, NULL, &enter, &exit) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  m3_motion_container_transform(start, end, 0.5f, &out, &enter, &exit);
  if (out.x != 50.0f || out.width != 150.0f)
    return 1;

  m3_motion_container_transform(start, end, 1.5f, &out, &enter, &exit);
  if (out.x != 100.0f || out.width != 200.0f)
    return 1;

  m3_motion_container_transform(start, end, -0.5f, &out, &enter, &exit);
  if (out.x != 0.0f || out.width != 100.0f)
    return 1;

  return 0;
}

static int test_predictive_back(void) {
  CMPPredictiveBackEvent event = {0};
  CMPRect start = {0.0f, 0.0f, 100.0f, 100.0f};
  CMPRect out = {0};
  CMPScalar radius = 0.0f, opacity = 0.0f;

  if (m3_motion_predictive_back(NULL, start, &out, &radius, &opacity) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_motion_predictive_back(&event, start, NULL, &radius, &opacity) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_motion_predictive_back(&event, start, &out, NULL, &opacity) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_motion_predictive_back(&event, start, &out, &radius, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  event.progress = 0.5f;
  event.edge = 1;
  if (m3_motion_predictive_back(&event, start, &out, &radius, &opacity) != CMP_OK)
    return 1;
  if (out.width != 95.0f || radius != 8.0f || opacity != 1.0f)
    return 1;

  event.progress = 0.9f;
  event.edge = 2;
  if (m3_motion_predictive_back(&event, start, &out, &radius, &opacity) != CMP_OK)
    return 1;
  if (opacity > 0.6f)
    return 1;

  event.progress = -0.5f;
  if (m3_motion_predictive_back(&event, start, &out, &radius, &opacity) != CMP_OK)
    return 1;

  event.progress = 1.5f;
  if (m3_motion_predictive_back(&event, start, &out, &radius, &opacity) != CMP_OK)
    return 1;

  return 0;
}

int main(void) {
  if (test_shared_axis() != 0)
    return 1;
  if (test_fade_through() != 0)
    return 1;
  if (test_container_transform() != 0)
    return 1;
  if (test_predictive_back() != 0)
    return 1;
  return 0;
}
