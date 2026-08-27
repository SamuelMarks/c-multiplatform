/**
 * @file ui_css_values.h
 * @brief CSS value structures, parsing, and destruction.
 *
 * This header defines structures, enumerations, and functions for handling
 * generic CSS values, math expressions, colors, images, shadows, transforms,
 * filters, and animations.
 */

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
  UI_CSS_UNIT_NONE = 0, /**< No unit (e.g., bare numbers). */

  /* Absolute lengths */
  UI_CSS_UNIT_PX, /**< Pixels (px). */
  UI_CSS_UNIT_CM, /**< Centimeters (cm). */
  UI_CSS_UNIT_MM, /**< Millimeters (mm). */
  UI_CSS_UNIT_IN, /**< Inches (in). */
  UI_CSS_UNIT_PT, /**< Points (pt). */
  UI_CSS_UNIT_PC, /**< Picas (pc). */
  UI_CSS_UNIT_Q,  /**< Quarter-millimeters (q). */

  /* Relative lengths */
  UI_CSS_UNIT_EM,  /**< Relative to font size of element (em). */
  UI_CSS_UNIT_EX,  /**< Relative to x-height of element's font (ex). */
  UI_CSS_UNIT_CH,  /**< Relative to width of '0' character (ch). */
  UI_CSS_UNIT_REM, /**< Relative to font size of root element (rem). */

  /* Viewport-relative lengths (Level 4) */
  UI_CSS_UNIT_VW,   /**< Viewport width (vw). */
  UI_CSS_UNIT_VH,   /**< Viewport height (vh). */
  UI_CSS_UNIT_VMIN, /**< Minimum of viewport width/height (vmin). */
  UI_CSS_UNIT_VMAX, /**< Maximum of viewport width/height (vmax). */
  UI_CSS_UNIT_VI,   /**< Viewport inline size (vi). */
  UI_CSS_UNIT_VB,   /**< Viewport block size (vb). */

  /* Container-relative lengths (Level 5) */
  UI_CSS_UNIT_CQW,   /**< Container width (cqw). */
  UI_CSS_UNIT_CQH,   /**< Container height (cqh). */
  UI_CSS_UNIT_CQI,   /**< Container inline size (cqi). */
  UI_CSS_UNIT_CQB,   /**< Container block size (cqb). */
  UI_CSS_UNIT_CQMIN, /**< Container minimum size (cqmin). */
  UI_CSS_UNIT_CQMAX, /**< Container maximum size (cqmax). */

  /* Percentages */
  UI_CSS_UNIT_PERCENT, /**< Percentage (%). */

  /* Angles */
  UI_CSS_UNIT_DEG,  /**< Degrees (deg). */
  UI_CSS_UNIT_GRAD, /**< Gradians (grad). */
  UI_CSS_UNIT_RAD,  /**< Radians (rad). */
  UI_CSS_UNIT_TURN, /**< Turns (turn). */

  /* Times */
  UI_CSS_UNIT_S,  /**< Seconds (s). */
  UI_CSS_UNIT_MS, /**< Milliseconds (ms). */

  /* Frequencies & Speech */
  UI_CSS_UNIT_HZ,  /**< Hertz (Hz). */
  UI_CSS_UNIT_KHZ, /**< Kilohertz (kHz). */
  UI_CSS_UNIT_DB,  /**< Decibels (dB). */
  UI_CSS_UNIT_ST,  /**< Semitones (st). */

  UI_CSS_UNIT_UNKNOWN /**< Unknown unit. */
};

/**
 * @brief Represents a numeric CSS value with its associated unit.
 */
struct ui_css_value {
  float value;           /**< The numeric portion of the value. */
  enum ui_css_unit unit; /**< The unit of the value. */
};

/**
 * @brief Represents a CSS math operator.
 */
enum ui_css_math_op {
  UI_CSS_MATH_OP_ADD,   /**< Addition. */
  UI_CSS_MATH_OP_SUB,   /**< Subtraction. */
  UI_CSS_MATH_OP_MUL,   /**< Multiplication. */
  UI_CSS_MATH_OP_DIV,   /**< Division. */
  UI_CSS_MATH_OP_MIN,   /**< Minimum function. */
  UI_CSS_MATH_OP_MAX,   /**< Maximum function. */
  UI_CSS_MATH_OP_CLAMP, /**< Clamp function. */

