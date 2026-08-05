#ifndef UI_CSS_MOTION_H
#define UI_CSS_MOTION_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief CSS offset-path type.
 */
enum ui_css_offset_path_type {
  UI_CSS_OFFSET_PATH_NONE,
  UI_CSS_OFFSET_PATH_RAY,
  UI_CSS_OFFSET_PATH_BASIC_SHAPE,
  UI_CSS_OFFSET_PATH_URL,
  UI_CSS_OFFSET_PATH_BOX
};

/**
 * @brief CSS ray() function for offset-path.
 */
struct ui_css_ray {
  struct ui_css_value angle;
  int has_size;
  /** \brief ui_css_geometry_box */
  enum ui_css_geometry_box
      size_box; /* closest-side, farthest-side, etc. maps to geometry box
                   generally, or specific enum if needed */
  int is_contain;
};

/**
 * @brief CSS offset-path property.
 */
struct ui_css_offset_path {
  enum ui_css_offset_path_type type;
  struct ui_css_ray ray;
  struct ui_css_basic_shape shape;
  enum ui_css_geometry_box box;
  char url[256];
};

/**
 * @brief CSS offset-position property.
 */
struct ui_css_offset_position {
  int is_auto;
  struct ui_css_value x;
  struct ui_css_value y;
};

/**
 * @brief CSS offset-anchor property.
 */
struct ui_css_offset_anchor {
  int is_auto;
  struct ui_css_value x;
  struct ui_css_value y;
};

/**
 * @brief CSS offset-rotate property.
 */
struct ui_css_offset_rotate {
  int is_auto;
  int is_reverse;
  struct ui_css_value angle;
};

/**
 * @brief Parses the offset-path CSS property.
 */
ui_error_t ui_css_parse_offset_path(const char *str,
                                    struct ui_css_offset_path *out_path);

/**
 * @brief Parses the offset-distance CSS property.
 */
ui_error_t ui_css_parse_offset_distance(const char *str,
                                        struct ui_css_value *out_distance);

/**
 * @brief Parses the offset-position CSS property.
 */
ui_error_t
ui_css_parse_offset_position(const char *str,
                             struct ui_css_offset_position *out_position);

/**
 * @brief Parses the offset-anchor CSS property.
 */
ui_error_t ui_css_parse_offset_anchor(const char *str,
                                      struct ui_css_offset_anchor *out_anchor);

/**
 * @brief Parses the offset-rotate CSS property.
 */
ui_error_t ui_css_parse_offset_rotate(const char *str,
                                      struct ui_css_offset_rotate *out_rotate);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_MOTION_H */
