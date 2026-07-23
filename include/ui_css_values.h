#ifndef UI_CSS_VALUES_H
#define UI_CSS_VALUES_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Represents the parsed unit of a CSS numeric value.
 */
enum ui_css_unit {
  UI_CSS_UNIT_NONE = 0,

  /* Absolute lengths */
  UI_CSS_UNIT_PX,
  UI_CSS_UNIT_CM,
  UI_CSS_UNIT_MM,
  UI_CSS_UNIT_IN,
  UI_CSS_UNIT_PT,
  UI_CSS_UNIT_PC,
  UI_CSS_UNIT_Q,

  /* Relative lengths */
  UI_CSS_UNIT_EM,
  UI_CSS_UNIT_EX,
  UI_CSS_UNIT_CH,
  UI_CSS_UNIT_REM,

  /* Viewport-relative lengths (Level 4) */
  UI_CSS_UNIT_VW,
  UI_CSS_UNIT_VH,
  UI_CSS_UNIT_VMIN,
  UI_CSS_UNIT_VMAX,
  UI_CSS_UNIT_VI,
  UI_CSS_UNIT_VB,

  /* Container-relative lengths (Level 5) */
  UI_CSS_UNIT_CQW,
  UI_CSS_UNIT_CQH,
  UI_CSS_UNIT_CQI,
  UI_CSS_UNIT_CQB,
  UI_CSS_UNIT_CQMIN,
  UI_CSS_UNIT_CQMAX,

  /* Percentages */
  UI_CSS_UNIT_PERCENT,

  /* Angles */
  UI_CSS_UNIT_DEG,
  UI_CSS_UNIT_GRAD,
  UI_CSS_UNIT_RAD,
  UI_CSS_UNIT_TURN,

  /* Times */
  UI_CSS_UNIT_S,
  UI_CSS_UNIT_MS,

  /* Frequencies & Speech */
  UI_CSS_UNIT_HZ,
  UI_CSS_UNIT_KHZ,
  UI_CSS_UNIT_DB,
  UI_CSS_UNIT_ST,

  UI_CSS_UNIT_UNKNOWN
};

/**
 * @brief Represents a numeric CSS value with its associated unit.
 */
struct ui_css_value {
  float value;
  enum ui_css_unit unit;
};

/**
 * @brief Represents a CSS math operator.
 */
enum ui_css_math_op {
  UI_CSS_MATH_OP_ADD,
  UI_CSS_MATH_OP_SUB,
  UI_CSS_MATH_OP_MUL,
  UI_CSS_MATH_OP_DIV,
  UI_CSS_MATH_OP_MIN,
  UI_CSS_MATH_OP_MAX,
  UI_CSS_MATH_OP_CLAMP,

  /* Trig functions (Level 5) */
  UI_CSS_MATH_OP_SIN,
  UI_CSS_MATH_OP_COS,
  UI_CSS_MATH_OP_TAN,
  UI_CSS_MATH_OP_ASIN,
  UI_CSS_MATH_OP_ACOS,
  UI_CSS_MATH_OP_ATAN,
  UI_CSS_MATH_OP_ATAN2
};

/**
 * @brief The type of extended CSS value (scalar vs math expression).
 */
enum ui_css_value_type {
  UI_CSS_VALUE_TYPE_SCALAR,
  UI_CSS_VALUE_TYPE_MATH,
  UI_CSS_VALUE_TYPE_ENV
};

struct ui_css_math_expr; /* Forward declaration */

/**
 * @brief Represents an env() variable reference with an optional fallback.
 */
struct ui_css_env_ref {
  char name[64];
  struct ui_css_value_ext *fallback;
};

/**
 * @brief An extended CSS value that can either be a scalar or a math
 * expression.
 */
struct ui_css_value_ext {
  enum ui_css_value_type type;
  /** \brief union */
  union {
    struct ui_css_value scalar;
    struct ui_css_math_expr *math;
    struct ui_css_env_ref *env;
  } value;
};

/**
 * @brief Represents a node in a CSS math expression AST.
 */
struct ui_css_math_expr {
  enum ui_css_math_op op;
  struct ui_css_value_ext *left;
  /** \brief ui_css_value_ext */
  struct ui_css_value_ext
      *right; /* Used for binary operators or as part of a list */
  struct ui_css_value_ext *ext; /* Used for clamp() which takes 3 arguments */
  struct ui_css_math_expr
      *next; /* For variadic functions like min() and max() */
};