  /* Trig functions (Level 5) */
  UI_CSS_MATH_OP_SIN,  /**< Sine function. */
  UI_CSS_MATH_OP_COS,  /**< Cosine function. */
  UI_CSS_MATH_OP_TAN,  /**< Tangent function. */
  UI_CSS_MATH_OP_ASIN, /**< Arcsine function. */
  UI_CSS_MATH_OP_ACOS, /**< Arccosine function. */
  UI_CSS_MATH_OP_ATAN, /**< Arctangent function. */
  UI_CSS_MATH_OP_ATAN2 /**< Two-argument arctangent function. */
};

/**
 * @brief The type of extended CSS value (scalar vs math expression).
 */
enum ui_css_value_type {
  UI_CSS_VALUE_TYPE_SCALAR, /**< Scalar value type. */
  UI_CSS_VALUE_TYPE_MATH,   /**< Math expression type. */
  UI_CSS_VALUE_TYPE_ENV     /**< Environment variable type. */
};

struct ui_css_math_expr; /* Forward declaration */

/**
 * @brief Represents an env() variable reference with an optional fallback.
 */
struct ui_css_env_ref {
  char name[64];                     /**< Environment variable name. */
  struct ui_css_value_ext *fallback; /**< Fallback value if not resolved. */
};

/**
 * @brief An extended CSS value that can either be a scalar or a math
 * expression.
 */
struct ui_css_value_ext {
  enum ui_css_value_type type; /**< The type of the value. */
  /** \brief Union for the different value states. */
  union {
    struct ui_css_value scalar;    /**< A scalar CSS value. */
    struct ui_css_math_expr *math; /**< A CSS math expression. */
    struct ui_css_env_ref *env;    /**< A CSS environment variable. */
  } value;                         /**< The union value */
};

/**
 * @brief Represents a node in a CSS math expression AST.
 */
struct ui_css_math_expr {
  enum ui_css_math_op op;         /**< The math operator or function. */
  struct ui_css_value_ext *left;  /**< Left hand side of the operation. */
  struct ui_css_value_ext *right; /**< Right hand side or second argument. */
  struct ui_css_value_ext *ext;   /**< Third argument (used for clamp). */
  struct ui_css_math_expr
      *next; /**< Next node for variadic functions like min/max. */
};

/**
 * @brief Parses a string into a simple numeric CSS value and its associated
 * unit.
 *
 * @param str The string to parse (e.g., "10.5px", "50%").
 * @param out_value Pointer to receive the parsed value structure.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_PARSE_FAILED` if invalid
 * format.
 */
ui_error_t ui_css_parse_value(const char *str, struct ui_css_value *out_value);

/**
 * @brief Parses an extended CSS value string, which can include math functions.
 *
 * @param str The string to parse (e.g., "calc(100% - 20px)").
 * @param out_value Pointer to receive the parsed extended value.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_PARSE_FAILED` if invalid
 * format.
 */
ui_error_t ui_css_parse_value_ext(const char *str,
                                  struct ui_css_value_ext **out_value);

/**
 * @brief Destroys an extended CSS value, freeing associated math expression
 * memory.
 *
 * @param val Pointer to the value to destroy.
 */
void ui_css_value_ext_destroy(struct ui_css_value_ext *val);

/**
 * @brief Represents color spaces for CSS Color Module Level 3, 4, 5, 6.
 */
enum ui_css_color_space {
  UI_CSS_COLOR_SPACE_SRGB,         /**< sRGB color space. */
  UI_CSS_COLOR_SPACE_HSL,          /**< HSL color space. */
  UI_CSS_COLOR_SPACE_HWB,          /**< HWB color space. */
  UI_CSS_COLOR_SPACE_LAB,          /**< CIELAB color space. */
  UI_CSS_COLOR_SPACE_LCH,          /**< LCH color space. */
  UI_CSS_COLOR_SPACE_OKLAB,        /**< Oklab color space. */
  UI_CSS_COLOR_SPACE_OKLCH,        /**< Oklch color space. */
  UI_CSS_COLOR_SPACE_DISPLAY_P3,   /**< Display P3 color space. */
  UI_CSS_COLOR_SPACE_A98_RGB,      /**< A98 RGB color space. */
  UI_CSS_COLOR_SPACE_PROPHOTO_RGB, /**< ProPhoto RGB color space. */
  UI_CSS_COLOR_SPACE_REC2020,      /**< Rec. 2020 color space. */
  UI_CSS_COLOR_SPACE_SRGB_LINEAR,  /**< Linear sRGB color space. */
  UI_CSS_COLOR_SPACE_XYZ_D50,      /**< CIE XYZ (D50) color space. */
  UI_CSS_COLOR_SPACE_XYZ_D65       /**< CIE XYZ (D65) color space. */
};

