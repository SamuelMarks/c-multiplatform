/* clang-format off */
#include "ui_scaffold_base.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t test_scaffold_base(void) {
  struct ui_scaffold_base *scaffold = NULL;
  struct ui_component *top_bar = NULL;
  struct ui_component *main_content = NULL;
  ui_error_t err;

  err = ui_scaffold_base_create(&scaffold);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_component_create(&top_bar);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &top_bar->shadow_root);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_tag_name(top_bar->shadow_root, "nav");
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_component_create(&main_content);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &main_content->shadow_root);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_tag_name(main_content->shadow_root, "main");
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_scaffold_base_set_top_bar(scaffold, top_bar);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_scaffold_base_set_main_content(scaffold, main_content);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (scaffold->slot_top_bar->first_child != top_bar->shadow_root) {
    return UI_ERROR_UNKNOWN;
  }

  if (scaffold->slot_main_content->first_child != main_content->shadow_root) {
    return UI_ERROR_UNKNOWN;
  }

  err = ui_component_destroy((struct ui_component *)scaffold);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  top_bar->shadow_root = NULL;
  err = ui_component_destroy(top_bar);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  main_content->shadow_root = NULL;
  err = ui_component_destroy(main_content);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

static ui_error_t test_scaffold_base_extra(void) {
  struct ui_scaffold_base *scaffold = NULL;
  ui_error_t err;

  err = ui_scaffold_base_create(NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    return UI_ERROR_UNKNOWN;
  }

  err = ui_scaffold_base_create(&scaffold);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_scaffold_base_set_top_bar(NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    return UI_ERROR_UNKNOWN;
  }

  err = ui_scaffold_base_set_top_bar(scaffold, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    return UI_ERROR_UNKNOWN;
  }

  err = ui_scaffold_base_set_main_content(NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    return UI_ERROR_UNKNOWN;
  }

  err = ui_scaffold_base_set_main_content(scaffold, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    return UI_ERROR_UNKNOWN;
  }

  err = ui_scaffold_base_bind_data(NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    return UI_ERROR_UNKNOWN;
  }

  err = ui_scaffold_base_bind_data(scaffold, NULL);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_component_destroy((struct ui_component *)scaffold);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
static ui_error_t test_scaffold_mocks(void) {
  struct ui_scaffold_base *scaffold = NULL;
  ui_error_t err;
  int idx;
  extern int g_scaffold_mock_fail;
  extern int g_scaffold_slot_fail_idx;

  /* 1: set_tag_name on ui-scaffold fails */
  g_scaffold_mock_fail = 1;
  err = ui_scaffold_base_create(&scaffold);
  if (err != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_scaffold_mock_fail = 0;

  /* 2: set_tag_name on div fails */
  g_scaffold_mock_fail = 2;
  err = ui_scaffold_base_create(&scaffold);
  if (err != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_scaffold_mock_fail = 0;

  /* 3: set_attribute fails */
  g_scaffold_mock_fail = 3;
  err = ui_scaffold_base_create(&scaffold);
  if (err != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_scaffold_mock_fail = 0;

  /* 4: append_child fails */
  g_scaffold_mock_fail = 4;
  err = ui_scaffold_base_create(&scaffold);
  if (err != UI_ERROR_UNKNOWN) {
    return UI_ERROR_UNKNOWN;
  }
  g_scaffold_mock_fail = 0;

  /* Slot failures 1, 2, 3, 4 */
  for (idx = 1; idx <= 4; idx++) {
    g_scaffold_slot_fail_idx = idx;
    err = ui_scaffold_base_create(&scaffold);
    if (err != UI_ERROR_UNKNOWN) {
      return UI_ERROR_UNKNOWN;
    }
    g_scaffold_slot_fail_idx = 0;
  }

  return UI_ERROR_NONE;
}
#endif

static ui_error_t test_scaffold_base_oom(void) {
  struct ui_scaffold_base *scaffold = NULL;
  ui_error_t err;
  int i;

  for (i = 0; i < 20; ++i) {
    g_malloc_fail_countdown = i;
    err = ui_scaffold_base_create(&scaffold);
    if (err == UI_ERROR_NONE) {
      err = ui_component_destroy((struct ui_component *)scaffold);
      if (err != UI_ERROR_NONE) {
        return err;
      }
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc;

  rc = test_scaffold_base();
  if (rc != UI_ERROR_NONE) {
    printf("test_scaffold_base failed\n");
    return 1;
  }

  rc = test_scaffold_base_extra();
  if (rc != UI_ERROR_NONE) {
    printf("test_scaffold_base_extra failed\n");
    return 1;
  }

#ifdef UI_TEST_MOCK_ALLOC
  rc = test_scaffold_mocks();
  if (rc != UI_ERROR_NONE) {
    printf("test_scaffold_mocks failed\n");
    return 1;
  }
#endif

  rc = test_scaffold_base_oom();
  if (rc != UI_ERROR_NONE) {
    printf("test_scaffold_base_oom failed\n");
    return 1;
  }

  printf("test_ui_scaffold_base passed\n");
  return 0;
}
