/* clang-format off */
#include "ui_label_base.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t test_label_creation(void) {
  struct ui_label_base *lbl = NULL;
  ui_error_t rc;

  rc = ui_label_base_create(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  g_malloc_fail_countdown = 0;
  rc = ui_label_base_create(&lbl);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1; /* ui_component_create allocation */
  rc = ui_label_base_create(&lbl);
  assert(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 2; /* root node creation allocation */
  rc = ui_label_base_create(&lbl);
  assert(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 3; /* tag string alloc */
  rc = ui_label_base_create(&lbl);
  assert(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  rc = ui_label_base_create(&lbl);
  assert(rc == UI_ERROR_NONE);
  assert(lbl != NULL);

  {
    ui_error_t rc_cleanup = ui_label_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_label_base_destroy(lbl);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  printf("test_label_creation passed\n");
  return UI_ERROR_NONE;
}

static ui_error_t test_label_set_for(void) {
  struct ui_label_base *lbl = NULL;
  ui_error_t rc;

  rc = ui_label_base_set_for(NULL, "target-id");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_label_base_create(&lbl);
  assert(rc == UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  rc = ui_label_base_set_for(lbl, "target-id");
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  rc = ui_label_base_set_for(lbl, "target-id");
  assert(rc == UI_ERROR_NONE);

  /* Overwrite existing target */
  rc = ui_label_base_set_for(lbl, "target-id-2");
  assert(rc == UI_ERROR_NONE);

  /* Destroy label while it has a target_id */
  {
    ui_error_t rc_cleanup = ui_label_base_destroy(lbl);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  rc = ui_label_base_create(&lbl);
  assert(rc == UI_ERROR_NONE);

  /* Clear target */
  rc = ui_label_base_set_for(lbl, NULL);
  assert(rc == UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_label_base_destroy(lbl);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Test with broken shadow_root to hit paths */
  rc = ui_label_base_create(&lbl);
  if (rc == UI_ERROR_NONE) {
    struct ui_component *comp;
    ui_label_base_get_component(lbl, &comp);
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(comp->shadow_root);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    comp->shadow_root = NULL;
    ui_label_base_set_for(lbl, "fail-target");
    ui_label_base_set_for(lbl, NULL);
    {
      ui_error_t rc_cleanup = ui_label_base_destroy(lbl);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  extern ui_error_t ui_test_label_base_set_for_no_component(void);
  ui_test_label_base_set_for_no_component();

  printf("test_label_set_for passed\n");
  return UI_ERROR_NONE;
}

static ui_error_t test_label_misc(void) {
  struct ui_label_base *lbl = NULL;
  struct ui_component *comp = NULL;
  struct ui_dom_node *node = (struct ui_dom_node *)1;
  struct ui_signal *sig = (struct ui_signal *)1;
  struct ui_event ev;
  ui_error_t rc;

  rc = ui_label_base_create(&lbl);
  assert(rc == UI_ERROR_NONE);

  rc = ui_label_base_get_component(NULL, &comp);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_label_base_get_component(lbl, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_label_base_get_component(lbl, &comp);
  assert(rc == UI_ERROR_NONE && comp != NULL);

  rc = ui_label_base_set_target_node(NULL, node);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_label_base_set_target_node(lbl, node);
  assert(rc == UI_ERROR_NONE);

  rc = ui_label_base_bind_text(NULL, sig);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_label_base_bind_text(lbl, sig);
  assert(rc == UI_ERROR_NONE);

  rc = ui_label_base_process_event(NULL, &ev, 0.0);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_label_base_process_event(lbl, NULL, 0.0);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_label_base_process_event(lbl, &ev, 0.0);
  assert(rc == UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_label_base_destroy(lbl);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  printf("test_label_misc passed\n");
  return UI_ERROR_NONE;
}

int main(void) {
  test_label_creation();
  test_label_set_for();
  test_label_misc();
  return 0;
}
