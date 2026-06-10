#ifndef CMP_CSS_TYPOGRAPHY_H
#define CMP_CSS_TYPOGRAPHY_H

/* clang-format off */
#include <stddef.h>
#include "cmp_css_color.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_typography.h
 * @brief CSS Typography and Text properties implementation.
 */

/**
 * @brief Represents text-align values.
 */
typedef enum cmp_text_align {
  CMP_TEXT_ALIGN_START = 0,
  CMP_TEXT_ALIGN_END,
  CMP_TEXT_ALIGN_LEFT,
  CMP_TEXT_ALIGN_RIGHT,
  CMP_TEXT_ALIGN_CENTER,
  CMP_TEXT_ALIGN_JUSTIFY,
  CMP_TEXT_ALIGN_MATCH_PARENT
} cmp_text_align_t;

/**
 * @brief Represents text-align and text-align-last properties.
 */
typedef struct cmp_prop_text_align_group {
  cmp_text_align_t text_align;
  cmp_text_align_t text_align_last;
} cmp_prop_text_align_group_t;

/**
 * @brief Represents text-justify property.
 */
typedef enum cmp_text_justify {
  CMP_TEXT_JUSTIFY_AUTO = 0,
  CMP_TEXT_JUSTIFY_INTER_CHARACTER,
  CMP_TEXT_JUSTIFY_INTER_WORD,
  CMP_TEXT_JUSTIFY_NONE
} cmp_text_justify_t;

/**
 * @brief Represents text-justify.
 */
typedef struct cmp_prop_text_justify {
  cmp_text_justify_t justify;
} cmp_prop_text_justify_t;

/**
 * @brief Represents text-indent property.
 */
typedef struct cmp_prop_text_indent {
  char *indent; /**< Indent string value */
} cmp_prop_text_indent_t;

/**
 * @brief Represents letter-spacing and word-spacing properties.
 */
typedef struct cmp_prop_spacing {
  char *letter_spacing; /**< letter-spacing string value */
  char *word_spacing;   /**< word-spacing string value */
} cmp_prop_spacing_t;

/**
 * @brief Represents line-height property.
 */
typedef struct cmp_prop_line_height {
  char *line_height; /**< line-height string value */
} cmp_prop_line_height_t;

/**
 * @brief Represents tab-size property.
 */
typedef struct cmp_prop_tab_size {
  char *tab_size; /**< tab-size string value */
} cmp_prop_tab_size_t;

/**
 * @brief Represents white-space property.
 */
typedef enum cmp_white_space {
  CMP_WHITE_SPACE_NORMAL = 0,
  CMP_WHITE_SPACE_NOWRAP,
  CMP_WHITE_SPACE_PRE,
  CMP_WHITE_SPACE_PRE_WRAP,
  CMP_WHITE_SPACE_PRE_LINE,
  CMP_WHITE_SPACE_BREAK_SPACES
} cmp_white_space_t;

/**
 * @brief Represents white-space property.
 */
typedef struct cmp_prop_white_space {
  cmp_white_space_t white_space;
} cmp_prop_white_space_t;

/**
 * @brief Represents word-break property.
 */
typedef enum cmp_word_break {
  CMP_WORD_BREAK_NORMAL = 0,
  CMP_WORD_BREAK_BREAK_ALL,
  CMP_WORD_BREAK_KEEP_ALL,
  CMP_WORD_BREAK_BREAK_WORD
} cmp_word_break_t;

/**
 * @brief Represents line-break property.
 */
typedef enum cmp_line_break {
  CMP_LINE_BREAK_AUTO = 0,
  CMP_LINE_BREAK_LOOSE,
  CMP_LINE_BREAK_NORMAL,
  CMP_LINE_BREAK_STRICT,
  CMP_LINE_BREAK_ANYWHERE
} cmp_line_break_t;

/**
 * @brief Represents overflow-wrap and word-wrap property.
 */
typedef enum cmp_overflow_wrap {
  CMP_OVERFLOW_WRAP_NORMAL = 0,
  CMP_OVERFLOW_WRAP_BREAK_WORD,
  CMP_OVERFLOW_WRAP_ANYWHERE
} cmp_overflow_wrap_t;

/**
 * @brief Represents word-break, line-break, overflow-wrap, word-wrap
 * properties.
 */
typedef struct cmp_prop_breaking_group {
  cmp_word_break_t word_break;
  cmp_line_break_t line_break;
  cmp_overflow_wrap_t overflow_wrap;
  cmp_overflow_wrap_t word_wrap;
} cmp_prop_breaking_group_t;

