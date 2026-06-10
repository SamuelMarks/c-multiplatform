/* clang-format off */
#ifndef CMP_CSS_SVG_H
#define CMP_CSS_SVG_H

#include "cmp_css_color.h"
#include "cmp_css_box_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents the fill-rule property.
 */
typedef enum cmp_prop_svg_fill_rule {
  CMP_SVG_FILL_RULE_NONZERO = 0,
  CMP_SVG_FILL_RULE_EVENODD
} cmp_prop_svg_fill_rule_t;

/**
 * @brief Represents SVG fill properties.
 */
typedef struct cmp_prop_svg_fill {
  cmp_prop_color_t fill;             /**< fill color */
  cmp_prop_svg_fill_rule_t rule;     /**< fill-rule */
  cmp_prop_opacity_t opacity;        /**< fill-opacity */
} cmp_prop_svg_fill_t;

/**
 * @brief Represents the stroke-linecap property.
 */
typedef enum cmp_prop_svg_stroke_linecap {
  CMP_SVG_STROKE_LINECAP_BUTT = 0,
  CMP_SVG_STROKE_LINECAP_ROUND,
  CMP_SVG_STROKE_LINECAP_SQUARE
} cmp_prop_svg_stroke_linecap_t;

/**
 * @brief Represents the stroke-linejoin property.
 */
typedef enum cmp_prop_svg_stroke_linejoin {
  CMP_SVG_STROKE_LINEJOIN_MITER = 0,
  CMP_SVG_STROKE_LINEJOIN_ROUND,
  CMP_SVG_STROKE_LINEJOIN_BEVEL
} cmp_prop_svg_stroke_linejoin_t;

/**
 * @brief Represents SVG stroke properties.
 */
typedef struct cmp_prop_svg_stroke {
  cmp_prop_color_t stroke;                         /**< stroke color */
  cmp_prop_size_t width;                           /**< stroke-width */
  cmp_prop_svg_stroke_linecap_t linecap;           /**< stroke-linecap */
  cmp_prop_svg_stroke_linejoin_t linejoin;         /**< stroke-linejoin */
  float miterlimit;                                /**< stroke-miterlimit */
  cmp_prop_size_t dasharray[16];                   /**< stroke-dasharray */
  int dasharray_count;                             /**< number of elements in dasharray */
  cmp_prop_size_t dashoffset;                      /**< stroke-dashoffset */
  cmp_prop_opacity_t opacity;                      /**< stroke-opacity */
} cmp_prop_svg_stroke_t;

/**
 * @brief Represents the vector-effect property.
 */
typedef enum cmp_prop_svg_vector_effect {
  CMP_SVG_VECTOR_EFFECT_NONE = 0,
  CMP_SVG_VECTOR_EFFECT_NON_SCALING_STROKE,
  CMP_SVG_VECTOR_EFFECT_NON_SCALING_SIZE,
  CMP_SVG_VECTOR_EFFECT_NON_ROTATION,
  CMP_SVG_VECTOR_EFFECT_FIXED_POSITION
} cmp_prop_svg_vector_effect_t;

/**
 * @brief Represents SVG geometry properties (cx, cy, r, rx, ry, x, y).
 */
typedef struct cmp_prop_svg_geom {
  cmp_prop_size_t cx; /**< cx */
  cmp_prop_size_t cy; /**< cy */
  cmp_prop_size_t r;  /**< r */
  cmp_prop_size_t rx; /**< rx */
  cmp_prop_size_t ry; /**< ry */
  cmp_prop_size_t x;  /**< x */
  cmp_prop_size_t y;  /**< y */
} cmp_prop_svg_geom_t;

/**
 * @brief Initialize SVG fill property.
 *
 * @param fill The fill property to initialize.
 * @return 0 on success, non-zero on failure.
 */
int cmp_prop_svg_fill_init(cmp_prop_svg_fill_t *fill);

/**
 * @brief Initialize SVG stroke property.
 *
 * @param stroke The stroke property to initialize.
 * @return 0 on success, non-zero on failure.
 */
int cmp_prop_svg_stroke_init(cmp_prop_svg_stroke_t *stroke);

/**
 * @brief Initialize SVG geometry property.
 *
 * @param geom The geometry property to initialize.
 * @return 0 on success, non-zero on failure.
 */
int cmp_prop_svg_geom_init(cmp_prop_svg_geom_t *geom);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_SVG_H */
/* clang-format on */