/**
 * @brief Represents a parsed CSS color.
 */
struct ui_css_color {
  enum ui_css_color_space space; /**< The color space. */
  float components[4]; /**< Color components (e.g., r, g, b, a or l, a, b,
                          alpha). */
};

/**
 * @brief Parses a CSS color string into a color structure.
 *
 * @param str The string to parse.
 * @param out_color Pointer to receive the parsed color.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_PARSE_FAILED` if invalid
 * format.
 */
ui_error_t ui_css_parse_color(const char *str, struct ui_css_color *out_color);

/**
 * @brief Types of CSS images.
 */
enum ui_css_image_type {
  UI_CSS_IMAGE_NONE,            /**< No image. */
  UI_CSS_IMAGE_URL,             /**< An image specified by URL. */
  UI_CSS_IMAGE_LINEAR_GRADIENT, /**< A linear gradient. */
  UI_CSS_IMAGE_RADIAL_GRADIENT, /**< A radial gradient. */
  UI_CSS_IMAGE_CONIC_GRADIENT,  /**< A conic gradient. */
  UI_CSS_IMAGE_IMAGE_SET        /**< An image set. */
};

/**
 * @brief Structure for gradient color stops.
 */
struct ui_css_gradient_stop {
  struct ui_css_color color; /**< The color of the stop. */
  struct ui_css_value
      position; /**< The position of the stop (length or percentage). */
};

/**
 * @brief Represents a CSS image (url, gradient, image-set).
 */
struct ui_css_image {
  enum ui_css_image_type type; /**< The type of the image. */
  /** \brief Union containing the data for the different image types. */
  union {
    char url[256]; /**< The image URL. */
    /** \brief Linear gradient configuration. */
    struct {
      float angle;                          /**< The linear gradient angle. */
      struct ui_css_gradient_stop stops[8]; /**< The color stops. */
      int stop_count;                       /**< The number of color stops. */
    } linear_gradient; /**< Linear gradient configuration */
    /** \brief Radial gradient configuration. */
    struct {
      struct ui_css_gradient_stop stops[8]; /**< The color stops. */
      int stop_count;                       /**< The number of color stops. */
    } radial_gradient; /**< Radial gradient configuration */
    /** \brief Conic gradient configuration. */
    struct {
      float angle;                          /**< The conic gradient angle. */
      struct ui_css_gradient_stop stops[8]; /**< The color stops. */
      int stop_count;                       /**< The number of color stops. */
    } conic_gradient;                       /**< Conic gradient configuration */
    /** \brief Image set configuration. */
    struct {
      char urls[4][256];  /**< Array of image URLs. */
      int resolutions[4]; /**< Array of resolutions (1x, 2x, etc). */
      int image_count;    /**< The total number of images in the set. */
    } image_set;          /**< Image set configuration */
  } data;                 /**< The union data */
};

/**
 * @brief Parses a CSS image value.
 *
 * @param str The string to parse.
 * @param out_image Pointer to receive the parsed image.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_image(const char *str, struct ui_css_image *out_image);

/**
 * @brief Geometry box used for clipping and masking.
 */
enum ui_css_geometry_box {
  UI_CSS_GEOMETRY_BOX_NONE,        /**< No geometry box. */
  UI_CSS_GEOMETRY_BOX_MARGIN_BOX,  /**< Margin box. */
  UI_CSS_GEOMETRY_BOX_BORDER_BOX,  /**< Border box. */
  UI_CSS_GEOMETRY_BOX_PADDING_BOX, /**< Padding box. */
  UI_CSS_GEOMETRY_BOX_CONTENT_BOX, /**< Content box. */
  UI_CSS_GEOMETRY_BOX_FILL_BOX,    /**< SVG fill box. */
  UI_CSS_GEOMETRY_BOX_STROKE_BOX,  /**< SVG stroke box. */
  UI_CSS_GEOMETRY_BOX_VIEW_BOX     /**< SVG view box. */
};

