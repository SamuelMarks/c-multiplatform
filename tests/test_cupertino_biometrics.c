/**
 * @file test_cupertino_biometrics.c
 * @brief Unit tests for Apple Biometric Prompt Sheet.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_biometrics.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_biometrics_suite);

TEST test_biometrics_invalid_arguments(void) {
  struct cupertino_biometrics_descriptor desc;
  struct cupertino_biometrics_prompt *prompt = NULL;
  struct ui_dialog_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_biometric_state state;
  float w, h, offset;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_biometrics_create(NULL, &desc, &prompt);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_create(dummy_engine, NULL, &prompt);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range type */
  desc.type = (enum cupertino_biometric_type) - 1;
  rc = cupertino_biometrics_create(dummy_engine, &desc, &prompt);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.type = (enum cupertino_biometric_type)99;
  rc = cupertino_biometrics_create(dummy_engine, &desc, &prompt);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_biometrics_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Present / dismiss / authenticate / passcode / tick invalid */
  rc = cupertino_biometrics_present(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_dismiss(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_authenticate(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_switch_to_passcode(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_tick(NULL, 16.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid prompt to test getters with NULL outputs and negative tick */
  memset(&desc, 0, sizeof(desc));
  desc.passcode_button_title = "Custom Passcode";
  desc.cancel_button_title = "Custom Cancel";
  rc = cupertino_biometrics_create(dummy_engine, &desc, &prompt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Custom Passcode", prompt->passcode_button_title);
  ASSERT_STR_EQ("Custom Cancel", prompt->cancel_button_title);

  rc = cupertino_biometrics_tick(prompt, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Getters invalid */
  rc = cupertino_biometrics_get_state(NULL, &state);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_get_state(prompt, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_biometrics_get_shake_offset(NULL, &offset);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_get_shake_offset(prompt, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_biometrics_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_get_dimensions(prompt, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_get_dimensions(prompt, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_biometrics_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_biometrics_get_base(prompt, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Scan progress rollover > 1.0f */
  rc = cupertino_biometrics_present(prompt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_biometrics_tick(prompt, 1500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy with prompt->base == NULL */
  struct ui_dialog_base *saved_base = prompt->base;
  prompt->base = NULL;
  rc = cupertino_biometrics_destroy(prompt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_dialog_base_destroy(saved_base);

  PASS();
}

TEST test_biometrics_lifecycle_and_shake(void) {
  struct cupertino_biometrics_descriptor desc;
  struct cupertino_biometrics_prompt *prompt = NULL;
  struct ui_dialog_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_biometric_state state;
  float offset = 0.0f;
  float w, h;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.type = CUPERTINO_BIOMETRIC_FACE_ID;
  desc.reason = "Confirm your purchase";

  rc = cupertino_biometrics_create(dummy_engine, &desc, &prompt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(prompt != NULL);

  rc = cupertino_biometrics_get_state(prompt, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_BIOMETRIC_STATE_READY, state);

  rc = cupertino_biometrics_get_dimensions(prompt, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_BIOMETRICS_WIDTH, w);
  ASSERT_EQ(CUPERTINO_BIOMETRICS_MIN_HEIGHT, h);

  rc = cupertino_biometrics_get_base(prompt, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Present prompt */
  rc = cupertino_biometrics_present(prompt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_biometrics_get_state(prompt, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_BIOMETRIC_STATE_SCANNING, state);

  /* Advance scan animation */
  rc = cupertino_biometrics_tick(prompt, 500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(prompt->scan_progress > 0.0f);

  /* Simulate match failure */
  rc = cupertino_biometrics_authenticate(prompt, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_biometrics_get_state(prompt, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_BIOMETRIC_STATE_FAILED, state);

  /* Advance shake animation */
  rc = cupertino_biometrics_tick(prompt, 100.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_biometrics_get_shake_offset(prompt, &offset);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Advance past shake duration */
  rc = cupertino_biometrics_tick(prompt, 500.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_biometrics_get_shake_offset(prompt, &offset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, offset);

  /* Passcode fallback */
  rc = cupertino_biometrics_switch_to_passcode(prompt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_biometrics_get_state(prompt, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_BIOMETRIC_STATE_PASSCODE_FALLBACK, state);

  /* Tick while in PASSCODE_FALLBACK (neither SCANNING nor FAILED) */
  rc = cupertino_biometrics_tick(prompt, 16.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Authenticate success */
  rc = cupertino_biometrics_authenticate(prompt, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_biometrics_get_state(prompt, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_BIOMETRIC_STATE_SUCCESS, state);

  /* Dismiss */
  rc = cupertino_biometrics_dismiss(prompt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_biometrics_get_state(prompt, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_BIOMETRIC_STATE_READY, state);

  rc = cupertino_biometrics_destroy(prompt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_biometrics_oom_mock(void) {
  struct cupertino_biometrics_descriptor desc;
  struct cupertino_biometrics_prompt *prompt = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_biometrics_mock_dialog_create_fail;
  extern int g_cupertino_biometrics_mock_dialog_destroy_fail;

  g_malloc_fail_countdown = 0;
  rc = cupertino_biometrics_create(dummy_engine, &desc, &prompt);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, prompt);
  g_malloc_fail_countdown = -1;

  /* Mock dialog_create failure */
  g_cupertino_biometrics_mock_dialog_create_fail = 1;
  rc = cupertino_biometrics_create(dummy_engine, &desc, &prompt);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, prompt);
  g_cupertino_biometrics_mock_dialog_create_fail = 0;

  /* Mock dialog_destroy failure */
  rc = cupertino_biometrics_create(dummy_engine, &desc, &prompt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_biometrics_mock_dialog_destroy_fail = 1;
  rc = cupertino_biometrics_destroy(prompt);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_biometrics_mock_dialog_destroy_fail = 0;
#else
  rc = cupertino_biometrics_create(NULL, &desc, &prompt);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_biometrics_suite) {
  RUN_TEST(test_biometrics_invalid_arguments);
  RUN_TEST(test_biometrics_lifecycle_and_shake);
  RUN_TEST(test_biometrics_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_biometrics_suite);
  GREATEST_MAIN_END();
}
