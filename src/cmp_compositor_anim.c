/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_compositor_anim {
  cmp_compositor_prop_t property;
  cmp_compositor_val_t start_val;
  cmp_compositor_val_t end_val;
  double elapsed_ms;
};

/**
 * @brief cmp_compositor_anim_create
 *
 * @param property Parameter description.
 * @param out_anim Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_anim_create(cmp_compositor_prop_t property,
                               cmp_compositor_anim_t **out_anim) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_compositor_anim_t *anim = NULL;

  if (out_anim == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_compositor_anim_create: Invalid argument (out_anim=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_compositor_anim_t), (void **)&anim);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_anim_create: Out of memory: %s\n", err_str);

    return rc;
  }

  anim->property = property;
  anim->elapsed_ms = 0.0;

  if (property == CMP_COMPOSITOR_PROP_OPACITY) {
    anim->start_val.opacity = 0.0f;
    anim->end_val.opacity = 1.0f;
  } else {
    int i;
    for (i = 0; i < 16; i++) {
      anim->start_val.transform_matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
      anim->end_val.transform_matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
  }

  *out_anim = anim;
  cmp_log_debug("cmp_compositor_anim_create: Successfully created compositor "
                "anim context\n");

  return rc;
}

/**
 * @brief cmp_compositor_anim_destroy
 *
 * @param anim Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_anim_destroy(cmp_compositor_anim_t *anim) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (anim == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_anim_destroy: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_FREE(anim);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_compositor_anim_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_compositor_anim_destroy: Successfully destroyed "
                "compositor anim context\n");
  return rc;
}

/**
 * @brief cmp_compositor_anim_set_range
 *
 * @param anim Parameter description.
 * @param start_val Parameter description.
 * @param end_val Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_anim_set_range(cmp_compositor_anim_t *anim,
                                  const cmp_compositor_val_t *start_val,
                                  const cmp_compositor_val_t *end_val) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (anim == NULL || start_val == NULL || end_val == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_anim_set_range: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  anim->start_val = *start_val;
  anim->end_val = *end_val;
  cmp_log_debug("cmp_compositor_anim_set_range: Successfully set anim range\n");

  return rc;
}

/**
 * @brief cmp_compositor_anim_step
 *
 * @param anim Parameter description.
 * @param dt_ms Parameter description.
 * @param duration_ms Parameter description.
 * @param out_val Parameter description.
 * @param out_finished Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_anim_step(cmp_compositor_anim_t *anim, double dt_ms,
                             double duration_ms, cmp_compositor_val_t *out_val,
                             int *out_finished) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  double progress;

  if (anim == NULL || out_val == NULL || out_finished == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_anim_step: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (dt_ms < 0.0) {
    dt_ms = 0.0;
  }

  anim->elapsed_ms += dt_ms;

  if (duration_ms <= 0.0) {
    progress = 1.0;
  } else {
    progress = anim->elapsed_ms / duration_ms;
  }

  if (progress >= 1.0) {
    progress = 1.0;
    *out_finished = 1;
  } else {
    *out_finished = 0;
  }

  if (anim->property == CMP_COMPOSITOR_PROP_OPACITY) {
    out_val->opacity =
        (float)(anim->start_val.opacity +
                (anim->end_val.opacity - anim->start_val.opacity) * progress);
  } else {
    int i;
    for (i = 0; i < 16; i++) {
      out_val->transform_matrix[i] =
          (float)(anim->start_val.transform_matrix[i] +
                  (anim->end_val.transform_matrix[i] -
                   anim->start_val.transform_matrix[i]) *
                      progress);
    }
  }

  cmp_log_debug("cmp_compositor_anim_step: Stepped animation progress=%.2f\n",
                progress);

  return rc;
}

/* Mock implementation of framebuffer capture and cross-fading for architectural
 * completeness */
struct cmp_framebuffer_capture {
  void *pixels;
  int width;
  int height;
};

/**
 * @brief cmp_compositor_capture_framebuffer
 *
 * @param window Parameter description.
 * @param out_capture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_capture_framebuffer(
    cmp_window_t *window, cmp_framebuffer_capture_t **out_capture) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_framebuffer_capture_t *capture = NULL;

  if (window == NULL || out_capture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_capture_framebuffer: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_framebuffer_capture_t), (void **)&capture);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_capture_framebuffer: Out of memory: %s\n",
                  err_str);

    return rc;
  }

  capture->pixels = NULL;
  capture->width = 1920;
  capture->height = 1080;
  *out_capture = capture;
  cmp_log_debug("cmp_compositor_capture_framebuffer: Successfully created mock "
                "framebuffer capture\n");

  return rc;
}

/**
 * @brief cmp_compositor_release_framebuffer
 *
 * @param capture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_release_framebuffer(cmp_framebuffer_capture_t *capture) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (capture == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_release_framebuffer: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  if (capture->pixels != NULL) {
    rc = CMP_FREE(capture->pixels);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_compositor_release_framebuffer: Failed freeing pixels\n");
    }
  }
  rc = CMP_FREE(capture);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug(
        "cmp_compositor_release_framebuffer: Failed freeing capture context\n");
  }

  cmp_log_debug("cmp_compositor_release_framebuffer: Successfully released "
                "framebuffer capture\n");
  return rc;
}

/**
 * @brief cmp_compositor_start_crossfade
 *
 * @param window Parameter description.
 * @param old_buffer Parameter description.
 * @param duration_ms Parameter description.
 * @param easing_curve Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_start_crossfade(cmp_window_t *window,
                                   cmp_framebuffer_capture_t *old_buffer,
                                   double duration_ms,
                                   const float *easing_curve) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (window == NULL || old_buffer == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_start_crossfade: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* Mock scheduling of hardware cross-fade compositing task.
     In a real Vulkan/Metal backend this would bind old_buffer to a texture unit
     and dispatch a transition fragment shader. */
  (void)duration_ms;
  (void)easing_curve;
  cmp_log_debug("cmp_compositor_start_crossfade: Mock scheduled crossfade\n");

  return rc;
}
