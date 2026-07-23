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
  double x;
  double y;
  double z;
  double w;
};

/**
 * @brief Represents a standardized DOMRect (Geometry Interfaces Level 1).
 */
struct ui_dom_rect {
  double x;
  double y;
  double width;
  double height;
  double top;
  double right;
  double bottom;
  double left;
};

/**
 * @brief Represents a standardized DOMQuad (Geometry Interfaces Level 1).
 */
struct ui_dom_quad {
  struct ui_dom_point p1;
  struct ui_dom_point p2;
  struct ui_dom_point p3;
  struct ui_dom_point p4;
  struct ui_dom_rect bounds;
};

/**
 * @brief Represents a standardized DOMMatrix (Geometry Interfaces Level 1).
 */
struct ui_dom_matrix {
  double m11, m12, m13, m14;
  double m21, m22, m23, m24;
  double m31, m32, m33, m34;
  double m41, m42, m43, m44;
};

/**
 * @brief Initializes a DOMPoint.
 *
 * @param point The point to initialize.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param z The z coordinate.
 * @param w The w coordinate.
 */
enum ui_error ui_dom_point_init(struct ui_dom_point *point, double x, double y,
                                double z, double w);

/**
 * @brief Initializes a DOMRect, automatically computing top/right/bottom/left.
 *
 * @param rect The rect to initialize.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param width The width.
 * @param height The height.
 */
enum ui_error ui_dom_rect_init(struct ui_dom_rect *rect, double x, double y,
                               double width, double height);

/**
 * @brief Initializes a DOMQuad from 4 points.
 *
 * @param quad The quad to initialize.
 * @param p1 Point 1.
 * @param p2 Point 2.
 * @param p3 Point 3.
 * @param p4 Point 4.
 */
enum ui_error ui_dom_quad_init(struct ui_dom_quad *quad,
                               const struct ui_dom_point *p1,
                               const struct ui_dom_point *p2,
                               const struct ui_dom_point *p3,
                               const struct ui_dom_point *p4);

/**
 * @brief Initializes a DOMMatrix to the identity matrix.
 *
 * @param matrix The matrix to initialize.
 */
enum ui_error ui_dom_matrix_init_identity(struct ui_dom_matrix *matrix);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_GEOMETRY_H */
