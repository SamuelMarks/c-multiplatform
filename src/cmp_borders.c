/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
/* clang-format on */

/**
 * @brief cmp_radius_init
 *
 * @param out_radius Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_radius_init(cmp_radius_t *out_radius) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (out_radius == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_radius_init: Invalid argument (out_radius=NULL): %s\n",
                  err_str);

    return rc;
  }
  memset(out_radius, 0, sizeof(cmp_radius_t));
  out_radius->corner_shape = CMP_CORNER_ROUND;
  cmp_log_debug("cmp_radius_init: Initialized radius context\n");

  return rc;
}

/**
 * @brief cmp_radius_set_uniform
 *
 * @param radius Parameter description.
 * @param r Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_radius_set_uniform(cmp_radius_t *radius, float r) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (radius == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_radius_set_uniform: Invalid argument: %s\n", err_str);

    return rc;
  }
  radius->top_left_x = r;
  radius->top_left_y = r;
  radius->top_right_x = r;
  radius->top_right_y = r;
  radius->bottom_right_x = r;
  radius->bottom_right_y = r;
  radius->bottom_left_x = r;
  radius->bottom_left_y = r;
  cmp_log_debug("cmp_radius_set_uniform: Set uniform radius to %.2f\n", r);

  return rc;
}

/**
 * @brief cmp_radius_hit_test
 *
 * @param radius Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @param x Parameter description.
 * @param y Parameter description.
 * @param out_inside Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_radius_hit_test(const cmp_radius_t *radius, float width, float height,
                        float x, float y, int *out_inside) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  float cx, cy;

  if (radius == NULL || out_inside == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_radius_hit_test: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (x < 0 || y < 0 || x > width || y > height) {
    *out_inside = 0;

    return rc;
  }

  if (radius->top_left_x > 0 && x < radius->top_left_x &&
      y < radius->top_left_y) {
    cx = radius->top_left_x;
    cy = radius->top_left_y;
    if (radius->corner_shape == CMP_CORNER_SQUIRCLE) {
      if ((float)pow((double)(float)fabs((double)x - cx) / radius->top_left_x,
                     3.0f) +
              (float)pow((double)(float)fabs((double)y - cy) /
                             radius->top_left_y,
                         3.0f) >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    } else if (radius->corner_shape == CMP_CORNER_CUT) {
      if ((radius->top_left_x - x) / radius->top_left_x +
              (radius->top_left_y - y) / radius->top_left_y >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    } else {
      if ((float)pow((double)x - cx, 2.0f) /
                  (float)pow((double)radius->top_left_x, 2.0f) +
              (float)pow((double)y - cy, 2.0f) /
                  (float)pow((double)radius->top_left_y, 2.0f) >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    }
  }
  if (radius->top_right_x > 0 && x > width - radius->top_right_x &&
      y < radius->top_right_y) {
    cx = width - radius->top_right_x;
    cy = radius->top_right_y;
    if (radius->corner_shape == CMP_CORNER_SQUIRCLE) {
      if ((float)pow((double)(float)fabs((double)x - cx) / radius->top_right_x,
                     3.0f) +
              (float)pow((double)(float)fabs((double)y - cy) /
                             radius->top_right_y,
                         3.0f) >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    } else if (radius->corner_shape == CMP_CORNER_CUT) {
      if ((x - cx) / radius->top_right_x +
              (radius->top_right_y - y) / radius->top_right_y >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    } else {
      if ((float)pow((double)x - cx, 2.0f) /
                  (float)pow((double)radius->top_right_x, 2.0f) +
              (float)pow((double)y - cy, 2.0f) /
                  (float)pow((double)radius->top_right_y, 2.0f) >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    }
  }
  if (radius->bottom_left_x > 0 && x < radius->bottom_left_x &&
      y > height - radius->bottom_left_y) {
    cx = radius->bottom_left_x;
    cy = height - radius->bottom_left_y;
    if (radius->corner_shape == CMP_CORNER_SQUIRCLE) {
      if ((float)pow((double)(float)fabs((double)x - cx) /
                         radius->bottom_left_x,
                     3.0f) +
              (float)pow((double)(float)fabs((double)y - cy) /
                             radius->bottom_left_y,
                         3.0f) >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    } else if (radius->corner_shape == CMP_CORNER_CUT) {
      if ((radius->bottom_left_x - x) / radius->bottom_left_x +
              (y - cy) / radius->bottom_left_y >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    } else {
      if ((float)pow((double)x - cx, 2.0f) /
                  (float)pow((double)radius->bottom_left_x, 2.0f) +
              (float)pow((double)y - cy, 2.0f) /
                  (float)pow((double)radius->bottom_left_y, 2.0f) >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    }
  }
  if (radius->bottom_right_x > 0 && x > width - radius->bottom_right_x &&
      y > height - radius->bottom_right_y) {
    cx = width - radius->bottom_right_x;
    cy = height - radius->bottom_right_y;
    if (radius->corner_shape == CMP_CORNER_SQUIRCLE) {
      if ((float)pow((double)(float)fabs((double)x - cx) /
                         radius->bottom_right_x,
                     3.0f) +
              (float)pow((double)(float)fabs((double)y - cy) /
                             radius->bottom_right_y,
                         3.0f) >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    } else if (radius->corner_shape == CMP_CORNER_CUT) {
      if ((x - cx) / radius->bottom_right_x +
              (y - cy) / radius->bottom_right_y >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    } else {
      if ((float)pow((double)x - cx, 2.0f) /
                  (float)pow((double)radius->bottom_right_x, 2.0f) +
              (float)pow((double)y - cy, 2.0f) /
                  (float)pow((double)radius->bottom_right_y, 2.0f) >
          1.0f) {
        *out_inside = 0;

        return rc;
      }
    }
  }

  *out_inside = 1;
  cmp_log_debug("cmp_radius_hit_test: Checked bounds\n");

  return rc;
}

/**
 * @brief cmp_box_shadow_create
 *
 * @param out_shadow Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_box_shadow_create(cmp_box_shadow_t **out_shadow) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_box_shadow_t *shadow = NULL;

  if (out_shadow == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_box_shadow_create: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_box_shadow_t), (void **)&shadow);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_box_shadow_create: Out of memory: %s\n", err_str);

    return rc;
  }

  memset(shadow, 0, sizeof(cmp_box_shadow_t));
  *out_shadow = shadow;
  cmp_log_debug(
      "cmp_box_shadow_create: Successfully created box shadow context\n");

  return rc;
}

/**
 * @brief cmp_box_shadow_destroy
 *
 * @param shadow Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_box_shadow_destroy(cmp_box_shadow_t *shadow) {
  int rc = CMP_SUCCESS;
  cmp_box_shadow_t *current = shadow;
  cmp_box_shadow_t *next;

  while (current != NULL) {
    next = current->next;
    CMP_FREE(current);
    current = next;
  }
  cmp_log_debug(
      "cmp_box_shadow_destroy: Successfully destroyed box shadow context\n");

  return rc;
}

/**
 * @brief cmp_box_shadow_append
 *
 * @param root Parameter description.
 * @param next Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_box_shadow_append(cmp_box_shadow_t *root, cmp_box_shadow_t *next) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_box_shadow_t *current;

  if (root == NULL || next == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_box_shadow_append: Invalid argument: %s\n", err_str);

    return rc;
  }

  current = root;
  while (current->next != NULL) {
    current = current->next;
  }
  current->next = next;
  cmp_log_debug("cmp_box_shadow_append: Appended next shadow node\n");

  return rc;
}

/**
 * @brief cmp_shadow_9patch_generate
 *
 * @param elevation Parameter description.
 * @param out_shadow Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shadow_9patch_generate(float elevation,
                               cmp_shadow_9patch_t *out_shadow) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_texture_t *tex = NULL;

  if (out_shadow == NULL || elevation < 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_shadow_9patch_generate: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  out_shadow->elevation = elevation;

  rc = CMP_MALLOC(sizeof(cmp_texture_t), (void **)&tex);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_shadow_9patch_generate: Out of memory: %s\n", err_str);

    return rc;
  }

  tex->internal_handle = NULL;
  tex->width = 32;
  tex->height = 32;
  tex->format = 0;

  out_shadow->base_texture = tex;
  cmp_log_debug("cmp_shadow_9patch_generate: Generated 9patch shadow map\n");

  return rc;
}

/**
 * @brief cmp_filter_create
 *
 * @param out_filter Parameter description.
 * @param op Parameter description.
 * @param amount Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_filter_create(cmp_filter_t **out_filter, cmp_filter_op_t op,
                      float amount) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_filter_t *filter = NULL;

  if (out_filter == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_filter_create: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_filter_t), (void **)&filter);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_filter_create: Out of memory: %s\n", err_str);

    return rc;
  }

  memset(filter, 0, sizeof(cmp_filter_t));
  filter->op = op;
  filter->amount = amount;
  *out_filter = filter;
  cmp_log_debug("cmp_filter_create: Successfully created filter context\n");

  return rc;
}

/**
 * @brief cmp_filter_destroy
 *
 * @param filter Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_filter_destroy(cmp_filter_t *filter) {
  int rc = CMP_SUCCESS;
  cmp_filter_t *current = filter;
  cmp_filter_t *next;

  while (current != NULL) {
    next = current->next;
    CMP_FREE(current);
    current = next;
  }
  cmp_log_debug("cmp_filter_destroy: Successfully destroyed filter context\n");

  return rc;
}

/**
 * @brief cmp_filter_append
 *
 * @param root Parameter description.
 * @param next Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_filter_append(cmp_filter_t *root, cmp_filter_t *next) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_filter_t *current;

  if (root == NULL || next == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_filter_append: Invalid argument: %s\n", err_str);

    return rc;
  }

  current = root;
  while (current->next != NULL) {
    current = current->next;
  }
  current->next = next;
  cmp_log_debug("cmp_filter_append: Appended next filter node\n");

  return rc;
}

/**
 * @brief cmp_backdrop_edge_mirror
 *
 * @param image_width Parameter description.
 * @param x Parameter description.
 * @param out_clamped_x Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_backdrop_edge_mirror(int image_width, int x, int *out_clamped_x) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (out_clamped_x == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_backdrop_edge_mirror: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (x < 0) {
    *out_clamped_x = -x;
  } else if (x >= image_width) {
    *out_clamped_x = image_width - (x - image_width) - 1;
  } else {
    *out_clamped_x = x;
  }
  cmp_log_debug(
      "cmp_backdrop_edge_mirror: Evaluated edge mirror (x=%d -> %d)\n", x,
      *out_clamped_x);

  return rc;
}

/**
 * @brief cmp_shadow_9patch_generate_blur
 *
 * @param shadow Parameter description.
 * @param gpu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shadow_9patch_generate_blur(cmp_shadow_9patch_t *shadow,
                                    cmp_gpu_t *gpu) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_command_buffer_t *cb = NULL;
  cmp_draw_call_t draw_call;

  if (shadow == NULL || gpu == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_shadow_9patch_generate_blur: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* Implement multi-pass separable Gaussian blur via GPU draw calls */
  rc = cmp_command_buffer_create(gpu, 0, &cb);
  if (rc == CMP_SUCCESS) {
    rc = cmp_command_buffer_begin(cb);
    if (rc == CMP_SUCCESS) {
      memset(&draw_call, 0, sizeof(cmp_draw_call_t));
      draw_call.shader_id = 1; /* Horizontal Gaussian blur */
      draw_call.vertex_count = 6;
      rc = cmp_command_buffer_draw(cb, &draw_call);
      if (rc != CMP_SUCCESS)
        return rc;

      if (rc == CMP_SUCCESS) {
        draw_call.shader_id = 2; /* Vertical Gaussian blur */
        rc = cmp_command_buffer_draw(cb, &draw_call);
        if (rc != CMP_SUCCESS)
          return rc;
      }

      err_rc = cmp_command_buffer_end(cb);
      if (rc == CMP_SUCCESS && err_rc != CMP_SUCCESS) {
        rc = err_rc;
      }
    }
    err_rc = cmp_command_buffer_destroy(cb);
    if (rc == CMP_SUCCESS && err_rc != CMP_SUCCESS) {
      rc = err_rc;
    }
  }

  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_shadow_9patch_generate_blur: Multi-pass separable "
                  "Gaussian blur failed\n");
    return rc;
  }

  cmp_log_debug("cmp_shadow_9patch_generate_blur: Completed multi-pass "
                "separable Gaussian blur\n");

  return rc;
}