/**
 * @brief Types of basic shapes for clip-path.
 */
enum ui_css_basic_shape_type {
  UI_CSS_BASIC_SHAPE_NONE,    /**< No basic shape. */
  UI_CSS_BASIC_SHAPE_INSET,   /**< Inset shape. */
  UI_CSS_BASIC_SHAPE_CIRCLE,  /**< Circle shape. */
  UI_CSS_BASIC_SHAPE_ELLIPSE, /**< Ellipse shape. */
  UI_CSS_BASIC_SHAPE_POLYGON, /**< Polygon shape. */
  UI_CSS_BASIC_SHAPE_PATH     /**< Path shape. */
};

/**
 * @brief Represents a CSS basic shape.
 */
struct ui_css_basic_shape {
  enum ui_css_basic_shape_type type; /**< The type of the basic shape. */
  char arguments[256]; /**< Unparsed arguments for the shape function. */
};

/**
 * @brief Represents a clip-path value.
 */
struct ui_css_clip_path {
  enum ui_css_geometry_box geometry_box; /**< The reference geometry box. */
  struct ui_css_basic_shape shape;       /**< The basic shape defined. */
  char url[256]; /**< An optional URL to an SVG clipPath. */
};

/**
 * @brief Parses a CSS clip-path value.
 *
 * @param str The string to parse.
 * @param out_clip_path Pointer to receive the parsed clip-path.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_clip_path(const char *str,
                                  struct ui_css_clip_path *out_clip_path);

/**
 * @brief CSS mask composite operator.
 */
enum ui_css_mask_composite {
  UI_CSS_MASK_COMPOSITE_ADD,       /**< Add operator. */
  UI_CSS_MASK_COMPOSITE_SUBTRACT,  /**< Subtract operator. */
  UI_CSS_MASK_COMPOSITE_INTERSECT, /**< Intersect operator. */
  UI_CSS_MASK_COMPOSITE_EXCLUDE    /**< Exclude operator. */
};

/**
 * @brief CSS mask mode.
 */
enum ui_css_mask_mode {
  UI_CSS_MASK_MODE_MATCH_SOURCE, /**< Match source mode. */
  UI_CSS_MASK_MODE_LUMINANCE,    /**< Luminance mode. */
  UI_CSS_MASK_MODE_ALPHA         /**< Alpha mode. */
};

/**
 * @brief Represents a single layer of a mask.
 */
struct ui_css_mask_layer {
  struct ui_css_image image;            /**< The mask image. */
  enum ui_css_mask_mode mode;           /**< The mask mode. */
  enum ui_css_geometry_box clip;        /**< The clipping box. */
  enum ui_css_geometry_box origin;      /**< The origin box. */
  enum ui_css_mask_composite composite; /**< The mask composite operation. */
};

/**
 * @brief Parses a CSS mask value.
 *
 * @param str The string to parse.
 * @param out_mask Pointer to receive the parsed mask layer.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_mask(const char *str,
                             struct ui_css_mask_layer *out_mask);

/**
 * @brief Represents a CSS shape-outside value.
 */
struct ui_css_shape_outside {
  enum ui_css_geometry_box box;    /**< The reference geometry box. */
  struct ui_css_basic_shape shape; /**< The basic shape. */
  struct ui_css_image image;       /**< The shape image. */
};

/**
 * @brief Parses a CSS shape-outside value.
 *
 * @param str The string to parse.
 * @param out_shape Pointer to receive the parsed shape-outside.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_shape_outside(const char *str,
                                      struct ui_css_shape_outside *out_shape);

/**
 * @brief CSS Transform function types.
 */