/**
 * @brief Represents hyphens property.
 */
typedef enum cmp_hyphens {
  CMP_HYPHENS_NONE = 0,
  CMP_HYPHENS_MANUAL,
  CMP_HYPHENS_AUTO
} cmp_hyphens_t;

/**
 * @brief Represents hyphens property.
 */
typedef struct cmp_prop_hyphens {
  cmp_hyphens_t hyphens;
} cmp_prop_hyphens_t;

/**
 * @brief Represents line-clamp property.
 */
typedef struct cmp_prop_line_clamp {
  char *line_clamp; /**< line-clamp string value */
} cmp_prop_line_clamp_t;

/**
 * @brief Represents hanging-punctuation property.
 */
typedef enum cmp_hanging_punctuation {
  CMP_HANGING_PUNCTUATION_NONE = 0,
  CMP_HANGING_PUNCTUATION_FIRST,
  CMP_HANGING_PUNCTUATION_LAST,
  CMP_HANGING_PUNCTUATION_ALLOW_END,
  CMP_HANGING_PUNCTUATION_FORCE_END
} cmp_hanging_punctuation_t;

/**
 * @brief Represents hanging-punctuation.
 */
typedef struct cmp_prop_hanging_punctuation {
  cmp_hanging_punctuation_t hanging_punctuation;
} cmp_prop_hanging_punctuation_t;

/**
 * @brief Represents text-decoration-line property.
 */
typedef enum cmp_text_decoration_line {
  CMP_TEXT_DECORATION_LINE_NONE = 0,
  CMP_TEXT_DECORATION_LINE_UNDERLINE = 1,
  CMP_TEXT_DECORATION_LINE_OVERLINE = 2,
  CMP_TEXT_DECORATION_LINE_LINE_THROUGH = 4,
  CMP_TEXT_DECORATION_LINE_BLINK = 8
} cmp_text_decoration_line_t;

/**
 * @brief Represents text-decoration-style property.
 */
typedef enum cmp_text_decoration_style {
  CMP_TEXT_DECORATION_STYLE_SOLID = 0,
  CMP_TEXT_DECORATION_STYLE_DOUBLE,
  CMP_TEXT_DECORATION_STYLE_DOTTED,
  CMP_TEXT_DECORATION_STYLE_DASHED,
  CMP_TEXT_DECORATION_STYLE_WAVY
} cmp_text_decoration_style_t;

/**
 * @brief Represents text-decoration properties.
 */
typedef struct cmp_prop_text_decoration_group {
  int text_decoration_line; /**< Bitmask of cmp_text_decoration_line_t */
  cmp_prop_color_t text_decoration_color;
  cmp_text_decoration_style_t text_decoration_style;
  char *text_decoration_thickness;
} cmp_prop_text_decoration_group_t;

/**
 * @brief Represents text-underline-position property.
 */
typedef enum cmp_text_underline_position {
  CMP_TEXT_UNDERLINE_POSITION_AUTO = 0,
  CMP_TEXT_UNDERLINE_POSITION_UNDER,
  CMP_TEXT_UNDERLINE_POSITION_LEFT,
  CMP_TEXT_UNDERLINE_POSITION_RIGHT
} cmp_text_underline_position_t;

/**
 * @brief Represents text-decoration-skip-ink property.
 */
typedef enum cmp_text_decoration_skip_ink {
  CMP_TEXT_DECORATION_SKIP_INK_AUTO = 0,
  CMP_TEXT_DECORATION_SKIP_INK_NONE,
  CMP_TEXT_DECORATION_SKIP_INK_ALL
} cmp_text_decoration_skip_ink_t;

/**
 * @brief Represents text-underline properties.
 */
typedef struct cmp_prop_text_underline_group {
  cmp_text_underline_position_t text_underline_position;
  char *text_underline_offset;
  cmp_text_decoration_skip_ink_t text_decoration_skip_ink;
} cmp_prop_text_underline_group_t;

/**
 * @brief Represents text-shadow property.
 */
typedef struct cmp_prop_text_shadow {
  char *text_shadow; /**< text-shadow string value */
} cmp_prop_text_shadow_t;

/**
 * @brief Represents text-transform property.
 */
