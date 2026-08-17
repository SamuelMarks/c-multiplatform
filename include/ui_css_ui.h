/**
 * @file ui_css_ui.h
 * @brief CSS UI properties and parsing.
 *
 * This header defines structures, enumerations, and functions for handling
 * CSS UI module properties such as outline, cursor, user-select, appearance,
 * pointer-events, and overscroll-behavior.
 */

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
  UI_CSS_OUTLINE_STYLE_NONE,   /**< No outline. */
  UI_CSS_OUTLINE_STYLE_HIDDEN, /**< Hidden outline. */
  UI_CSS_OUTLINE_STYLE_DOTTED, /**< Dotted outline. */
  UI_CSS_OUTLINE_STYLE_DASHED, /**< Dashed outline. */
  UI_CSS_OUTLINE_STYLE_SOLID,  /**< Solid outline. */
  UI_CSS_OUTLINE_STYLE_DOUBLE, /**< Double outline. */
  UI_CSS_OUTLINE_STYLE_GROOVE, /**< Groove outline. */
  UI_CSS_OUTLINE_STYLE_RIDGE,  /**< Ridge outline. */
  UI_CSS_OUTLINE_STYLE_INSET,  /**< Inset outline. */
  UI_CSS_OUTLINE_STYLE_OUTSET, /**< Outset outline. */
  UI_CSS_OUTLINE_STYLE_AUTO    /**< Automatic outline styling. */
};

/**
 * @brief CSS outline properties.
 */
struct ui_css_outline {
  struct ui_css_value width;       /**< Width of the outline. */
  enum ui_css_outline_style style; /**< Style of the outline. */
  struct ui_css_color color;       /**< Color of the outline. */
  struct ui_css_value offset;      /**< Offset of the outline. */
  int is_color_invert; /**< 1 if outline-color is 'invert', 0 otherwise. */
};

/**
 * @brief CSS cursor keywords.
 */
enum ui_css_cursor_keyword {
  UI_CSS_CURSOR_AUTO,          /**< Automatic cursor. */
  UI_CSS_CURSOR_DEFAULT,       /**< Default cursor. */
  UI_CSS_CURSOR_NONE,          /**< No cursor. */
  UI_CSS_CURSOR_CONTEXT_MENU,  /**< Context menu cursor. */
  UI_CSS_CURSOR_HELP,          /**< Help cursor. */
  UI_CSS_CURSOR_POINTER,       /**< Pointer cursor. */
  UI_CSS_CURSOR_PROGRESS,      /**< Progress cursor. */
  UI_CSS_CURSOR_WAIT,          /**< Wait cursor. */
  UI_CSS_CURSOR_CELL,          /**< Cell cursor. */
  UI_CSS_CURSOR_CROSSHAIR,     /**< Crosshair cursor. */
  UI_CSS_CURSOR_TEXT,          /**< Text cursor. */
  UI_CSS_CURSOR_VERTICAL_TEXT, /**< Vertical text cursor. */
  UI_CSS_CURSOR_ALIAS,         /**< Alias cursor. */
  UI_CSS_CURSOR_COPY,          /**< Copy cursor. */
  UI_CSS_CURSOR_MOVE,          /**< Move cursor. */
  UI_CSS_CURSOR_NO_DROP,       /**< No drop cursor. */
  UI_CSS_CURSOR_NOT_ALLOWED,   /**< Not allowed cursor. */
  UI_CSS_CURSOR_GRAB,          /**< Grab cursor. */
  UI_CSS_CURSOR_GRABBING,      /**< Grabbing cursor. */
  UI_CSS_CURSOR_E_RESIZE,      /**< East resize cursor. */
  UI_CSS_CURSOR_N_RESIZE,      /**< North resize cursor. */
  UI_CSS_CURSOR_NE_RESIZE,     /**< North-east resize cursor. */
  UI_CSS_CURSOR_NW_RESIZE,     /**< North-west resize cursor. */
  UI_CSS_CURSOR_S_RESIZE,      /**< South resize cursor. */
  UI_CSS_CURSOR_SE_RESIZE,     /**< South-east resize cursor. */
  UI_CSS_CURSOR_SW_RESIZE,     /**< South-west resize cursor. */
  UI_CSS_CURSOR_W_RESIZE,      /**< West resize cursor. */
  UI_CSS_CURSOR_EW_RESIZE,     /**< East-west resize cursor. */
  UI_CSS_CURSOR_NS_RESIZE,     /**< North-south resize cursor. */
  UI_CSS_CURSOR_NESW_RESIZE,   /**< North-east to south-west resize cursor. */
  UI_CSS_CURSOR_NWSE_RESIZE,   /**< North-west to south-east resize cursor. */
  UI_CSS_CURSOR_COL_RESIZE,    /**< Column resize cursor. */
  UI_CSS_CURSOR_ROW_RESIZE,    /**< Row resize cursor. */
  UI_CSS_CURSOR_ALL_SCROLL,    /**< All-scroll cursor. */
  UI_CSS_CURSOR_ZOOM_IN,       /**< Zoom in cursor. */
  UI_CSS_CURSOR_ZOOM_OUT       /**< Zoom out cursor. */
};

