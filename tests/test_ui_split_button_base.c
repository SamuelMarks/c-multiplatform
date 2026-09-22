struct ui_split_button_base {
  struct ui_component *component;
  struct ui_button_base *main_button;
  struct ui_button_base *trigger_button;
  struct ui_signal *disabled_signal;
  struct ui_signal *text_signal;
};
/* clang-format off */
#include "ui_split_button_base.h"
#include "ui_error.h"
#include "../src/ui_internal_mem.h"
#include <assert.h>
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
extern int g_button_mock_fail;
extern int g_split_button_mock_destroy_fail;
extern int g_split_button_mock_btn_destroy_fail;
extern int g_split_button_mock_get_comp_fail;
extern int g_split_button_mock_append_fail;
extern int g_split_button_mock_node_destroy_fail;
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

  {
    ui_error_t rc_cleanup = ui_split_button_base_destroy(btn);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_split_button_base_destroy(NULL);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
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

  {
    ui_error_t rc_cleanup = ui_split_button_base_destroy(btn);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  return 0;
}

static int test_ui_split_button_base_disabled(void) {
  struct ui_split_button_base *btn = NULL;

  ASSERT_SUCCESS(ui_split_button_base_create(&btn));

  ASSERT_EQ(ui_split_button_base_set_disabled(NULL, 1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_split_button_base_set_disabled(btn, 1));
  ASSERT_SUCCESS(ui_split_button_base_set_disabled(btn, 0));

#ifdef UI_TEST_MOCK_ALLOC
  /* Test branch where main_button set_disabled fails */
  g_button_mock_fail = 290;
  ASSERT_EQ(ui_split_button_base_set_disabled(btn, 1), UI_ERROR_UNKNOWN);
  g_button_mock_fail = 0;
#endif

  {
    ui_error_t rc_cleanup = ui_split_button_base_destroy(btn);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
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

  {
    ui_error_t rc_cleanup = ui_split_button_base_destroy(btn);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
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
      {
        ui_error_t rc_cleanup = ui_split_button_base_destroy(btn);
        assert(rc_cleanup == UI_ERROR_NONE);
      }
      break;
    }
  }
#endif
  return 0;
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_split_button_mock_parse_css_fail;
extern int g_split_button_mock_set_style_fail;

static int test_ui_split_button_base_mock_failures(void) {
  struct ui_split_button_base *btn = NULL;
  ui_error_t rc;

  /* 1. ui_button_base_get_component failure during create (1st call:
   * main_button) */
  g_split_button_mock_get_comp_fail = 1;
  rc = ui_split_button_base_create(&btn);
  assert(rc == UI_ERROR_UNKNOWN);
  assert(btn == NULL);
  g_split_button_mock_get_comp_fail = 0;

  /* 1b. ui_button_base_get_component failure during create (2nd call:
   * trigger_button) */
  g_split_button_mock_get_comp_fail = 2;
  rc = ui_split_button_base_create(&btn);
  assert(rc == UI_ERROR_UNKNOWN);
  assert(btn == NULL);
  g_split_button_mock_get_comp_fail = 0;

  /* 2. ui_dom_node_append_child failure during create (1st call) */
  g_split_button_mock_append_fail = 1;
  rc = ui_split_button_base_create(&btn);
  assert(rc == UI_ERROR_UNKNOWN);
  assert(btn == NULL);
  g_split_button_mock_append_fail = 0;

  /* 2b. ui_dom_node_append_child failure during create (2nd call) */
  g_split_button_mock_append_fail = 2;
  rc = ui_split_button_base_create(&btn);
  assert(rc == UI_ERROR_UNKNOWN);
  assert(btn == NULL);
  g_split_button_mock_append_fail = 0;

  /* 2c. parse_css failure during create */
  g_split_button_mock_parse_css_fail = 1;
  rc = ui_split_button_base_create(&btn);
  assert(rc == UI_ERROR_UNKNOWN);
  assert(btn == NULL);
  g_split_button_mock_parse_css_fail = 0;

  /* 2d. set_default_style failure during create */
  g_split_button_mock_set_style_fail = 1;
  rc = ui_split_button_base_create(&btn);
  assert(rc == UI_ERROR_UNKNOWN);
  assert(btn == NULL);
  g_split_button_mock_set_style_fail = 0;

  /* 3. root_node destroy failure during create cleanup */
  g_split_button_mock_node_destroy_fail = 1;
  g_split_button_mock_append_fail = 1;
  rc = ui_split_button_base_create(&btn);
  assert(rc != UI_ERROR_NONE);
  assert(btn == NULL);
  g_split_button_mock_node_destroy_fail = 0;
  g_split_button_mock_append_fail = 0;

  /* 4. button destroy failure during create cleanup */
  g_split_button_mock_btn_destroy_fail = 1;
  g_split_button_mock_append_fail = 1;
  rc = ui_split_button_base_create(&btn);
  assert(rc != UI_ERROR_NONE);
  assert(btn == NULL);
  g_split_button_mock_btn_destroy_fail = 0;
  g_split_button_mock_append_fail = 0;

  /* 5. component destroy failure during create cleanup */
  g_split_button_mock_destroy_fail = 1;
  g_split_button_mock_append_fail = 1;
  rc = ui_split_button_base_create(&btn);
  assert(rc != UI_ERROR_NONE);
  assert(btn == NULL);
  g_split_button_mock_destroy_fail = 0;
  g_split_button_mock_append_fail = 0;

  /* 6. component destroy failure during normal destroy */
  rc = ui_split_button_base_create(&btn);
  assert(rc == UI_ERROR_NONE);
  assert(btn != NULL);
  g_split_button_mock_destroy_fail = 1;
  rc = ui_split_button_base_destroy(btn);
  assert(rc == UI_ERROR_UNKNOWN);
  g_split_button_mock_destroy_fail = 0;

  /* 7. button destroy failure during normal destroy */
  rc = ui_split_button_base_create(&btn);
  assert(rc == UI_ERROR_NONE);
  assert(btn != NULL);
  g_split_button_mock_btn_destroy_fail = 1;
  rc = ui_split_button_base_destroy(btn);
  assert(rc == UI_ERROR_UNKNOWN);
  g_split_button_mock_btn_destroy_fail = 0;

  /* 8. Partial split button destruction */
  {
    /* Case A: trigger_button is NULL */
    rc = ui_split_button_base_create(&btn);
    assert(rc == UI_ERROR_NONE);
    assert(btn != NULL);
    ui_button_base_destroy(btn->trigger_button);
    btn->trigger_button = NULL;
    rc = ui_split_button_base_destroy(btn);
    assert(rc == UI_ERROR_NONE);

    /* Case B: component is NULL (with trigger_button and main_button intact) */
    rc = ui_split_button_base_create(&btn);
    assert(rc == UI_ERROR_NONE);
    assert(btn != NULL);
    btn->component->shadow_root = NULL;
    ui_component_destroy(btn->component);
    btn->component = NULL;
    rc = ui_split_button_base_destroy(btn);
    assert(rc == UI_ERROR_NONE);

    /* Case C: main_button is NULL */
    rc = ui_split_button_base_create(&btn);
    assert(rc == UI_ERROR_NONE);
    assert(btn != NULL);
    ui_button_base_destroy(btn->main_button);
    btn->main_button = NULL;
    rc = ui_split_button_base_destroy(btn);
    assert(rc == UI_ERROR_NONE);
  }

  /* 9. get_component returns NULL tmp_comp during destroy */
  {
    /* 9a. trigger_button get_comp fails during normal destroy */
    rc = ui_split_button_base_create(&btn);
    assert(rc == UI_ERROR_NONE);
    g_split_button_mock_get_comp_fail = 1;
    rc = ui_split_button_base_destroy(btn);
    assert(rc == UI_ERROR_NONE);
    g_split_button_mock_get_comp_fail = 0;

    /* 9b. main_button get_comp fails during normal destroy */
    rc = ui_split_button_base_create(&btn);
    assert(rc == UI_ERROR_NONE);
    g_split_button_mock_get_comp_fail = 2;
    rc = ui_split_button_base_destroy(btn);
    assert(rc == UI_ERROR_NONE);
    g_split_button_mock_get_comp_fail = 0;
  }
  return 0;
}
#endif

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
#ifdef UI_TEST_MOCK_ALLOC
  if (test_ui_split_button_base_mock_failures())
    return 1;
#endif
  return 0;
}
