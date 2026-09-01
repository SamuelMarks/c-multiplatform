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
  g_malloc_fail_countdown = 0; /* Base component creation */
  err = ui_link_base_create(&link);
  assert(err == UI_ERROR_OUT_OF_MEMORY);

  g_malloc_fail_countdown = 1; /* ui_link_base struct alloc */
  err = ui_link_base_create(&link);
  assert(err == UI_ERROR_OUT_OF_MEMORY);

  g_malloc_fail_countdown = 2; /* root node creation */
  err = ui_link_base_create(&link);
  assert(err == UI_ERROR_OUT_OF_MEMORY);

  g_malloc_fail_countdown = 3; /* tag name alloc */
  err = ui_link_base_create(&link);
  assert(err == UI_ERROR_OUT_OF_MEMORY);

  g_malloc_fail_countdown = -1;

  err = ui_link_base_create(&link);
  assert(err == UI_ERROR_NONE);

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
      (void)rc_cleanup; /* Avoid override */
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
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return UI_ERROR_NONE;
}

int main(void) {
  test_link_base();
  printf("test_ui_link_base passed\n");
  return 0;
}
