#ifndef CUPERTINO_SHAPE_H
#define CUPERTINO_SHAPE_H

/**
 * @file cupertino_shape.h
 * @brief Apple Cupertino continuous corner curves (squircles) for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_path.h"
/* clang-format on */

/**
 * @brief Appends a continuous corner curve (squircle) rectangle to a path.
 *
 * This function uses cubic bezier approximations to draw Apple-style
 * continuous corners, which eliminate the tangent breaks found in
 * standard circular arcs.
 *
 * @param path The path to append to. Must be initialized.
 * @param bounds The bounding rectangle of the shape.
 * @param tl_radius Top-left corner radius.
 * @param tr_radius Top-right corner radius.
 * @param br_radius Bottom-right corner radius.
 * @param bl_radius Bottom-left corner radius.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cupertino_shape_append_squircle(
    CMPPath *path, CMPRect bounds, CMPScalar tl_radius, CMPScalar tr_radius,
    CMPScalar br_radius, CMPScalar bl_radius);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_SHAPE_H */
