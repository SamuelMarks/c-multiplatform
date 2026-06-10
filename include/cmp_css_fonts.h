#ifndef CMP_CSS_FONTS_H
#define CMP_CSS_FONTS_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_fonts.h
 * @brief CSS Fonts (Level 3 & 4) implementation.
 */

/**
 * @brief Represents font-style property.
 */
typedef enum cmp_font_style {
  CMP_FONT_STYLE_NORMAL = 0,
  CMP_FONT_STYLE_ITALIC,
  CMP_FONT_STYLE_OBLIQUE
} cmp_font_style_t;

/**
 * @brief Represents font-stretch property.
 */
typedef enum cmp_font_stretch {
  CMP_FONT_STRETCH_NORMAL = 0,
  CMP_FONT_STRETCH_ULTRA_CONDENSED,
  CMP_FONT_STRETCH_EXTRA_CONDENSED,
  CMP_FONT_STRETCH_CONDENSED,
  CMP_FONT_STRETCH_SEMI_CONDENSED,
  CMP_FONT_STRETCH_SEMI_EXPANDED,
  CMP_FONT_STRETCH_EXPANDED,
  CMP_FONT_STRETCH_EXTRA_EXPANDED,
  CMP_FONT_STRETCH_ULTRA_EXPANDED,
  CMP_FONT_STRETCH_PERCENTAGE /* For exact percentage value */
} cmp_font_stretch_t;

/**
 * @brief Represents font basic properties group.
 */
typedef struct cmp_prop_font_basic_group {
  char **font_family; /**< Array of font family strings, NULL-terminated */
  int font_weight;    /**< E.g., 400 for normal, 700 for bold */
  cmp_font_style_t font_style;
  cmp_font_stretch_t font_stretch;
  float font_stretch_percent; /**< Valid if font_stretch == PERCENTAGE */
  char *font_size;            /**< String representation of font-size */
} cmp_prop_font_basic_group_t;

/**
 * @brief Represents font-size-adjust property.
 */
typedef struct cmp_prop_font_size_adjust {
  char *size_adjust; /**< String representation, e.g., "0.5", "none", "ex-height
                        0.5" */
} cmp_prop_font_size_adjust_t;

/**
 * @brief Represents font-variant-caps property.
 */
typedef enum cmp_font_variant_caps {
  CMP_FONT_VARIANT_CAPS_NORMAL = 0,
  CMP_FONT_VARIANT_CAPS_SMALL_CAPS,
  CMP_FONT_VARIANT_CAPS_ALL_SMALL_CAPS,
  CMP_FONT_VARIANT_CAPS_PETITE_CAPS,
  CMP_FONT_VARIANT_CAPS_ALL_PETITE_CAPS,
  CMP_FONT_VARIANT_CAPS_UNICASE,
  CMP_FONT_VARIANT_CAPS_TITLING_CAPS
} cmp_font_variant_caps_t;

/**
 * @brief Represents font-variant-numeric property.
 */
typedef enum cmp_font_variant_numeric {
  CMP_FONT_VARIANT_NUMERIC_NORMAL = 0,
  CMP_FONT_VARIANT_NUMERIC_LINING_NUMS = 1,
  CMP_FONT_VARIANT_NUMERIC_OLDSTYLE_NUMS = 2,
  CMP_FONT_VARIANT_NUMERIC_PROPORTIONAL_NUMS = 4,
  CMP_FONT_VARIANT_NUMERIC_TABULAR_NUMS = 8,
  CMP_FONT_VARIANT_NUMERIC_DIAGONAL_FRACTIONS = 16,
  CMP_FONT_VARIANT_NUMERIC_STACKED_FRACTIONS = 32,
  CMP_FONT_VARIANT_NUMERIC_ORDINAL = 64,
  CMP_FONT_VARIANT_NUMERIC_SLASHED_ZERO = 128
} cmp_font_variant_numeric_t;

/**
 * @brief Represents font-variant-east-asian property.
 */
