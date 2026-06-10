#ifndef CMP_CSS_COLOR_H
#define CMP_CSS_COLOR_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_color.h
 * @brief CSS Color Module Level 4 & 5 implementation.
 */

/**
 * @brief Represents a legacy CSS color (`rgb()`, `rgba()`, `hsl()`, `hsla()`,
 * `<named-color>`, `<hex-color>`).
 */
typedef struct cmp_color_legacy {
  float r;    /**< Red component (0.0 to 255.0) or Hue */
  float g;    /**< Green component (0.0 to 255.0) or Saturation */
  float b;    /**< Blue component (0.0 to 255.0) or Lightness */
  float a;    /**< Alpha component (0.0 to 1.0) */
  int is_hsl; /**< Non-zero if the values represent HSL instead of RGB */
} cmp_color_legacy_t;

/**
 * @brief Modern CSS color functions (`hwb()`, `lab()`, `lch()`, `oklab()`,
 * `oklch()`).
 */
typedef enum cmp_color_modern_type {
  CMP_COLOR_MODERN_HWB = 0,
  CMP_COLOR_MODERN_LAB,
  CMP_COLOR_MODERN_LCH,
  CMP_COLOR_MODERN_OKLAB,
  CMP_COLOR_MODERN_OKLCH
} cmp_color_modern_type_t;

typedef struct cmp_color_modern {
  cmp_color_modern_type_t type;
  float c1;    /**< Component 1 (e.g. H, L, L) */
  float c2;    /**< Component 2 (e.g. W, a, C) */
  float c3;    /**< Component 3 (e.g. B, b, h) */
  float alpha; /**< Alpha component (0.0 to 1.0) */
} cmp_color_modern_t;

/**
 * @brief Color spaces for `color()` function.
 */
typedef enum cmp_color_space_type {
  CMP_COLOR_SPACE_SRGB = 0,
  CMP_COLOR_SPACE_SRGB_LINEAR,
  CMP_COLOR_SPACE_DISPLAY_P3,
  CMP_COLOR_SPACE_A98_RGB,
  CMP_COLOR_SPACE_PROPHOTO_RGB,
  CMP_COLOR_SPACE_REC2020,
  CMP_COLOR_SPACE_XYZ,
  CMP_COLOR_SPACE_XYZ_D50,
  CMP_COLOR_SPACE_XYZ_D65
} cmp_color_space_type_t;

typedef struct cmp_color_space {
  cmp_color_space_type_t space;
  float p1;    /**< Param 1 */
  float p2;    /**< Param 2 */
  float p3;    /**< Param 3 */
  float alpha; /**< Alpha component (0.0 to 1.0) */
} cmp_color_space_t;

/**
 * @brief Device CMYK color.
 */
typedef struct cmp_color_cmyk {
  float c;              /**< Cyan */
  float m;              /**< Magenta */
  float y;              /**< Yellow */
  float k;              /**< Black */
  float alpha;          /**< Alpha */
  char *fallback_color; /**< Optional fallback color string */
} cmp_color_cmyk_t;

/**
 * @brief High-level type for a CSS Color.
 */
typedef enum cmp_prop_color_type {
  CMP_PROP_COLOR_CURRENTCOLOR = 0,
  CMP_PROP_COLOR_TRANSPARENT,
  CMP_PROP_COLOR_LEGACY,
  CMP_PROP_COLOR_MODERN,
  CMP_PROP_COLOR_SPACE,
  CMP_PROP_COLOR_CMYK,
  CMP_PROP_COLOR_SYSTEM, /* System color name stored as string */
  CMP_PROP_COLOR_MIX,
  CMP_PROP_COLOR_CONTRAST,
  CMP_PROP_COLOR_RELATIVE
} cmp_prop_color_type_t;

/**
 * @brief Represents `color-interpolation-method`.
 */
typedef enum cmp_color_interpolation {
  CMP_COLOR_INTERP_SRGB = 0,
  CMP_COLOR_INTERP_SRGB_LINEAR,
  CMP_COLOR_INTERP_LAB,
  CMP_COLOR_INTERP_OKLAB,
  CMP_COLOR_INTERP_XYZ,
  CMP_COLOR_INTERP_XYZ_D50,
  CMP_COLOR_INTERP_XYZ_D65,
  CMP_COLOR_INTERP_HSL,
  CMP_COLOR_INTERP_HWB,
  CMP_COLOR_INTERP_LCH,
  CMP_COLOR_INTERP_OKLCH
} cmp_color_interpolation_t;