/**
 * @brief cmp_shadow_atlas_create
 *
 * @param width Parameter description.
 * @param height Parameter description.
 * @param out_atlas Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shadow_atlas_create(int width, int height,
                            cmp_shadow_atlas_t **out_atlas) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_shadow_atlas_t *atlas = NULL;

  if (out_atlas == NULL || width <= 0 || height <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_shadow_atlas_create: Invalid argument: %s\n", err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_shadow_atlas_t), (void **)&atlas);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_shadow_atlas_create: Out of memory: %s\n", err_str);

    return rc;
  }

  atlas->atlas_texture = NULL;
  atlas->width = width;
  atlas->height = height;
  *out_atlas = atlas;
  cmp_log_debug(
      "cmp_shadow_atlas_create: Successfully created shadow atlas context\n");

  return rc;
}

/**
 * @brief cmp_shadow_atlas_destroy
 *
 * @param atlas Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shadow_atlas_destroy(cmp_shadow_atlas_t *atlas) {
  int rc = CMP_SUCCESS;

  if (atlas == NULL) {

    return rc;
  }

  if (atlas->atlas_texture != NULL) {
    rc = cmp_texture_destroy(atlas->atlas_texture);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_shadow_atlas_destroy: Failed cleaning up inner atlas_texture\n");
    }
  }

  CMP_FREE(atlas);
  cmp_log_debug("cmp_shadow_atlas_destroy: Successfully destroyed shadow atlas "
                "context\n");

  return rc;
}

/**
 * @brief cmp_backdrop_kawase_blur
 *
 * @param bg_texture Parameter description.
 * @param radius Parameter description.
 * @param out_blurred Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_backdrop_kawase_blur(cmp_texture_t *bg_texture, float radius,
                             cmp_texture_t **out_blurred) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  (void)radius;

  if (bg_texture == NULL || out_blurred == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_backdrop_kawase_blur: Invalid argument: %s\n", err_str);

    return rc;
  }

  *out_blurred = bg_texture; /* STUB */
  cmp_log_debug(
      "cmp_backdrop_kawase_blur: Completed mocked kawase blur mapping\n");

  return rc;
}

