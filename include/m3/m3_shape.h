#ifndef M3_SHAPE_H
#define M3_SHAPE_H

/**
 * @file m3_shape.h
 * @brief Material 3 Shape system and expressive fluid radii for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_layout.h"

/**
 * @brief Shape corner size specification type.
 */
typedef enum M3ShapeSizeType {
  M3_SHAPE_SIZE_ABSOLUTE = 0, /**< Absolute size in pixels. */
  M3_SHAPE_SIZE_PERCENT = 1 /**< Relative size (0.0 to 1.0) of shortest edge. */
} M3ShapeSizeType;

/**
 * @brief Descriptor for a single corner radius.
 */
typedef struct M3CornerSize {
  M3ShapeSizeType type; /**< Type of the size specification. */
  CMPScalar value;      /**< Value (pixels or percentage). */
} M3CornerSize;

/**
 * @brief Descriptor for a complete shape with 4 corners.
 */
typedef struct M3Shape {
  M3CornerSize top_left;     /**< Top-left corner size. */
  M3CornerSize top_right;    /**< Top-right corner size. */
  M3CornerSize bottom_right; /**< Bottom-right corner size. */
  M3CornerSize bottom_left;  /**< Bottom-left corner size. */
} M3Shape;

/**
 * @brief Initialize a shape with a uniform absolute radius.
 * @param shape Shape to initialize.
 * @param radius_px Absolute radius in pixels.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_shape_init_absolute(M3Shape *shape,
                                            CMPScalar radius_px);

/**
 * @brief Initialize a shape with a uniform percentage radius.
 * @param shape Shape to initialize.
 * @param percent Percentage (0.0 to 1.0). 0.5 makes a full pill/circle.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_shape_init_percent(M3Shape *shape, CMPScalar percent);

/**
 * @brief Initialize an asymmetrical shape with specific corner sizes.
 * @param shape Shape to initialize.
 * @param tl Top-left corner size.
 * @param tr Top-right corner size.
 * @param br Bottom-right corner size.
 * @param bl Bottom-left corner size.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_shape_init_asymmetrical(M3Shape *shape, M3CornerSize tl,
                                                M3CornerSize tr,
                                                M3CornerSize br,
                                                M3CornerSize bl);

/**
 * @brief Resolve a shape's corners to absolute pixel radii based on bounds.
 * @param shape The shape to resolve.
 * @param bounds The layout bounds to resolve against.
 * @param out_tl_px Receives top-left radius in pixels.
 * @param out_tr_px Receives top-right radius in pixels.
 * @param out_br_px Receives bottom-right radius in pixels.
 * @param out_bl_px Receives bottom-left radius in pixels.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_shape_resolve(const M3Shape *shape, CMPRect bounds,
                                      CMPScalar *out_tl_px,
                                      CMPScalar *out_tr_px,
                                      CMPScalar *out_br_px,
                                      CMPScalar *out_bl_px);

/**
 * @brief Interpolate (morph) between two shapes.
 * @param start_shape The starting shape.
 * @param end_shape The ending shape.
 * @param fraction Interpolation fraction (0.0 to 1.0).
 * @param out_shape Receives the interpolated shape.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_shape_morph(const M3Shape *start_shape,
                                    const M3Shape *end_shape,
                                    CMPScalar fraction, M3Shape *out_shape);

/* Predefined Material 3 Shape Scales */

/**
 * @brief Standard shape scale: None (0dp).
 * @param shape The shape structure to update.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_shape_scale_none(M3Shape *shape);

/**
 * @brief Standard shape scale: Extra Small (4dp).
 * @param shape The shape structure to update.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_shape_scale_extra_small(M3Shape *shape);

/**
 * @brief Standard shape scale: Small (8dp).
 * @param shape The shape structure to update.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_shape_scale_small(M3Shape *shape);

/**
 * @brief Standard shape scale: Medium (12dp).
 * @param shape The shape structure to update.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_shape_scale_medium(M3Shape *shape);

/**
 * @brief Standard shape scale: Large (16dp).
 * @param shape The shape structure to update.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_shape_scale_large(M3Shape *shape);

/**
 * @brief Standard shape scale: Extra Large (28dp).
 * @param shape The shape structure to update.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_shape_scale_extra_large(M3Shape *shape);

/**
 * @brief Standard shape scale: Full (fully rounded).
 * @param shape The shape structure to update.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_shape_scale_full(M3Shape *shape);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_SHAPE_H */