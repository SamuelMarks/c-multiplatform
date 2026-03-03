#include "m3/m3_motion.h"
#include "test_utils.h"

static int test_shared_axis(void) {
  M3MotionResult enter, exit;

  if (m3_motion_shared_axis(M3_SHARED_AXIS_X, CMP_TRUE, 0.0f, NULL, &exit) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_motion_shared_axis(M3_SHARED_AXIS_X, CMP_TRUE, 0.0f, &enter, NULL) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_motion_shared_axis(999, CMP_TRUE, 0.0f, &enter, &exit) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;

  m3_motion_shared_axis(M3_SHARED_AXIS_X, CMP_TRUE, 0.0f, &enter, &exit);
  if (enter.opacity != 0.0f || exit.opacity != 1.0f)
    return 1;
  if (enter.offset_x != M3_SHARED_AXIS_SLIDE_DISTANCE)
    return 1;
  if (exit.offset_x != 0.0f)
    return 1;

  m3_motion_shared_axis(M3_SHARED_AXIS_X, CMP_TRUE, 1.0f, &enter, &exit);
  if (enter.opacity != 1.0f || exit.opacity != 0.0f)
    return 1;
  if (enter.offset_x != 0.0f)
    return 1;
  if (exit.offset_x != -M3_SHARED_AXIS_SLIDE_DISTANCE)
    return 1;

  m3_motion_shared_axis(M3_SHARED_AXIS_Y, CMP_FALSE, 1.0f, &enter, &exit);
  if (enter.offset_y != 0.0f)
    return 1;
  if (exit.offset_y != M3_SHARED_AXIS_SLIDE_DISTANCE)
    return 1;

  m3_motion_shared_axis(M3_SHARED_AXIS_Z, CMP_TRUE, 1.0f, &enter, &exit);
  if (enter.scale_x != 1.0f)
    return 1;

  m3_motion_shared_axis(M3_SHARED_AXIS_Z, CMP_FALSE, 1.0f, &enter, &exit);
  m3_motion_shared_axis(M3_SHARED_AXIS_Z, CMP_TRUE, 0.0f, &enter, &exit);
  m3_motion_shared_axis(M3_SHARED_AXIS_Z, CMP_FALSE, 0.0f, &enter, &exit);
  m3_motion_shared_axis(M3_SHARED_AXIS_Z, CMP_TRUE, 0.5f, &enter, &exit);

  m3_motion_shared_axis(M3_SHARED_AXIS_X, CMP_TRUE, -0.5f, &enter, &exit);
  m3_motion_shared_axis(M3_SHARED_AXIS_X, CMP_TRUE, 1.5f, &enter, &exit);

  return 0;
}

static int test_fade_through(void) {
  M3MotionResult enter, exit;

  if (m3_motion_fade_through(0.0f, NULL, &exit) != CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_motion_fade_through(0.0f, &enter, NULL) != CMP_ERR_INVALID_ARGUMENT)
    return 1;

  m3_motion_fade_through(0.0f, &enter, &exit);
  if (enter.opacity != 0.0f || exit.opacity != 1.0f)
    return 1;
  if (enter.scale_x != 0.92f)
    return 1;

  m3_motion_fade_through(1.0f, &enter, &exit);
  if (enter.opacity != 1.0f || exit.opacity != 0.0f)
    return 1;
  if (enter.scale_x != 1.0f)
    return 1;

  m3_motion_fade_through(1.5f, &enter, &exit);
  if (enter.opacity != 1.0f)
    return 1;
  m3_motion_fade_through(-0.5f, &enter, &exit);
  if (enter.opacity != 0.0f)
    return 1;

  return 0;
}

static int test_container_transform(void) {
  CMPRect start = {0, 0, 100, 100};
  CMPRect end = {100, 100, 200, 200};
  CMPRect out;
  M3MotionResult enter, exit;

  if (m3_motion_container_transform(start, end, 0.5f, NULL, &enter, &exit) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_motion_container_transform(start, end, 0.5f, &out, NULL, &exit) !=
      CMP_ERR_INVALID_ARGUMENT)
    return 1;
  if (m3_motion_container_transform(start, end, 0.5f, &out, &enter, NULL) !=
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

int main(void) {
  if (test_shared_axis() != 0)
    return 1;
  if (test_fade_through() != 0)
    return 1;
  if (test_container_transform() != 0)
    return 1;
  return 0;
}
