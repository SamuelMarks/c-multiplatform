/* clang-format off */
#include "m3/m3_shape.h"
/* clang-format on */

CMP_API int CMP_CALL m3_shape_init_absolute(M3Shape *shape,
                                            CMPScalar radius_px) {
  if (shape == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  shape->top_left.type = M3_SHAPE_SIZE_ABSOLUTE;
  shape->top_left.value = radius_px;
  shape->top_right = shape->top_left;
  shape->bottom_right = shape->top_left;
  shape->bottom_left = shape->top_left;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_shape_init_percent(M3Shape *shape, CMPScalar percent) {
  if (shape == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  shape->top_left.type = M3_SHAPE_SIZE_PERCENT;
  shape->top_left.value = percent;
  shape->top_right = shape->top_left;
  shape->bottom_right = shape->top_left;
  shape->bottom_left = shape->top_left;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_shape_init_asymmetrical(M3Shape *shape, M3CornerSize tl,
                                                M3CornerSize tr,
                                                M3CornerSize br,
                                                M3CornerSize bl) {
  if (shape == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  shape->top_left = tl;
  shape->top_right = tr;
  shape->bottom_right = br;
  shape->bottom_left = bl;
  return CMP_OK;
}

static CMPScalar resolve_corner(M3CornerSize corner, CMPScalar shortest_edge) {
  if (corner.type == M3_SHAPE_SIZE_PERCENT) {
    return shortest_edge * corner.value;
  }
  return corner.value; /* Absolute */
}

CMP_API int CMP_CALL m3_shape_resolve(const M3Shape *shape, CMPRect bounds,
                                      CMPScalar *out_tl_px,
                                      CMPScalar *out_tr_px,
                                      CMPScalar *out_br_px,
                                      CMPScalar *out_bl_px) {
  CMPScalar shortest_edge;

  if (shape == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  shortest_edge = bounds.width < bounds.height ? bounds.width : bounds.height;
  if (shortest_edge < 0.0f) {
    shortest_edge = 0.0f;
  }

  if (out_tl_px)
    *out_tl_px = resolve_corner(shape->top_left, shortest_edge);
  if (out_tr_px)
    *out_tr_px = resolve_corner(shape->top_right, shortest_edge);
  if (out_br_px)
    *out_br_px = resolve_corner(shape->bottom_right, shortest_edge);
  if (out_bl_px)
    *out_bl_px = resolve_corner(shape->bottom_left, shortest_edge);

  return CMP_OK;
}

static M3CornerSize morph_corner(M3CornerSize start, M3CornerSize end,
                                 CMPScalar fraction) {
  M3CornerSize result;
  /* If types match, interpolate values directly.
     If types mismatch, standard interpolation is tricky without bounds.
     We default to just morphing the value and adopting the end type at 50%.
     For full exact morphing of mixed types, layout bounds are needed,
     which is typically done post-resolution.
     For this API, we snap type at 50% but lerp the raw value. */
  if (start.type == end.type) {
    result.type = start.type;
    result.value = start.value + (end.value - start.value) * fraction;
  } else {
    result.type = (fraction >= 0.5f) ? end.type : start.type;
    result.value = start.value + (end.value - start.value) * fraction;
  }
  return result;
}

CMP_API int CMP_CALL m3_shape_morph(const M3Shape *start_shape,
                                    const M3Shape *end_shape,
                                    CMPScalar fraction, M3Shape *out_shape) {
  if (start_shape == NULL || end_shape == NULL || out_shape == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (fraction < 0.0f)
    fraction = 0.0f;
  if (fraction > 1.0f)
    fraction = 1.0f;

  out_shape->top_left =
      morph_corner(start_shape->top_left, end_shape->top_left, fraction);
  out_shape->top_right =
      morph_corner(start_shape->top_right, end_shape->top_right, fraction);
  out_shape->bottom_right = morph_corner(start_shape->bottom_right,
                                         end_shape->bottom_right, fraction);
  out_shape->bottom_left =
      morph_corner(start_shape->bottom_left, end_shape->bottom_left, fraction);

  return CMP_OK;
}

CMP_API int CMP_CALL m3_shape_scale_none(M3Shape *shape) {
  return m3_shape_init_absolute(shape, 0.0f);
}

CMP_API int CMP_CALL m3_shape_scale_extra_small(M3Shape *shape) {
  return m3_shape_init_absolute(shape, 4.0f);
}

CMP_API int CMP_CALL m3_shape_scale_small(M3Shape *shape) {
  return m3_shape_init_absolute(shape, 8.0f);
}

CMP_API int CMP_CALL m3_shape_scale_medium(M3Shape *shape) {
  return m3_shape_init_absolute(shape, 12.0f);
}

CMP_API int CMP_CALL m3_shape_scale_large(M3Shape *shape) {
  return m3_shape_init_absolute(shape, 16.0f);
}

CMP_API int CMP_CALL m3_shape_scale_extra_large(M3Shape *shape) {
  return m3_shape_init_absolute(shape, 28.0f);
}

CMP_API int CMP_CALL m3_shape_scale_full(M3Shape *shape) {
  return m3_shape_init_percent(shape, 0.5f);
}