typedef enum cmp_text_transform {
  CMP_TEXT_TRANSFORM_NONE = 0,
  CMP_TEXT_TRANSFORM_CAPITALIZE,
  CMP_TEXT_TRANSFORM_UPPERCASE,
  CMP_TEXT_TRANSFORM_LOWERCASE,
  CMP_TEXT_TRANSFORM_FULL_WIDTH,
  CMP_TEXT_TRANSFORM_FULL_SIZE_KANA
} cmp_text_transform_t;

/**
 * @brief Represents text-transform property.
 */
typedef struct cmp_prop_text_transform {
  cmp_text_transform_t transform;
} cmp_prop_text_transform_t;

/**
 * @brief Represents text-emphasis-style property.
 */
typedef enum cmp_text_emphasis_style {
  CMP_TEXT_EMPHASIS_STYLE_NONE = 0,
  CMP_TEXT_EMPHASIS_STYLE_FILLED,
  CMP_TEXT_EMPHASIS_STYLE_OPEN,
  CMP_TEXT_EMPHASIS_STYLE_DOT,
  CMP_TEXT_EMPHASIS_STYLE_CIRCLE,
  CMP_TEXT_EMPHASIS_STYLE_DOUBLE_CIRCLE,
  CMP_TEXT_EMPHASIS_STYLE_TRIANGLE,
  CMP_TEXT_EMPHASIS_STYLE_SESAME,
  CMP_TEXT_EMPHASIS_STYLE_STRING
} cmp_text_emphasis_style_t;

/**
 * @brief Represents text-emphasis-position property.
 */
typedef enum cmp_text_emphasis_position {
  CMP_TEXT_EMPHASIS_POSITION_OVER_RIGHT = 0,
  CMP_TEXT_EMPHASIS_POSITION_OVER_LEFT,
  CMP_TEXT_EMPHASIS_POSITION_UNDER_RIGHT,
  CMP_TEXT_EMPHASIS_POSITION_UNDER_LEFT
} cmp_text_emphasis_position_t;

/**
 * @brief Represents text-emphasis properties.
 */
typedef struct cmp_prop_text_emphasis_group {
  cmp_prop_color_t text_emphasis_color;
  cmp_text_emphasis_style_t text_emphasis_style;
  char *text_emphasis_string; /**< If style is string, the custom string */
  cmp_text_emphasis_position_t text_emphasis_position;
} cmp_prop_text_emphasis_group_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a text-align group.
 * @param group The group to initialize.
 * @param align text-align value.
 * @param align_last text-align-last value.
 * @return 0 on success.
 */
int cmp_prop_text_align_group_init(cmp_prop_text_align_group_t *group,
                                   cmp_text_align_t align,
                                   cmp_text_align_t align_last);

/**
 * @brief Initializes a text-justify property.
 * @param prop The property to initialize.
 * @param justify text-justify value.
 * @return 0 on success.
 */
int cmp_prop_text_justify_init(cmp_prop_text_justify_t *prop,
                               cmp_text_justify_t justify);

/**
 * @brief Initializes a text-indent property.
 * @param prop The property to initialize.
 * @param indent text-indent string value.
 * @return 0 on success.
 */
int cmp_prop_text_indent_init(cmp_prop_text_indent_t *prop, const char *indent);

/**
 * @brief Frees resources in a text-indent property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_text_indent_free(cmp_prop_text_indent_t *prop);

/**
 * @brief Initializes a spacing property.
 * @param prop The property to initialize.
 * @param letter_spacing letter-spacing string value.
 * @param word_spacing word-spacing string value.
 * @return 0 on success.
 */
int cmp_prop_spacing_init(cmp_prop_spacing_t *prop, const char *letter_spacing,
                          const char *word_spacing);

/**
 * @brief Frees resources in a spacing property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_spacing_free(cmp_prop_spacing_t *prop);

/**
 * @brief Initializes a line-height property.
 * @param prop The property to initialize.
 * @param line_height line-height string value.
 * @return 0 on success.
 */
int cmp_prop_line_height_init(cmp_prop_line_height_t *prop,
                              const char *line_height);

/**
 * @brief Frees resources in a line-height property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_line_height_free(cmp_prop_line_height_t *prop);

/**
 * @brief Initializes a tab-size property.
 * @param prop The property to initialize.
 * @param tab_size tab-size string value.
 * @return 0 on success.
 */
int cmp_prop_tab_size_init(cmp_prop_tab_size_t *prop, const char *tab_size);

