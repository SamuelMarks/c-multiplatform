#ifndef UI_CSS_UI_H
#define UI_CSS_UI_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief CSS outline style values.
 */
enum ui_css_outline_style {
  UI_CSS_OUTLINE_STYLE_NONE,
  UI_CSS_OUTLINE_STYLE_HIDDEN,
  UI_CSS_OUTLINE_STYLE_DOTTED,
  UI_CSS_OUTLINE_STYLE_DASHED,
  UI_CSS_OUTLINE_STYLE_SOLID,
  UI_CSS_OUTLINE_STYLE_DOUBLE,
  UI_CSS_OUTLINE_STYLE_GROOVE,
  UI_CSS_OUTLINE_STYLE_RIDGE,
  UI_CSS_OUTLINE_STYLE_INSET,
  UI_CSS_OUTLINE_STYLE_OUTSET,
  UI_CSS_OUTLINE_STYLE_AUTO
};

/**
 * @brief CSS outline properties.
 */
struct ui_css_outline {
  struct ui_css_value width;
  enum ui_css_outline_style style;
  struct ui_css_color color;
  struct ui_css_value offset;
  int is_color_invert; /* true if outline-color is 'invert' */
};

/**
 * @brief CSS cursor keywords.
 */
enum ui_css_cursor_keyword {
  UI_CSS_CURSOR_AUTO,
  UI_CSS_CURSOR_DEFAULT,
  UI_CSS_CURSOR_NONE,
  UI_CSS_CURSOR_CONTEXT_MENU,
  UI_CSS_CURSOR_HELP,
  UI_CSS_CURSOR_POINTER,
  UI_CSS_CURSOR_PROGRESS,
  UI_CSS_CURSOR_WAIT,
  UI_CSS_CURSOR_CELL,
  UI_CSS_CURSOR_CROSSHAIR,
  UI_CSS_CURSOR_TEXT,
  UI_CSS_CURSOR_VERTICAL_TEXT,
  UI_CSS_CURSOR_ALIAS,
  UI_CSS_CURSOR_COPY,
  UI_CSS_CURSOR_MOVE,
  UI_CSS_CURSOR_NO_DROP,
  UI_CSS_CURSOR_NOT_ALLOWED,
  UI_CSS_CURSOR_GRAB,
  UI_CSS_CURSOR_GRABBING,
  UI_CSS_CURSOR_E_RESIZE,
  UI_CSS_CURSOR_N_RESIZE,
  UI_CSS_CURSOR_NE_RESIZE,
  UI_CSS_CURSOR_NW_RESIZE,
  UI_CSS_CURSOR_S_RESIZE,
  UI_CSS_CURSOR_SE_RESIZE,
  UI_CSS_CURSOR_SW_RESIZE,
  UI_CSS_CURSOR_W_RESIZE,
  UI_CSS_CURSOR_EW_RESIZE,
  UI_CSS_CURSOR_NS_RESIZE,
  UI_CSS_CURSOR_NESW_RESIZE,
  UI_CSS_CURSOR_NWSE_RESIZE,
  UI_CSS_CURSOR_COL_RESIZE,
  UI_CSS_CURSOR_ROW_RESIZE,
  UI_CSS_CURSOR_ALL_SCROLL,
  UI_CSS_CURSOR_ZOOM_IN,
  UI_CSS_CURSOR_ZOOM_OUT
};

/**
 * @brief Represents a single cursor image fallback with optional hotspot.
 */
struct ui_css_cursor_image {
  struct ui_css_image image;
  float hotspot_x;
  float hotspot_y;
  int has_hotspot;
  struct ui_css_cursor_image *next;
};

/**
 * @brief CSS cursor property.
 */
struct ui_css_cursor {
  struct ui_css_cursor_image *images;
  enum ui_css_cursor_keyword keyword;
};

/**
 * @brief CSS user-select values.
 */
enum ui_css_user_select {
  UI_CSS_USER_SELECT_AUTO,
  UI_CSS_USER_SELECT_NONE,
  UI_CSS_USER_SELECT_TEXT,
  UI_CSS_USER_SELECT_ALL,
  UI_CSS_USER_SELECT_CONTAIN
};

/**
 * @brief CSS appearance values.
 */
