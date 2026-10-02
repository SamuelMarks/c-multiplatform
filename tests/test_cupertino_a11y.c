/**
 * @file test_cupertino_a11y.c
 * @brief Unit tests for Cupertino & Apple HIG Accessibility (a11y),
 * VoiceOver semantic traits, touch targets, notifications, and settings.
 */

/* clang-format off */
#include "cupertino/cupertino_a11y.h"
#include "greatest.h"
#include "ui_aria.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_cupertino_a11y_touch_targets(void) {
  float pad_x = 0.0f;
  float pad_y = 0.0f;
  int is_valid = 0;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_a11y_enforce_touch_target(-1.0f, 20.0f, &pad_x, &pad_y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_enforce_touch_target(20.0f, -1.0f, &pad_x, &pad_y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_enforce_touch_target(20.0f, 20.0f, NULL, &pad_y);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_enforce_touch_target(20.0f, 20.0f, &pad_x, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_a11y_validate_touch_target(-5.0f, 20.0f, &is_valid);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_validate_touch_target(20.0f, -5.0f, &is_valid);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_validate_touch_target(20.0f, 20.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Touch target enforcement: 20x30pt element needs padding to reach 44x44pt */
  rc = cupertino_a11y_enforce_touch_target(20.0f, 30.0f, &pad_x, &pad_y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(12.0f, pad_x); /* (44 - 20) / 2 = 12 */
  ASSERT_EQ(7.0f, pad_y);  /* (44 - 30) / 2 = 7 */

  /* Already meeting or exceeding 44pt needs 0 padding */
  rc = cupertino_a11y_enforce_touch_target(50.0f, 44.0f, &pad_x, &pad_y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, pad_x);
  ASSERT_EQ(0.0f, pad_y);

  /* Validate minimum boundary */
  rc = cupertino_a11y_validate_touch_target(44.0f, 44.0f, &is_valid);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_valid);

  rc = cupertino_a11y_validate_touch_target(43.5f, 44.0f, &is_valid);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_valid);

  rc = cupertino_a11y_validate_touch_target(44.0f, 40.0f, &is_valid);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_valid);

  PASS();
}

TEST test_cupertino_a11y_traits_and_aria(void) {
  enum ui_aria_role role;
  ui_error_t rc;

  /* Invalid args */
  rc = cupertino_a11y_traits_to_aria_role(CUPERTINO_A11Y_TRAIT_BUTTON, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Button trait */
  rc = cupertino_a11y_traits_to_aria_role(CUPERTINO_A11Y_TRAIT_BUTTON, &role);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_ARIA_ROLE_BUTTON, role);

  /* Link trait */
  rc = cupertino_a11y_traits_to_aria_role(CUPERTINO_A11Y_TRAIT_LINK, &role);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_ARIA_ROLE_LINK, role);

  /* Header trait */
  rc = cupertino_a11y_traits_to_aria_role(CUPERTINO_A11Y_TRAIT_HEADER, &role);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_ARIA_ROLE_HEADING, role);

  /* Search field trait */
  rc = cupertino_a11y_traits_to_aria_role(CUPERTINO_A11Y_TRAIT_SEARCH_FIELD,
                                          &role);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_ARIA_ROLE_TEXTBOX, role);

  /* Adjustable trait */
  rc = cupertino_a11y_traits_to_aria_role(CUPERTINO_A11Y_TRAIT_ADJUSTABLE,
                                          &role);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_ARIA_ROLE_SLIDER, role);

  /* Tab bar trait */
  rc = cupertino_a11y_traits_to_aria_role(CUPERTINO_A11Y_TRAIT_TAB_BAR, &role);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_ARIA_ROLE_TABLIST, role);

  /* None trait */
  rc = cupertino_a11y_traits_to_aria_role(CUPERTINO_A11Y_TRAIT_NONE, &role);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_ARIA_ROLE_NONE, role);

  PASS();
}

TEST test_cupertino_a11y_notifications_and_settings(void) {
  struct cupertino_a11y_settings settings;
  char buf[64];
  int needs_backing = 0;
  ui_error_t rc;

  /* Notifications invalid args */
  rc = cupertino_a11y_post_notification((enum cupertino_a11y_notification)0,
                                        NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_post_notification((enum cupertino_a11y_notification)99,
                                        NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      cupertino_a11y_post_notification(CUPERTINO_A11Y_NOTIF_ANNOUNCEMENT, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid notifications */
  rc = cupertino_a11y_post_notification(CUPERTINO_A11Y_NOTIF_SCREEN_CHANGED,
                                        NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_a11y_post_notification(CUPERTINO_A11Y_NOTIF_LAYOUT_CHANGED,
                                        NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_a11y_post_notification(CUPERTINO_A11Y_NOTIF_ANNOUNCEMENT,
                                        "Welcome");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_a11y_post_notification(CUPERTINO_A11Y_NOTIF_PAGE_SCROLLED,
                                        NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Default settings invalid args */
  rc = cupertino_a11y_get_default_settings(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_a11y_get_default_settings(&settings);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, settings.smart_invert_protection);
  ASSERT_EQ(0, settings.reduce_motion);
  ASSERT_EQ(0, settings.reduce_transparency);

  /* Button shapes */
  rc = cupertino_a11y_should_render_button_shapes(NULL, 1, &needs_backing);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_should_render_button_shapes(&settings, 1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_a11y_should_render_button_shapes(&settings, 1, &needs_backing);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, needs_backing);

  settings.button_shapes = 1;
  rc = cupertino_a11y_should_render_button_shapes(&settings, 1, &needs_backing);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, needs_backing);

  rc = cupertino_a11y_should_render_button_shapes(&settings, 0, &needs_backing);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, needs_backing);

  /* Page scrolled announcement formatting */
  rc = cupertino_a11y_format_page_scrolled_announcement(0, 5, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_format_page_scrolled_announcement(2, 0, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_format_page_scrolled_announcement(6, 5, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc =
      cupertino_a11y_format_page_scrolled_announcement(2, 5, NULL, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_format_page_scrolled_announcement(2, 5, buf, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_a11y_format_page_scrolled_announcement(100, 100, buf, 14);
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, rc);

  rc = cupertino_a11y_format_page_scrolled_announcement(3, 8, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Page 3 of 8", buf);

  PASS();
}

SUITE(cupertino_a11y_suite) {
  RUN_TEST(test_cupertino_a11y_touch_targets);
  RUN_TEST(test_cupertino_a11y_traits_and_aria);
  RUN_TEST(test_cupertino_a11y_notifications_and_settings);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_a11y_suite);
  GREATEST_MAIN_END();
}
