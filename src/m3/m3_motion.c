/* clang-format off */
#include "m3/m3_motion.h"
/* clang-format on */

static CMPScalar map_fraction(CMPScalar fraction, CMPScalar start,
                              CMPScalar end) {
  if (fraction <= start)
    return 0.0f;
  if (fraction >= end)
    return 1.0f;
  return (fraction - start) / (end - start);
}

int CMP_CALL m3_motion_shared_axis(cmp_u32 axis, CMPBool forward,
                                   CMPScalar fraction,
                                   M3MotionResult *out_entering,
                                   M3MotionResult *out_exiting) {
  CMPScalar enter_alpha, exit_alpha;
  CMPScalar enter_offset, exit_offset;
  CMPScalar enter_scale, exit_scale;
  int sign;

  if (out_entering == NULL || out_exiting == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (fraction < 0.0f)
    fraction = 0.0f;
  if (fraction > 1.0f)
    fraction = 1.0f;

  exit_alpha = 1.0f - map_fraction(fraction, 0.0f, 0.3f);
  enter_alpha = map_fraction(fraction, 0.3f, 1.0f);

  out_entering->opacity = enter_alpha;
  out_exiting->opacity = exit_alpha;

  sign = forward ? 1 : -1;

  if (axis == M3_SHARED_AXIS_X) {
    enter_offset =
        (1.0f - fraction) * M3_SHARED_AXIS_SLIDE_DISTANCE * (CMPScalar)sign;
    exit_offset = -fraction * M3_SHARED_AXIS_SLIDE_DISTANCE * (CMPScalar)sign;

    out_entering->offset_x = enter_offset;
    out_entering->offset_y = 0.0f;
    out_exiting->offset_x = exit_offset;
    out_exiting->offset_y = 0.0f;

    out_entering->scale_x = 1.0f;
    out_entering->scale_y = 1.0f;
    out_exiting->scale_x = 1.0f;
    out_exiting->scale_y = 1.0f;
  } else if (axis == M3_SHARED_AXIS_Y) {
    enter_offset =
        (1.0f - fraction) * M3_SHARED_AXIS_SLIDE_DISTANCE * (CMPScalar)sign;
    exit_offset = -fraction * M3_SHARED_AXIS_SLIDE_DISTANCE * (CMPScalar)sign;

    out_entering->offset_x = 0.0f;
    out_entering->offset_y = enter_offset;
    out_exiting->offset_x = 0.0f;
    out_exiting->offset_y = exit_offset;

    out_entering->scale_x = 1.0f;
    out_entering->scale_y = 1.0f;
    out_exiting->scale_x = 1.0f;
    out_exiting->scale_y = 1.0f;
  } else if (axis == M3_SHARED_AXIS_Z) {
    enter_scale = forward ? (0.8f + 0.2f * fraction) : (1.1f - 0.1f * fraction);
    exit_scale = forward ? (1.0f + 0.1f * fraction) : (1.0f - 0.2f * fraction);

    out_entering->offset_x = 0.0f;
    out_entering->offset_y = 0.0f;
    out_exiting->offset_x = 0.0f;
    out_exiting->offset_y = 0.0f;

    out_entering->scale_x = enter_scale;
    out_entering->scale_y = enter_scale;
    out_exiting->scale_x = exit_scale;
    out_exiting->scale_y = exit_scale;
  } else {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return CMP_OK;
}

int CMP_CALL m3_motion_fade_through(CMPScalar fraction,
                                    M3MotionResult *out_entering,
                                    M3MotionResult *out_exiting) {
  if (out_entering == NULL || out_exiting == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (fraction < 0.0f)
    fraction = 0.0f;
  if (fraction > 1.0f)
    fraction = 1.0f;

  out_exiting->opacity = 1.0f - map_fraction(fraction, 0.0f, 0.35f);
  out_entering->opacity = map_fraction(fraction, 0.35f, 1.0f);

  out_entering->scale_x = 0.92f + 0.08f * fraction;
  out_entering->scale_y = 0.92f + 0.08f * fraction;
  out_exiting->scale_x = 1.0f;
  out_exiting->scale_y = 1.0f;

  out_entering->offset_x = 0.0f;
  out_entering->offset_y = 0.0f;
  out_exiting->offset_x = 0.0f;
  out_exiting->offset_y = 0.0f;

  return CMP_OK;
}

int CMP_CALL m3_motion_container_transform(CMPRect start_bounds,
                                           CMPRect end_bounds,
                                           CMPScalar fraction,
                                           CMPRect *out_bounds,
                                           M3MotionResult *out_entering,
                                           M3MotionResult *out_exiting) {
  if (out_bounds == NULL || out_entering == NULL || out_exiting == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (fraction < 0.0f)
    fraction = 0.0f;
  if (fraction > 1.0f)
    fraction = 1.0f;

  out_bounds->x = start_bounds.x + (end_bounds.x - start_bounds.x) * fraction;
  out_bounds->y = start_bounds.y + (end_bounds.y - start_bounds.y) * fraction;
  out_bounds->width =
      start_bounds.width + (end_bounds.width - start_bounds.width) * fraction;
  out_bounds->height = start_bounds.height +
                       (end_bounds.height - start_bounds.height) * fraction;

  out_exiting->opacity = 1.0f - map_fraction(fraction, 0.0f, 0.3f);
  out_entering->opacity = map_fraction(fraction, 0.3f, 1.0f);

  out_entering->scale_x = 1.0f;
  out_entering->scale_y = 1.0f;
  out_exiting->scale_x = 1.0f;
  out_exiting->scale_y = 1.0f;

  out_entering->offset_x = 0.0f;
  out_entering->offset_y = 0.0f;
  out_exiting->offset_x = 0.0f;
  out_exiting->offset_y = 0.0f;

  return CMP_OK;
}

int CMP_CALL m3_motion_predictive_back(const CMPPredictiveBackEvent *event,
                                       CMPRect start_bounds,
                                       CMPRect *out_bounds,
                                       CMPScalar *out_corner_radius,
                                       CMPScalar *out_opacity) {
  CMPScalar progress;
  CMPScalar scale;
  CMPScalar offset_x;

  if (event == NULL || out_bounds == NULL || out_corner_radius == NULL ||
      out_opacity == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  progress = event->progress;
  if (progress < 0.0f)
    progress = 0.0f;
  if (progress > 1.0f)
    progress = 1.0f;

  /* Predictive back typically scales down to 90% and shifts inwards by 8dp */
  scale = 1.0f - (0.1f * progress);

  /* Assume edge is LEFT for now (1 = left, 2 = right, 3 = top, 4 = bottom per
   * typical Android constants) */
  if (event->edge == 2) { /* CMP_PREDICTIVE_BACK_EDGE_RIGHT */
    offset_x = -8.0f * progress;
  } else {
    offset_x = 8.0f * progress;
  }

  out_bounds->width = start_bounds.width * scale;
  out_bounds->height = start_bounds.height * scale;

  /* Center the scaled bounds within the original frame, then apply offset */
  out_bounds->x = start_bounds.x +
                  ((start_bounds.width - out_bounds->width) * 0.5f) + offset_x;
  out_bounds->y =
      start_bounds.y + ((start_bounds.height - out_bounds->height) * 0.5f);

  /* Corner radius interpolates up to 16dp */
  *out_corner_radius = 16.0f * progress;

  /* Opacity fades slightly if approaching commit threshold */
  if (progress > 0.8f) {
    *out_opacity =
        1.0f - ((progress - 0.8f) * 5.0f); /* Fades out at the very end */
    if (*out_opacity <= 0.0f)
      *out_opacity = 0.0f;
  } else {
    *out_opacity = 1.0f;
  }

  return CMP_OK;
}
