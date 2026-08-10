/* clang-format off */
#include "ui_page_control_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static void test_page_control_base(void) {
  struct ui_page_control_base *control;
  ui_error_t err;
  const char *attr_val;
  struct ui_signal *signal = (struct ui_signal *)0x1234;

  err = ui_page_control_base_create(&control);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_page_control_base_create failed\n");
    exit(1);
  }

  err = ui_page_control_base_set_number_of_pages(control, 5);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_page_control_base_set_number_of_pages failed\n");
    exit(1);
  }

  err = ui_page_control_base_set_current_page(control, 2);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_page_control_base_set_current_page failed\n");
    exit(1);
  }

  err = ui_dom_node_get_attribute(control->base.shadow_root, "data-total-pages",
                                  &attr_val);
  if (err != UI_ERROR_NONE || strcmp(attr_val, "5") != 0) {
    fprintf(stderr, "ui_page_control_base_set_number_of_pages did not set "
                    "attribute correctly\n");
    exit(1);
  }

  err = ui_dom_node_get_attribute(control->base.shadow_root,
                                  "data-current-page", &attr_val);
  if (err != UI_ERROR_NONE || strcmp(attr_val, "2") != 0) {
    fprintf(stderr, "ui_page_control_base_set_current_page did not set "
                    "attribute correctly\n");
    exit(1);
  }

  /* Test bind_current_page */
  err = ui_page_control_base_bind_current_page(control, signal);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_page_control_base_bind_current_page failed\n");
    exit(1);
  }
  if (control->current_page_signal != signal) {
    fprintf(stderr,
            "ui_page_control_base_bind_current_page did not set signal\n");
    exit(1);
  }

  (void)ui_component_destroy((struct ui_component *)control);
}

static void test_page_control_base_errors(void) {
  struct ui_page_control_base *control = NULL;
  ui_error_t err;

  /* NULL checks */
  if (ui_page_control_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);

  if (ui_page_control_base_set_number_of_pages(NULL, 1) !=
      UI_ERROR_INVALID_ARGUMENT)
    exit(1);

  if (ui_page_control_base_set_current_page(NULL, 1) !=
      UI_ERROR_INVALID_ARGUMENT)
    exit(1);

  if (ui_page_control_base_bind_current_page(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    exit(1);

  /* Create valid control for more tests */
  err = ui_page_control_base_create(&control);
  if (err != UI_ERROR_NONE)
    exit(1);

  /* Invalid ranges */
  if (ui_page_control_base_set_number_of_pages(control, -1) !=
      UI_ERROR_INVALID_ARGUMENT)
    exit(1);
  if (ui_page_control_base_set_current_page(control, -1) !=
      UI_ERROR_INVALID_ARGUMENT)
    exit(1);

  /* Force update_page_control_dom NULL */
  /* We can't directly call update_page_control_dom with NULL, it's static.
     Wait, it's called with NULL inside the set functions if control is NULL,
     but the set functions already check if control is NULL and return early.
     Wait, is update_page_control_dom's NULL check unreachable?
     Let's check `ui_page_control_base.c`:
     if (!control || count < 0) return UI_ERROR_INVALID_ARGUMENT;
     (void)update_page_control_dom(control);
     So update_page_control_dom is only called with non-null control.
     Its own !control check is unreachable!
  */

  (void)ui_component_destroy((struct ui_component *)control);
}

static void test_page_control_base_oom(void) {
  struct ui_page_control_base *control;
  int countdown;

  for (countdown = 0; countdown < 5; countdown++) {
    g_malloc_fail_countdown = countdown;
    if (ui_page_control_base_create(&control) == UI_ERROR_NONE) {
      (void)ui_component_destroy((struct ui_component *)control);
    }
  }
  g_malloc_fail_countdown = -1;

  if (ui_page_control_base_create(&control) == UI_ERROR_NONE) {
    for (countdown = 0; countdown < 10; countdown++) {
      g_malloc_fail_countdown = countdown;
      (void)ui_page_control_base_set_number_of_pages(control, 10);
      g_malloc_fail_countdown = -1;
    }
    for (countdown = 0; countdown < 10; countdown++) {
      g_malloc_fail_countdown = countdown;
      (void)ui_page_control_base_set_current_page(control, 5);
      g_malloc_fail_countdown = -1;
    }
    (void)ui_component_destroy((struct ui_component *)control);
  }
}

int main(void) {
  test_page_control_base();
  test_page_control_base_errors();
  test_page_control_base_oom();
  printf("test_ui_page_control_base passed\n");
  return 0;
}
