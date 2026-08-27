/**
 * @file ui_css_motion.h
 */
/**
 * \file ui_css_motion.h
 * \brief CSS Motion Path properties definitions and parsing.
 * \author UI Framework Team
 * \date 2026
 */

#ifndef UI_CSS_MOTION_H
#define UI_CSS_MOTION_H

/**
 * \defgroup ui_css_motion CSS Motion Path
 * \brief Types and functions for CSS Motion Path properties (offset-path,
 * etc.).
 * \{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * \brief CSS offset-path type.
 */
enum ui_css_offset_path_type {
  UI_CSS_OFFSET_PATH_NONE,        /**< No offset path. */
  UI_CSS_OFFSET_PATH_RAY,         /**< Ray path. */
  UI_CSS_OFFSET_PATH_BASIC_SHAPE, /**< Basic shape path. */
  UI_CSS_OFFSET_PATH_URL,         /**< URL path. */
  UI_CSS_OFFSET_PATH_BOX          /**< Box path. */
};

/**
 * \brief CSS ray() function for offset-path.
 */
struct ui_css_ray {
  struct ui_css_value angle; /**< Angle of the ray. */
  int has_size;              /**< Non-zero if size is defined. */
  /** \brief ui_css_geometry_box */
  enum ui_css_geometry_box
      size_box;   /**< Closest-side, farthest-side, etc. maps to geometry box
                     generally, or specific enum if needed */
  int is_contain; /**< Non-zero if 'contain' is set. */
};

/**
 * \brief CSS offset-path property.
 */
struct ui_css_offset_path {
  enum ui_css_offset_path_type type; /**< Type of the offset path. */
  struct ui_css_ray
      ray; /**< Ray structure (used if type == UI_CSS_OFFSET_PATH_RAY). */
  struct ui_css_basic_shape shape; /**< Shape structure (used if type ==
                                      UI_CSS_OFFSET_PATH_BASIC_SHAPE). */
  enum ui_css_geometry_box box;    /**< Geometry box. */
  char url[256]; /**< URL string (used if type == UI_CSS_OFFSET_PATH_URL). */
};

/**
 * \brief CSS offset-position property.
 */
struct ui_css_offset_position {
  int is_auto;           /**< Non-zero if auto. */
  struct ui_css_value x; /**< X coordinate. */
  struct ui_css_value y; /**< Y coordinate. */
};

/**
 * \brief CSS offset-anchor property.
 */
struct ui_css_offset_anchor {
  int is_auto;           /**< Non-zero if auto. */
  struct ui_css_value x; /**< X coordinate. */
  struct ui_css_value y; /**< Y coordinate. */
};

/**
 * \brief CSS offset-rotate property.
 */
struct ui_css_offset_rotate {
  int is_auto;               /**< Non-zero if auto. */
  int is_reverse;            /**< Non-zero if reverse is set. */
  struct ui_css_value angle; /**< Rotation angle. */
};

/**
 * \brief Parses the offset-path CSS property.
 *
 * \param str The string to parse.
 * \param out_path Pointer to receive the parsed offset path.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_offset_path(const char *str,
                                    struct ui_css_offset_path *out_path);

/**
 * \brief Parses the offset-distance CSS property.
 *
 * \param str The string to parse.
 * \param out_distance Pointer to receive the parsed offset distance.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_offset_distance(const char *str,
                                        struct ui_css_value *out_distance);

/**
 * \brief Parses the offset-position CSS property.
 *
 * \param str The string to parse.
 * \param out_position Pointer to receive the parsed offset position.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_css_parse_offset_position(const char *str,
                             struct ui_css_offset_position *out_position);

/**
 * \brief Parses the offset-anchor CSS property.
 *
 * \param str The string to parse.
 * \param out_anchor Pointer to receive the parsed offset anchor.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_offset_anchor(const char *str,
                                      struct ui_css_offset_anchor *out_anchor);

/**
 * \brief Parses the offset-rotate CSS property.
 *
 * \param str The string to parse.
 * \param out_rotate Pointer to receive the parsed offset rotate.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_offset_rotate(const char *str,
                                      struct ui_css_offset_rotate *out_rotate);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** \} */

#endif /* UI_CSS_MOTION_H */
