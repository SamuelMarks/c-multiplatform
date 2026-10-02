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

  {
    ui_error_t rc_cleanup =
        ui_component_destroy((struct ui_component *)control);
    if (rc_cleanup != UI_ERROR_NONE) {
      exit(1);
    }
  }
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

  /* Test ui_page_control_base_destroy */
  if (ui_page_control_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    exit(1);

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_page_control_mock_fail;
    g_page_control_mock_fail = 2;
    if (ui_page_control_base_destroy(control) != UI_ERROR_UNKNOWN)
      exit(1);
    g_page_control_mock_fail = 0;
  }
#endif

  err = ui_page_control_base_destroy(control);
  if (err != UI_ERROR_NONE)
    exit(1);

  /* Test destroy with NULL shadow_root */
  err = ui_page_control_base_create(&control);
  if (err != UI_ERROR_NONE)
    exit(1);
  if (control->base.shadow_root) {
    ui_error_t rc_sr = ui_dom_node_destroy(control->base.shadow_root);
    if (rc_sr != UI_ERROR_NONE)
      exit(1);
    control->base.shadow_root = NULL;
  }
  err = ui_page_control_base_destroy(control);
  if (err != UI_ERROR_NONE)
    exit(1);
}

static void test_page_control_base_oom(void) {
  struct ui_page_control_base *control;
  int countdown;

  for (countdown = 0; countdown < 5; countdown++) {
    g_malloc_fail_countdown = countdown;
    if (ui_page_control_base_create(&control) == UI_ERROR_NONE) {
      ui_error_t rc_cleanup =
          ui_component_destroy((struct ui_component *)control);
      if (rc_cleanup != UI_ERROR_NONE) {
        exit(1);
      }
    }
  }
  g_malloc_fail_countdown = -1;

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_page_control_mock_fail;
    g_page_control_mock_fail = 1;
    g_malloc_fail_countdown = 1;
    if (ui_page_control_base_create(&control) == UI_ERROR_NONE) {
      exit(1);
    }

    g_page_control_mock_fail = 2;
    g_malloc_fail_countdown = 3;
    if (ui_page_control_base_create(&control) == UI_ERROR_NONE) {
      exit(1);
    }

    g_page_control_mock_fail = 0;
    g_malloc_fail_countdown = -1;
  }
#endif

  if (ui_page_control_base_create(&control) == UI_ERROR_NONE) {
    for (countdown = 0; countdown < 10; countdown++) {
      ui_error_t rc_set;
      g_malloc_fail_countdown = countdown;
      rc_set = ui_page_control_base_set_number_of_pages(control, 10);
      if (rc_set != UI_ERROR_NONE && rc_set != UI_ERROR_OUT_OF_MEMORY) {
        exit(1);
      }
      g_malloc_fail_countdown = -1;
    }
    for (countdown = 0; countdown < 10; countdown++) {
      ui_error_t rc_set;
      g_malloc_fail_countdown = countdown;
      rc_set = ui_page_control_base_set_current_page(control, 5);
      if (rc_set != UI_ERROR_NONE && rc_set != UI_ERROR_OUT_OF_MEMORY) {
        exit(1);
      }
      g_malloc_fail_countdown = -1;
    }
    {
      ui_error_t rc_cleanup =
          ui_component_destroy((struct ui_component *)control);
      if (rc_cleanup != UI_ERROR_NONE) {
        exit(1);
      }
    }
  }
}

int main(void) {
  test_page_control_base();
  test_page_control_base_errors();
  test_page_control_base_oom();
  printf("test_ui_page_control_base passed\n");
  return 0;
}
