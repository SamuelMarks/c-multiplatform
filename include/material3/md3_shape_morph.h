/**
 * @file md3_shape_morph.h
 * @brief Dynamic shape morphing and expressive polygon definitions for
 * Material 3.
 */

#ifndef MATERIAL3_MD3_SHAPE_MORPH_H
#define MATERIAL3_MD3_SHAPE_MORPH_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def MD3_SHAPE_MAX_VERTICES
 * @brief Maximum number of polygon vertices supported for dynamic morphing.
 */
#define MD3_SHAPE_MAX_VERTICES 64

/**
 * @enum md3_expressive_shape_type
 * @brief Distinct geometric shapes in the Material 3 Expressive shape library.
 */
enum md3_expressive_shape_type {
  MD3_EXPRESSIVE_SHAPE_RECTANGLE = 0,
  MD3_EXPRESSIVE_SHAPE_CIRCLE,
  MD3_EXPRESSIVE_SHAPE_PILL,
  MD3_EXPRESSIVE_SHAPE_OVAL,
  MD3_EXPRESSIVE_SHAPE_ARCH,
  MD3_EXPRESSIVE_SHAPE_CLOVER_4,
  MD3_EXPRESSIVE_SHAPE_CLOVER_8,
  MD3_EXPRESSIVE_SHAPE_SCALLOP_6,
  MD3_EXPRESSIVE_SHAPE_SCALLOP_12,
  MD3_EXPRESSIVE_SHAPE_STAR_4,
  MD3_EXPRESSIVE_SHAPE_STAR_8,
  MD3_EXPRESSIVE_SHAPE_STAR_12,
  MD3_EXPRESSIVE_SHAPE_BURST,
  MD3_EXPRESSIVE_SHAPE_FLOWER_4,
  MD3_EXPRESSIVE_SHAPE_FLOWER_8,
  MD3_EXPRESSIVE_SHAPE_CUT_CORNER_RECTANGLE,
  MD3_EXPRESSIVE_SHAPE_CUT_CORNER_TOP_RIGHT,
  MD3_EXPRESSIVE_SHAPE_ASYMMETRIC_ROUNDED_1,
  MD3_EXPRESSIVE_SHAPE_ASYMMETRIC_ROUNDED_2,
  MD3_EXPRESSIVE_SHAPE_SLANTED,
  MD3_EXPRESSIVE_SHAPE_OCTAGON,
  MD3_EXPRESSIVE_SHAPE_HEXAGON,
  MD3_EXPRESSIVE_SHAPE_PENTAGON,
  MD3_EXPRESSIVE_SHAPE_DIAMOND,
  MD3_EXPRESSIVE_SHAPE_HEART,
  MD3_EXPRESSIVE_SHAPE_DROP,
  MD3_EXPRESSIVE_SHAPE_BUNNY_EARS,
  MD3_EXPRESSIVE_SHAPE_SUNNY,
  MD3_EXPRESSIVE_SHAPE_COOKIE,
  MD3_EXPRESSIVE_SHAPE_PUFF,
  MD3_EXPRESSIVE_SHAPE_TRIANGLE,
  MD3_EXPRESSIVE_SHAPE_WAVE,
  MD3_EXPRESSIVE_SHAPE_CLOUD,
  MD3_EXPRESSIVE_SHAPE_GEM,
  MD3_EXPRESSIVE_SHAPE_SHIELD,
  MD3_EXPRESSIVE_SHAPE_TYPE_COUNT
};

/**
 * @struct md3_shape_vertex
 * @brief 2D normalized vertex coordinate [-1.0, 1.0].
 */
struct md3_shape_vertex {
  float x; /**< Normalized X coordinate */
  float y; /**< Normalized Y coordinate */
};

/**
 * @struct md3_shape
 * @brief Definition of a polygonal shape for morphing.
 */
struct md3_shape {
  enum md3_expressive_shape_type type;
  size_t vertex_count;
  struct md3_shape_vertex vertices[MD3_SHAPE_MAX_VERTICES];
};

/**
 * @brief Generates polygon vertices for a given expressive shape type.
 *
 * @param type The expressive shape type.
 * @param vertex_count Number of sample vertices to generate (minimum 4, maximum
 * MD3_SHAPE_MAX_VERTICES).
 * @param out_shape Pointer to destination shape struct.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_shape_create_expressive(enum md3_expressive_shape_type type,
                            size_t vertex_count, struct md3_shape *out_shape);

/**
 * @brief Interpolates smoothly between two shapes at normalized progress
 * [0.0, 1.0].
 *
 * @param from Source shape.
 * @param to Target shape.
 * @param progress Morph progress in range [0.0, 1.0].
 * @param out_shape Destination shape struct receiving interpolated coordinates.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_shape_morph_interpolate(
    const struct md3_shape *from, const struct md3_shape *to, float progress,
    struct md3_shape *out_shape);

/**
 * @brief Generates an SVG path string from a shape scaled to specified width
 * and height.
 *
 * @param shape The shape definition.
 * @param width Target width in dp/px.
 * @param height Target height in dp/px.
 * @param out_buffer Destination string buffer.
 * @param buffer_capacity Capacity of destination buffer in bytes.
 * @return UI_ERROR_NONE on success, UI_ERROR_BUFFER_TOO_SMALL, or
 * UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_shape_to_svg_path(const struct md3_shape *shape, float width, float height,
                      char *out_buffer, size_t buffer_capacity);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_SHAPE_MORPH_H */
