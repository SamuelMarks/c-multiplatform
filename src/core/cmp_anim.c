#include "cmpc/cmp_anim.h"

static CMPScalar cmp_anim_abs(CMPScalar value) {
  return (value < 0.0f) ? -value : value;
}

static CMPScalar cmp_anim_clamp01(CMPScalar value) {
  if (value < 0.0f) {
    return 0.0f;
  }
  if (value > 1.0f) {
    return 1.0f;
  }
  return value;
}

static int cmp_anim_ease_apply(cmp_u32 easing, CMPScalar t,
                               CMPScalar *out_value) {
  CMPScalar clamped;
  CMPScalar inv;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  clamped = cmp_anim_clamp01(t);

  switch (easing) {
  case CMP_ANIM_EASE_LINEAR:
    *out_value = clamped;
    return CMP_OK;
  case CMP_ANIM_EASE_IN:
    *out_value = clamped * clamped;
    return CMP_OK;
  case CMP_ANIM_EASE_OUT:
    inv = 1.0f - clamped;
    *out_value = 1.0f - inv * inv;
    return CMP_OK;
  case CMP_ANIM_EASE_IN_OUT:
    if (clamped < 0.5f) {
      *out_value = 2.0f * clamped * clamped;
    } else {
      inv = -2.0f * clamped + 2.0f;
      *out_value = 1.0f - (inv * inv) * 0.5f;
    }
    return CMP_OK;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_anim_timing_init(CMPAnimTiming *timing, CMPScalar from,
                                  CMPScalar to, CMPScalar duration,
                                  cmp_u32 easing) {
  CMPScalar eased;
  int rc;

  if (timing == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (duration < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_anim_ease_apply(easing, 0.0f, &eased);
  if (rc != CMP_OK) {
    return rc;
  }

  timing->from = from;
  timing->to = to;
  timing->duration = duration;
  timing->elapsed = 0.0f;
  timing->easing = easing;
  return CMP_OK;
}

int CMP_CALL cmp_anim_timing_step(CMPAnimTiming *timing, CMPScalar dt,
                                  CMPScalar *out_value, CMPBool *out_finished) {
  CMPScalar new_elapsed;
  CMPScalar progress;
  CMPScalar eased;
  CMPBool finished;
  int rc;

  if (timing == NULL || out_value == NULL || out_finished == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (timing->duration < 0.0f || timing->elapsed < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (timing->duration == 0.0f) {
    timing->elapsed = timing->duration;
    *out_value = timing->to;
    *out_finished = CMP_TRUE;
    return CMP_OK;
  }

  new_elapsed = timing->elapsed + dt;
  if (new_elapsed >= timing->duration) {
    new_elapsed = timing->duration;
    finished = CMP_TRUE;
  } else {
    finished = CMP_FALSE;
  }

  progress = new_elapsed / timing->duration;

  rc = cmp_anim_ease_apply(timing->easing, progress, &eased);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_value = timing->from + (timing->to - timing->from) * eased;
  *out_finished = finished;
  timing->elapsed = new_elapsed;
  return CMP_OK;
}

int CMP_CALL cmp_spring_init(CMPSpring *spring, CMPScalar position,
                             CMPScalar target, CMPScalar stiffness,
                             CMPScalar damping, CMPScalar mass) {
  if (spring == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (stiffness <= 0.0f || damping < 0.0f || mass <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  spring->position = position;
  spring->velocity = 0.0f;
  spring->target = target;
  spring->stiffness = stiffness;
  spring->damping = damping;
  spring->mass = mass;
  spring->tolerance = 0.001f;
  spring->rest_velocity = 0.001f;
  return CMP_OK;
}

int CMP_CALL cmp_spring_set_target(CMPSpring *spring, CMPScalar target) {
  if (spring == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  spring->target = target;
  return CMP_OK;
}

int CMP_CALL cmp_spring_set_tolerance(CMPSpring *spring, CMPScalar tolerance,
                                      CMPScalar rest_velocity) {
  if (spring == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (tolerance < 0.0f || rest_velocity < 0.0f) {
    return CMP_ERR_RANGE;
  }

  spring->tolerance = tolerance;
  spring->rest_velocity = rest_velocity;
  return CMP_OK;
}

int CMP_CALL cmp_spring_step(CMPSpring *spring, CMPScalar dt,
                             CMPBool *out_finished) {
  CMPScalar displacement;
  CMPScalar acceleration;

  if (spring == NULL || out_finished == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (spring->mass <= 0.0f || spring->stiffness <= 0.0f ||
      spring->damping < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (spring->tolerance < 0.0f || spring->rest_velocity < 0.0f) {
    return CMP_ERR_RANGE;
  }

  displacement = spring->position - spring->target;

  if (dt > 0.0f) {
    acceleration = (-spring->stiffness * displacement -
                    spring->damping * spring->velocity) /
                   spring->mass;
    spring->velocity += acceleration * dt;
    spring->position += spring->velocity * dt;
  }

  displacement = spring->position - spring->target;
  if (cmp_anim_abs(displacement) <= spring->tolerance &&
      cmp_anim_abs(spring->velocity) <= spring->rest_velocity) {
    spring->position = spring->target;
    spring->velocity = 0.0f;
    *out_finished = CMP_TRUE;
  } else {
    *out_finished = CMP_FALSE;
  }

  return CMP_OK;
}

int CMP_CALL cmp_anim_controller_init(CMPAnimController *controller) {
  if (controller == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  controller->mode = CMP_ANIM_MODE_NONE;
  controller->running = CMP_FALSE;
  controller->value = 0.0f;
  controller->timing.from = 0.0f;
  controller->timing.to = 0.0f;
  controller->timing.duration = 0.0f;
  controller->timing.elapsed = 0.0f;
  controller->timing.easing = CMP_ANIM_EASE_LINEAR;
  controller->spring.position = 0.0f;
  controller->spring.velocity = 0.0f;
  controller->spring.target = 0.0f;
  controller->spring.stiffness = 1.0f;
  controller->spring.damping = 0.0f;
  controller->spring.mass = 1.0f;
  controller->spring.tolerance = 0.001f;
  controller->spring.rest_velocity = 0.001f;
  return CMP_OK;
}

int CMP_CALL cmp_anim_controller_start_timing(CMPAnimController *controller,
                                              CMPScalar from, CMPScalar to,
                                              CMPScalar duration,
                                              cmp_u32 easing) {
  int rc;

  if (controller == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_anim_timing_init(&controller->timing, from, to, duration, easing);
  if (rc != CMP_OK) {
    return rc;
  }

  controller->mode = CMP_ANIM_MODE_TIMING;
  controller->value = from;
  if (duration == 0.0f) {
    controller->value = to;
    controller->running = CMP_FALSE;
  } else {
    controller->running = CMP_TRUE;
  }
  return CMP_OK;
}

int CMP_CALL cmp_anim_controller_start_spring(
    CMPAnimController *controller, CMPScalar position, CMPScalar target,
    CMPScalar stiffness, CMPScalar damping, CMPScalar mass) {
  int rc;

  if (controller == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_spring_init(&controller->spring, position, target, stiffness,
                       damping, mass);
  if (rc != CMP_OK) {
    return rc;
  }

  controller->mode = CMP_ANIM_MODE_SPRING;
  controller->running = CMP_TRUE;
  controller->value = position;
  return CMP_OK;
}

int CMP_CALL cmp_anim_controller_set_spring_target(
    CMPAnimController *controller, CMPScalar target) {
  if (controller == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (controller->mode != CMP_ANIM_MODE_SPRING) {
    return CMP_ERR_STATE;
  }

  controller->spring.target = target;
  controller->running = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL cmp_anim_controller_step(CMPAnimController *controller,
                                      CMPScalar dt, CMPScalar *out_value,
                                      CMPBool *out_finished) {
  CMPScalar value;
  CMPBool finished;
  int rc;

  if (controller == NULL || out_value == NULL || out_finished == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (controller->mode == CMP_ANIM_MODE_NONE) {
    return CMP_ERR_STATE;
  }

  if (controller->running == CMP_FALSE) {
    *out_value = controller->value;
    *out_finished = CMP_TRUE;
    return CMP_OK;
  }

  switch (controller->mode) {
  case CMP_ANIM_MODE_TIMING:
    rc = cmp_anim_timing_step(&controller->timing, dt, &value, &finished);
    if (rc != CMP_OK) {
      return rc;
    }
    controller->value = value;
    if (finished == CMP_TRUE) {
      controller->running = CMP_FALSE;
    }
    *out_value = value;
    *out_finished = finished;
    return CMP_OK;
  case CMP_ANIM_MODE_SPRING:
    rc = cmp_spring_step(&controller->spring, dt, &finished);
    if (rc != CMP_OK) {
      return rc;
    }
    controller->value = controller->spring.position;
    if (finished == CMP_TRUE) {
      controller->running = CMP_FALSE;
    }
    *out_value = controller->value;
    *out_finished = finished;
    return CMP_OK;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_anim_controller_get_value(const CMPAnimController *controller,
                                           CMPScalar *out_value) {
  if (controller == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_value = controller->value;
  return CMP_OK;
}

int CMP_CALL cmp_anim_controller_is_running(const CMPAnimController *controller,
                                            CMPBool *out_running) {
  if (controller == NULL || out_running == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_running = controller->running;
  return CMP_OK;
}

int CMP_CALL cmp_anim_controller_stop(CMPAnimController *controller) {
  if (controller == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  controller->running = CMP_FALSE;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_anim_test_apply_ease(cmp_u32 easing, CMPScalar t,
                                      CMPScalar *out_value) {
  return cmp_anim_ease_apply(easing, t, out_value);
}
#endif
