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
  if (!out_radius) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_radius_init: Invalid argument: %s\n", err_str);
    }
    return rc;
  }
  memset(out_radius, 0, sizeof(cmp_radius_t));
  out_radius->corner_shape = CMP_CORNER_ROUND;
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
  if (!radius) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_radius_set_uniform: Invalid argument: %s\n", err_str);
    }
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
  float cx, cy;

  if (!radius || !out_inside) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_radius_hit_test: Invalid argument: %s\n", err_str);
    }
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
  cmp_box_shadow_t *shadow = NULL;

  if (!out_shadow) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_box_shadow_create: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_box_shadow_t), (void **)&shadow);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_box_shadow_create: Out of memory: %s\n", err_str);
    }
    return rc;
  }

  memset(shadow, 0, sizeof(cmp_box_shadow_t));
  *out_shadow = shadow;
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

  while (current) {
    next = current->next;
    CMP_FREE(current);
    current = next;
  }
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
  cmp_box_shadow_t *current;

  if (!root || !next) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_box_shadow_append: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  current = root;
  while (current->next) {
    current = current->next;
  }
  current->next = next;
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
  cmp_texture_t *tex = NULL;

  if (!out_shadow || elevation < 0.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_shadow_9patch_generate: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  out_shadow->elevation = elevation;

  rc = CMP_MALLOC(sizeof(cmp_texture_t), (void **)&tex);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_shadow_9patch_generate: Out of memory: %s\n", err_str);
    }
    return rc;
  }

  tex->internal_handle = NULL;
  tex->width = 32;
  tex->height = 32;
  tex->format = 0;

  out_shadow->base_texture = tex;
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
  cmp_filter_t *filter = NULL;

  if (!out_filter) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_filter_create: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_filter_t), (void **)&filter);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_filter_create: Out of memory: %s\n", err_str);
    }
    return rc;
  }

  memset(filter, 0, sizeof(cmp_filter_t));
  filter->op = op;
  filter->amount = amount;
  *out_filter = filter;
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

  while (current) {
    next = current->next;
    CMP_FREE(current);
    current = next;
  }
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
  cmp_filter_t *current;

  if (!root || !next) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_filter_append: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  current = root;
  while (current->next) {
    current = current->next;
  }
  current->next = next;
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

  if (!out_clamped_x) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_backdrop_edge_mirror: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  if (x < 0) {
    *out_clamped_x = -x;
  } else if (x >= image_width) {
    *out_clamped_x = image_width - (x - image_width) - 1;
  } else {
    *out_clamped_x = x;
  }
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

  if (!shadow || !gpu) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_shadow_9patch_generate_blur: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  /* STUB: Implement multi-pass separable Gaussian blur */
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
  cmp_shadow_atlas_t *atlas = NULL;

  if (!out_atlas || width <= 0 || height <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_shadow_atlas_create: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_shadow_atlas_t), (void **)&atlas);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_shadow_atlas_create: Out of memory: %s\n", err_str);
    }
    return rc;
  }

  atlas->atlas_texture = NULL;
  atlas->width = width;
  atlas->height = height;
  *out_atlas = atlas;
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

  if (!atlas)
    return rc;

  if (atlas->atlas_texture)
    rc = cmp_texture_destroy(atlas->atlas_texture);

  CMP_FREE(atlas);
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
  (void)radius;

  if (!bg_texture || !out_blurred) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_backdrop_kawase_blur: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  *out_blurred = bg_texture; /* STUB */
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

  if (!out_gpu_blend_state) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_blend_mode_resolve: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  *out_gpu_blend_state = (int)mode; /* STUB */
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

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_isolation_context_begin: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  ctx->is_isolated = 1;
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

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_isolation_context_end: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  ctx->is_isolated = 0;
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

  if (!source || !mask || !out_result) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_mask_image_apply: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  *out_result = source; /* STUB */
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

  if (!source || !matrix || !out_result) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_svg_filter_fe_color_matrix: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  *out_result = source; /* STUB */
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

  if (!source || !map || !out_result) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_svg_filter_fe_displacement_map: Invalid argument: %s\n",
                err_str);
    }
    return rc;
  }

  *out_result = source; /* STUB */
  return rc;
}
