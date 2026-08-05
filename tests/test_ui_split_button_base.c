/* clang-format off */
#include "ui_split_button_base.h"
#include "ui_error.h"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    ui_error_t _err = (expr);                                                  \
    if (_err != UI_ERROR_NONE) {                                               \
      printf("Failed at line %d: %d\n", __LINE__, _err);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    ui_error_t _err = (expr);                                                  \
    if (_err != (expected)) {                                                  \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             _err);                                                            \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_PTR_EQ(expr, expected)                                          \
  do {                                                                         \
    void *val = (expr);                                                        \
    if (val != (expected)) {                                                   \
      printf("Failed at line %d: expected %p, got %p\n", __LINE__,             \
             (void *)(expected), (void *)val);                                 \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static int test_ui_split_button_base_create_destroy(void) {
  struct ui_split_button_base *btn = NULL;

  ASSERT_EQ(ui_split_button_base_create(NULL), UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_split_button_base_create(&btn));
  if (!btn)
    return 1;

  (void)ui_split_button_base_destroy(btn);
  (void)ui_split_button_base_destroy(NULL);
  return 0;
}

static int test_ui_split_button_base_getters(void) {
  struct ui_split_button_base *btn = NULL;
  struct ui_button_base *main_btn = NULL;
  struct ui_button_base *trigger_btn = NULL;
  struct ui_component *comp = NULL;

  ASSERT_SUCCESS(ui_split_button_base_create(&btn));

  ASSERT_EQ(ui_split_button_base_get_main_button(NULL, &main_btn),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_get_main_button(btn, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_split_button_base_get_main_button(btn, &main_btn));

  ASSERT_EQ(ui_split_button_base_get_trigger_button(NULL, &trigger_btn),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_get_trigger_button(btn, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_split_button_base_get_trigger_button(btn, &trigger_btn));

  ASSERT_EQ(ui_split_button_base_get_component(NULL, &comp),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_get_component(btn, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_split_button_base_get_component(btn, &comp));

  (void)ui_split_button_base_destroy(btn);
  return 0;
}

static int test_ui_split_button_base_disabled(void) {
  struct ui_split_button_base *btn = NULL;

  ASSERT_SUCCESS(ui_split_button_base_create(&btn));

  ASSERT_EQ(ui_split_button_base_set_disabled(NULL, 1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_split_button_base_set_disabled(btn, 1));
  ASSERT_SUCCESS(ui_split_button_base_set_disabled(btn, 0));

  (void)ui_split_button_base_destroy(btn);
  return 0;
}

static int test_ui_split_button_base_bindings(void) {
  struct ui_split_button_base *btn = NULL;

  ASSERT_SUCCESS(ui_split_button_base_create(&btn));

  ASSERT_EQ(ui_split_button_base_bind_disabled(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_split_button_base_bind_disabled(btn, NULL));

  ASSERT_EQ(ui_split_button_base_bind_text(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_split_button_base_bind_text(btn, NULL));

  (void)ui_split_button_base_destroy(btn);
  return 0;
}

static int test_ui_split_button_base_allocation_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_split_button_base *btn = NULL;
  int i;
  ui_error_t err;

  /* Fail split button struct alloc */
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(ui_split_button_base_create(&btn), UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  for (i = 1; i < 2000; ++i) {
    g_malloc_fail_countdown = i;
    err = ui_split_button_base_create(&btn);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_NONE) {
      (void)ui_split_button_base_destroy(btn);
      break;
    }
  }
#endif
  return 0;
}

int main(void) {
  if (test_ui_split_button_base_create_destroy())
    return 1;
  if (test_ui_split_button_base_getters())
    return 1;
  if (test_ui_split_button_base_disabled())
    return 1;
  if (test_ui_split_button_base_bindings())
    return 1;
  if (test_ui_split_button_base_allocation_failures())
    return 1;
  return 0;
}