/**
 * @brief Represents a single cursor image fallback with optional hotspot.
 */
struct ui_css_cursor_image {
  struct ui_css_image image; /**< The cursor image. */
  float hotspot_x;           /**< X coordinate of the hotspot. */
  float hotspot_y;           /**< Y coordinate of the hotspot. */
  int has_hotspot;           /**< 1 if a hotspot is provided, 0 otherwise. */
  struct ui_css_cursor_image *next; /**< Pointer to the next image fallback. */
};

/**
 * @brief CSS cursor property.
 */
struct ui_css_cursor {
  struct ui_css_cursor_image *images; /**< Linked list of cursor images. */
  enum ui_css_cursor_keyword keyword; /**< The fallback cursor keyword. */
};

/**
 * @brief CSS user-select values.
 */
enum ui_css_user_select {
  UI_CSS_USER_SELECT_AUTO,   /**< Auto user-select. */
  UI_CSS_USER_SELECT_NONE,   /**< No user-select. */
  UI_CSS_USER_SELECT_TEXT,   /**< Text user-select. */
  UI_CSS_USER_SELECT_ALL,    /**< All user-select. */
  UI_CSS_USER_SELECT_CONTAIN /**< Contain user-select. */
};

/**
 * @brief CSS appearance values.
 */
enum ui_css_appearance {
  UI_CSS_APPEARANCE_NONE,              /**< No appearance. */
  UI_CSS_APPEARANCE_AUTO,              /**< Auto appearance. */
  UI_CSS_APPEARANCE_MENULIST_BUTTON,   /**< Menulist button appearance. */
  UI_CSS_APPEARANCE_TEXTFIELD,         /**< Textfield appearance. */
  UI_CSS_APPEARANCE_BUTTON,            /**< Button appearance. */
  UI_CSS_APPEARANCE_SEARCHFIELD,       /**< Searchfield appearance. */
  UI_CSS_APPEARANCE_TEXTAREA,          /**< Textarea appearance. */
  UI_CSS_APPEARANCE_PUSH_BUTTON,       /**< Push button appearance. */
  UI_CSS_APPEARANCE_SLIDER_HORIZONTAL, /**< Horizontal slider appearance. */
  UI_CSS_APPEARANCE_CHECKBOX,          /**< Checkbox appearance. */
  UI_CSS_APPEARANCE_RADIO,             /**< Radio button appearance. */
  UI_CSS_APPEARANCE_SQUARE_BUTTON,     /**< Square button appearance. */
  UI_CSS_APPEARANCE_MENULIST,          /**< Menulist appearance. */
  UI_CSS_APPEARANCE_LISTBOX,           /**< Listbox appearance. */
  UI_CSS_APPEARANCE_METER,             /**< Meter appearance. */
  UI_CSS_APPEARANCE_PROGRESS_BAR       /**< Progress bar appearance. */
};

/**
 * @brief CSS pointer-events values.
 */
