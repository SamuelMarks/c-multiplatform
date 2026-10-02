/**
 * @file test_cupertino_button.c
 * @brief Unit tests for Cupertino Button component.
 */

/* clang-format off */
#include "cupertino/cupertino_button.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

SUITE(cupertino_button_suite);

TEST test_button_lifecycle_and_properties(void) {
  struct cupertino_button_descriptor desc;
  struct cupertino_button *button = NULL;
  struct cupertino_button *button2 = NULL;
  struct cupertino_button *button3 = NULL;
  struct ui_button_base *base = NULL;
  struct ui_button_base *saved_base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_button_create(NULL, NULL, &button);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_button_create(dummy_engine, NULL, &button);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  memset(&desc, 0, sizeof(desc));
  rc = cupertino_button_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.style = (enum cupertino_button_style) - 1;
  rc = cupertino_button_create(dummy_engine, &desc, &button);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.style = CUPERTINO_BUTTON_STYLE_COUNT;
  rc = cupertino_button_create(dummy_engine, &desc, &button);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation with text and disabled state */
  desc.style = CUPERTINO_BUTTON_FILLED;
  desc.size = CUPERTINO_BUTTON_SIZE_MEDIUM;
  desc.text = "Continue";
  desc.is_disabled = 1;

  rc = cupertino_button_create(dummy_engine, &desc, &button);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(button != NULL);
  ASSERT_EQ(CUPERTINO_BUTTON_FILLED, button->style);
  ASSERT_EQ(CUPERTINO_BUTTON_SIZE_MEDIUM, button->size);
  ASSERT_STR_EQ("Continue", button->label);
  ASSERT_EQ(0, button->is_pressed);

  /* Base retrieval */
  rc = cupertino_button_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_button_get_base(button, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_button_get_base(button, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Set style */
  rc = cupertino_button_set_style(NULL, CUPERTINO_BUTTON_TINTED);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_button_set_style(button, (enum cupertino_button_style) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_button_set_style(button, CUPERTINO_BUTTON_STYLE_COUNT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_button_set_style(button, CUPERTINO_BUTTON_CAPSULE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_BUTTON_CAPSULE, button->style);

  /* Set text */
  rc = cupertino_button_set_text(NULL, "New Text");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_button_set_text(button, "Submit Order");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Submit Order", button->label);

  rc = cupertino_button_set_text(button, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Press state and Apple spring recoil properties */
  rc = cupertino_button_set_pressed(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Pressed down: 0.96 scale, 0.60 opacity */
  rc = cupertino_button_set_pressed(button, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, button->is_pressed);
  ASSERT(button->press_scale < 0.97f && button->press_scale > 0.95f);
  ASSERT(button->press_opacity < 0.61f && button->press_opacity > 0.59f);

  /* Released: 1.0 scale, 1.0 opacity */
  rc = cupertino_button_set_pressed(button, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, button->is_pressed);
  ASSERT(button->press_scale > 0.99f);
  ASSERT(button->press_opacity > 0.99f);

  /* Button with no text and not disabled */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_BUTTON_PLAIN;
  rc = cupertino_button_create(dummy_engine, &desc, &button3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(button3 != NULL);
  rc = cupertino_button_destroy(button3);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Button with base == NULL */
  rc = cupertino_button_create(dummy_engine, &desc, &button2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  saved_base = button2->base;
  button2->base = NULL;
  rc = cupertino_button_set_text(button2, "No Base");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_button_destroy(button2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_button_base_destroy(saved_base);

  /* Destroy */
  rc = cupertino_button_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_button_destroy(button);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_button_oom_mock(void) {
  struct cupertino_button_descriptor desc;
  struct cupertino_button *button = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_BUTTON_FILLED;
  desc.size = CUPERTINO_BUTTON_SIZE_MEDIUM;
  desc.text = "OOM Test";

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_button_mock_set_text_fail;
  extern int g_cupertino_button_mock_set_disabled_fail;
  extern int g_cupertino_button_mock_destroy_fail;

  g_malloc_fail_countdown = 0;
  rc = cupertino_button_create(dummy_engine, &desc, &button);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, button);

  g_malloc_fail_countdown = 1;
  rc = cupertino_button_create(dummy_engine, &desc, &button);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, button);

  g_malloc_fail_countdown = -1;

  /* Mock set_text failure */
  g_cupertino_button_mock_set_text_fail = 1;
  rc = cupertino_button_create(dummy_engine, &desc, &button);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, button);

  /* Mock set_text failure + destroy failure */
  g_cupertino_button_mock_destroy_fail = 1;
  rc = cupertino_button_create(dummy_engine, &desc, &button);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, button);
  g_cupertino_button_mock_set_text_fail = 0;
  g_cupertino_button_mock_destroy_fail = 0;

  /* Mock set_disabled failure */
  desc.is_disabled = 1;
  g_cupertino_button_mock_set_disabled_fail = 1;
  rc = cupertino_button_create(dummy_engine, &desc, &button);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, button);

  /* Mock set_disabled failure + destroy failure */
  g_cupertino_button_mock_destroy_fail = 1;
  rc = cupertino_button_create(dummy_engine, &desc, &button);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, button);
  g_cupertino_button_mock_set_disabled_fail = 0;
  g_cupertino_button_mock_destroy_fail = 0;

  /* Mock set_text failure on existing button */
  desc.is_disabled = 0;
  rc = cupertino_button_create(dummy_engine, &desc, &button);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_button_mock_set_text_fail = 1;
  rc = cupertino_button_set_text(button, "Fail");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_button_mock_set_text_fail = 0;

  /* Mock destroy failure */
  g_cupertino_button_mock_destroy_fail = 1;
  rc = cupertino_button_destroy(button);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_button_mock_destroy_fail = 0;
#endif

  PASS();
}

SUITE(cupertino_button_suite) {
  RUN_TEST(test_button_lifecycle_and_properties);
  RUN_TEST(test_button_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_button_suite);
  GREATEST_MAIN_END();
}
