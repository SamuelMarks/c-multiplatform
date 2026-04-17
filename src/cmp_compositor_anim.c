/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
/* clang-format on */

struct cmp_compositor_anim {
  cmp_compositor_prop_t property;
  cmp_compositor_val_t start_val;
  cmp_compositor_val_t end_val;
  double elapsed_ms;
};

int cmp_compositor_anim_create(cmp_compositor_prop_t property,
                               cmp_compositor_anim_t **out_anim) {
  int rc = CMP_SUCCESS;
  cmp_compositor_anim_t *anim = NULL;

  if (!out_anim) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_compositor_anim_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_compositor_anim_t), (void **)&anim);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_compositor_anim_create: Out of memory\n");
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
  return rc;
}

int cmp_compositor_anim_destroy(cmp_compositor_anim_t *anim) {
  int rc = CMP_SUCCESS;

  if (!anim) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_compositor_anim_destroy: Invalid argument\n");
    return rc;
  }
  CMP_FREE(anim);
  return rc;
}

int cmp_compositor_anim_set_range(cmp_compositor_anim_t *anim,
                                  const cmp_compositor_val_t *start_val,
                                  const cmp_compositor_val_t *end_val) {
  int rc = CMP_SUCCESS;

  if (!anim || !start_val || !end_val) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_compositor_anim_set_range: Invalid argument\n");
    return rc;
  }

  anim->start_val = *start_val;
  anim->end_val = *end_val;
  return rc;
}

int cmp_compositor_anim_step(cmp_compositor_anim_t *anim, double dt_ms,
                             double duration_ms, cmp_compositor_val_t *out_val,
                             int *out_finished) {
  int rc = CMP_SUCCESS;
  double progress;

  if (!anim || !out_val || !out_finished) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_compositor_anim_step: Invalid argument\n");
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

  return rc;
}

/* Mock implementation of framebuffer capture and cross-fading for architectural
 * completeness */
struct cmp_framebuffer_capture {
  void *pixels;
  int width;
  int height;
};

int cmp_compositor_capture_framebuffer(
    cmp_window_t *window, cmp_framebuffer_capture_t **out_capture) {
  int rc = CMP_SUCCESS;
  cmp_framebuffer_capture_t *capture = NULL;

  if (!window || !out_capture) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_compositor_capture_framebuffer: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_framebuffer_capture_t), (void **)&capture);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_compositor_capture_framebuffer: Out of memory\n");
    return rc;
  }

  capture->pixels = NULL;
  capture->width = 1920;
  capture->height = 1080;
  *out_capture = capture;
  return rc;
}

int cmp_compositor_release_framebuffer(cmp_framebuffer_capture_t *capture) {
  int rc = CMP_SUCCESS;

  if (!capture) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_compositor_release_framebuffer: Invalid argument\n");
    return rc;
  }

  if (capture->pixels) {
    CMP_FREE(capture->pixels);
  }
  CMP_FREE(capture);
  return rc;
}

int cmp_compositor_start_crossfade(cmp_window_t *window,
                                   cmp_framebuffer_capture_t *old_buffer,
                                   double duration_ms,
                                   const float *easing_curve) {
  int rc = CMP_SUCCESS;

  if (!window || !old_buffer) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_compositor_start_crossfade: Invalid argument\n");
    return rc;
  }

  /* Mock scheduling of hardware cross-fade compositing task.
     In a real Vulkan/Metal backend this would bind old_buffer to a texture unit
     and dispatch a transition fragment shader. */
  (void)duration_ms;
  (void)easing_curve;
  return rc;
}
