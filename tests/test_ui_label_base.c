/* clang-format off */
#include "ui_label_base.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern ui_error_t ui_test_label_base_set_for_no_component(void);

struct ui_label_base {
  struct ui_component *component;
  char *target_id;
  struct ui_dom_node *target_node;
  struct ui_signal *text_signal;
};

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

  memset(&ev, 0, sizeof(ev));

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
    (void)rc_cleanup;
  }
  printf("test_label_misc passed\n");
  return UI_ERROR_NONE;
}

static ui_error_t test_label_set_text(void) {
  struct ui_label_base *lbl = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_label_base_create(&lbl);
  assert(rc == UI_ERROR_NONE);

  /* NULL checks */
  rc = ui_label_base_set_text(NULL, "hello");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_label_base_set_text(lbl, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* First time: creates text node child */
  rc = ui_label_base_set_text(lbl, "Initial text");
  assert(rc == UI_ERROR_NONE);

  /* Second time: updates existing text node child */
  rc = ui_label_base_set_text(lbl, "Updated text");
  assert(rc == UI_ERROR_NONE);

  /* Test with component shadow_root == NULL */
  comp = lbl->component;
  comp->shadow_root = NULL;
  rc = ui_label_base_set_text(lbl, "fail");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Test with lbl->component == NULL */
  lbl->component = NULL;
  rc = ui_label_base_set_text(lbl, "fail");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  lbl->component = comp;
  ui_label_base_destroy(lbl);

  /* OOM test for text node creation */
  rc = ui_label_base_create(&lbl);
  assert(rc == UI_ERROR_NONE);
  g_malloc_fail_countdown = 0;
  rc = ui_label_base_set_text(lbl, "fail_oom");
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
  ui_label_base_destroy(lbl);

  /* Test where first_child is an element node, not a text node */
  {
    struct ui_dom_node *elem_child = NULL;
    rc = ui_label_base_create(&lbl);
    assert(rc == UI_ERROR_NONE);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &elem_child);
    assert(rc == UI_ERROR_NONE);
    rc = ui_dom_node_append_child(lbl->component->shadow_root, elem_child);
    assert(rc == UI_ERROR_NONE);
    rc = ui_label_base_set_text(lbl, "Text after element");
    assert(rc == UI_ERROR_NONE);
    ui_label_base_destroy(lbl);
  }

  printf("test_label_set_text passed\n");
  return UI_ERROR_NONE;
}

int main(void) {
  test_label_creation();
  test_label_set_for();
  test_label_misc();
  test_label_set_text();
  return 0;
}
