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
 * @brief Represents the types of commands in an SVG path.
 */
enum ui_svg_command_type {
  UI_SVG_CMD_MOVE_TO,
  UI_SVG_CMD_LINE_TO,
  UI_SVG_CMD_CUBIC_BEZIER,
  UI_SVG_CMD_QUADRATIC_BEZIER,
  UI_SVG_CMD_ARC,
  UI_SVG_CMD_CLOSE_PATH
};

/**
 * @brief A 2D point used in SVG commands.
 */
struct ui_svg_point {
  float x;
  float y;
};

/**
 * @brief Represents a single parsed SVG path command.
 */
struct ui_svg_command {
  enum ui_svg_command_type type;
  /** \brief union */
  union {
    /** \brief struct */
    struct {
      struct ui_svg_point p; /* Destination */
    } move_to;
    /** \brief struct */
    struct {
      struct ui_svg_point p; /* Destination */
    } line_to;
    /** \brief struct */
    struct {
      struct ui_svg_point cp1; /* Control point 1 */
      struct ui_svg_point cp2; /* Control point 2 */
      struct ui_svg_point p;   /* Destination */
    } cubic_bezier;
    /** \brief struct */
    struct {
      struct ui_svg_point cp; /* Control point */
      struct ui_svg_point p;  /* Destination */
    } quadratic_bezier;
    /** \brief struct */
    struct {
      float rx;
      float ry;
      float x_axis_rotation;
      ui_uint8 large_arc_flag;
      ui_uint8 sweep_flag;
      struct ui_svg_point p; /* Destination */
    } arc;
  } data;
};

/**
 * @brief Holds an array of SVG commands parsed from a path attribute.
 */
struct ui_svg_path {
  struct ui_svg_command *commands;
  ui_uint32 count;
  ui_uint32 capacity;
};

/**
 * @brief A flattened subpath consisting only of line segments.
 */
struct ui_svg_subpath {
  struct ui_svg_point *points;
  ui_uint32 count;
  ui_uint32 capacity;
  ui_uint8 closed;
};

/**
 * @brief A flattened path, consisting of multiple subpaths.
 */
struct ui_svg_flattened_path {
  struct ui_svg_subpath *subpaths;
  ui_uint32 count;
  ui_uint32 capacity;
};

/**
 * @brief Triangulated geometry for rendering.
 */
struct ui_svg_geometry {
  struct ui_svg_point *vertices;
  ui_uint32 vertex_count;
  ui_uint32 vertex_capacity;
  ui_uint32 *indices;
  ui_uint32 index_count;
  ui_uint32 index_capacity;
};

/**
 * @brief Initialize a new, empty SVG path.
 *
 * @param path Pointer to the path to initialize.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if path is NULL.
 */
ui_error_t ui_svg_path_init(struct ui_svg_path *path);

/**
 * @brief Destroy an SVG path and free its resources.
 *
 * @param path Pointer to the path to destroy.
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
 */
ui_error_t ui_svg_flattened_path_init(struct ui_svg_flattened_path *path);

/**
 * @brief Destroy a flattened path and free its resources.
 */
ui_error_t ui_svg_flattened_path_destroy(struct ui_svg_flattened_path *path);

/**
 * @brief Flattens an SVG path into line segments.
 *
 * @param flattened Output flattened path.
 * @param path Input parsed SVG path.
 * @param tolerance Maximum allowed error (distance from true curve).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_svg_path_flatten(struct ui_svg_flattened_path *flattened,
                               const struct ui_svg_path *path, float tolerance);

/**
 * @brief Initialize a new geometry structure.
 */
ui_error_t ui_svg_geometry_init(struct ui_svg_geometry *geom);

/**
 * @brief Destroy a geometry structure.
 */
ui_error_t ui_svg_geometry_destroy(struct ui_svg_geometry *geom);

/**
 * @brief Tessellates a flattened path for filling (using ear-clipping).
 */
ui_error_t
ui_svg_tessellate_fill(struct ui_svg_geometry *geom,
                       const struct ui_svg_flattened_path *flattened);

/**
 * @brief Tessellates a flattened path for stroking.
 */
ui_error_t
ui_svg_tessellate_stroke(struct ui_svg_geometry *geom,
                         const struct ui_svg_flattened_path *flattened,
                         float stroke_width);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SVG_H */