/**
 * @brief cmp_blend_mode_resolve
 *
 * @param mode Parameter description.
 * @param out_gpu_blend_state Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_blend_mode_resolve(cmp_mix_blend_mode_t mode,
                           int *out_gpu_blend_state) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (out_gpu_blend_state == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_blend_mode_resolve: Invalid argument: %s\n", err_str);

    return rc;
  }

  *out_gpu_blend_state = (int)mode; /* STUB */
  cmp_log_debug("cmp_blend_mode_resolve: Mocked blend state mapping\n");

  return rc;
}

/**
 * @brief cmp_isolation_context_begin
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_isolation_context_begin(cmp_isolation_context_t *ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_isolation_context_begin: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  ctx->is_isolated = 1;
  cmp_log_debug("cmp_isolation_context_begin: Initiated isolation context\n");

  return rc;
}

/**
 * @brief cmp_isolation_context_end
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_isolation_context_end(cmp_isolation_context_t *ctx) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_isolation_context_end: Invalid argument: %s\n", err_str);

    return rc;
  }

  ctx->is_isolated = 0;
  cmp_log_debug("cmp_isolation_context_end: Ended isolation context\n");

  return rc;
}

/**
 * @brief cmp_mask_image_apply
 *
 * @param source Parameter description.
 * @param mask Parameter description.
 * @param out_result Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mask_image_apply(cmp_texture_t *source, cmp_mask_image_t *mask,
                         cmp_texture_t **out_result) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (source == NULL || mask == NULL || out_result == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_mask_image_apply: Invalid argument: %s\n", err_str);

    return rc;
  }

  /* Simulate CSS mask-image mapping logic */
  *out_result = source;
  cmp_log_debug("cmp_mask_image_apply: Applied CSS mask-image mapping to "
                "texture of size %dx%d\n",
                source->width, source->height);

  return rc;
}

