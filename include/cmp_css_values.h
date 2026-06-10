/* clang-format off */
#ifndef CMP_CSS_VALUES_H
#define CMP_CSS_VALUES_H

#include <stddef.h>

/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_values.h
 * @brief CSS Values and Units implementation.
 */

/**
 * @brief Represents a generic CSS number or integer.
 */
typedef struct cmp_val_num {
  float value;    /**< The numeric value */
  int is_integer; /**< 1 if it must be treated as an integer, 0 otherwise */
} cmp_val_num_t;

/**
 * @brief Represents a CSS percentage.
 */
typedef struct cmp_val_percent {
  float value; /**< The percentage value (e.g. 50.0 for 50%) */
} cmp_val_percent_t;

/**
 * @brief Represents a CSS ratio (e.g., 16/9).
 */
typedef struct cmp_val_ratio {
  float numerator;   /**< The top part of the ratio */
  float denominator; /**< The bottom part of the ratio */
} cmp_val_ratio_t;

/**
 * @brief Represents a CSS flex value (fr).
 */
typedef struct cmp_val_flex {
  float value; /**< The flex value (e.g. 1.0 for 1fr) */
} cmp_val_flex_t;

/**
 * @brief Represents a CSS string.
 */
typedef struct cmp_val_string {
  const char *value; /**< Pointer to the string value */
  size_t length;     /**< Length of the string */
} cmp_val_string_t;

/**
 * @brief Represents a CSS custom-ident.
 */
typedef struct cmp_val_custom_ident {
  const char *value; /**< Pointer to the ident value */
  size_t length;     /**< Length of the ident */
} cmp_val_custom_ident_t;

/**
 * @brief Represents a CSS url().
 */
typedef struct cmp_val_url {
  const char *url; /**< Pointer to the url string */
  size_t length;   /**< Length of the url */
} cmp_val_url_t;

/**
 * @brief Represents CSS physical length units.
 */
typedef enum cmp_length_phys_unit {
  CMP_LENGTH_PX,
  CMP_LENGTH_IN,
  CMP_LENGTH_CM,
  CMP_LENGTH_MM,
  CMP_LENGTH_PT,
  CMP_LENGTH_PC
} cmp_length_phys_unit_t;

/**
 * @brief Represents a CSS absolute length.
 */
typedef struct cmp_length_phys {
  float value;                 /**< Numeric value */
  cmp_length_phys_unit_t unit; /**< Unit type */
} cmp_length_phys_t;

/**
 * @brief Represents CSS relative length units.
 */
typedef enum cmp_length_rel_unit {
  CMP_LENGTH_EM,
  CMP_LENGTH_REM,
  CMP_LENGTH_EX,
  CMP_LENGTH_CH
} cmp_length_rel_unit_t;

/**
 * @brief Represents a CSS relative length.
 */
typedef struct cmp_length_rel {
  float value;                /**< Numeric value */
  cmp_length_rel_unit_t unit; /**< Unit type */
} cmp_length_rel_t;

/**
 * @brief Represents CSS Level 4 relative length units.
 */
typedef enum cmp_length_rel_l4_unit {
  CMP_LENGTH_CAP,
  CMP_LENGTH_RCAP,
  CMP_LENGTH_IC,
  CMP_LENGTH_RIC,
  CMP_LENGTH_LH,
  CMP_LENGTH_RLH,
  CMP_LENGTH_REX,
  CMP_LENGTH_RCH
} cmp_length_rel_l4_unit_t;

/**
 * @brief Represents a CSS Level 4 relative length.
 */
typedef struct cmp_length_rel_l4 {
  float value;                   /**< Numeric value */
  cmp_length_rel_l4_unit_t unit; /**< Unit type */
} cmp_length_rel_l4_t;

/**
 * @brief Represents CSS viewport length units.
 */
typedef enum cmp_length_vw_unit {
  CMP_LENGTH_VW,
  CMP_LENGTH_VH,
  CMP_LENGTH_VMIN,
  CMP_LENGTH_VMAX
} cmp_length_vw_unit_t;

/**
 * @brief Represents a CSS viewport length.
 */
typedef struct cmp_length_vw {
  float value;               /**< Numeric value */
  cmp_length_vw_unit_t unit; /**< Unit type */
} cmp_length_vw_t;

/**
 * @brief Represents CSS Level 4 viewport length units.
 */
typedef enum cmp_length_viewport_l4_unit {
  CMP_LENGTH_DVH,
  CMP_LENGTH_DVW,
  CMP_LENGTH_LVH,
  CMP_LENGTH_LVW,
  CMP_LENGTH_SVH,
  CMP_LENGTH_SVW,
  CMP_LENGTH_VI,
  CMP_LENGTH_VB,
  CMP_LENGTH_DVI,
  CMP_LENGTH_DVB,
  CMP_LENGTH_LVI,
  CMP_LENGTH_LVB,
  CMP_LENGTH_SVI,
  CMP_LENGTH_SVB
} cmp_length_viewport_l4_unit_t;

/**
 * @brief Represents a CSS Level 4 viewport length.
 */
typedef struct cmp_length_viewport_l4 {
  float value;                        /**< Numeric value */
  cmp_length_viewport_l4_unit_t unit; /**< Unit type */
} cmp_length_viewport_l4_t;

/**
 * @brief Represents CSS container query length units.
 */
typedef enum cmp_length_cq_unit {
  CMP_LENGTH_CQW,
  CMP_LENGTH_CQH,
  CMP_LENGTH_CQI,
  CMP_LENGTH_CQB,
  CMP_LENGTH_CQMIN,
  CMP_LENGTH_CQMAX
} cmp_length_cq_unit_t;

