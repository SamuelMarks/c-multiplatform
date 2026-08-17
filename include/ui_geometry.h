/**
 * @file ui_geometry.h
 * @brief Standardized geometry primitives.
 *
 * This header defines DOM-compliant geometry primitives (DOMPoint, DOMRect,
 * DOMQuad, and DOMMatrix) and functions to initialize them.
 */

#ifndef UI_GEOMETRY_H
#define UI_GEOMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Represents a standardized DOMPoint (Geometry Interfaces Level 1).
 */
struct ui_dom_point {
  double x; /**< The x coordinate. */
  double y; /**< The y coordinate. */
  double z; /**< The z coordinate. */
  double w; /**< The w coordinate. */
};

/**
 * @brief Represents a standardized DOMRect (Geometry Interfaces Level 1).
 */
struct ui_dom_rect {
  double x;      /**< The x coordinate. */
  double y;      /**< The y coordinate. */
  double width;  /**< The width. */
  double height; /**< The height. */
  double top;    /**< The top edge. */
  double right;  /**< The right edge. */
  double bottom; /**< The bottom edge. */
  double left;   /**< The left edge. */
};

/**
 * @brief Represents a standardized DOMQuad (Geometry Interfaces Level 1).
 */
struct ui_dom_quad {
  struct ui_dom_point p1;    /**< First point. */
  struct ui_dom_point p2;    /**< Second point. */
  struct ui_dom_point p3;    /**< Third point. */
  struct ui_dom_point p4;    /**< Fourth point. */
  struct ui_dom_rect bounds; /**< Bounding rectangle for the quad. */
};

/**
 * @brief Represents a standardized DOMMatrix (Geometry Interfaces Level 1).
 */
struct ui_dom_matrix {
  double m11; /**< Row 1, Column 1. */
  double m12; /**< Row 1, Column 2. */
  double m13; /**< Row 1, Column 3. */
  double m14; /**< Row 1, Column 4. */
  double m21; /**< Row 2, Column 1. */
  double m22; /**< Row 2, Column 2. */
  double m23; /**< Row 2, Column 3. */
  double m24; /**< Row 2, Column 4. */
  double m31; /**< Row 3, Column 1. */
  double m32; /**< Row 3, Column 2. */
  double m33; /**< Row 3, Column 3. */
  double m34; /**< Row 3, Column 4. */
  double m41; /**< Row 4, Column 1. */
  double m42; /**< Row 4, Column 2. */
  double m43; /**< Row 4, Column 3. */
  double m44; /**< Row 4, Column 4. */
};

/**
 * @brief Initializes a DOMPoint.
 *
 * @param point Pointer to the point to initialize.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param z The z coordinate.
 * @param w The w coordinate.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_point_init(struct ui_dom_point *point, double x, double y,
                             double z, double w);

/**
 * @brief Initializes a DOMRect, automatically computing top/right/bottom/left.
 *
 * @param rect Pointer to the rect to initialize.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param width The width.
 * @param height The height.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_rect_init(struct ui_dom_rect *rect, double x, double y,
                            double width, double height);

/**
 * @brief Initializes a DOMQuad from 4 points.
 *
 * @param quad Pointer to the quad to initialize.
 * @param p1 Pointer to the first point.
 * @param p2 Pointer to the second point.
 * @param p3 Pointer to the third point.
 * @param p4 Pointer to the fourth point.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_quad_init(struct ui_dom_quad *quad,
                            const struct ui_dom_point *p1,
                            const struct ui_dom_point *p2,
                            const struct ui_dom_point *p3,
                            const struct ui_dom_point *p4);

/**
 * @brief Initializes a DOMMatrix to the identity matrix.
 *
 * @param matrix Pointer to the matrix to initialize.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_matrix_init_identity(struct ui_dom_matrix *matrix);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_GEOMETRY_H */
