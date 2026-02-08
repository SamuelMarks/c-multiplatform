#include "m3/m3_anim.h"

static M3Scalar m3_anim_abs(M3Scalar value)
{
    return (value < 0.0f) ? -value : value;
}

static M3Scalar m3_anim_clamp01(M3Scalar value)
{
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

static int m3_anim_ease_apply(m3_u32 easing, M3Scalar t, M3Scalar *out_value)
{
    M3Scalar clamped;
    M3Scalar inv;

    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    clamped = m3_anim_clamp01(t);

    switch (easing) {
    case M3_ANIM_EASE_LINEAR:
        *out_value = clamped;
        return M3_OK;
    case M3_ANIM_EASE_IN:
        *out_value = clamped * clamped;
        return M3_OK;
    case M3_ANIM_EASE_OUT:
        inv = 1.0f - clamped;
        *out_value = 1.0f - inv * inv;
        return M3_OK;
    case M3_ANIM_EASE_IN_OUT:
        if (clamped < 0.5f) {
            *out_value = 2.0f * clamped * clamped;
        } else {
            inv = -2.0f * clamped + 2.0f;
            *out_value = 1.0f - (inv * inv) * 0.5f;
        }
        return M3_OK;
    default:
        return M3_ERR_INVALID_ARGUMENT;
    }
}

int M3_CALL m3_anim_timing_init(M3AnimTiming *timing, M3Scalar from, M3Scalar to, M3Scalar duration, m3_u32 easing)
{
    M3Scalar eased;
    int rc;

    if (timing == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (duration < 0.0f) {
        return M3_ERR_RANGE;
    }

    rc = m3_anim_ease_apply(easing, 0.0f, &eased);
    if (rc != M3_OK) {
        return rc;
    }

    timing->from = from;
    timing->to = to;
    timing->duration = duration;
    timing->elapsed = 0.0f;
    timing->easing = easing;
    return M3_OK;
}

int M3_CALL m3_anim_timing_step(M3AnimTiming *timing, M3Scalar dt, M3Scalar *out_value, M3Bool *out_finished)
{
    M3Scalar new_elapsed;
    M3Scalar progress;
    M3Scalar eased;
    M3Bool finished;
    int rc;

    if (timing == NULL || out_value == NULL || out_finished == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (dt < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (timing->duration < 0.0f || timing->elapsed < 0.0f) {
        return M3_ERR_RANGE;
    }

    if (timing->duration == 0.0f) {
        timing->elapsed = timing->duration;
        *out_value = timing->to;
        *out_finished = M3_TRUE;
        return M3_OK;
    }

    new_elapsed = timing->elapsed + dt;
    if (new_elapsed >= timing->duration) {
        new_elapsed = timing->duration;
        finished = M3_TRUE;
    } else {
        finished = M3_FALSE;
    }

    progress = new_elapsed / timing->duration;

    rc = m3_anim_ease_apply(timing->easing, progress, &eased);
    if (rc != M3_OK) {
        return rc;
    }

    *out_value = timing->from + (timing->to - timing->from) * eased;
    *out_finished = finished;
    timing->elapsed = new_elapsed;
    return M3_OK;
}

int M3_CALL m3_spring_init(M3Spring *spring, M3Scalar position, M3Scalar target, M3Scalar stiffness, M3Scalar damping, M3Scalar mass)
{
    if (spring == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (stiffness <= 0.0f || damping < 0.0f || mass <= 0.0f) {
        return M3_ERR_RANGE;
    }

    spring->position = position;
    spring->velocity = 0.0f;
    spring->target = target;
    spring->stiffness = stiffness;
    spring->damping = damping;
    spring->mass = mass;
    spring->tolerance = 0.001f;
    spring->rest_velocity = 0.001f;
    return M3_OK;
}

int M3_CALL m3_spring_set_target(M3Spring *spring, M3Scalar target)
{
    if (spring == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    spring->target = target;
    return M3_OK;
}

int M3_CALL m3_spring_set_tolerance(M3Spring *spring, M3Scalar tolerance, M3Scalar rest_velocity)
{
    if (spring == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (tolerance < 0.0f || rest_velocity < 0.0f) {
        return M3_ERR_RANGE;
    }

    spring->tolerance = tolerance;
    spring->rest_velocity = rest_velocity;
    return M3_OK;
}

int M3_CALL m3_spring_step(M3Spring *spring, M3Scalar dt, M3Bool *out_finished)
{
    M3Scalar displacement;
    M3Scalar acceleration;

    if (spring == NULL || out_finished == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (dt < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (spring->mass <= 0.0f || spring->stiffness <= 0.0f || spring->damping < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (spring->tolerance < 0.0f || spring->rest_velocity < 0.0f) {
        return M3_ERR_RANGE;
    }

    displacement = spring->position - spring->target;

    if (dt > 0.0f) {
        acceleration = (-spring->stiffness * displacement - spring->damping * spring->velocity) / spring->mass;
        spring->velocity += acceleration * dt;
        spring->position += spring->velocity * dt;
    }

    displacement = spring->position - spring->target;
    if (m3_anim_abs(displacement) <= spring->tolerance && m3_anim_abs(spring->velocity) <= spring->rest_velocity) {
        spring->position = spring->target;
        spring->velocity = 0.0f;
        *out_finished = M3_TRUE;
    } else {
        *out_finished = M3_FALSE;
    }

    return M3_OK;
}

int M3_CALL m3_anim_controller_init(M3AnimController *controller)
{
    if (controller == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    controller->mode = M3_ANIM_MODE_NONE;
    controller->running = M3_FALSE;
    controller->value = 0.0f;
    controller->timing.from = 0.0f;
    controller->timing.to = 0.0f;
    controller->timing.duration = 0.0f;
    controller->timing.elapsed = 0.0f;
    controller->timing.easing = M3_ANIM_EASE_LINEAR;
    controller->spring.position = 0.0f;
    controller->spring.velocity = 0.0f;
    controller->spring.target = 0.0f;
    controller->spring.stiffness = 1.0f;
    controller->spring.damping = 0.0f;
    controller->spring.mass = 1.0f;
    controller->spring.tolerance = 0.001f;
    controller->spring.rest_velocity = 0.001f;
    return M3_OK;
}

int M3_CALL m3_anim_controller_start_timing(M3AnimController *controller, M3Scalar from, M3Scalar to, M3Scalar duration, m3_u32 easing)
{
    int rc;

    if (controller == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_anim_timing_init(&controller->timing, from, to, duration, easing);
    if (rc != M3_OK) {
        return rc;
    }

    controller->mode = M3_ANIM_MODE_TIMING;
    controller->value = from;
    if (duration == 0.0f) {
        controller->value = to;
        controller->running = M3_FALSE;
    } else {
        controller->running = M3_TRUE;
    }
    return M3_OK;
}

int M3_CALL m3_anim_controller_start_spring(M3AnimController *controller, M3Scalar position, M3Scalar target, M3Scalar stiffness, M3Scalar damping, M3Scalar mass)
{
    int rc;

    if (controller == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_spring_init(&controller->spring, position, target, stiffness, damping, mass);
    if (rc != M3_OK) {
        return rc;
    }

    controller->mode = M3_ANIM_MODE_SPRING;
    controller->running = M3_TRUE;
    controller->value = position;
    return M3_OK;
}

int M3_CALL m3_anim_controller_set_spring_target(M3AnimController *controller, M3Scalar target)
{
    if (controller == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (controller->mode != M3_ANIM_MODE_SPRING) {
        return M3_ERR_STATE;
    }

    controller->spring.target = target;
    controller->running = M3_TRUE;
    return M3_OK;
}

int M3_CALL m3_anim_controller_step(M3AnimController *controller, M3Scalar dt, M3Scalar *out_value, M3Bool *out_finished)
{
    M3Scalar value;
    M3Bool finished;
    int rc;

    if (controller == NULL || out_value == NULL || out_finished == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (dt < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (controller->mode == M3_ANIM_MODE_NONE) {
        return M3_ERR_STATE;
    }

    if (controller->running == M3_FALSE) {
        *out_value = controller->value;
        *out_finished = M3_TRUE;
        return M3_OK;
    }

    switch (controller->mode) {
    case M3_ANIM_MODE_TIMING:
        rc = m3_anim_timing_step(&controller->timing, dt, &value, &finished);
        if (rc != M3_OK) {
            return rc;
        }
        controller->value = value;
        if (finished == M3_TRUE) {
            controller->running = M3_FALSE;
        }
        *out_value = value;
        *out_finished = finished;
        return M3_OK;
    case M3_ANIM_MODE_SPRING:
        rc = m3_spring_step(&controller->spring, dt, &finished);
        if (rc != M3_OK) {
            return rc;
        }
        controller->value = controller->spring.position;
        if (finished == M3_TRUE) {
            controller->running = M3_FALSE;
        }
        *out_value = controller->value;
        *out_finished = finished;
        return M3_OK;
    default:
        return M3_ERR_INVALID_ARGUMENT;
    }
}

int M3_CALL m3_anim_controller_get_value(const M3AnimController *controller, M3Scalar *out_value)
{
    if (controller == NULL || out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_value = controller->value;
    return M3_OK;
}

int M3_CALL m3_anim_controller_is_running(const M3AnimController *controller, M3Bool *out_running)
{
    if (controller == NULL || out_running == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_running = controller->running;
    return M3_OK;
}

int M3_CALL m3_anim_controller_stop(M3AnimController *controller)
{
    if (controller == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    controller->running = M3_FALSE;
    return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_anim_test_apply_ease(m3_u32 easing, M3Scalar t, M3Scalar *out_value)
{
    return m3_anim_ease_apply(easing, t, out_value);
}
#endif
