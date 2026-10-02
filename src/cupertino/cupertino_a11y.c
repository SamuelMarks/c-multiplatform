/**
 * @file cupertino_a11y.c
 * @brief Implementation of Cupertino & Apple HIG Accessibility (a11y)
 * traits, notifications, touch bounds, and settings.
 */

/* clang-format off */
#include "cupertino/cupertino_a11y.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

ui_error_t cupertino_a11y_enforce_touch_target(float width, float height,
                                               float *out_pad_x,
                                               float *out_pad_y) {
  float diff_x;
  float diff_y;

  if (width < 0.0f || height < 0.0f || !out_pad_x || !out_pad_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Apple HIG specifies 44x44pt minimum touch boundary */
  diff_x = 44.0f - width;
  diff_y = 44.0f - height;

  *out_pad_x = (diff_x > 0.0f) ? (diff_x * 0.5f) : 0.0f;
  *out_pad_y = (diff_y > 0.0f) ? (diff_y * 0.5f) : 0.0f;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_a11y_validate_touch_target(float width, float height,
                                                int *out_is_valid) {
  if (width < 0.0f || height < 0.0f || !out_is_valid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_valid = (width >= 44.0f && height >= 44.0f) ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_a11y_traits_to_aria_role(unsigned int traits,
                                              enum ui_aria_role *out_role) {
  if (!out_role) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (traits & CUPERTINO_A11Y_TRAIT_BUTTON) {
    *out_role = UI_ARIA_ROLE_BUTTON;
  } else if (traits & CUPERTINO_A11Y_TRAIT_LINK) {
    *out_role = UI_ARIA_ROLE_LINK;
  } else if (traits & CUPERTINO_A11Y_TRAIT_HEADER) {
    *out_role = UI_ARIA_ROLE_HEADING;
  } else if (traits & CUPERTINO_A11Y_TRAIT_SEARCH_FIELD) {
    *out_role = UI_ARIA_ROLE_TEXTBOX;
  } else if (traits & CUPERTINO_A11Y_TRAIT_ADJUSTABLE) {
    *out_role = UI_ARIA_ROLE_SLIDER;
  } else if (traits & CUPERTINO_A11Y_TRAIT_TAB_BAR) {
    *out_role = UI_ARIA_ROLE_TABLIST;
  } else {
    *out_role = UI_ARIA_ROLE_NONE;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_a11y_post_notification(enum cupertino_a11y_notification notif,
                                 const char *announcement) {
  if (notif != CUPERTINO_A11Y_NOTIF_SCREEN_CHANGED &&
      notif != CUPERTINO_A11Y_NOTIF_LAYOUT_CHANGED &&
      notif != CUPERTINO_A11Y_NOTIF_ANNOUNCEMENT &&
      notif != CUPERTINO_A11Y_NOTIF_PAGE_SCROLLED) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (notif == CUPERTINO_A11Y_NOTIF_ANNOUNCEMENT && !announcement) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_a11y_get_default_settings(
    struct cupertino_a11y_settings *out_settings) {
  if (!out_settings) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(out_settings, 0, sizeof(*out_settings));
  out_settings->smart_invert_protection = 1;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_a11y_should_render_button_shapes(
    const struct cupertino_a11y_settings *settings, int is_borderless,
    int *out_needs_backing) {
  if (!settings || !out_needs_backing) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_needs_backing = (settings->button_shapes && is_borderless) ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_a11y_format_page_scrolled_announcement(size_t current_page,
                                                            size_t total_pages,
                                                            char *out_buf,
                                                            size_t buf_size) {
  char temp[64];
  size_t len;

  if (!out_buf || buf_size < 12 || current_page == 0 || total_pages == 0 ||
      current_page > total_pages) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  sprintf_s(temp, sizeof(temp), "Page %lu of %lu", (unsigned long)current_page,
            (unsigned long)total_pages);
#else
  snprintf(temp, sizeof(temp), "Page %lu of %lu", (unsigned long)current_page,
           (unsigned long)total_pages);
#endif

  len = strlen(temp);
  if (len >= buf_size) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

#if defined(_MSC_VER)
  strcpy_s(out_buf, buf_size, temp);
#else
  strcpy(out_buf, temp);
#endif

  return UI_ERROR_NONE;
}
