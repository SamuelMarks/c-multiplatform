/**
 * @file cupertino_a11y.h
 * @brief Cupertino & Apple Human Interface Guidelines (HIG) Accessibility
 * (a11y) and VoiceOver semantic traits, notifications, rotor actions, and
 * accessibility adaptations.
 */

#ifndef CUPERTINO_CUPERTINO_A11Y_H
#define CUPERTINO_CUPERTINO_A11Y_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_aria.h"
#include "ui_error.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

/**
 * @enum cupertino_a11y_trait
 * @brief Apple VoiceOver semantic accessibility traits (UIAccessibilityTraits
 * bitmask).
 */
enum cupertino_a11y_trait {
  CUPERTINO_A11Y_TRAIT_NONE = 0,
  CUPERTINO_A11Y_TRAIT_BUTTON = (1 << 0), /**< Clickable button or pill */
  CUPERTINO_A11Y_TRAIT_LINK = (1 << 1),   /**< Hyperlinked text */
  CUPERTINO_A11Y_TRAIT_HEADER =
      (1 << 2), /**< Section header or nav bar title */
  CUPERTINO_A11Y_TRAIT_SEARCH_FIELD = (1 << 3), /**< Search input field */
  CUPERTINO_A11Y_TRAIT_IMAGE = (1 << 4),        /**< Image or vector graphic */
  CUPERTINO_A11Y_TRAIT_SELECTED = (1 << 5),     /**< Selected tab or item */
  CUPERTINO_A11Y_TRAIT_PLAYS_SOUND = (1 << 6),  /**< Produces audio/sound */
  CUPERTINO_A11Y_TRAIT_KEYBOARD_KEY = (1 << 7), /**< Onscreen keyboard key */
  CUPERTINO_A11Y_TRAIT_STATIC_TEXT =
      (1 << 8), /**< Informative non-interactive text */
  CUPERTINO_A11Y_TRAIT_SUMMARY_ELEMENT =
      (1 << 9), /**< Overview/summary description */
  CUPERTINO_A11Y_TRAIT_NOT_ENABLED = (1 << 10), /**< Disabled control */
  CUPERTINO_A11Y_TRAIT_UPDATES_FREQUENTLY =
      (1 << 11), /**< Live clock / frequent updates */
  CUPERTINO_A11Y_TRAIT_STARTS_MEDIA = (1 << 12), /**< Media playback trigger */
  CUPERTINO_A11Y_TRAIT_ADJUSTABLE =
      (1 << 13),                           /**< Slider, stepper, date wheel */
  CUPERTINO_A11Y_TRAIT_TAB_BAR = (1 << 14) /**< Bottom tab bar container */
};

/**
 * @enum cupertino_a11y_notification
 * @brief VoiceOver accessibility notification events.
 */
enum cupertino_a11y_notification {
  CUPERTINO_A11Y_NOTIF_SCREEN_CHANGED =
      1, /**< Major UI transition (navigation push/modal) */
  CUPERTINO_A11Y_NOTIF_LAYOUT_CHANGED, /**< Subsection layout shift (search
                                          focus/collapse) */
  CUPERTINO_A11Y_NOTIF_ANNOUNCEMENT,   /**< Screen-reader spoken announcement */
  CUPERTINO_A11Y_NOTIF_PAGE_SCROLLED   /**< Page control / scroll step update */
};

/**
 * @struct cupertino_a11y_settings
 * @brief System accessibility settings and preference overrides.
 */
struct cupertino_a11y_settings {
  int reduce_motion; /**< Non-zero to disable spring bounce and heavy motion */
  int reduce_transparency; /**< Non-zero to bypass blurs with opaque fills */
  int differentiate_without_color; /**< Non-zero to add text/underlines next to
                                      tinted indicators */
  int on_off_labels; /**< Non-zero to render 'I'/'O' inside switch tracks */
  int button_shapes; /**< Non-zero to render tint backing or underlines on
                        buttons */
  int bold_text;     /**< Non-zero to elevate text weights by one step */
  int smart_invert_protection; /**< Non-zero to protect media/icons from color
                                  inversion */
};

/**
 * @brief Enforces minimum tap target size (44x44pt on Apple platforms).
 *
 * @param width Input width in points.
 * @param height Input height in points.
 * @param out_pad_x Pointer to receive horizontal padding in points.
 * @param out_pad_y Pointer to receive vertical padding in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_a11y_enforce_touch_target(
    float width, float height, float *out_pad_x, float *out_pad_y);

/**
 * @brief Validates if dimensions meet the 44x44pt minimum touch boundary.
 *
 * @param width Input width in points.
 * @param height Input height in points.
 * @param out_is_valid Pointer to receive 1 if valid, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_a11y_validate_touch_target(
    float width, float height, int *out_is_valid);

/**
 * @brief Maps Cupertino accessibility traits bitmask to UI ARIA role.
 *
 * @param traits Bitwise OR of cupertino_a11y_trait.
 * @param out_role Pointer to receive mapped ARIA role.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_a11y_traits_to_aria_role(
    unsigned int traits, enum ui_aria_role *out_role);

/**
 * @brief Dispatches an accessibility notification for VoiceOver screen readers.
 *
 * @param notif Notification type.
 * @param announcement Optional text announcement or payload (can be NULL).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_a11y_post_notification(
    enum cupertino_a11y_notification notif, const char *announcement);

/**
 * @brief Retrieves default system accessibility settings.
 *
 * @param out_settings Pointer to receive accessibility settings structure.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_a11y_get_default_settings(
    struct cupertino_a11y_settings *out_settings);

/**
 * @brief Evaluates whether a control requires visible shape backings under
 * Button Shapes mode.
 *
 * @param settings Accessibility settings pointer.
 * @param is_borderless Non-zero if the button has no visible border.
 * @param out_needs_backing Pointer to receive 1 if backing/underline needed, 0
 * otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_a11y_should_render_button_shapes(
    const struct cupertino_a11y_settings *settings, int is_borderless,
    int *out_needs_backing);

/**
 * @brief Formats page scroll announcement string (e.g., "Page 2 of 5").
 *
 * @param current_page 1-based current page.
 * @param total_pages Total page count.
 * @param out_buf Buffer to store formatted string.
 * @param buf_size Size of out_buf.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_BOUNDS.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_a11y_format_page_scrolled_announcement(size_t current_page,
                                                 size_t total_pages,
                                                 char *out_buf,
                                                 size_t buf_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_A11Y_H */