/**
 * @brief Parses a string into a simple numeric CSS value and its associated
 * unit.
 *
 * @param str The string to parse (e.g., "10.5px", "50%").
 * @param out_value Pointer to receive the parsed value structure.
 * @return UI_ERROR_NONE on success, UI_ERROR_PARSE_FAILED if the format is
 * invalid.
 */
enum ui_error ui_css_parse_value(const char *str,
                                 struct ui_css_value *out_value);

/**
 * @brief Parses an extended CSS value string, which can include math functions
 * like calc().
 *
 * @param str The string to parse (e.g., "calc(100% - 20px)").
 * @param out_value Pointer to receive the parsed extended value.
 * @return UI_ERROR_NONE on success, UI_ERROR_PARSE_FAILED if the format is
 * invalid.
 */
enum ui_error ui_css_parse_value_ext(const char *str,
                                     struct ui_css_value_ext **out_value);

/**
 * @brief Destroys an extended CSS value, freeing associated math expression
 * memory.
 *
 * @param val The value to destroy.
 */
enum ui_error ui_css_value_ext_destroy(struct ui_css_value_ext *val);

/**
 * @brief Represents color spaces for CSS Color Module Level 3, 4, 5, 6.
 */
enum ui_css_color_space {
  UI_CSS_COLOR_SPACE_SRGB,
  UI_CSS_COLOR_SPACE_HSL,
  UI_CSS_COLOR_SPACE_HWB,
  UI_CSS_COLOR_SPACE_LAB,
  UI_CSS_COLOR_SPACE_LCH,
  UI_CSS_COLOR_SPACE_OKLAB,
  UI_CSS_COLOR_SPACE_OKLCH,
  UI_CSS_COLOR_SPACE_DISPLAY_P3,
  UI_CSS_COLOR_SPACE_A98_RGB,
  UI_CSS_COLOR_SPACE_PROPHOTO_RGB,
  UI_CSS_COLOR_SPACE_REC2020,
  UI_CSS_COLOR_SPACE_SRGB_LINEAR,
  UI_CSS_COLOR_SPACE_XYZ_D50,
  UI_CSS_COLOR_SPACE_XYZ_D65
};

/**
 * @brief Represents a parsed CSS color.
 */
struct ui_css_color {
  enum ui_css_color_space space;
  float components[4]; /* e.g., r, g, b, a or l, a, b, alpha */
};

/**
 * @brief Parses a CSS color string into a color structure.
 *
 * @param str The string to parse (e.g., "red", "#ff0000", "rgb(255, 0, 0)",
 * "color(display-p3 1 0 0)").
 * @param out_color Pointer to receive the parsed color.
 * @return UI_ERROR_NONE on success, UI_ERROR_PARSE_FAILED if the format is
 * invalid.
 */
enum ui_error ui_css_parse_color(const char *str,
                                 struct ui_css_color *out_color);

/**
 * @brief Types of CSS images.
 */
enum ui_css_image_type {
  UI_CSS_IMAGE_NONE,
  UI_CSS_IMAGE_URL,
  UI_CSS_IMAGE_LINEAR_GRADIENT,
  UI_CSS_IMAGE_RADIAL_GRADIENT,
  UI_CSS_IMAGE_CONIC_GRADIENT,
  UI_CSS_IMAGE_IMAGE_SET
};

/**
 * @brief Structure for gradient color stops
 */
struct ui_css_gradient_stop {
  struct ui_css_color color;
  struct ui_css_value position; /* Could be length or percentage */
};

/**
 * @brief Represents a CSS image (url, gradient, image-set).
 */
struct ui_css_image {
  enum ui_css_image_type type;
  /** \brief union */
  union {
    char url[256];
    /* Simplified gradient data */
    struct {
      float angle; /* linear gradient angle */
      struct ui_css_gradient_stop stops[8];
      int stop_count;
    } linear_gradient;
    /** \brief struct */
    struct {
      struct ui_css_gradient_stop stops[8];
      int stop_count;
    } radial_gradient;
    /** \brief struct */
    struct {
      float angle;
      struct ui_css_gradient_stop stops[8];
      int stop_count;
    } conic_gradient;
    /** \brief struct */
    struct {
      char urls[4][256];
      int resolutions[4]; /* 1x, 2x, etc. */
      int image_count;
    } image_set;
  } data;
};