enum ui_css_pointer_events {
  UI_CSS_POINTER_EVENTS_AUTO,            /**< Auto pointer-events. */
  UI_CSS_POINTER_EVENTS_NONE,            /**< No pointer-events. */
  UI_CSS_POINTER_EVENTS_VISIBLE_PAINTED, /**< Visible painted events. */
  UI_CSS_POINTER_EVENTS_VISIBLE_FILL,    /**< Visible fill events. */
  UI_CSS_POINTER_EVENTS_VISIBLE_STROKE,  /**< Visible stroke events. */
  UI_CSS_POINTER_EVENTS_VISIBLE,         /**< Visible events. */
  UI_CSS_POINTER_EVENTS_PAINTED,         /**< Painted events. */
  UI_CSS_POINTER_EVENTS_FILL,            /**< Fill events. */
  UI_CSS_POINTER_EVENTS_STROKE,          /**< Stroke events. */
  UI_CSS_POINTER_EVENTS_ALL              /**< All pointer-events. */
};

/**
 * @brief CSS overscroll-behavior values.
 */
enum ui_css_overscroll_behavior {
  UI_CSS_OVERSCROLL_BEHAVIOR_AUTO,    /**< Auto overscroll behavior. */
  UI_CSS_OVERSCROLL_BEHAVIOR_CONTAIN, /**< Contain overscroll behavior. */
  UI_CSS_OVERSCROLL_BEHAVIOR_NONE     /**< None overscroll behavior. */
};

/**
 * @brief CSS overscroll-behavior shorthand property.
 */
struct ui_css_overscroll_behavior_shorthand {
  enum ui_css_overscroll_behavior x; /**< Behavior for the x-axis. */
  enum ui_css_overscroll_behavior y; /**< Behavior for the y-axis. */
};

/**
 * @brief Parses the outline-width CSS property.
 *
 * @param str The string to parse.
 * @param out_width Pointer to receive the parsed width.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_parse_outline_width(const char *str,
                                      struct ui_css_value *out_width);

/**
 * @brief Parses the outline-style CSS property.
 *
 * @param str The string to parse.
 * @param out_style Pointer to receive the parsed style.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_parse_outline_style(const char *str,
                                      enum ui_css_outline_style *out_style);

/**
 * @brief Parses the outline shorthand CSS property.
 *
 * @param str The string to parse.
 * @param out_outline Pointer to receive the parsed outline struct.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_parse_outline(const char *str,
                                struct ui_css_outline *out_outline);

/**
 * @brief Destroys a parsed cursor property (frees linked images).
 *
 * @param cursor Pointer to the cursor to clean up.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_cursor_destroy(struct ui_css_cursor *cursor);

/**
 * @brief Parses the cursor CSS property.
 *
 * @param str The string to parse.
 * @param out_cursor Pointer to receive the parsed cursor property.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_parse_cursor(const char *str,
                               struct ui_css_cursor *out_cursor);

/**
 * @brief Parses the user-select CSS property.
 *
 * @param str The string to parse.
 * @param out_select Pointer to receive the parsed user-select value.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_parse_user_select(const char *str,
                                    enum ui_css_user_select *out_select);

/**
 * @brief Parses the appearance CSS property.
 *
 * @param str The string to parse.
 * @param out_appearance Pointer to receive the parsed appearance value.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_parse_appearance(const char *str,
                                   enum ui_css_appearance *out_appearance);

/**
 * @brief Parses the pointer-events CSS property.
 *
 * @param str The string to parse.
 * @param out_events Pointer to receive the parsed pointer-events value.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_parse_pointer_events(const char *str,
                                       enum ui_css_pointer_events *out_events);

/**
 * @brief Parses the overscroll-behavior-x or overscroll-behavior-y CSS
 * property.
 *
 * @param str The string to parse.
 * @param out_behavior Pointer to receive the parsed behavior.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_parse_overscroll_behavior_axis(
    const char *str, enum ui_css_overscroll_behavior *out_behavior);

/**
 * @brief Parses the overscroll-behavior shorthand CSS property.
 *
 * @param str The string to parse.
 * @param out_shorthand Pointer to receive the parsed overscroll-behavior
 * structure.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_parse_overscroll_behavior(
    const char *str,
    struct ui_css_overscroll_behavior_shorthand *out_shorthand);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_UI_H */
