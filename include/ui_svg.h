/**
 * @file ui_svg.h
 * @brief SVG parsing and tessellation functionalities.
 *
 * @defgroup Svg SVG Processing
 * @brief Parser and tessellator for SVG path data.
 * @{
 */

#ifndef UI_SVG_H
#define UI_SVG_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum ui_svg_command_type
 * @brief Represents the types of commands in an SVG path.
 */
enum ui_svg_command_type {
  /** @brief Move to command (M/m). */
  UI_SVG_CMD_MOVE_TO,
  /** @brief Line to command (L/l). */
  UI_SVG_CMD_LINE_TO,
  /** @brief Cubic Bezier curve command (C/c). */
  UI_SVG_CMD_CUBIC_BEZIER,
  /** @brief Quadratic Bezier curve command (Q/q). */
  UI_SVG_CMD_QUADRATIC_BEZIER,
  /** @brief Elliptical arc command (A/a). */
  UI_SVG_CMD_ARC,
  /** @brief Close path command (Z/z). */
  UI_SVG_CMD_CLOSE_PATH
};

/**
 * @struct ui_svg_point
 * @brief A 2D point used in SVG commands.
 */
struct ui_svg_point {
  /** @brief X coordinate. */
  float x;
  /** @brief Y coordinate. */
  float y;
};

/**
 * @struct ui_svg_command
 * @brief Represents a single parsed SVG path command.
 */
struct ui_svg_command {
  /** @brief Type of the SVG command. */
  enum ui_svg_command_type type;
  /** @brief Union containing the command specific data. */
  union {
    /** @brief Data for move_to. */
    struct {
      /** @brief Destination point. */
      struct ui_svg_point p;
    } move_to; /**< Data for move_to. */
    /** @brief Data for line_to. */
    struct {
      /** @brief Destination point. */
      struct ui_svg_point p;
    } line_to; /**< Data for line_to. */
    /** @brief Data for cubic_bezier. */
    struct {
      /** @brief Control point 1. */
      struct ui_svg_point cp1;
      /** @brief Control point 2. */
      struct ui_svg_point cp2;
      /** @brief Destination point. */
      struct ui_svg_point p;
    } cubic_bezier; /**< Data for cubic bezier. */
    /** @brief Data for quadratic_bezier. */
    struct {
      /** @brief Control point. */
      struct ui_svg_point cp;
      /** @brief Destination point. */
      struct ui_svg_point p;
    } quadratic_bezier; /**< Data for quadratic bezier. */
    /** @brief Data for arc. */
    struct {
      /** @brief X radius. */
      float rx;
      /** @brief Y radius. */
      float ry;
      /** @brief X axis rotation. */
      float x_axis_rotation;
      /** @brief Large arc flag (1 or 0). */
      ui_uint8 large_arc_flag;
      /** @brief Sweep flag (1 or 0). */
      ui_uint8 sweep_flag;
      /** @brief Destination point. */
      struct ui_svg_point p;
    } arc; /**< Data for arc. */
  } data;  /**< The union data. */
};

/**
 * @struct ui_svg_path
 * @brief Holds an array of SVG commands parsed from a path attribute.
 */
struct ui_svg_path {
  /** @brief Array of parsed SVG commands. */
  struct ui_svg_command *commands;
  /** @brief Number of active commands. */
  ui_uint32 count;
  /** @brief Total allocated capacity for commands. */
  ui_uint32 capacity;
};

/**
 * @struct ui_svg_subpath
 * @brief A flattened subpath consisting only of line segments.
 */
struct ui_svg_subpath {
  /** @brief Array of points forming the subpath. */
  struct ui_svg_point *points;
  /** @brief Number of points in the subpath. */
  ui_uint32 count;
  /** @brief Total allocated capacity for points. */
  ui_uint32 capacity;
  /** @brief Non-zero if the subpath is closed. */
  ui_uint8 closed;
};

/**
 * @struct ui_svg_flattened_path
 * @brief A flattened path, consisting of multiple subpaths.
 */
struct ui_svg_flattened_path {
  /** @brief Array of subpaths. */
  struct ui_svg_subpath *subpaths;
  /** @brief Number of subpaths. */
  ui_uint32 count;
  /** @brief Total allocated capacity for subpaths. */
  ui_uint32 capacity;
};

/**
 * @struct ui_svg_geometry
 * @brief Triangulated geometry for rendering.
 */
struct ui_svg_geometry {
  /** @brief Array of vertices. */
  struct ui_svg_point *vertices;
  /** @brief Number of vertices. */
  ui_uint32 vertex_count;
  /** @brief Capacity of the vertices array. */
  ui_uint32 vertex_capacity;
  /** @brief Array of indices. */
  ui_uint32 *indices;
  /** @brief Number of indices. */
  ui_uint32 index_count;
  /** @brief Capacity of the indices array. */
  ui_uint32 index_capacity;
};

/**
 * @brief Initialize a new, empty SVG path.
 *
 * @param path Pointer to the path to initialize.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_svg_path_init(struct ui_svg_path *path);

/**
 * @brief Destroy an SVG path and free its resources.
 *
 * @param path Pointer to the path to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_svg_path_destroy(struct ui_svg_path *path);

/**
 * @brief Parse an SVG path 'd' attribute string into a list of commands.
 *
 * @param path Pointer to the initialized path.
 * @param d_attr The 'd' attribute string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_svg_path_parse(struct ui_svg_path *path, const char *d_attr);

/**
 * @brief Initialize a new, empty flattened path.
 *
 * @param path Pointer to the flattened path to initialize.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_svg_flattened_path_init(struct ui_svg_flattened_path *path);

/**
 * @brief Destroy a flattened path and free its resources.
 *
 * @param path Pointer to the flattened path to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_svg_flattened_path_destroy(struct ui_svg_flattened_path *path);

/**
 * @brief Flattens an SVG path into line segments.
 *
 * @param flattened Output flattened path.
 * @param path Input parsed SVG path.
 * @param tolerance Maximum allowed error (distance from true curve).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_svg_path_flatten(struct ui_svg_flattened_path *flattened,
                               const struct ui_svg_path *path, float tolerance);

/**
 * @brief Initialize a new geometry structure.
 *
 * @param geom Pointer to the geometry structure to initialize.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_svg_geometry_init(struct ui_svg_geometry *geom);

/**
 * @brief Destroy a geometry structure.
 *
 * @param geom Pointer to the geometry structure to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_svg_geometry_destroy(struct ui_svg_geometry *geom);

/**
 * @brief Tessellates a flattened path for filling (using ear-clipping).
 *
 * @param geom Pointer to the geometry structure to populate.
 * @param flattened Pointer to the flattened path.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_svg_tessellate_fill(struct ui_svg_geometry *geom,
                       const struct ui_svg_flattened_path *flattened);

/**
 * @brief Tessellates a flattened path for stroking.
 *
 * @param geom Pointer to the geometry structure to populate.
 * @param flattened Pointer to the flattened path.
 * @param stroke_width The width of the stroke.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_svg_tessellate_stroke(struct ui_svg_geometry *geom,
                         const struct ui_svg_flattened_path *flattened,
                         float stroke_width);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SVG_H */

/** @} */