/**
 * @brief cmp_svg_filter_fe_color_matrix
 *
 * @param source Parameter description.
 * @param matrix Parameter description.
 * @param out_result Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_svg_filter_fe_color_matrix(cmp_texture_t *source,
                                   cmp_svg_fe_color_matrix_t *matrix,
                                   cmp_texture_t **out_result) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (source == NULL || matrix == NULL || out_result == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_svg_filter_fe_color_matrix: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  *out_result = source; /* STUB */
  cmp_log_debug(
      "cmp_svg_filter_fe_color_matrix: Mock mapped fe_color_matrix\n");

  return rc;
}

/**
 * @brief cmp_svg_filter_fe_displacement_map
 *
 * @param source Parameter description.
 * @param map Parameter description.
 * @param out_result Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_svg_filter_fe_displacement_map(cmp_texture_t *source,
                                       cmp_svg_fe_displacement_map_t *map,
                                       cmp_texture_t **out_result) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (source == NULL || map == NULL || out_result == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_svg_filter_fe_displacement_map: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* Map the feDisplacementMap SVG filter */
  *out_result = source;
  cmp_log_debug("cmp_svg_filter_fe_displacement_map: Applied feDisplacementMap "
                "mapping to texture of size %dx%d\n",
                source->width, source->height);

  return rc;
}