/**
 * @brief Represents a CSS container query length.
 */
typedef struct cmp_length_cq {
  float value;               /**< Numeric value */
  cmp_length_cq_unit_t unit; /**< Unit type */
} cmp_length_cq_t;

/**
 * @brief Parses a number or integer.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_val_num_parse(const char *str, cmp_val_num_t *out_val);

/**
 * @brief Parses a percentage.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_val_percent_parse(const char *str, cmp_val_percent_t *out_val);

/**
 * @brief Parses a ratio.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_val_ratio_parse(const char *str, cmp_val_ratio_t *out_val);

/**
 * @brief Parses a flex value.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_val_flex_parse(const char *str, cmp_val_flex_t *out_val);

/**
 * @brief Parses a string.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_val_string_parse(const char *str, cmp_val_string_t *out_val);

/**
 * @brief Parses a custom ident.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_val_custom_ident_parse(const char *str,
                               cmp_val_custom_ident_t *out_val);

/**
 * @brief Parses a URL.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_val_url_parse(const char *str, cmp_val_url_t *out_val);

/**
 * @brief Parses a physical length.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_length_phys_parse(const char *str, cmp_length_phys_t *out_val);

/**
 * @brief Parses a relative length.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_length_rel_parse(const char *str, cmp_length_rel_t *out_val);

/**
 * @brief Parses a Level 4 relative length.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_length_rel_l4_parse(const char *str, cmp_length_rel_l4_t *out_val);

/**
 * @brief Parses a viewport length.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_length_vw_parse(const char *str, cmp_length_vw_t *out_val);

/**
 * @brief Parses a Level 4 viewport length.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_length_viewport_l4_parse(const char *str,
                                 cmp_length_viewport_l4_t *out_val);

/**
 * @brief Parses a container query length.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_length_cq_parse(const char *str, cmp_length_cq_t *out_val);

/**
 * @brief Represents CSS angle units.
 */
typedef enum cmp_angle_unit {
  CMP_ANGLE_DEG,
  CMP_ANGLE_GRAD,
  CMP_ANGLE_RAD,
  CMP_ANGLE_TURN
} cmp_angle_unit_t;

/**
 * @brief Represents a CSS angle.
 */
typedef struct cmp_angle {
  float value;           /**< Numeric value */
  cmp_angle_unit_t unit; /**< Unit type */
} cmp_angle_t;

/**
 * @brief Represents CSS time units.
 */
typedef enum cmp_time_unit { CMP_TIME_S, CMP_TIME_MS } cmp_time_unit_t;

/**
 * @brief Represents a CSS time value.
 */
typedef struct cmp_time {
  float value;          /**< Numeric value */
  cmp_time_unit_t unit; /**< Unit type */
} cmp_time_t;

/**
 * @brief Represents CSS resolution units.
 */
typedef enum cmp_resolution_unit {
  CMP_RESOLUTION_DPI,
  CMP_RESOLUTION_DPCM,
  CMP_RESOLUTION_DPPX,
  CMP_RESOLUTION_X
} cmp_resolution_unit_t;

/**
 * @brief Represents a CSS resolution.
 */
typedef struct cmp_resolution {
  float value;                /**< Numeric value */
  cmp_resolution_unit_t unit; /**< Unit type */
} cmp_resolution_t;

/**
 * @brief Represents CSS frequency units.
 */
typedef enum cmp_frequency_unit {
  CMP_FREQUENCY_HZ,
  CMP_FREQUENCY_KHZ
} cmp_frequency_unit_t;

/**
 * @brief Represents a CSS frequency.
 */
typedef struct cmp_frequency {
  float value;               /**< Numeric value */
  cmp_frequency_unit_t unit; /**< Unit type */
} cmp_frequency_t;

/**
 * @brief Parses an angle.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_angle_parse(const char *str, cmp_angle_t *out_val);

/**
 * @brief Parses a time value.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_time_parse(const char *str, cmp_time_t *out_val);

/**
 * @brief Parses a resolution.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_resolution_parse(const char *str, cmp_resolution_t *out_val);

/**
 * @brief Parses a frequency.
 * @param str Input string.
 * @param out_val Parsed value.
 * @return 0 on success.
 */
int cmp_frequency_parse(const char *str, cmp_frequency_t *out_val);

/**
 * @brief Represents a generic calc() expression tree node.
 */
typedef struct cmp_calc {
  const char *expression;
} cmp_calc_t;

/**
 * @brief Represents min(), max(), clamp().
 */
typedef struct cmp_math_bounds {
  const char *expression;
} cmp_math_bounds_t;

/**
 * @brief Represents Trigonometry: sin(), cos(), tan(), asin(), acos(), atan(),
 * atan2().
 */
typedef struct cmp_math_trig {
  const char *expression;
} cmp_math_trig_t;

/**
 * @brief Represents Exponents: pow(), sqrt(), hypot(), log(), exp().
 */
typedef struct cmp_math_exp {
  const char *expression;
} cmp_math_exp_t;

/**
 * @brief Represents Rounding/Sign: round(), mod(), rem(), abs(), sign().
 */
typedef struct cmp_math_round {
  const char *expression;
} cmp_math_round_t;

/**
 * @brief Represents Environment variables env(safe-area-inset-*), etc.
 */
typedef struct cmp_env_var {
  const char *name;
} cmp_env_var_t;

/**
 * @brief Represents Environment env(keyboard-inset-*).
 */
typedef struct cmp_env_keyboard {
  const char *name;
} cmp_env_keyboard_t;

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_VALUES_H */