typedef enum cmp_font_variant_east_asian {
  CMP_FONT_VARIANT_EAST_ASIAN_NORMAL = 0,
  CMP_FONT_VARIANT_EAST_ASIAN_JIS78 = 1,
  CMP_FONT_VARIANT_EAST_ASIAN_JIS83 = 2,
  CMP_FONT_VARIANT_EAST_ASIAN_JIS90 = 4,
  CMP_FONT_VARIANT_EAST_ASIAN_JIS04 = 8,
  CMP_FONT_VARIANT_EAST_ASIAN_SIMPLIFIED = 16,
  CMP_FONT_VARIANT_EAST_ASIAN_TRADITIONAL = 32,
  CMP_FONT_VARIANT_EAST_ASIAN_FULL_WIDTH = 64,
  CMP_FONT_VARIANT_EAST_ASIAN_PROPORTIONAL_WIDTH = 128,
  CMP_FONT_VARIANT_EAST_ASIAN_RUBY = 256
} cmp_font_variant_east_asian_t;

/**
 * @brief Represents font-variant-ligatures property.
 */
typedef enum cmp_font_variant_ligatures {
  CMP_FONT_VARIANT_LIGATURES_NORMAL = 0,
  CMP_FONT_VARIANT_LIGATURES_NONE = 1,
  CMP_FONT_VARIANT_LIGATURES_COMMON = 2,
  CMP_FONT_VARIANT_LIGATURES_NO_COMMON = 4,
  CMP_FONT_VARIANT_LIGATURES_DISCRETIONARY = 8,
  CMP_FONT_VARIANT_LIGATURES_NO_DISCRETIONARY = 16,
  CMP_FONT_VARIANT_LIGATURES_HISTORICAL = 32,
  CMP_FONT_VARIANT_LIGATURES_NO_HISTORICAL = 64,
  CMP_FONT_VARIANT_LIGATURES_CONTEXTUAL = 128,
  CMP_FONT_VARIANT_LIGATURES_NO_CONTEXTUAL = 256
} cmp_font_variant_ligatures_t;

/**
 * @brief Represents font-variant basic group.
 */
typedef struct cmp_prop_font_variant_group {
  cmp_font_variant_caps_t caps;
  int numeric;    /**< Bitmask of cmp_font_variant_numeric_t */
  int east_asian; /**< Bitmask of cmp_font_variant_east_asian_t */
  int ligatures;  /**< Bitmask of cmp_font_variant_ligatures_t */
} cmp_prop_font_variant_group_t;

/**
 * @brief Represents font-variant-position property.
 */
typedef enum cmp_font_variant_position {
  CMP_FONT_VARIANT_POSITION_NORMAL = 0,
  CMP_FONT_VARIANT_POSITION_SUB,
  CMP_FONT_VARIANT_POSITION_SUPER
} cmp_font_variant_position_t;

/**
 * @brief Represents font-variant advanced properties.
 */
typedef struct cmp_prop_font_variant_advanced {
  char *alternates; /**< String representation of font-variant-alternates */
  cmp_font_variant_position_t position;
} cmp_prop_font_variant_advanced_t;

/**
 * @brief Represents font feature and variation settings.
 */
typedef struct cmp_prop_font_settings_group {
  char *feature_settings; /**< String representation of font-feature-settings */
  char *variation_settings; /**< String representation of
                               font-variation-settings */
} cmp_prop_font_settings_group_t;

/**
 * @brief Represents font-synthesis property flags.
 */
typedef enum cmp_font_synthesis {
  CMP_FONT_SYNTHESIS_NONE = 0,
  CMP_FONT_SYNTHESIS_WEIGHT = 1,
  CMP_FONT_SYNTHESIS_STYLE = 2,
  CMP_FONT_SYNTHESIS_SMALL_CAPS = 4,
  CMP_FONT_SYNTHESIS_POSITION = 8
} cmp_font_synthesis_t;

/**
 * @brief Represents font-synthesis properties.
 */
typedef struct cmp_prop_font_synthesis_group {
  int synthesis; /**< Bitmask of cmp_font_synthesis_t */
} cmp_prop_font_synthesis_group_t;

/**
 * @brief Represents font-kerning property.
 */
typedef enum cmp_font_kerning {
  CMP_FONT_KERNING_AUTO = 0,
  CMP_FONT_KERNING_NORMAL,
  CMP_FONT_KERNING_NONE
} cmp_font_kerning_t;

/**
 * @brief Represents font-kerning.
 */
typedef struct cmp_prop_font_kerning {
  cmp_font_kerning_t kerning;
} cmp_prop_font_kerning_t;

/**
 * @brief Represents font-language-override property.
 */