enum ui_css_transform_type {
  UI_CSS_TRANSFORM_NONE,        /**< No transform function. */
  UI_CSS_TRANSFORM_MATRIX,      /**< matrix() */
  UI_CSS_TRANSFORM_MATRIX3D,    /**< matrix3d() */
  UI_CSS_TRANSFORM_TRANSLATE,   /**< translate() */
  UI_CSS_TRANSFORM_TRANSLATEX,  /**< translateX() */
  UI_CSS_TRANSFORM_TRANSLATEY,  /**< translateY() */
  UI_CSS_TRANSFORM_TRANSLATEZ,  /**< translateZ() */
  UI_CSS_TRANSFORM_TRANSLATE3D, /**< translate3d() */
  UI_CSS_TRANSFORM_SCALE,       /**< scale() */
  UI_CSS_TRANSFORM_SCALEX,      /**< scaleX() */
  UI_CSS_TRANSFORM_SCALEY,      /**< scaleY() */
  UI_CSS_TRANSFORM_SCALEZ,      /**< scaleZ() */
  UI_CSS_TRANSFORM_SCALE3D,     /**< scale3d() */
  UI_CSS_TRANSFORM_ROTATE,      /**< rotate() */
  UI_CSS_TRANSFORM_ROTATEX,     /**< rotateX() */
  UI_CSS_TRANSFORM_ROTATEY,     /**< rotateY() */
  UI_CSS_TRANSFORM_ROTATEZ,     /**< rotateZ() */
  UI_CSS_TRANSFORM_ROTATE3D,    /**< rotate3d() */
  UI_CSS_TRANSFORM_SKEW,        /**< skew() */
  UI_CSS_TRANSFORM_SKEWX,       /**< skewX() */
  UI_CSS_TRANSFORM_SKEWY,       /**< skewY() */
  UI_CSS_TRANSFORM_PERSPECTIVE  /**< perspective() */
};

/**
 * @brief Represents a single CSS transform function.
 */
struct ui_css_transform_function {
  enum ui_css_transform_type type;        /**< The transform function type. */
  struct ui_css_value values[16];         /**< Parsed arguments. */
  int value_count;                        /**< Argument count. */
  struct ui_css_transform_function *next; /**< Pointer to the next function. */
};

/**
 * @brief Represents a list of transform functions.
 */
struct ui_css_transform {
  struct ui_css_transform_function
      *functions; /**< Linked list of transform functions. */
};

/**
 * @brief Parses a CSS transform value.
 *
 * @param str The string to parse.
 * @param out_transform Pointer to receive the parsed transform.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_transform(const char *str,
                                  struct ui_css_transform **out_transform);

/**
 * @brief Destroys a CSS transform list.
 *
 * @param transform Pointer to the transform to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_transform_destroy(struct ui_css_transform *transform);

/**
 * @brief CSS Filter function types.
 */
enum ui_css_filter_type {
  UI_CSS_FILTER_NONE,        /**< No filter. */
  UI_CSS_FILTER_URL,         /**< URL filter (e.g. SVG). */
  UI_CSS_FILTER_BLUR,        /**< blur() */
  UI_CSS_FILTER_BRIGHTNESS,  /**< brightness() */
  UI_CSS_FILTER_CONTRAST,    /**< contrast() */
  UI_CSS_FILTER_DROP_SHADOW, /**< drop-shadow() */
  UI_CSS_FILTER_GRAYSCALE,   /**< grayscale() */
  UI_CSS_FILTER_HUE_ROTATE,  /**< hue-rotate() */
  UI_CSS_FILTER_INVERT,      /**< invert() */
  UI_CSS_FILTER_OPACITY,     /**< opacity() */
  UI_CSS_FILTER_SATURATE,    /**< saturate() */
  UI_CSS_FILTER_SEPIA        /**< sepia() */
};

/**
 * @brief Represents a single CSS filter function.
 */
struct ui_css_filter_function {
  enum ui_css_filter_type type; /**< The filter function type. */
  /** \brief Union storing filter data. */
  union {
    char url[256];             /**< The filter URL. */
    struct ui_css_value value; /**< Scalar value used by most filters. */
    /** \brief Details for drop-shadow. */
    struct {
      struct ui_css_value offset_x;    /**< X offset. */
      struct ui_css_value offset_y;    /**< Y offset. */
      struct ui_css_value blur_radius; /**< Blur radius. */
      struct ui_css_color color;       /**< Shadow color. */
      int has_color; /**< 1 if color is specified, 0 otherwise. */
    } drop_shadow;   /**< Drop shadow details */
  } data;            /**< The filter function data */
  struct ui_css_filter_function
      *next; /**< Pointer to the next filter function. */
};

