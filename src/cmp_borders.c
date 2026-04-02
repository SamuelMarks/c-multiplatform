/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

int cmp_radius_init(cmp_radius_t *out_radius) {
  if (!out_radius)
    return CMP_ERROR_INVALID_ARG;
  memset(out_radius, 0, sizeof(cmp_radius_t));
  out_radius->corner_shape = CMP_CORNER_ROUND;
  return CMP_SUCCESS;
}

int cmp_radius_set_uniform(cmp_radius_t *radius, float r) {
  if (!radius)
    return CMP_ERROR_INVALID_ARG;
  radius->top_left_x = r;
  radius->top_left_y = r;
  radius->top_right_x = r;
  radius->top_right_y = r;
  radius->bottom_right_x = r;
  radius->bottom_right_y = r;
  radius->bottom_left_x = r;
  radius->bottom_left_y = r;
  return CMP_SUCCESS;
}

int cmp_radius_hit_test(const cmp_radius_t *radius, float width, float height,
                        float x, float y, int *out_inside) {
  if (!radius || !out_inside)
    return CMP_ERROR_INVALID_ARG;

  if (x < 0 || y < 0 || x > width || y > height) {
    *out_inside = 0;
    return CMP_SUCCESS;
  }

  if (radius->top_left_x > 0 && x < radius->top_left_x &&
      y < radius->top_left_y) {
    float cx = radius->top_left_x;
    float cy = radius->top_left_y;
    if (radius->corner_shape == CMP_CORNER_SQUIRCLE) {
      if (powf(fabsf(x - cx) / radius->top_left_x, 3.0f) +
              powf(fabsf(y - cy) / radius->top_left_y, 3.0f) >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    } else if (radius->corner_shape == CMP_CORNER_CUT) {
      if ((radius->top_left_x - x) / radius->top_left_x +
              (radius->top_left_y - y) / radius->top_left_y >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    } else {
      if (powf(x - cx, 2.0f) / powf(radius->top_left_x, 2.0f) +
              powf(y - cy, 2.0f) / powf(radius->top_left_y, 2.0f) >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    }
  }
  if (radius->top_right_x > 0 && x > width - radius->top_right_x &&
      y < radius->top_right_y) {
    float cx = width - radius->top_right_x;
    float cy = radius->top_right_y;
    if (radius->corner_shape == CMP_CORNER_SQUIRCLE) {
      if (powf(fabsf(x - cx) / radius->top_right_x, 3.0f) +
              powf(fabsf(y - cy) / radius->top_right_y, 3.0f) >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    } else if (radius->corner_shape == CMP_CORNER_CUT) {
      if ((x - cx) / radius->top_right_x +
              (radius->top_right_y - y) / radius->top_right_y >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    } else {
      if (powf(x - cx, 2.0f) / powf(radius->top_right_x, 2.0f) +
              powf(y - cy, 2.0f) / powf(radius->top_right_y, 2.0f) >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    }
  }
  if (radius->bottom_left_x > 0 && x < radius->bottom_left_x &&
      y > height - radius->bottom_left_y) {
    float cx = radius->bottom_left_x;
    float cy = height - radius->bottom_left_y;
    if (radius->corner_shape == CMP_CORNER_SQUIRCLE) {
      if (powf(fabsf(x - cx) / radius->bottom_left_x, 3.0f) +
              powf(fabsf(y - cy) / radius->bottom_left_y, 3.0f) >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    } else if (radius->corner_shape == CMP_CORNER_CUT) {
      if ((radius->bottom_left_x - x) / radius->bottom_left_x +
              (y - cy) / radius->bottom_left_y >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    } else {
      if (powf(x - cx, 2.0f) / powf(radius->bottom_left_x, 2.0f) +
              powf(y - cy, 2.0f) / powf(radius->bottom_left_y, 2.0f) >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    }
  }
  if (radius->bottom_right_x > 0 && x > width - radius->bottom_right_x &&
      y > height - radius->bottom_right_y) {
    float cx = width - radius->bottom_right_x;
    float cy = height - radius->bottom_right_y;
    if (radius->corner_shape == CMP_CORNER_SQUIRCLE) {
      if (powf(fabsf(x - cx) / radius->bottom_right_x, 3.0f) +
              powf(fabsf(y - cy) / radius->bottom_right_y, 3.0f) >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    } else if (radius->corner_shape == CMP_CORNER_CUT) {
      if ((x - cx) / radius->bottom_right_x +
              (y - cy) / radius->bottom_right_y >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    } else {
      if (powf(x - cx, 2.0f) / powf(radius->bottom_right_x, 2.0f) +
              powf(y - cy, 2.0f) / powf(radius->bottom_right_y, 2.0f) >
          1.0f) {
        *out_inside = 0;
        return CMP_SUCCESS;
      }
    }
  }

  *out_inside = 1;
  return CMP_SUCCESS;
}

int cmp_box_shadow_create(cmp_box_shadow_t **out_shadow) {
  cmp_box_shadow_t *shadow;
  if (!out_shadow)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_box_shadow_t), (void **)&shadow) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(shadow, 0, sizeof(cmp_box_shadow_t));
  *out_shadow = shadow;
  return CMP_SUCCESS;
}

int cmp_box_shadow_destroy(cmp_box_shadow_t *shadow) {
  cmp_box_shadow_t *current = shadow;
  cmp_box_shadow_t *next;
  while (current) {
    next = current->next;
    CMP_FREE(current);
    current = next;
  }
  return CMP_SUCCESS;
}

int cmp_box_shadow_append(cmp_box_shadow_t *root, cmp_box_shadow_t *next) {
  cmp_box_shadow_t *current;
  if (!root || !next)
    return CMP_ERROR_INVALID_ARG;
  current = root;
  while (current->next) {
    current = current->next;
  }
  current->next = next;
  return CMP_SUCCESS;
}

int cmp_shadow_9patch_generate(float elevation,
                               cmp_shadow_9patch_t *out_shadow) {
  if (!out_shadow)
    return CMP_ERROR_INVALID_ARG;
  out_shadow->elevation = elevation;
  out_shadow->base_texture = NULL; /* Stub */
  return CMP_SUCCESS;
}

int cmp_filter_create(cmp_filter_t **out_filter, cmp_filter_op_t op,
                      float amount) {
  cmp_filter_t *filter;
  if (!out_filter)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_filter_t), (void **)&filter) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(filter, 0, sizeof(cmp_filter_t));
  filter->op = op;
  filter->amount = amount;
  *out_filter = filter;
  return CMP_SUCCESS;
}

int cmp_filter_destroy(cmp_filter_t *filter) {
  cmp_filter_t *current = filter;
  cmp_filter_t *next;
  while (current) {
    next = current->next;
    CMP_FREE(current);
    current = next;
  }
  return CMP_SUCCESS;
}

int cmp_filter_append(cmp_filter_t *root, cmp_filter_t *next) {
  cmp_filter_t *current;
  if (!root || !next)
    return CMP_ERROR_INVALID_ARG;
  current = root;
  while (current->next) {
    current = current->next;
  }
  current->next = next;
  return CMP_SUCCESS;
}

int cmp_backdrop_edge_mirror(int image_width, int x, int *out_clamped_x) {
  if (!out_clamped_x)
    return CMP_ERROR_INVALID_ARG;
  if (x < 0) {
    *out_clamped_x = -x;
  } else if (x >= image_width) {
    *out_clamped_x = image_width - (x - image_width) - 1;
  } else {
    *out_clamped_x = x;
  }
  return CMP_SUCCESS;
}