enum ui_css_appearance {
  UI_CSS_APPEARANCE_NONE,
  UI_CSS_APPEARANCE_AUTO,
  UI_CSS_APPEARANCE_MENULIST_BUTTON,
  UI_CSS_APPEARANCE_TEXTFIELD,
  UI_CSS_APPEARANCE_BUTTON,
  UI_CSS_APPEARANCE_SEARCHFIELD,
  UI_CSS_APPEARANCE_TEXTAREA,
  UI_CSS_APPEARANCE_PUSH_BUTTON,
  UI_CSS_APPEARANCE_SLIDER_HORIZONTAL,
  UI_CSS_APPEARANCE_CHECKBOX,
  UI_CSS_APPEARANCE_RADIO,
  UI_CSS_APPEARANCE_SQUARE_BUTTON,
  UI_CSS_APPEARANCE_MENULIST,
  UI_CSS_APPEARANCE_LISTBOX,
  UI_CSS_APPEARANCE_METER,
  UI_CSS_APPEARANCE_PROGRESS_BAR
};

/**
 * @brief CSS pointer-events values.
 */
enum ui_css_pointer_events {
  UI_CSS_POINTER_EVENTS_AUTO,
  UI_CSS_POINTER_EVENTS_NONE,
  UI_CSS_POINTER_EVENTS_VISIBLE_PAINTED,
  UI_CSS_POINTER_EVENTS_VISIBLE_FILL,
  UI_CSS_POINTER_EVENTS_VISIBLE_STROKE,
  UI_CSS_POINTER_EVENTS_VISIBLE,
  UI_CSS_POINTER_EVENTS_PAINTED,
  UI_CSS_POINTER_EVENTS_FILL,
  UI_CSS_POINTER_EVENTS_STROKE,
  UI_CSS_POINTER_EVENTS_ALL
};

/**
 * @brief CSS overscroll-behavior values.
 */
enum ui_css_overscroll_behavior {
  UI_CSS_OVERSCROLL_BEHAVIOR_AUTO,
  UI_CSS_OVERSCROLL_BEHAVIOR_CONTAIN,
  UI_CSS_OVERSCROLL_BEHAVIOR_NONE
};

/**
 * @brief CSS overscroll-behavior shorthand property.
 */
struct ui_css_overscroll_behavior_shorthand {
  enum ui_css_overscroll_behavior x;
  enum ui_css_overscroll_behavior y;
};

/**
 * @brief Parses the outline-width CSS property.
 */
ui_error_t ui_css_parse_outline_width(const char *str,
                                      struct ui_css_value *out_width);

/**
 * @brief Parses the outline-style CSS property.
 */
ui_error_t ui_css_parse_outline_style(const char *str,
                                      enum ui_css_outline_style *out_style);

/**
 * @brief Parses the outline shorthand CSS property.
 */
ui_error_t ui_css_parse_outline(const char *str,
                                struct ui_css_outline *out_outline);

/**
 * @brief Destroys a parsed cursor property (frees linked images).
 */
ui_error_t ui_css_cursor_destroy(struct ui_css_cursor *cursor);

/**
 * @brief Parses the cursor CSS property.
 */
ui_error_t ui_css_parse_cursor(const char *str,
                               struct ui_css_cursor *out_cursor);

/**
 * @brief Parses the user-select CSS property.
 */
ui_error_t ui_css_parse_user_select(const char *str,
                                    enum ui_css_user_select *out_select);

/**
 * @brief Parses the appearance CSS property.
 */
ui_error_t ui_css_parse_appearance(const char *str,
                                   enum ui_css_appearance *out_appearance);

/**
 * @brief Parses the pointer-events CSS property.
 */
ui_error_t ui_css_parse_pointer_events(const char *str,
                                       enum ui_css_pointer_events *out_events);

/**
 * @brief Parses the overscroll-behavior-x or overscroll-behavior-y CSS
 * property.
 *
 * @param str The string to parse.
 * @param out_behavior Pointer to receive the parsed behavior.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_parse_overscroll_behavior_axis(
    const char *str, enum ui_css_overscroll_behavior *out_behavior);

/**
 * @brief Parses the overscroll-behavior shorthand CSS property.
 *
 * @param str The string to parse.
 * @param out_shorthand Pointer to receive the parsed overscroll-behavior
 * structure.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_parse_overscroll_behavior(
    const char *str,
    struct ui_css_overscroll_behavior_shorthand *out_shorthand);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_UI_H */