/**
 * @brief Frees resources in a tab-size property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_tab_size_free(cmp_prop_tab_size_t *prop);

/**
 * @brief Initializes a white-space property.
 * @param prop The property to initialize.
 * @param white_space white-space value.
 * @return 0 on success.
 */
int cmp_prop_white_space_init(cmp_prop_white_space_t *prop,
                              cmp_white_space_t white_space);

/**
 * @brief Initializes a breaking group property.
 * @param group The group to initialize.
 * @param word_break word-break value.
 * @param line_break line-break value.
 * @param overflow_wrap overflow-wrap value.
 * @param word_wrap word-wrap value.
 * @return 0 on success.
 */
int cmp_prop_breaking_group_init(cmp_prop_breaking_group_t *group,
                                 cmp_word_break_t word_break,
                                 cmp_line_break_t line_break,
                                 cmp_overflow_wrap_t overflow_wrap,
                                 cmp_overflow_wrap_t word_wrap);

/**
 * @brief Initializes a hyphens property.
 * @param prop The property to initialize.
 * @param hyphens hyphens value.
 * @return 0 on success.
 */
int cmp_prop_hyphens_init(cmp_prop_hyphens_t *prop, cmp_hyphens_t hyphens);

/**
 * @brief Initializes a line-clamp property.
 * @param prop The property to initialize.
 * @param line_clamp line-clamp string value.
 * @return 0 on success.
 */
int cmp_prop_line_clamp_init(cmp_prop_line_clamp_t *prop,
                             const char *line_clamp);

/**
 * @brief Frees resources in a line-clamp property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_line_clamp_free(cmp_prop_line_clamp_t *prop);

/**
 * @brief Initializes a hanging-punctuation property.
 * @param prop The property to initialize.
 * @param hanging_punctuation hanging-punctuation value.
 * @return 0 on success.
 */
int cmp_prop_hanging_punctuation_init(
    cmp_prop_hanging_punctuation_t *prop,
    cmp_hanging_punctuation_t hanging_punctuation);

/**
 * @brief Initializes a text-decoration group.
 * @param group The group to initialize.
 * @param line line value.
 * @param color color value.
 * @param style style value.
 * @param thickness thickness string value.
 * @return 0 on success.
 */
int cmp_prop_text_decoration_group_init(cmp_prop_text_decoration_group_t *group,
                                        int line, const cmp_prop_color_t *color,
                                        cmp_text_decoration_style_t style,
                                        const char *thickness);

/**
 * @brief Frees resources in a text-decoration group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_text_decoration_group_free(
    cmp_prop_text_decoration_group_t *group);

/**
 * @brief Initializes a text-underline group.
 * @param group The group to initialize.
 * @param position position value.
 * @param offset offset string value.
 * @param skip_ink skip-ink value.
 * @return 0 on success.
 */
int cmp_prop_text_underline_group_init(cmp_prop_text_underline_group_t *group,
                                       cmp_text_underline_position_t position,
                                       const char *offset,
                                       cmp_text_decoration_skip_ink_t skip_ink);

/**
 * @brief Frees resources in a text-underline group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_text_underline_group_free(cmp_prop_text_underline_group_t *group);

/**
 * @brief Initializes a text-shadow property.
 * @param prop The property to initialize.
 * @param text_shadow text-shadow string value.
 * @return 0 on success.
 */
int cmp_prop_text_shadow_init(cmp_prop_text_shadow_t *prop,
                              const char *text_shadow);

/**
 * @brief Frees resources in a text-shadow property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_text_shadow_free(cmp_prop_text_shadow_t *prop);

/**
 * @brief Initializes a text-transform property.
 * @param prop The property to initialize.
 * @param transform transform value.
 * @return 0 on success.
 */
int cmp_prop_text_transform_init(cmp_prop_text_transform_t *prop,
                                 cmp_text_transform_t transform);

/**
 * @brief Initializes a text-emphasis group.
 * @param group The group to initialize.
 * @param color color value.
 * @param style style value.
 * @param string_val custom string value (if applicable).
 * @param position position value.
 * @return 0 on success.
 */
int cmp_prop_text_emphasis_group_init(cmp_prop_text_emphasis_group_t *group,
                                      const cmp_prop_color_t *color,
                                      cmp_text_emphasis_style_t style,
                                      const char *string_val,
                                      cmp_text_emphasis_position_t position);

/**
 * @brief Frees resources in a text-emphasis group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_text_emphasis_group_free(cmp_prop_text_emphasis_group_t *group);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_TYPOGRAPHY_H */
