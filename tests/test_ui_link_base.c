/* clang-format off */
#include "ui_link_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t test_link_base(void) {
  struct ui_link_base *link;
  ui_error_t err;
  const char *attr_val;
  struct ui_signal *signal = (struct ui_signal *)1;

  /* Null checks */
  assert(ui_link_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_link_base_set_href(NULL, "a") == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_link_base_set_text(NULL, "a") == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_link_base_bind_disabled(NULL, signal) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_link_base_bind_text(NULL, signal) == UI_ERROR_INVALID_ARGUMENT);

  /* Create OOM scenarios */
  {
    int oom_i;
    for (oom_i = 0; oom_i < 12; oom_i++) {
      g_malloc_fail_countdown = oom_i;
      err = ui_link_base_create(&link);
      if (err == UI_ERROR_NONE) {
        ui_component_destroy((struct ui_component *)link);
        break;
      }
    }
    g_malloc_fail_countdown = -1;
  }

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_link_base_mock_fail;
    g_link_base_mock_fail = 1;
    g_malloc_fail_countdown = 1;
    err = ui_link_base_create(&link);
    assert(err != UI_ERROR_NONE);

    g_link_base_mock_fail = 2;
    g_malloc_fail_countdown = 3;
    err = ui_link_base_create(&link);
    assert(err != UI_ERROR_NONE);

    g_link_base_mock_fail = 0;
    g_malloc_fail_countdown = -1;
  }
#endif

  err = ui_link_base_create(&link);
  assert(err == UI_ERROR_NONE);

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_link_base_mock_fail;
    g_link_base_mock_fail = 3;
    err = ui_link_base_set_text(link, "Fail Append");
    assert(err != UI_ERROR_NONE);
    g_link_base_mock_fail = 0;
  }
#endif

  assert(ui_link_base_set_href(link, NULL) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_link_base_set_text(link, NULL) == UI_ERROR_INVALID_ARGUMENT);

  err = ui_link_base_set_href(link, "https://example.com");
  assert(err == UI_ERROR_NONE);

  err = ui_link_base_set_text(link, "Click Here");
  assert(err == UI_ERROR_NONE);

  err = ui_dom_node_get_attribute(link->base.shadow_root, "href", &attr_val);
  assert(err == UI_ERROR_NONE && strcmp(attr_val, "https://example.com") == 0);

  assert(link->base.shadow_root->first_child != NULL);
  assert(strcmp(link->base.shadow_root->first_child->text_content,
                "Click Here") == 0);

  /* Set text again to cover existing text node branch */
  err = ui_link_base_set_text(link, "Click Again");
  assert(err == UI_ERROR_NONE);
  assert(strcmp(link->base.shadow_root->first_child->text_content,
                "Click Again") == 0);

  /* Set text with OOM for text node creation */
  {
    ui_error_t rc_cleanup =
        ui_dom_node_destroy(link->base.shadow_root->first_child);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }
  link->base.shadow_root->first_child = NULL;

  g_malloc_fail_countdown = 0;
  err = ui_link_base_set_text(link, "OOM Text");
  assert(err == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  /* Binding */
  err = ui_link_base_bind_disabled(link, signal);
  assert(err == UI_ERROR_NONE);
  assert(link->disabled_signal == signal);

  err = ui_link_base_bind_text(link, signal);
  assert(err == UI_ERROR_NONE);
  assert(link->text_signal == signal);

  {
    ui_error_t rc_cleanup = ui_component_destroy((struct ui_component *)link);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
  }

  return UI_ERROR_NONE;
}

int main(void) {
  test_link_base();
  printf("test_ui_link_base passed\n");
  return 0;
}