/**
 * @brief Represents a list of filter functions.
 */
struct ui_css_filter {
  struct ui_css_filter_function
      *functions; /**< Linked list of filter functions. */
};

/**
 * @brief Parses a CSS filter value.
 *
 * @param str The string to parse.
 * @param out_filter Pointer to receive the parsed filter list.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_filter(const char *str,
                               struct ui_css_filter **out_filter);

/**
 * @brief Destroys a CSS filter list.
 *
 * @param filter Pointer to the filter to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_filter_destroy(struct ui_css_filter *filter);

/**
 * @brief CSS blend modes.
 */
enum ui_css_blend_mode {
  UI_CSS_BLEND_MODE_NORMAL,      /**< Normal blend mode. */
  UI_CSS_BLEND_MODE_MULTIPLY,    /**< Multiply blend mode. */
  UI_CSS_BLEND_MODE_SCREEN,      /**< Screen blend mode. */
  UI_CSS_BLEND_MODE_OVERLAY,     /**< Overlay blend mode. */
  UI_CSS_BLEND_MODE_DARKEN,      /**< Darken blend mode. */
  UI_CSS_BLEND_MODE_LIGHTEN,     /**< Lighten blend mode. */
  UI_CSS_BLEND_MODE_COLOR_DODGE, /**< Color-dodge blend mode. */
  UI_CSS_BLEND_MODE_COLOR_BURN,  /**< Color-burn blend mode. */
  UI_CSS_BLEND_MODE_HARD_LIGHT,  /**< Hard-light blend mode. */
  UI_CSS_BLEND_MODE_SOFT_LIGHT,  /**< Soft-light blend mode. */
  UI_CSS_BLEND_MODE_DIFFERENCE,  /**< Difference blend mode. */
  UI_CSS_BLEND_MODE_EXCLUSION,   /**< Exclusion blend mode. */
  UI_CSS_BLEND_MODE_HUE,         /**< Hue blend mode. */
  UI_CSS_BLEND_MODE_SATURATION,  /**< Saturation blend mode. */
  UI_CSS_BLEND_MODE_COLOR,       /**< Color blend mode. */
  UI_CSS_BLEND_MODE_LUMINOSITY   /**< Luminosity blend mode. */
};

/**
 * @brief Parses a CSS blend mode value.
 *
 * @param str The string to parse.
 * @param out_blend_mode Pointer to receive the parsed blend mode.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_blend_mode(const char *str,
                                   enum ui_css_blend_mode *out_blend_mode);

/**
 * @brief Represents a single CSS shadow (box-shadow or text-shadow).
 */
struct ui_css_shadow {
  struct ui_css_value offset_x;      /**< Shadow X offset. */
  struct ui_css_value offset_y;      /**< Shadow Y offset. */
  struct ui_css_value blur_radius;   /**< Shadow blur radius. */
  struct ui_css_value spread_radius; /**< Shadow spread radius. */
  struct ui_css_color color;         /**< Shadow color. */
  int has_color;                     /**< 1 if color is set, 0 otherwise. */
  int is_inset;                      /**< 1 if inset, 0 otherwise. */
  struct ui_css_shadow *next; /**< Pointer to the next shadow in the list. */
};

/**
 * @brief Represents a list of CSS shadows.
 */
struct ui_css_shadow_list {
  struct ui_css_shadow *shadows; /**< Linked list of shadows. */
};

/**
 * @brief Parses a CSS shadow list value (box-shadow or text-shadow).
 *
 * @param str The string to parse.
 * @param out_shadows Pointer to receive the parsed shadow list.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_shadow(const char *str,
                               struct ui_css_shadow_list **out_shadows);

/**
 * @brief Destroys a CSS shadow list.
 *
 * @param list Pointer to the shadow list to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_shadow_list_destroy(struct ui_css_shadow_list *list);

/**
 * @brief SVG Fill Rule.
 */
enum ui_css_fill_rule {
  UI_CSS_FILL_RULE_NONZERO, /**< Non-zero fill rule. */
  UI_CSS_FILL_RULE_EVENODD  /**< Even-odd fill rule. */
};

