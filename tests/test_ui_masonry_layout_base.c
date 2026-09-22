/* clang-format off */
#include "ui_masonry_layout_base.h"
#include "ui_component.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

struct ui_masonry_layout_base {
  struct ui_component *component;
  struct ui_computed *data_signal;
};

extern int g_malloc_fail_countdown;

static ui_error_t test_masonry_creation(void) {
  struct ui_masonry_layout_base *masonry = NULL;
  ui_error_t rc;

  rc = ui_masonry_layout_base_create(&masonry);
  assert(rc == UI_ERROR_NONE);
  assert(masonry != NULL);

  rc = ui_masonry_layout_base_destroy(masonry);
  assert(rc == UI_ERROR_NONE);

  printf("test_masonry_creation passed\n");
  return UI_ERROR_NONE;
}

static ui_error_t test_masonry_reflow(void) {
  struct ui_masonry_layout_base *masonry = NULL;
  ui_error_t rc;

  rc = ui_masonry_layout_base_create(&masonry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_masonry_layout_base_reflow(masonry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_masonry_layout_base_destroy(masonry);
  assert(rc == UI_ERROR_NONE);

  printf("test_masonry_reflow passed\n");
  return UI_ERROR_NONE;
}

static ui_error_t test_masonry_errors(void) {
  int i;
  struct ui_masonry_layout_base *masonry = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_masonry_layout_base_create(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_masonry_layout_base_destroy(NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_masonry_layout_base_reflow(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_masonry_layout_base_get_component(NULL, &comp);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_masonry_layout_base_get_component(NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_masonry_layout_base_bind_data(NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_masonry_layout_base_create(&masonry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_masonry_layout_base_get_component(masonry, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_masonry_layout_base_get_component(masonry, &comp);
  assert(rc == UI_ERROR_NONE);

  rc = ui_masonry_layout_base_bind_data(masonry, NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_masonry_layout_base_destroy(masonry);
  assert(rc == UI_ERROR_NONE);

  /* Test destroying masonry with NULL component */
  rc = ui_masonry_layout_base_create(&masonry);
  assert(rc == UI_ERROR_NONE);
  rc = ui_component_destroy(masonry->component);
  assert(rc == UI_ERROR_NONE);
  masonry->component = NULL;
  rc = ui_masonry_layout_base_destroy(masonry);
  assert(rc == UI_ERROR_NONE);

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_masonry_mock_fail;
    g_masonry_mock_fail = 1;
    rc = ui_masonry_layout_base_create(&masonry);
    assert(rc == UI_ERROR_UNKNOWN);
    g_masonry_mock_fail = 0;
  }
#endif

  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_masonry_layout_base_create(&masonry);
    if (rc == UI_ERROR_NONE) {
      rc = ui_masonry_layout_base_destroy(masonry);
      assert(rc == UI_ERROR_NONE);
      break;
    }
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
  }
  g_malloc_fail_countdown = -1;

  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc;

  rc = test_masonry_errors();
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  rc = test_masonry_creation();
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  rc = test_masonry_reflow();
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  return 0;
}