/**
 * @brief Parses a CSS image value.
 *
 * @param str The string to parse.
 * @param out_image Pointer to receive the parsed image.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_image(const char *str,
                                 struct ui_css_image *out_image);

/**
 * @brief Geometry box used for clipping and masking.
 */
enum ui_css_geometry_box {
  UI_CSS_GEOMETRY_BOX_NONE,
  UI_CSS_GEOMETRY_BOX_MARGIN_BOX,
  UI_CSS_GEOMETRY_BOX_BORDER_BOX,
  UI_CSS_GEOMETRY_BOX_PADDING_BOX,
  UI_CSS_GEOMETRY_BOX_CONTENT_BOX,
  UI_CSS_GEOMETRY_BOX_FILL_BOX,
  UI_CSS_GEOMETRY_BOX_STROKE_BOX,
  UI_CSS_GEOMETRY_BOX_VIEW_BOX
};

/**
 * @brief Types of basic shapes for clip-path.
 */
enum ui_css_basic_shape_type {
  UI_CSS_BASIC_SHAPE_NONE,
  UI_CSS_BASIC_SHAPE_INSET,
  UI_CSS_BASIC_SHAPE_CIRCLE,
  UI_CSS_BASIC_SHAPE_ELLIPSE,
  UI_CSS_BASIC_SHAPE_POLYGON,
  UI_CSS_BASIC_SHAPE_PATH
};

/**
 * @brief Represents a CSS basic shape.
 */
struct ui_css_basic_shape {
  enum ui_css_basic_shape_type type;
  char arguments[256];
};

/**
 * @brief Represents a clip-path value.
 */
struct ui_css_clip_path {
  enum ui_css_geometry_box geometry_box;
  struct ui_css_basic_shape shape;
  char url[256];
};

/**
 * @brief Parses a CSS clip-path value.
 *
 * @param str The string to parse.
 * @param out_clip_path Pointer to receive the parsed clip-path.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_clip_path(const char *str,
                                     struct ui_css_clip_path *out_clip_path);

/**
 * @brief CSS mask composite operator.
 */
enum ui_css_mask_composite {
  UI_CSS_MASK_COMPOSITE_ADD,
  UI_CSS_MASK_COMPOSITE_SUBTRACT,
  UI_CSS_MASK_COMPOSITE_INTERSECT,
  UI_CSS_MASK_COMPOSITE_EXCLUDE
};

/**
 * @brief CSS mask mode.
 */
enum ui_css_mask_mode {
  UI_CSS_MASK_MODE_MATCH_SOURCE,
  UI_CSS_MASK_MODE_LUMINANCE,
  UI_CSS_MASK_MODE_ALPHA
};

/**
 * @brief Represents a single layer of a mask.
 */
struct ui_css_mask_layer {
  struct ui_css_image image;
  enum ui_css_mask_mode mode;
  enum ui_css_geometry_box clip;
  enum ui_css_geometry_box origin;
  enum ui_css_mask_composite composite;
};

/**
 * @brief Parses a CSS mask value.
 *
 * @param str The string to parse.
 * @param out_mask Pointer to receive the parsed mask layer.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_mask(const char *str,
                                struct ui_css_mask_layer *out_mask);

/**
 * @brief Represents a CSS shape-outside value.
 */
struct ui_css_shape_outside {
  enum ui_css_geometry_box box;
  struct ui_css_basic_shape shape;
  struct ui_css_image image;
};

/**
 * @brief Parses a CSS shape-outside value.
 *
 * @param str The string to parse.
 * @param out_shape Pointer to receive the parsed shape-outside.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_shape_outside(const char *str,
                           struct ui_css_shape_outside *out_shape);

/**
 * @brief CSS Transform function types.
 */
