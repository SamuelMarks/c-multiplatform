/**
 * @file test_cupertino_toolbar.c
 * @brief Unit tests for Cupertino Toolbar component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_toolbar.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_toolbar_suite);

TEST test_toolbar_invalid_args(void) {
  struct cupertino_toolbar_descriptor desc;
  struct cupertino_toolbar *toolbar = NULL;
  struct ui_toolbar_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  size_t count = 0;
  size_t idx = 0;
  float height = 0.0f;
  int disabled = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid */
  rc = cupertino_toolbar_create(NULL, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_create(dummy_engine, NULL, &toolbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid */
  rc = cupertino_toolbar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add action invalid */
  rc = cupertino_toolbar_add_action(NULL, "Delete", 1, 0, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add spaces invalid */
  rc = cupertino_toolbar_add_flexible_space(NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_add_fixed_space(NULL, 10.0f, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Disabled state invalid */
  rc = cupertino_toolbar_set_item_disabled(NULL, 0, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_is_item_disabled(NULL, 0, &disabled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Height / count / base invalid */
  rc = cupertino_toolbar_get_height(NULL, &height);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_get_item_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid toolbar to test NULL out_index and NULL output getters */
  rc = cupertino_toolbar_create(dummy_engine, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_toolbar_add_action(toolbar, NULL, 0, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_add_action(toolbar, "A", 0, 0, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_toolbar_add_flexible_space(toolbar, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_toolbar_add_fixed_space(toolbar, 10.0f, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_toolbar_is_item_disabled(toolbar, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_get_height(toolbar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_get_item_count(toolbar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_get_base(toolbar, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy with toolbar->base == NULL */
  struct ui_toolbar_base *saved_b = toolbar->base;
  toolbar->base = NULL;
  rc = cupertino_toolbar_destroy(toolbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_toolbar_base_destroy(saved_b);

  PASS();
}

TEST test_toolbar_lifecycle_and_items(void) {
  struct cupertino_toolbar_descriptor desc;
  struct cupertino_toolbar *toolbar = NULL;
  struct ui_toolbar_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  size_t idx = 0;
  size_t count = 0;
  float height = 0.0f;
  int disabled = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "Actions";
  desc.is_translucent = 1;

  rc = cupertino_toolbar_create(dummy_engine, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(toolbar != NULL);

  /* Check base */
  rc = cupertino_toolbar_get_base(toolbar, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Check height */
  rc = cupertino_toolbar_get_height(toolbar, &height);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(height > 43.9f && height < 44.1f);

  /* Add items: Share, Flexible Space, Delete */
  rc = cupertino_toolbar_add_action(toolbar, "Share", 0, 0, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  rc = cupertino_toolbar_add_flexible_space(toolbar, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  rc = cupertino_toolbar_add_fixed_space(toolbar, 16.0f, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, idx);

  /* Fixed space negative width invalid */
  rc = cupertino_toolbar_add_fixed_space(toolbar, -5.0f, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_toolbar_add_action(toolbar, "Delete", 1, 0, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, idx);

  rc = cupertino_toolbar_get_item_count(toolbar, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, count);

  /* Test disabled state */
  rc = cupertino_toolbar_is_item_disabled(toolbar, 0, &disabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, disabled);

  rc = cupertino_toolbar_set_item_disabled(toolbar, 0, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_toolbar_is_item_disabled(toolbar, 0, &disabled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, disabled);

  /* Out of bounds item index */
  rc = cupertino_toolbar_set_item_disabled(toolbar, 99, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_toolbar_is_item_disabled(toolbar, 99, &disabled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Fill up toolbar to max items */
  while (toolbar->item_count < CUPERTINO_TOOLBAR_MAX_ITEMS) {
    rc = cupertino_toolbar_add_action(toolbar, "Extra", 0, 0, &idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Exceeding max items returns OUT_OF_MEMORY */
  rc = cupertino_toolbar_add_action(toolbar, "Overflow", 0, 0, &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  rc = cupertino_toolbar_add_flexible_space(toolbar, &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  rc = cupertino_toolbar_add_fixed_space(toolbar, 8.0f, &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* Destroy */
  rc = cupertino_toolbar_destroy(toolbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_toolbar_oom_mock(void) {
  struct cupertino_toolbar_descriptor desc;
  struct cupertino_toolbar *toolbar = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_toolbar_mock_create_fail;
  extern int g_cupertino_toolbar_mock_destroy_fail;
  extern int g_cupertino_toolbar_mock_set_title_fail;
  extern int g_cupertino_toolbar_mock_set_mode_fail;

  /* Fail malloc for struct cupertino_toolbar */
  g_malloc_fail_countdown = 0;
  rc = cupertino_toolbar_create(dummy_engine, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(toolbar == NULL);

  /* Fail malloc for ui_toolbar_base */
  g_malloc_fail_countdown = 1;
  rc = cupertino_toolbar_create(dummy_engine, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(toolbar == NULL);
  g_malloc_fail_countdown = -1;

  /* Fail mock create */
  g_cupertino_toolbar_mock_create_fail = 1;
  rc = cupertino_toolbar_create(dummy_engine, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(toolbar == NULL);
  g_cupertino_toolbar_mock_create_fail = 0;

  /* Fail mock set_title */
  desc.title = "Title";
  g_cupertino_toolbar_mock_set_title_fail = 1;
  rc = cupertino_toolbar_create(dummy_engine, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(toolbar == NULL);

  /* Fail mock set_title with mock destroy failure */
  g_cupertino_toolbar_mock_destroy_fail = 1;
  rc = cupertino_toolbar_create(dummy_engine, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(toolbar == NULL);
  g_cupertino_toolbar_mock_set_title_fail = 0;
  g_cupertino_toolbar_mock_destroy_fail = 0;

  /* Fail mock set_mode */
  desc.title = NULL;
  g_cupertino_toolbar_mock_set_mode_fail = 1;
  rc = cupertino_toolbar_create(dummy_engine, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(toolbar == NULL);

  /* Fail mock set_mode with mock destroy failure */
  g_cupertino_toolbar_mock_destroy_fail = 1;
  rc = cupertino_toolbar_create(dummy_engine, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(toolbar == NULL);
  g_cupertino_toolbar_mock_set_mode_fail = 0;
  g_cupertino_toolbar_mock_destroy_fail = 0;

  /* Fail mock destroy in cupertino_toolbar_destroy */
  rc = cupertino_toolbar_create(dummy_engine, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_toolbar_mock_destroy_fail = 1;
  rc = cupertino_toolbar_destroy(toolbar);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_toolbar_mock_destroy_fail = 0;

  rc = cupertino_toolbar_destroy(toolbar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = cupertino_toolbar_create(NULL, &desc, &toolbar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_toolbar_suite) {
  RUN_TEST(test_toolbar_invalid_args);
  RUN_TEST(test_toolbar_lifecycle_and_items);
  RUN_TEST(test_toolbar_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_toolbar_suite);
  GREATEST_MAIN_END();
}
