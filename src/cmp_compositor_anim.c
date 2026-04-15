/* clang-format off */
#include "cmp.h"
/* clang-format on */

struct cmp_compositor_anim {
  cmp_compositor_prop_t property;
  cmp_compositor_val_t start_val;
  cmp_compositor_val_t end_val;
  double elapsed_ms;
};

int cmp_compositor_anim_create(cmp_compositor_prop_t property,
                               cmp_compositor_anim_t **out_anim) {
  cmp_compositor_anim_t *anim;

  if (!out_anim) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_compositor_anim_t), (void **)&anim) !=
      CMP_SUCCESS) {
    return CMP_ERROR_OOM;
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
  return CMP_SUCCESS;
}

int cmp_compositor_anim_destroy(cmp_compositor_anim_t *anim) {
  if (!anim) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(anim);
  return CMP_SUCCESS;
}

int cmp_compositor_anim_set_range(cmp_compositor_anim_t *anim,
                                  const cmp_compositor_val_t *start_val,
                                  const cmp_compositor_val_t *end_val) {
  if (!anim || !start_val || !end_val) {
    return CMP_ERROR_INVALID_ARG;
  }

  anim->start_val = *start_val;
  anim->end_val = *end_val;
  return CMP_SUCCESS;
}

int cmp_compositor_anim_step(cmp_compositor_anim_t *anim, double dt_ms,
                             double duration_ms, cmp_compositor_val_t *out_val,
                             int *out_finished) {
  double progress;

  if (!anim || !out_val || !out_finished) {
    return CMP_ERROR_INVALID_ARG;
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

  return CMP_SUCCESS;
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
  cmp_framebuffer_capture_t *capture;
  if (!window || !out_capture)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_framebuffer_capture_t), (void **)&capture) !=
      CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  capture->pixels = NULL;
  capture->width = 1920;
  capture->height = 1080;
  *out_capture = capture;
  return CMP_SUCCESS;
}

int cmp_compositor_release_framebuffer(cmp_framebuffer_capture_t *capture) {
  if (!capture)
    return CMP_ERROR_INVALID_ARG;
  if (capture->pixels) {
    CMP_FREE(capture->pixels);
  }
  CMP_FREE(capture);
  return CMP_SUCCESS;
}

int cmp_compositor_start_crossfade(cmp_window_t *window,
                                   cmp_framebuffer_capture_t *old_buffer,
                                   double duration_ms,
                                   const float *easing_curve) {
  if (!window || !old_buffer)
    return CMP_ERROR_INVALID_ARG;
  /* Mock scheduling of hardware cross-fade compositing task.
     In a real Vulkan/Metal backend this would bind old_buffer to a texture unit
     and dispatch a transition fragment shader. */
  (void)duration_ms;
  (void)easing_curve;
  return CMP_SUCCESS;
}