enum ui_css_transform_type {
  UI_CSS_TRANSFORM_NONE,
  UI_CSS_TRANSFORM_MATRIX,
  UI_CSS_TRANSFORM_MATRIX3D,
  UI_CSS_TRANSFORM_TRANSLATE,
  UI_CSS_TRANSFORM_TRANSLATEX,
  UI_CSS_TRANSFORM_TRANSLATEY,
  UI_CSS_TRANSFORM_TRANSLATEZ,
  UI_CSS_TRANSFORM_TRANSLATE3D,
  UI_CSS_TRANSFORM_SCALE,
  UI_CSS_TRANSFORM_SCALEX,
  UI_CSS_TRANSFORM_SCALEY,
  UI_CSS_TRANSFORM_SCALEZ,
  UI_CSS_TRANSFORM_SCALE3D,
  UI_CSS_TRANSFORM_ROTATE,
  UI_CSS_TRANSFORM_ROTATEX,
  UI_CSS_TRANSFORM_ROTATEY,
  UI_CSS_TRANSFORM_ROTATEZ,
  UI_CSS_TRANSFORM_ROTATE3D,
  UI_CSS_TRANSFORM_SKEW,
  UI_CSS_TRANSFORM_SKEWX,
  UI_CSS_TRANSFORM_SKEWY,
  UI_CSS_TRANSFORM_PERSPECTIVE
};

/**
 * @brief Represents a single CSS transform function.
 */
struct ui_css_transform_function {
  enum ui_css_transform_type type;
  struct ui_css_value values[16];
  int value_count;
  struct ui_css_transform_function *next;
};

/**
 * @brief Represents a list of transform functions.
 */
struct ui_css_transform {
  struct ui_css_transform_function *functions;
};

/**
 * @brief Parses a CSS transform value.
 *
 * @param str The string to parse.
 * @param out_transform Pointer to receive the parsed transform.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_transform(const char *str,
                                     struct ui_css_transform **out_transform);

/**
 * @brief Destroys a CSS transform list.
 *
 * @param transform The transform to destroy.
 */
enum ui_error ui_css_transform_destroy(struct ui_css_transform *transform);

/**
 * @brief CSS Filter function types.
 */
enum ui_css_filter_type {
  UI_CSS_FILTER_NONE,
  UI_CSS_FILTER_URL,
  UI_CSS_FILTER_BLUR,
  UI_CSS_FILTER_BRIGHTNESS,
  UI_CSS_FILTER_CONTRAST,
  UI_CSS_FILTER_DROP_SHADOW,
  UI_CSS_FILTER_GRAYSCALE,
  UI_CSS_FILTER_HUE_ROTATE,
  UI_CSS_FILTER_INVERT,
  UI_CSS_FILTER_OPACITY,
  UI_CSS_FILTER_SATURATE,
  UI_CSS_FILTER_SEPIA
};

/**
 * @brief Represents a single CSS filter function.
 */
struct ui_css_filter_function {
  enum ui_css_filter_type type;
  /** \brief union */
  union {
    char url[256];
    /** \brief ui_css_value */
    struct ui_css_value
        value; /* For blur, brightness, contrast, grayscale, hue-rotate, invert,
                  opacity, saturate, sepia */
    struct {
      struct ui_css_value offset_x;
      struct ui_css_value offset_y;
      struct ui_css_value blur_radius;
      struct ui_css_color color;
      int has_color;
    } drop_shadow;
  } data;
  struct ui_css_filter_function *next;
};

/**
 * @brief Represents a list of filter functions.
 */
struct ui_css_filter {
  struct ui_css_filter_function *functions;
};

/**
 * @brief Parses a CSS filter value.
 *
 * @param str The string to parse.
 * @param out_filter Pointer to receive the parsed filter.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_filter(const char *str,
                                  struct ui_css_filter **out_filter);

/**
 * @brief Destroys a CSS filter list.
 *
 * @param filter The filter to destroy.
 */
enum ui_error ui_css_filter_destroy(struct ui_css_filter *filter);

/**
 * @brief CSS blend modes.
 */
enum ui_css_blend_mode {
  UI_CSS_BLEND_MODE_NORMAL,
  UI_CSS_BLEND_MODE_MULTIPLY,
  UI_CSS_BLEND_MODE_SCREEN,
  UI_CSS_BLEND_MODE_OVERLAY,
  UI_CSS_BLEND_MODE_DARKEN,
  UI_CSS_BLEND_MODE_LIGHTEN,
  UI_CSS_BLEND_MODE_COLOR_DODGE,
  UI_CSS_BLEND_MODE_COLOR_BURN,
  UI_CSS_BLEND_MODE_HARD_LIGHT,
  UI_CSS_BLEND_MODE_SOFT_LIGHT,
  UI_CSS_BLEND_MODE_DIFFERENCE,
  UI_CSS_BLEND_MODE_EXCLUSION,
  UI_CSS_BLEND_MODE_HUE,
  UI_CSS_BLEND_MODE_SATURATION,
  UI_CSS_BLEND_MODE_COLOR,
  UI_CSS_BLEND_MODE_LUMINOSITY
};