typedef struct cmp_prop_font_language_override {
  char *language_override; /**< String representation, e.g., "normal", "ENG" */
} cmp_prop_font_language_override_t;

/**
 * @brief Represents font-palette property.
 */
typedef struct cmp_prop_font_palette {
  char *palette; /**< String representation, e.g., "normal", "light", "dark",
                    "--custom" */
} cmp_prop_font_palette_t;

/**
 * @brief Represents @font-face rule.
 */
typedef struct cmp_at_rule_font_face {
  char *font_family;
  char **src; /**< Array of source URLs/formats, NULL-terminated */
  char *font_weight;
  char *font_style;
  char *font_display;
  char *unicode_range;
} cmp_at_rule_font_face_t;

/**
 * @brief Represents @font-feature-values and @font-palette-values (abstracted
 * as a string blob for now).
 */
typedef struct cmp_at_rule_font_advanced {
  char *font_family;
  char *values_blob;
} cmp_at_rule_font_advanced_t;

/**
 * @brief Represents initial-letter property.
 */
typedef struct cmp_prop_initial_letter {
  char *initial_letter; /**< String representation, e.g., "normal", "3 2" */
} cmp_prop_initial_letter_t;

/**
 * @brief Represents initial-letter-align property.
 */
typedef enum cmp_initial_letter_align {
  CMP_INITIAL_LETTER_ALIGN_AUTO = 0,
  CMP_INITIAL_LETTER_ALIGN_ALPHABETIC,
  CMP_INITIAL_LETTER_ALIGN_HANGING,
  CMP_INITIAL_LETTER_ALIGN_IDEOGRAPHIC
} cmp_initial_letter_align_t;

/**
 * @brief Represents initial-letter-align.
 */
typedef struct cmp_prop_initial_letter_align {
  cmp_initial_letter_align_t align;
} cmp_prop_initial_letter_align_t;

/**
 * @brief Represents font-smoothing property.
 */
typedef enum cmp_font_smoothing {
  CMP_FONT_SMOOTHING_AUTO = 0,
  CMP_FONT_SMOOTHING_NONE,
  CMP_FONT_SMOOTHING_ANTIALIASED,
  CMP_FONT_SMOOTHING_SUBPIXEL_ANTIALIASED
} cmp_font_smoothing_t;

/**
 * @brief Represents font-smoothing.
 */
typedef struct cmp_prop_font_smoothing {
  cmp_font_smoothing_t smoothing;
} cmp_prop_font_smoothing_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a font basic group.
 * @param group The group to initialize.
 * @param family Array of font family strings. Must be NULL-terminated.
 * @param weight font-weight.
 * @param style font-style.
 * @param stretch font-stretch.
 * @param stretch_percent valid if stretch is CMP_FONT_STRETCH_PERCENTAGE.
 * @param size font-size string.
 * @return 0 on success.
 */
int cmp_prop_font_basic_group_init(cmp_prop_font_basic_group_t *group,
                                   const char **family, int weight,
                                   cmp_font_style_t style,
                                   cmp_font_stretch_t stretch,
                                   float stretch_percent, const char *size);

/**
 * @brief Frees a font basic group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_font_basic_group_free(cmp_prop_font_basic_group_t *group);

/**
 * @brief Initializes a font-size-adjust property.
 * @param prop The property to initialize.
 * @param size_adjust The size adjust string.
 * @return 0 on success.
 */
int cmp_prop_font_size_adjust_init(cmp_prop_font_size_adjust_t *prop,
                                   const char *size_adjust);

/**
 * @brief Frees a font-size-adjust property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_font_size_adjust_free(cmp_prop_font_size_adjust_t *prop);

/**
 * @brief Initializes a font-variant group.
 * @param group The group to initialize.
 * @param caps caps value.
 * @param numeric numeric bitmask.
 * @param east_asian east_asian bitmask.
 * @param ligatures ligatures bitmask.
 * @return 0 on success.
 */
int cmp_prop_font_variant_group_init(cmp_prop_font_variant_group_t *group,
                                     cmp_font_variant_caps_t caps, int numeric,
                                     int east_asian, int ligatures);

/**
 * @brief Initializes a font-variant advanced property.
 * @param adv The advanced property to initialize.
 * @param alternates alternates string.
 * @param position position value.
 * @return 0 on success.
 */
int cmp_prop_font_variant_advanced_init(cmp_prop_font_variant_advanced_t *adv,
                                        const char *alternates,
                                        cmp_font_variant_position_t position);

