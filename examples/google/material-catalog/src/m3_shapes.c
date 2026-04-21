#include "m3_shapes.h"

static float get_radius(m3_shape_scale_t scale) {
  switch (scale) {
  case M3_SHAPE_NONE:
    return 0.0f;
  case M3_SHAPE_EXTRA_SMALL:
    return 4.0f;
  case M3_SHAPE_SMALL:
    return 8.0f;
  case M3_SHAPE_MEDIUM:
    return 12.0f;
  case M3_SHAPE_LARGE:
    return 16.0f;
  case M3_SHAPE_EXTRA_LARGE:
    return 28.0f;
  case M3_SHAPE_FULL:
    return 9999.0f; /* Handled via is_full flag usually, but fallback logic */
  default:
    return 0.0f;
  }
}

int m3_shape_init_uniform(m3_shape_scale_t scale, m3_shape_t *out_shape) {
  float r;
  if (!out_shape)
    return 1;

  r = get_radius(scale);
  out_shape->top_left_dp = r;
  out_shape->top_right_dp = r;
  out_shape->bottom_right_dp = r;
  out_shape->bottom_left_dp = r;
  out_shape->is_full = (scale == M3_SHAPE_FULL) ? 1 : 0;

  return 0;
}

int m3_shape_init_asymmetric(m3_shape_scale_t top_left,
                             m3_shape_scale_t top_right,
                             m3_shape_scale_t bottom_right,
                             m3_shape_scale_t bottom_left,
                             m3_shape_t *out_shape) {
  if (!out_shape)
    return 1;

  out_shape->top_left_dp = get_radius(top_left);
  out_shape->top_right_dp = get_radius(top_right);
  out_shape->bottom_right_dp = get_radius(bottom_right);
  out_shape->bottom_left_dp = get_radius(bottom_left);

  /* If all are FULL, mark as full. Otherwise, it's just big radiuses */
  out_shape->is_full =
      (top_left == M3_SHAPE_FULL && top_right == M3_SHAPE_FULL &&
       bottom_right == M3_SHAPE_FULL && bottom_left == M3_SHAPE_FULL)
          ? 1
          : 0;

  return 0;
}

int m3_shape_tween(const m3_shape_t *start, const m3_shape_t *end, float t,
                   m3_shape_t *out_shape) {
  if (!start || !end || !out_shape)
    return 1;

  if (t < 0.0f)
    t = 0.0f;
  if (t > 1.0f)
    t = 1.0f;

  /* Handle transitions involving FULL shapes.
   * When tweening, if one is FULL, we use a very large radius to simulate a
   * smooth morph to pill/circle, but we don't instantly set is_full unless t
   * == 1.0 for the target.
   */
  {
    float s_tl = start->is_full ? 9999.0f : start->top_left_dp;
    float s_tr = start->is_full ? 9999.0f : start->top_right_dp;
    float s_br = start->is_full ? 9999.0f : start->bottom_right_dp;
    float s_bl = start->is_full ? 9999.0f : start->bottom_left_dp;

    float e_tl = end->is_full ? 9999.0f : end->top_left_dp;
    float e_tr = end->is_full ? 9999.0f : end->top_right_dp;
    float e_br = end->is_full ? 9999.0f : end->bottom_right_dp;
    float e_bl = end->is_full ? 9999.0f : end->bottom_left_dp;

    out_shape->top_left_dp = s_tl + (e_tl - s_tl) * t;
    out_shape->top_right_dp = s_tr + (e_tr - s_tr) * t;
    out_shape->bottom_right_dp = s_br + (e_br - s_br) * t;
    out_shape->bottom_left_dp = s_bl + (e_bl - s_bl) * t;

    /* Only mark as perfectly FULL if interpolation is complete and end is full
     */
    out_shape->is_full = (t >= 0.999f && end->is_full) ? 1 : 0;
  }

  return 0;
}
