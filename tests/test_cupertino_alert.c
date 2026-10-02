/**
 * @file test_cupertino_alert.c
 * @brief Unit tests for Cupertino Alert Dialog component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_alert.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_alert_suite);

TEST test_alert_invalid_args(void) {
  struct cupertino_alert_descriptor desc;
  struct cupertino_alert *alert = NULL;
  struct ui_alert_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *str = NULL;
  size_t idx = 0;
  size_t count = 0;
  float scale = 0.0f;
  int is_open = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid arguments */
  rc = cupertino_alert_create(NULL, &desc, &alert);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_create(dummy_engine, NULL, &alert);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid argument */
  rc = cupertino_alert_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create valid alert for method-level invalid checks */
  desc.title = "Test";
  desc.message = "Message";
  rc = cupertino_alert_create(dummy_engine, &desc, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(alert != NULL);

  /* Set / get title invalid */
  rc = cupertino_alert_set_title(NULL, "Title");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_set_title(alert, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_title(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_title(alert, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set / get message invalid */
  rc = cupertino_alert_set_message(NULL, "Message");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_set_message(alert, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_message(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_message(alert, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add action invalid */
  rc = cupertino_alert_add_action(NULL, "OK", CUPERTINO_ALERT_ACTION_DEFAULT,
                                  &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_add_action(alert, NULL, CUPERTINO_ALERT_ACTION_DEFAULT,
                                  &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add text field invalid */
  rc = cupertino_alert_add_text_field(NULL, "Hint", 0, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Text field set / get invalid */
  rc = cupertino_alert_set_text_field_text(NULL, 0, "Hello");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_set_text_field_text(alert, 0, "Hello");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_text_field_text(NULL, 0, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_text_field_text(alert, 0, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Open / is_open invalid */
  rc = cupertino_alert_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_is_open(NULL, &is_open);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_is_open(alert, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Action count invalid */
  rc = cupertino_alert_get_action_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_action_count(alert, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Spring scale invalid */
  rc = cupertino_alert_get_spring_scale(NULL, 0.5f, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_spring_scale(alert, 0.5f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_spring_scale(alert, -0.1f, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_spring_scale(alert, 1.1f, &scale);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get base invalid */
  rc = cupertino_alert_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_base(alert, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy */
  rc = cupertino_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_alert_lifecycle_and_actions(void) {
  struct cupertino_alert_descriptor desc;
  struct cupertino_alert *alert = NULL;
  struct ui_alert_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *str = NULL;
  size_t idx = 0;
  size_t count = 0;
  float scale = 0.0f;
  int is_open = 0;
  ui_error_t rc;

  /* Test creation with no title or message, is_status_role = 1 */
  memset(&desc, 0, sizeof(desc));
  desc.is_status_role = 1;
  rc = cupertino_alert_create(dummy_engine, &desc, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(alert != NULL);

  /* Verify empty strings */
  rc = cupertino_alert_get_title(alert, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", str);

  rc = cupertino_alert_get_message(alert, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", str);

  /* Add text field with NULL placeholder and NULL out_index */
  rc = cupertino_alert_add_text_field(alert, NULL, 0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add action with NULL out_index */
  rc = cupertino_alert_add_action(alert, "Dismiss",
                                  CUPERTINO_ALERT_ACTION_CANCEL, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test setting text field with NULL text */
  rc = cupertino_alert_set_text_field_text(alert, 0, "Initial");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_alert_get_text_field_text(alert, 0, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Initial", str);

  rc = cupertino_alert_set_text_field_text(alert, 0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_alert_get_text_field_text(alert, 0, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", str);

  /* Out of range / NULL out_text check */
  rc = cupertino_alert_get_text_field_text(alert, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy alert with base == NULL branch */
  base = alert->base;
  alert->base = NULL;
  rc = cupertino_alert_set_open(alert, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Destroy the freed base */
  rc = ui_alert_base_destroy(base);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Second alert with full title/message and status role = 0 */
  memset(&desc, 0, sizeof(desc));
  desc.title = "Delete File?";
  desc.message = "This action cannot be undone.";
  desc.is_status_role = 0;

  rc = cupertino_alert_create(dummy_engine, &desc, &alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(alert != NULL);

  /* Check base */
  rc = cupertino_alert_get_base(alert, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Check title and message */
  rc = cupertino_alert_get_title(alert, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Delete File?", str);

  rc = cupertino_alert_get_message(alert, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("This action cannot be undone.", str);

  /* Modify title and message */
  rc = cupertino_alert_set_title(alert, "Warning");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_alert_get_title(alert, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Warning", str);

  rc = cupertino_alert_set_message(alert, "Low disk space.");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_alert_get_message(alert, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Low disk space.", str);

  /* Add actions: Cancel, Delete */
  rc = cupertino_alert_add_action(alert, "Cancel",
                                  CUPERTINO_ALERT_ACTION_CANCEL, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  rc = cupertino_alert_add_action(alert, "Delete",
                                  CUPERTINO_ALERT_ACTION_DESTRUCTIVE, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  rc = cupertino_alert_get_action_count(alert, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, count);

  /* Add text field */
  rc = cupertino_alert_add_text_field(alert, "Password", 1, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  rc = cupertino_alert_get_text_field_text(alert, 0, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("", str);

  rc = cupertino_alert_set_text_field_text(alert, 0, "secret123");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_alert_get_text_field_text(alert, 0, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("secret123", str);

  /* Text field out of bounds */
  rc = cupertino_alert_set_text_field_text(alert, 5, "test");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_alert_get_text_field_text(alert, 5, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Open / close state */
  rc = cupertino_alert_is_open(alert, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  rc = cupertino_alert_set_open(alert, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_alert_is_open(alert, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_open);

  rc = cupertino_alert_set_open(alert, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_alert_is_open(alert, &is_open);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_open);

  /* Spring scale animation check */
  rc = cupertino_alert_get_spring_scale(alert, 0.0f, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scale > 0.84f && scale < 0.86f);

  rc = cupertino_alert_get_spring_scale(alert, 1.0f, &scale);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scale > 0.95f && scale < 1.05f);

  /* Max text fields limit */
  rc = cupertino_alert_add_text_field(alert, "User", 0, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);
  /* 3rd text field returns OUT_OF_MEMORY */
  rc = cupertino_alert_add_text_field(alert, "Extra", 0, &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* Fill actions to max */
  while (alert->action_count < CUPERTINO_ALERT_MAX_ACTIONS) {
    rc = cupertino_alert_add_action(alert, "More",
                                    CUPERTINO_ALERT_ACTION_DEFAULT, &idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceeding max actions */
  rc = cupertino_alert_add_action(alert, "Overflow",
                                  CUPERTINO_ALERT_ACTION_DEFAULT, &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* Destroy */
  rc = cupertino_alert_destroy(alert);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_alert_mock_failures(void) {
  struct cupertino_alert_descriptor desc;
  struct cupertino_alert *alert = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float scale = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "Mock Test";
  desc.message = "Testing error paths";

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_cupertino_alert_mock_base_create_fail;
    extern int g_cupertino_alert_mock_base_set_role_fail;
    extern int g_cupertino_alert_mock_base_set_dismissible_fail;
    extern int g_cupertino_alert_mock_base_set_open_fail;
    extern int g_cupertino_alert_mock_base_destroy_fail;
    extern int g_cupertino_alert_mock_spring_get_preset_fail;
    extern int g_cupertino_alert_mock_spring_evaluate_fail;
    extern struct ui_alert_base *g_cupertino_alert_last_created_base;

    /* Base create fail */
    g_cupertino_alert_mock_base_create_fail = 1;
    rc = cupertino_alert_create(dummy_engine, &desc, &alert);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(alert == NULL);
    g_cupertino_alert_mock_base_create_fail = 0;

    /* Set role fail with destroy success */
    g_cupertino_alert_mock_base_set_role_fail = 1;
    g_cupertino_alert_mock_base_destroy_fail = 0;
    rc = cupertino_alert_create(dummy_engine, &desc, &alert);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    ASSERT(alert == NULL);

    /* Set role fail with destroy fail */
    g_cupertino_alert_mock_base_set_role_fail = 1;
    g_cupertino_alert_mock_base_destroy_fail = 1;
    rc = cupertino_alert_create(dummy_engine, &desc, &alert);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    ASSERT(alert == NULL);
    g_cupertino_alert_mock_base_set_role_fail = 0;
    g_cupertino_alert_mock_base_destroy_fail = 0;
    if (g_cupertino_alert_last_created_base) {
      rc = ui_alert_base_destroy(g_cupertino_alert_last_created_base);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      g_cupertino_alert_last_created_base = NULL;
    }

    /* Set dismissible fail with destroy success */
    g_cupertino_alert_mock_base_set_dismissible_fail = 1;
    g_cupertino_alert_mock_base_destroy_fail = 0;
    rc = cupertino_alert_create(dummy_engine, &desc, &alert);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    ASSERT(alert == NULL);

    /* Set dismissible fail with destroy fail */
    g_cupertino_alert_mock_base_set_dismissible_fail = 1;
    g_cupertino_alert_mock_base_destroy_fail = 1;
    rc = cupertino_alert_create(dummy_engine, &desc, &alert);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    ASSERT(alert == NULL);
    g_cupertino_alert_mock_base_set_dismissible_fail = 0;
    g_cupertino_alert_mock_base_destroy_fail = 0;
    if (g_cupertino_alert_last_created_base) {
      rc = ui_alert_base_destroy(g_cupertino_alert_last_created_base);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      g_cupertino_alert_last_created_base = NULL;
    }

    /* Create normal alert */
    rc = cupertino_alert_create(dummy_engine, &desc, &alert);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(alert != NULL);

    /* Set open fail */
    g_cupertino_alert_mock_base_set_open_fail = 1;
    rc = cupertino_alert_set_open(alert, 1);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_alert_mock_base_set_open_fail = 0;

    /* Spring preset fail */
    g_cupertino_alert_mock_spring_get_preset_fail = 1;
    rc = cupertino_alert_get_spring_scale(alert, 0.5f, &scale);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_alert_mock_spring_get_preset_fail = 0;

    /* Spring evaluate fail */
    g_cupertino_alert_mock_spring_evaluate_fail = 1;
    rc = cupertino_alert_get_spring_scale(alert, 0.5f, &scale);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_alert_mock_spring_evaluate_fail = 0;

    /* Destroy fail */
    g_cupertino_alert_mock_base_destroy_fail = 1;
    rc = cupertino_alert_destroy(alert);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_cupertino_alert_mock_base_destroy_fail = 0;

    /* Normal destroy */
    rc = cupertino_alert_destroy(alert);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
#endif

  PASS();
}

TEST test_alert_oom_mock(void) {
  struct cupertino_alert_descriptor desc;
  struct cupertino_alert *alert = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail malloc for struct cupertino_alert */
  g_malloc_fail_countdown = 0;
  rc = cupertino_alert_create(dummy_engine, &desc, &alert);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(alert == NULL);

  /* Fail malloc for ui_alert_base */
  g_malloc_fail_countdown = 1;
  rc = cupertino_alert_create(dummy_engine, &desc, &alert);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(alert == NULL);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_alert_create(NULL, &desc, &alert);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_alert_suite) {
  RUN_TEST(test_alert_invalid_args);
  RUN_TEST(test_alert_lifecycle_and_actions);
  RUN_TEST(test_alert_mock_failures);
  RUN_TEST(test_alert_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_alert_suite);
  GREATEST_MAIN_END();
}