/**
 * @brief Frees a font-variant advanced property.
 * @param adv The property to free.
 * @return 0 on success.
 */
int cmp_prop_font_variant_advanced_free(cmp_prop_font_variant_advanced_t *adv);

/**
 * @brief Initializes a font settings group.
 * @param group The group to initialize.
 * @param feature feature settings string.
 * @param variation variation settings string.
 * @return 0 on success.
 */
int cmp_prop_font_settings_group_init(cmp_prop_font_settings_group_t *group,
                                      const char *feature,
                                      const char *variation);

/**
 * @brief Frees a font settings group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_font_settings_group_free(cmp_prop_font_settings_group_t *group);

/**
 * @brief Initializes a font-synthesis group.
 * @param group The group to initialize.
 * @param synthesis bitmask of synthesis flags.
 * @return 0 on success.
 */
int cmp_prop_font_synthesis_group_init(cmp_prop_font_synthesis_group_t *group,
                                       int synthesis);

/**
 * @brief Initializes a font-kerning property.
 * @param prop The property to initialize.
 * @param kerning kerning value.
 * @return 0 on success.
 */
int cmp_prop_font_kerning_init(cmp_prop_font_kerning_t *prop,
                               cmp_font_kerning_t kerning);

/**
 * @brief Initializes a font-language-override property.
 * @param prop The property to initialize.
 * @param override override string.
 * @return 0 on success.
 */
int cmp_prop_font_language_override_init(
    cmp_prop_font_language_override_t *prop, const char *override);

/**
 * @brief Frees a font-language-override property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_font_language_override_free(
    cmp_prop_font_language_override_t *prop);

/**
 * @brief Initializes a font-palette property.
 * @param prop The property to initialize.
 * @param palette palette string.
 * @return 0 on success.
 */
int cmp_prop_font_palette_init(cmp_prop_font_palette_t *prop,
                               const char *palette);

/**
 * @brief Frees a font-palette property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_font_palette_free(cmp_prop_font_palette_t *prop);

/**
 * @brief Initializes an @font-face rule.
 * @param rule The rule to initialize.
 * @param family font-family string.
 * @param src NULL-terminated array of source strings.
 * @param weight font-weight string.
 * @param style font-style string.
 * @param display font-display string.
 * @param range unicode-range string.
 * @return 0 on success.
 */
int cmp_at_rule_font_face_init(cmp_at_rule_font_face_t *rule,
                               const char *family, const char **src,
                               const char *weight, const char *style,
                               const char *display, const char *range);

/**
 * @brief Frees an @font-face rule.
 * @param rule The rule to free.
 * @return 0 on success.
 */
int cmp_at_rule_font_face_free(cmp_at_rule_font_face_t *rule);

/**
 * @brief Initializes an advanced font at-rule.
 * @param rule The rule to initialize.
 * @param family font-family string.
 * @param blob values blob.
 * @return 0 on success.
 */
int cmp_at_rule_font_advanced_init(cmp_at_rule_font_advanced_t *rule,
                                   const char *family, const char *blob);

/**
 * @brief Frees an advanced font at-rule.
 * @param rule The rule to free.
 * @return 0 on success.
 */
int cmp_at_rule_font_advanced_free(cmp_at_rule_font_advanced_t *rule);

/**
 * @brief Initializes an initial-letter property.
 * @param prop The property to initialize.
 * @param letter initial letter string.
 * @return 0 on success.
 */
int cmp_prop_initial_letter_init(cmp_prop_initial_letter_t *prop,
                                 const char *letter);

/**
 * @brief Frees an initial-letter property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_initial_letter_free(cmp_prop_initial_letter_t *prop);

/**
 * @brief Initializes an initial-letter-align property.
 * @param prop The property to initialize.
 * @param align align value.
 * @return 0 on success.
 */
int cmp_prop_initial_letter_align_init(cmp_prop_initial_letter_align_t *prop,
                                       cmp_initial_letter_align_t align);

/**
 * @brief Initializes a font-smoothing property.
 * @param prop The property to initialize.
 * @param smoothing smoothing value.
 * @return 0 on success.
 */
int cmp_prop_font_smoothing_init(cmp_prop_font_smoothing_t *prop,
                                 cmp_font_smoothing_t smoothing);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_FONTS_H */