/**
 * @brief Parses a CSS blend mode value.
 *
 * @param str The string to parse.
 * @param out_blend_mode Pointer to receive the parsed blend mode.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_blend_mode(const char *str,
                                      enum ui_css_blend_mode *out_blend_mode);

/**
 * @brief Represents a single CSS shadow (box-shadow or text-shadow).
 */
struct ui_css_shadow {
  struct ui_css_value offset_x;
  struct ui_css_value offset_y;
  struct ui_css_value blur_radius;
  struct ui_css_value spread_radius;
  struct ui_css_color color;
  int has_color;
  int is_inset;
  struct ui_css_shadow *next;
};

/**
 * @brief Represents a list of CSS shadows.
 */
struct ui_css_shadow_list {
  struct ui_css_shadow *shadows;
};

/**
 * @brief Parses a CSS shadow list value (box-shadow or text-shadow).
 *
 * @param str The string to parse.
 * @param out_shadows Pointer to receive the parsed shadow list.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_shadow(const char *str,
                                  struct ui_css_shadow_list **out_shadows);

/**
 * @brief Destroys a CSS shadow list.
 *
 * @param list The shadow list to destroy.
 */
enum ui_error ui_css_shadow_list_destroy(struct ui_css_shadow_list *list);

/**
 * @brief SVG Fill Rule.
 */
enum ui_css_fill_rule { UI_CSS_FILL_RULE_NONZERO, UI_CSS_FILL_RULE_EVENODD };

/**
 * @brief SVG Stroke Linecap.
 */
enum ui_css_stroke_linecap {
  UI_CSS_STROKE_LINECAP_BUTT,
  UI_CSS_STROKE_LINECAP_ROUND,
  UI_CSS_STROKE_LINECAP_SQUARE
};

/**
 * @brief SVG Stroke Linejoin.
 */
enum ui_css_stroke_linejoin {
  UI_CSS_STROKE_LINEJOIN_MITER,
  UI_CSS_STROKE_LINEJOIN_ROUND,
  UI_CSS_STROKE_LINEJOIN_BEVEL
};

/**
 * @brief SVG Paint Type.
 */
enum ui_css_paint_type {
  UI_CSS_PAINT_NONE,
  UI_CSS_PAINT_CURRENT_COLOR,
  UI_CSS_PAINT_COLOR,
  UI_CSS_PAINT_URL
};

/**
 * @brief Represents a CSS Paint value (for fill/stroke).
 */
struct ui_css_paint {
  enum ui_css_paint_type type;
  struct ui_css_color color;
  char url[256]; /* For url(#gradient) */
};

/**
 * @brief Represents a stroke-dasharray value.
 */
struct ui_css_dasharray {
  struct ui_css_value values[16];
  int count;
};

/**
 * @brief Parses a fill-rule value.
 *
 * @param str The string to parse.
 * @param out_rule Pointer to receive the parsed rule.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_fill_rule(const char *str,
                                     enum ui_css_fill_rule *out_rule);

/**
 * @brief Parses a stroke-linecap value.
 *
 * @param str The string to parse.
 * @param out_linecap Pointer to receive the parsed linecap.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_stroke_linecap(const char *str,
                            enum ui_css_stroke_linecap *out_linecap);

/**
 * @brief Parses a stroke-linejoin value.
 *
 * @param str The string to parse.
 * @param out_linejoin Pointer to receive the parsed linejoin.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_stroke_linejoin(const char *str,
                             enum ui_css_stroke_linejoin *out_linejoin);

/**
 * @brief Parses a paint value (for fill or stroke).
 *
 * @param str The string to parse.
 * @param out_paint Pointer to receive the parsed paint.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_paint(const char *str,
                                 struct ui_css_paint *out_paint);

/**
 * @brief Parses a stroke-dasharray value.
 *
 * @param str The string to parse.
 * @param out_dasharray Pointer to receive the parsed dasharray.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_dasharray(const char *str,
                                     struct ui_css_dasharray *out_dasharray);

/**
 * @brief CSS Easing function types.
 */
