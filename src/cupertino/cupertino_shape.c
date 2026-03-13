/* clang-format off */
#include "cupertino/cupertino_shape.h"
/* clang-format on */

/* Squircle bezier control point factor.
   Standard circular is ~0.4477 (i.e., 1.0 - 0.5523).
   Apple continuous is flatter and extends further, approximated with 0.8. */
#define CUPERTINO_SQUIRCLE_K 0.8f

CMP_API int CMP_CALL cupertino_shape_append_squircle(
    CMPPath *path, CMPRect bounds, CMPScalar tl_radius, CMPScalar tr_radius,
    CMPScalar br_radius, CMPScalar bl_radius) {
  CMPScalar w;
  CMPScalar h;
  CMPScalar tl;
  CMPScalar tr;
  CMPScalar br;
  CMPScalar bl;
  int rc;

  if (!path) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  w = bounds.width;
  h = bounds.height;

  /* Clamp radii to half width/height */
  tl = tl_radius > w / 2.0f ? w / 2.0f : tl_radius;
  tl = tl > h / 2.0f ? h / 2.0f : tl;

  tr = tr_radius > w / 2.0f ? w / 2.0f : tr_radius;
  tr = tr > h / 2.0f ? h / 2.0f : tr;

  br = br_radius > w / 2.0f ? w / 2.0f : br_radius;
  br = br > h / 2.0f ? h / 2.0f : br;

  bl = bl_radius > w / 2.0f ? w / 2.0f : bl_radius;
  bl = bl > h / 2.0f ? h / 2.0f : bl;

  /* Start at top-left, after the curve */
  rc = cmp_path_move_to(path, bounds.x + tl, bounds.y);
  if (rc != CMP_OK)
    return rc;

  /* Top edge to top-right corner */
  rc = cmp_path_line_to(path, bounds.x + w - tr, bounds.y);
  if (rc != CMP_OK)
    return rc;

  if (tr > 0.0f) {
    rc = cmp_path_cubic_to(
        path, bounds.x + w - tr * (1.0f - CUPERTINO_SQUIRCLE_K), bounds.y,
        bounds.x + w, bounds.y + tr * (1.0f - CUPERTINO_SQUIRCLE_K),
        bounds.x + w, bounds.y + tr);
    if (rc != CMP_OK)
      return rc;
  }

  /* Right edge to bottom-right corner */
  rc = cmp_path_line_to(path, bounds.x + w, bounds.y + h - br);
  if (rc != CMP_OK)
    return rc;

  if (br > 0.0f) {
    rc = cmp_path_cubic_to(path, bounds.x + w,
                           bounds.y + h - br * (1.0f - CUPERTINO_SQUIRCLE_K),
                           bounds.x + w - br * (1.0f - CUPERTINO_SQUIRCLE_K),
                           bounds.y + h, bounds.x + w - br, bounds.y + h);
    if (rc != CMP_OK)
      return rc;
  }

  /* Bottom edge to bottom-left corner */
  rc = cmp_path_line_to(path, bounds.x + bl, bounds.y + h);
  if (rc != CMP_OK)
    return rc;

  if (bl > 0.0f) {
    rc = cmp_path_cubic_to(path, bounds.x + bl * (1.0f - CUPERTINO_SQUIRCLE_K),
                           bounds.y + h, bounds.x,
                           bounds.y + h - bl * (1.0f - CUPERTINO_SQUIRCLE_K),
                           bounds.x, bounds.y + h - bl);
    if (rc != CMP_OK)
      return rc;
  }

  /* Left edge to top-left corner */
  rc = cmp_path_line_to(path, bounds.x, bounds.y + tl);
  if (rc != CMP_OK)
    return rc;

  if (tl > 0.0f) {
    rc = cmp_path_cubic_to(path, bounds.x,
                           bounds.y + tl * (1.0f - CUPERTINO_SQUIRCLE_K),
                           bounds.x + tl * (1.0f - CUPERTINO_SQUIRCLE_K),
                           bounds.y, bounds.x + tl, bounds.y);
    if (rc != CMP_OK)
      return rc;
  }

  /* Close path */
  rc = cmp_path_close(path);
  if (rc != CMP_OK)
    return rc;

  return CMP_OK;
}