/**
 * @brief SVG Stroke Linecap.
 */
enum ui_css_stroke_linecap {
  UI_CSS_STROKE_LINECAP_BUTT,  /**< Butt linecap. */
  UI_CSS_STROKE_LINECAP_ROUND, /**< Round linecap. */
  UI_CSS_STROKE_LINECAP_SQUARE /**< Square linecap. */
};

/**
 * @brief SVG Stroke Linejoin.
 */
enum ui_css_stroke_linejoin {
  UI_CSS_STROKE_LINEJOIN_MITER, /**< Miter linejoin. */
  UI_CSS_STROKE_LINEJOIN_ROUND, /**< Round linejoin. */
  UI_CSS_STROKE_LINEJOIN_BEVEL  /**< Bevel linejoin. */
};

/**
 * @brief SVG Paint Type.
 */
enum ui_css_paint_type {
  UI_CSS_PAINT_NONE,          /**< No paint. */
  UI_CSS_PAINT_CURRENT_COLOR, /**< Current color paint. */
  UI_CSS_PAINT_COLOR,         /**< Specific color paint. */
  UI_CSS_PAINT_URL            /**< URL paint (e.g., gradient). */
};

/**
 * @brief Represents a CSS Paint value (for fill/stroke).
 */
struct ui_css_paint {
  enum ui_css_paint_type type; /**< The paint type. */
  struct ui_css_color color;   /**< The paint color. */
  char url[256];               /**< URL if type is `UI_CSS_PAINT_URL`. */
};

/**
 * @brief Represents a stroke-dasharray value.
 */
struct ui_css_dasharray {
  struct ui_css_value values[16]; /**< The dash values. */
  int count;                      /**< The number of dashes provided. */
};

/**
 * @brief Parses a fill-rule value.
 *
 * @param str The string to parse.
 * @param out_rule Pointer to receive the parsed rule.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_fill_rule(const char *str,
                                  enum ui_css_fill_rule *out_rule);

/**
 * @brief Parses a stroke-linecap value.
 *
 * @param str The string to parse.
 * @param out_linecap Pointer to receive the parsed linecap.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_stroke_linecap(const char *str,
                                       enum ui_css_stroke_linecap *out_linecap);

/**
 * @brief Parses a stroke-linejoin value.
 *
 * @param str The string to parse.
 * @param out_linejoin Pointer to receive the parsed linejoin.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_css_parse_stroke_linejoin(const char *str,
                             enum ui_css_stroke_linejoin *out_linejoin);

/**
 * @brief Parses a paint value (for fill or stroke).
 *
 * @param str The string to parse.
 * @param out_paint Pointer to receive the parsed paint.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_paint(const char *str, struct ui_css_paint *out_paint);

/**
 * @brief Parses a stroke-dasharray value.
 *
 * @param str The string to parse.
 * @param out_dasharray Pointer to receive the parsed dasharray.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_dasharray(const char *str,
                                  struct ui_css_dasharray *out_dasharray);

/**
 * @brief CSS Easing function types.
 */
enum ui_css_easing_type {
  UI_CSS_EASING_LINEAR,       /**< Linear easing. */
  UI_CSS_EASING_EASE,         /**< Ease easing. */
  UI_CSS_EASING_EASE_IN,      /**< Ease-in easing. */
  UI_CSS_EASING_EASE_OUT,     /**< Ease-out easing. */
  UI_CSS_EASING_EASE_IN_OUT,  /**< Ease-in-out easing. */
  UI_CSS_EASING_CUBIC_BEZIER, /**< Cubic bezier easing. */
  UI_CSS_EASING_STEPS         /**< Steps easing. */
};

/**
 * @brief CSS Steps position for easing functions.
 */
enum ui_css_steps_position {
  UI_CSS_STEPS_JUMP_START, /**< Jump start. */
  UI_CSS_STEPS_JUMP_END,   /**< Jump end. */
  UI_CSS_STEPS_JUMP_NONE,  /**< Jump none. */
  UI_CSS_STEPS_JUMP_BOTH,  /**< Jump both. */
  UI_CSS_STEPS_START,      /**< Start position (alias). */
  UI_CSS_STEPS_END         /**< End position (alias). */
};

/**
 * @brief Represents a CSS easing function.
 */