/**
 * @brief Represents a full CSS color property.
 * Includes support for mix(), contrast(), relative colors, etc via
 * recursive/string pointers for simplicity in AST.
 */
typedef struct cmp_prop_color {
  cmp_prop_color_type_t type;
  union {
    cmp_color_legacy_t legacy;
    cmp_color_modern_t modern;
    cmp_color_space_t space;
    cmp_color_cmyk_t cmyk;
    char *system_color; /**< Name of the system color, e.g. "WindowText" */
    char *complex_func; /**< String representation for color-mix,
                           color-contrast, or relative colors */
  } value;
} cmp_prop_color_t;

/**
 * @brief Represents `color-scheme` property.
 */
typedef enum cmp_prop_color_scheme {
  CMP_COLOR_SCHEME_NORMAL = 0,
  CMP_COLOR_SCHEME_LIGHT,
  CMP_COLOR_SCHEME_DARK,
  CMP_COLOR_SCHEME_LIGHT_DARK,
  CMP_COLOR_SCHEME_ONLY_LIGHT
} cmp_prop_color_scheme_t;

/**
 * @brief Represents `opacity` property.
 */
typedef struct cmp_prop_opacity {
  float value; /**< 0.0 to 1.0 */
} cmp_prop_opacity_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a color property to `currentcolor`.
 * @param color The color property.
 * @return 0 on success.
 */
int cmp_prop_color_init_currentcolor(cmp_prop_color_t *color);

/**
 * @brief Initializes a color property to `transparent`.
 * @param color The color property.
 * @return 0 on success.
 */
int cmp_prop_color_init_transparent(cmp_prop_color_t *color);

/**
 * @brief Initializes a color property to a legacy RGB/RGBA value.
 * @param color The color property.
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 * @param a Alpha component (0.0-1.0).
 * @return 0 on success.
 */
int cmp_prop_color_init_rgba(cmp_prop_color_t *color, float r, float g, float b,
                             float a);

/**
 * @brief Initializes a color property to a legacy HSL/HSLA value.
 * @param color The color property.
 * @param h Hue component.
 * @param s Saturation component.
 * @param l Lightness component.
 * @param a Alpha component (0.0-1.0).
 * @return 0 on success.
 */
int cmp_prop_color_init_hsla(cmp_prop_color_t *color, float h, float s, float l,
                             float a);

/**
 * @brief Initializes a color property to a modern color function (hwb, lab,
 * lch, oklab, oklch).
 * @param color The color property.
 * @param modern_type The specific modern color function.
 * @param c1 Component 1.
 * @param c2 Component 2.
 * @param c3 Component 3.
 * @param a Alpha component (0.0-1.0).
 * @return 0 on success.
 */
int cmp_prop_color_init_modern(cmp_prop_color_t *color,
                               cmp_color_modern_type_t modern_type, float c1,
                               float c2, float c3, float a);

/**
 * @brief Initializes a color property to a specific color space (`color()`).
 * @param color The color property.
 * @param space_type The color space type.
 * @param p1 Parameter 1.
 * @param p2 Parameter 2.
 * @param p3 Parameter 3.
 * @param a Alpha component (0.0-1.0).
 * @return 0 on success.
 */
int cmp_prop_color_init_space(cmp_prop_color_t *color,
                              cmp_color_space_type_t space_type, float p1,
                              float p2, float p3, float a);

/**
 * @brief Initializes a color property to a system color string.
 * @param color The color property.
 * @param sys_color The system color name (e.g., "WindowText").
 * @return 0 on success.
 */
int cmp_prop_color_init_system(cmp_prop_color_t *color, const char *sys_color);

/**
 * @brief Initializes a color property to a complex function (color-mix,
 * contrast, relative).
 * @param color The color property.
 * @param func_type The complex type.
 * @param func_string The full function string representation.
 * @return 0 on success.
 */
int cmp_prop_color_init_complex(cmp_prop_color_t *color,
                                cmp_prop_color_type_t func_type,
                                const char *func_string);

/**
 * @brief Initializes a CMYK color property.
 * @param color The color property.
 * @param c Cyan.
 * @param m Magenta.
 * @param y Yellow.
 * @param k Black.
 * @param a Alpha.
 * @param fallback Optional fallback color string.
 * @return 0 on success.
 */
int cmp_prop_color_init_cmyk(cmp_prop_color_t *color, float c, float m, float y,
                             float k, float a, const char *fallback);

/**
 * @brief Frees dynamically allocated resources inside a color property.
 * @param color The color property.
 * @return 0 on success.
 */
int cmp_prop_color_free(cmp_prop_color_t *color);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_COLOR_H */