enum ui_css_easing_type {
  UI_CSS_EASING_LINEAR,
  UI_CSS_EASING_EASE,
  UI_CSS_EASING_EASE_IN,
  UI_CSS_EASING_EASE_OUT,
  UI_CSS_EASING_EASE_IN_OUT,
  UI_CSS_EASING_CUBIC_BEZIER,
  UI_CSS_EASING_STEPS
};

/**
 * @brief CSS Steps position for easing functions.
 */
enum ui_css_steps_position {
  UI_CSS_STEPS_JUMP_START,
  UI_CSS_STEPS_JUMP_END,
  UI_CSS_STEPS_JUMP_NONE,
  UI_CSS_STEPS_JUMP_BOTH,
  UI_CSS_STEPS_START,
  UI_CSS_STEPS_END
};

/**
 * @brief Represents a CSS easing function.
 */
struct ui_css_easing_function {
  enum ui_css_easing_type type;
  /** \brief union */
  union {
    /** \brief struct */
    struct {
      float x1, y1, x2, y2;
    } cubic_bezier;
    /** \brief struct */
    struct {
      int count;
      enum ui_css_steps_position position;
    } steps;
  } data;
};

/**
 * @brief Represents a single CSS transition definition.
 */
struct ui_css_transition {
  char property_name[64];
  struct ui_css_value duration;
  struct ui_css_easing_function timing_function;
  struct ui_css_value delay;
  struct ui_css_transition *next;
};

/**
 * @brief Parses a CSS easing function.
 *
 * @param str The string to parse.
 * @param out_easing Pointer to receive the parsed easing function.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_easing_function(const char *str,
                             struct ui_css_easing_function *out_easing);

/**
 * @brief Parses a CSS transition list.
 *
 * @param str The string to parse.
 * @param out_transitions Pointer to receive the parsed transitions.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_transition(const char *str,
                        struct ui_css_transition **out_transitions);

/**
 * @brief Destroys a CSS transition list.
 *
 * @param transitions The transition list to destroy.
 */
enum ui_error ui_css_transition_destroy(struct ui_css_transition *transitions);

/**
 * @brief CSS Animation Direction.
 */
enum ui_css_animation_direction {
  UI_CSS_ANIMATION_DIRECTION_NORMAL,
  UI_CSS_ANIMATION_DIRECTION_REVERSE,
  UI_CSS_ANIMATION_DIRECTION_ALTERNATE,
  UI_CSS_ANIMATION_DIRECTION_ALTERNATE_REVERSE
};

/**
 * @brief CSS Animation Fill Mode.
 */
enum ui_css_animation_fill_mode {
  UI_CSS_ANIMATION_FILL_MODE_NONE,
  UI_CSS_ANIMATION_FILL_MODE_FORWARDS,
  UI_CSS_ANIMATION_FILL_MODE_BACKWARDS,
  UI_CSS_ANIMATION_FILL_MODE_BOTH
};

/**
 * @brief CSS Animation Play State.
 */
enum ui_css_animation_play_state {
  UI_CSS_ANIMATION_PLAY_STATE_RUNNING,
  UI_CSS_ANIMATION_PLAY_STATE_PAUSED
};

/**
 * @brief Represents a CSS Animation definition.
 */
struct ui_css_animation {
  char name[64];
  struct ui_css_value duration;
  struct ui_css_easing_function timing_function;
  struct ui_css_value delay;
  float iteration_count; /* -1.0f for infinite */
  enum ui_css_animation_direction direction;
  enum ui_css_animation_fill_mode fill_mode;
  enum ui_css_animation_play_state play_state;
  struct ui_css_animation *next;
};

/**
 * @brief Parses a CSS animation list.
 *
 * @param str The string to parse.
 * @param out_animations Pointer to receive the parsed animations.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_animation(const char *str,
                                     struct ui_css_animation **out_animations);

/**
 * @brief Destroys a CSS animation list.
 *
 * @param animations The animation list to destroy.
 */
enum ui_error ui_css_animation_destroy(struct ui_css_animation *animations);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_VALUES_H */