struct ui_css_easing_function {
  enum ui_css_easing_type type; /**< The easing function type. */
  /** \brief Union storing parameters for complex functions. */
  union {
    /** \brief Parameters for cubic-bezier. */
    struct {
      float x1;     /**< First control point X */
      float y1;     /**< First control point Y */
      float x2;     /**< Second control point X */
      float y2;     /**< Second control point Y */
    } cubic_bezier; /**< Cubic bezier parameters */
    /** \brief Parameters for steps. */
    struct {
      int count;                           /**< Number of steps */
      enum ui_css_steps_position position; /**< Step position */
    } steps;                               /**< Step parameters */
  } data;                                  /**< The union data */
};

/**
 * @brief Represents a single CSS transition definition.
 */
struct ui_css_transition {
  char property_name[64];       /**< The transitioned property name. */
  struct ui_css_value duration; /**< The transition duration. */
  struct ui_css_easing_function timing_function; /**< The easing function. */
  struct ui_css_value delay;                     /**< The transition delay. */
  struct ui_css_transition
      *next; /**< Pointer to the next transition in the list. */
};

/**
 * @brief Parses a CSS easing function.
 *
 * @param str The string to parse.
 * @param out_easing Pointer to receive the parsed easing function.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_css_parse_easing_function(const char *str,
                             struct ui_css_easing_function *out_easing);

/**
 * @brief Parses a CSS transition list.
 *
 * @param str The string to parse.
 * @param out_transitions Pointer to receive the parsed transitions.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_transition(const char *str,
                                   struct ui_css_transition **out_transitions);

/**
 * @brief Destroys a CSS transition list.
 *
 * @param transitions Pointer to the transition list to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_transition_destroy(struct ui_css_transition *transitions);

/**
 * @brief CSS Animation Direction.
 */
enum ui_css_animation_direction {
  UI_CSS_ANIMATION_DIRECTION_NORMAL,           /**< Normal direction. */
  UI_CSS_ANIMATION_DIRECTION_REVERSE,          /**< Reverse direction. */
  UI_CSS_ANIMATION_DIRECTION_ALTERNATE,        /**< Alternate direction. */
  UI_CSS_ANIMATION_DIRECTION_ALTERNATE_REVERSE /**< Alternate reverse direction.
                                                */
};

/**
 * @brief CSS Animation Fill Mode.
 */
enum ui_css_animation_fill_mode {
  UI_CSS_ANIMATION_FILL_MODE_NONE,      /**< No fill mode. */
  UI_CSS_ANIMATION_FILL_MODE_FORWARDS,  /**< Forwards fill mode. */
  UI_CSS_ANIMATION_FILL_MODE_BACKWARDS, /**< Backwards fill mode. */
  UI_CSS_ANIMATION_FILL_MODE_BOTH /**< Both forwards and backwards fill mode. */
};

/**
 * @brief CSS Animation Play State.
 */
enum ui_css_animation_play_state {
  UI_CSS_ANIMATION_PLAY_STATE_RUNNING, /**< Animation is running. */
  UI_CSS_ANIMATION_PLAY_STATE_PAUSED   /**< Animation is paused. */
};

/**
 * @brief Represents a CSS Animation definition.
 */
struct ui_css_animation {
  char name[64];                /**< Animation keyframes name. */
  struct ui_css_value duration; /**< Animation duration. */
  struct ui_css_easing_function timing_function; /**< Easing function. */
  struct ui_css_value delay;                     /**< Animation delay. */
  float iteration_count; /**< Iterations (-1.0f for infinite). */
  enum ui_css_animation_direction direction;   /**< Animation direction. */
  enum ui_css_animation_fill_mode fill_mode;   /**< Animation fill mode. */
  enum ui_css_animation_play_state play_state; /**< Animation play state. */
  struct ui_css_animation *next; /**< Pointer to the next animation. */
};

/**
 * @brief Parses a CSS animation list.
 *
 * @param str The string to parse.
 * @param out_animations Pointer to receive the parsed animations.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_animation(const char *str,
                                  struct ui_css_animation **out_animations);

/**
 * @brief Destroys a CSS animation list.
 *
 * @param animations Pointer to the animation list to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_animation_destroy(struct ui_css_animation *animations);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_VALUES_H */
