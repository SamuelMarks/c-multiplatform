/* clang-format off */
#include "greatest.h"
#include "ui_stepper_base.h"
#include "ui_error.h"
#include "ui_dom_node.h"
#include "ui_component.h"
#include "ui_signal.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_stepper_step_entry {
  char *id;
  struct ui_dom_node *header_node;
  struct ui_dom_node *content_node;
  enum ui_stepper_step_state explicit_state;
};

struct ui_stepper_base {
  struct ui_component *component;
  struct ui_dom_node *header_container_node;
  struct ui_dom_node *content_container_node;
  struct ui_stepper_step_entry *steps;
  int step_count;
  int step_capacity;
  int active_index;
  enum ui_stepper_mode mode;
  ui_stepper_validate_t validate_hook;
  void *user_data;
  struct ui_signal *active_index_signal;
};

extern int g_malloc_fail_countdown;
extern int g_stepper_mock_fail;
extern int g_stepper_mock_append_fail_target;
extern int g_stepper_mock_set_attr_fail_target;
extern int g_stepper_mock_remove_attr_fail_target;

static int mock_validator_allow(struct ui_stepper_base *stepper, int step_index,
                                void *user_data) {
  int unused_idx = step_index;
  void *unused_ud = user_data;
  struct ui_stepper_base *unused_s = stepper;
  stepper = unused_s;
  step_index = unused_idx;
  user_data = unused_ud;
  return 1;
}

static int mock_validator_disallow(struct ui_stepper_base *stepper,
                                   int step_index, void *user_data) {
  int unused_idx = step_index;
  void *unused_ud = user_data;
  struct ui_stepper_base *unused_s = stepper;
  stepper = unused_s;
  step_index = unused_idx;
  user_data = unused_ud;
  return 0;
}

TEST test_stepper_invalid_args(void) {
  struct ui_stepper_base *stepper = NULL;
  struct ui_component *comp = NULL;
  struct ui_dom_node *h = NULL;
  struct ui_dom_node *c = NULL;
  int idx = 0;
  enum ui_stepper_step_state state;
  ui_error_t rc;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_stepper_base_create(NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_stepper_base_destroy(NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_get_component(NULL, &comp));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_set_mode(NULL, UI_STEPPER_MODE_LINEAR));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_set_validate_hook(NULL, NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_bind_active_index(NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_add_step(NULL, "s1", NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_set_active_index(NULL, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_get_active_index(NULL, &idx));
  ASSERT_EQ(
      UI_ERROR_INVALID_ARGUMENT,
      ui_stepper_base_set_step_state(NULL, 0, UI_STEPPER_STEP_STATE_DEFAULT));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_get_step_state(NULL, 0, &state));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_stepper_base_next_step(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_stepper_base_prev_step(NULL));

  rc = ui_stepper_base_create(&stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_get_component(stepper, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_get_active_index(stepper, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_get_step_state(stepper, 0, NULL));

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_add_step(stepper, NULL, h, c));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_add_step(stepper, "s1", NULL, c));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_stepper_base_add_step(stepper, "s1", h, NULL));

  rc = ui_dom_node_destroy(h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(c);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_stepper_base_destroy(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_stepper_lifecycle_and_steps(void) {
  struct ui_stepper_base *stepper = NULL;
  struct ui_component *comp = NULL;
  struct ui_dom_node *h[6];
  struct ui_dom_node *c[6];
  enum ui_stepper_step_state state;
  int idx = -1;
  ui_error_t rc;
  int i;

  rc = ui_stepper_base_create(&stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_stepper_base_get_component(stepper, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);

  rc = ui_stepper_base_bind_active_index(stepper, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_stepper_base_set_mode(stepper, UI_STEPPER_MODE_NON_LINEAR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_set_mode(stepper, UI_STEPPER_MODE_LINEAR);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add 5 steps to trigger capacity reallocation (initially cap 4 -> grows to
   * 8) */
  for (i = 0; i < 5; i++) {
    char sid[16];
    sprintf(sid, "step%d", i);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_stepper_base_add_step(stepper, sid, h[i], c[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = ui_stepper_base_get_active_index(stepper, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  /* Active step returns ACTIVE state */
  rc = ui_stepper_base_get_step_state(stepper, 0, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_STEPPER_STEP_STATE_ACTIVE, state);

  /* Other steps return their explicit state (initially DEFAULT) */
  rc = ui_stepper_base_get_step_state(stepper, 1, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_STEPPER_STEP_STATE_DEFAULT, state);

  /* Bounds checks on get_step_state and set_step_state */
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS,
            ui_stepper_base_get_step_state(stepper, -1, &state));
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS,
            ui_stepper_base_get_step_state(stepper, 10, &state));
  ASSERT_EQ(
      UI_ERROR_OUT_OF_BOUNDS,
      ui_stepper_base_set_step_state(stepper, -1, UI_STEPPER_STEP_STATE_ERROR));
  ASSERT_EQ(
      UI_ERROR_OUT_OF_BOUNDS,
      ui_stepper_base_set_step_state(stepper, 10, UI_STEPPER_STEP_STATE_ERROR));

  /* Test setting explicit step states */
  rc = ui_stepper_base_set_step_state(stepper, 2, UI_STEPPER_STEP_STATE_ERROR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_get_step_state(stepper, 2, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_STEPPER_STEP_STATE_ERROR, state);

  rc = ui_stepper_base_set_step_state(stepper, 3,
                                      UI_STEPPER_STEP_STATE_COMPLETED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_get_step_state(stepper, 3, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_STEPPER_STEP_STATE_COMPLETED, state);

  rc =
      ui_stepper_base_set_step_state(stepper, 4, UI_STEPPER_STEP_STATE_DEFAULT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_get_step_state(stepper, 4, &state);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_STEPPER_STEP_STATE_DEFAULT, state);

  /* Test setting unknown step state to hit default branch */
  rc = ui_stepper_base_set_step_state(stepper, 4,
                                      (enum ui_stepper_step_state)999);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test navigation past beginning */
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, ui_stepper_base_prev_step(stepper));

  /* Linear navigation with validator allowing */
  rc = ui_stepper_base_set_validate_hook(stepper, mock_validator_allow, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_next_step(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_get_active_index(stepper, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  /* Linear navigation with validator disallowing */
  rc =
      ui_stepper_base_set_validate_hook(stepper, mock_validator_disallow, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_next_step(stepper);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Remove validator and advance to end */
  rc = ui_stepper_base_set_validate_hook(stepper, NULL, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_set_active_index(stepper, 4);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Non-linear mode navigation (takes false branch of mode == LINEAR) */
  rc = ui_stepper_base_set_mode(stepper, UI_STEPPER_MODE_NON_LINEAR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_set_active_index(stepper, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_set_mode(stepper, UI_STEPPER_MODE_LINEAR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_set_active_index(stepper, 4);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test navigation past end */
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, ui_stepper_base_next_step(stepper));

  /* Prev step */
  rc = ui_stepper_base_prev_step(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_get_active_index(stepper, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, idx);

  /* Set same index is no-op */
  rc = ui_stepper_base_set_active_index(stepper, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set out of bounds index */
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS,
            ui_stepper_base_set_active_index(stepper, -1));
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS,
            ui_stepper_base_set_active_index(stepper, 5));

  rc = ui_stepper_base_destroy(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_stepper_error_branches(void) {
  struct ui_stepper_base *stepper = NULL;
  struct ui_dom_node *h = NULL;
  struct ui_dom_node *c = NULL;
  ui_error_t rc;
  int i;

  /* 1. Append child failures in create */
  for (i = 1; i <= 2; i++) {
    g_stepper_mock_append_fail_target = i;
    stepper = NULL;
    rc = ui_stepper_base_create(&stepper);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    ASSERT(stepper == NULL);
  }
  g_stepper_mock_append_fail_target = 0;

  /* 2. Component set default style failure in create */
  g_stepper_mock_fail = 1;
  stepper = NULL;
  rc = ui_stepper_base_create(&stepper);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_stepper_mock_fail = 0;

  /* 3. Cleanup failures in create */
  for (i = 2; i <= 3; i++) {
    g_stepper_mock_append_fail_target = 1;
    g_stepper_mock_fail = i;
    stepper = NULL;
    rc = ui_stepper_base_create(&stepper);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_stepper_mock_fail = 0;
    g_stepper_mock_append_fail_target = 0;
  }

  /* 4. Component destroy failure in destroy */
  rc = ui_stepper_base_create(&stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_stepper_mock_fail = 3;
  rc = ui_stepper_base_destroy(stepper);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_stepper_mock_fail = 0;

  /* 5. Destroy with stepper->component == NULL */
  rc = ui_stepper_base_create(&stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(stepper->component);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  stepper->component = NULL;
  rc = ui_stepper_base_destroy(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 6. Append failures in add_step */
  for (i = 1; i <= 2; i++) {
    rc = ui_stepper_base_create(&stepper);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_stepper_mock_append_fail_target = i;
    rc = ui_stepper_base_add_step(stepper, "s", h, c);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_stepper_mock_append_fail_target = 0;
    if (i == 1) {
      rc = ui_dom_node_destroy(h);
      ASSERT_EQ(UI_ERROR_NONE, rc);
    }
    rc = ui_dom_node_destroy(c);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_stepper_base_destroy(stepper);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 7. Set attribute mock failures in add_step */
  for (i = 1; i <= 6; i++) {
    rc = ui_stepper_base_create(&stepper);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_stepper_mock_set_attr_fail_target = i;
    rc = ui_stepper_base_add_step(stepper, "s", h, c);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_stepper_mock_set_attr_fail_target = 0;
    rc = ui_dom_node_destroy(h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_destroy(c);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_stepper_base_destroy(stepper);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 8. Remove attribute failure in apply_step_state_attributes */
  rc = ui_stepper_base_create(&stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_stepper_mock_remove_attr_fail_target = 1;
  rc = ui_stepper_base_add_step(stepper, "s", h, c);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_stepper_mock_remove_attr_fail_target = 0;
  rc = ui_stepper_base_destroy(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 9. Attribute failure on ACTIVE step */
  for (i = 7; i <= 9; i++) {
    rc = ui_stepper_base_create(&stepper);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    if (i < 9) {
      g_stepper_mock_set_attr_fail_target = i;
    } else {
      g_stepper_mock_remove_attr_fail_target = 2; /* remove hidden attr */
    }
    rc = ui_stepper_base_add_step(stepper, "s", h, c);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_stepper_mock_set_attr_fail_target = 0;
    g_stepper_mock_remove_attr_fail_target = 0;
    rc = ui_stepper_base_destroy(stepper);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 10. Attribute failures on set_step_state (COMPLETED, ERROR, DEFAULT) */
  rc = ui_stepper_base_create(&stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_stepper_base_add_step(stepper, "s1", h, c);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  {
    struct ui_dom_node *h2 = NULL;
    struct ui_dom_node *c2 = NULL;
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_stepper_base_add_step(stepper, "s2", h2, c2);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  for (i = 1; i <= 3; i++) {
    g_stepper_mock_set_attr_fail_target = i;
    rc = ui_stepper_base_set_step_state(stepper, 1,
                                        UI_STEPPER_STEP_STATE_COMPLETED);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_stepper_mock_set_attr_fail_target = 0;

    g_stepper_mock_set_attr_fail_target = i;
    rc =
        ui_stepper_base_set_step_state(stepper, 1, UI_STEPPER_STEP_STATE_ERROR);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_stepper_mock_set_attr_fail_target = 0;

    g_stepper_mock_set_attr_fail_target = i;
    rc = ui_stepper_base_set_step_state(stepper, 1,
                                        UI_STEPPER_STEP_STATE_DEFAULT);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_stepper_mock_set_attr_fail_target = 0;
  }

  /* 11. Failure during set_active_index apply step loop */
  g_stepper_mock_set_attr_fail_target = 1;
  rc = ui_stepper_base_set_active_index(stepper, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_stepper_mock_set_attr_fail_target = 0;

  /* 12. format_id mock failures */
  for (i = 4; i <= 5; i++) {
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_stepper_mock_fail = i;
    rc = ui_stepper_base_add_step(stepper, "s_fid", h, c);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_stepper_mock_fail = 0;
    rc = ui_dom_node_destroy(h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_destroy(c);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = ui_stepper_base_destroy(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_stepper_oom(void) {
  struct ui_stepper_base *stepper = NULL;
  struct ui_dom_node *h = NULL;
  struct ui_dom_node *c = NULL;
  ui_error_t rc;
  int i;

  /* Creation OOM loop */
  for (i = 0; i < 100; i++) {
    g_malloc_fail_countdown = i;
    stepper = NULL;
    rc = ui_stepper_base_create(&stepper);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      rc = ui_stepper_base_destroy(stepper);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(stepper == NULL);
  }
  g_malloc_fail_countdown = -1;

  /* Add step OOM loop */
  for (i = 0; i < 50; i++) {
    rc = ui_stepper_base_create(&stepper);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    g_malloc_fail_countdown = i;
    rc = ui_stepper_base_add_step(stepper, "s_oom", h, c);
    g_malloc_fail_countdown = -1;

    if (rc == UI_ERROR_NONE) {
      rc = ui_stepper_base_destroy(stepper);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    if (!h->parent) {
      rc = ui_dom_node_destroy(h);
      ASSERT_EQ(UI_ERROR_NONE, rc);
    }
    if (!c->parent) {
      rc = ui_dom_node_destroy(c);
      ASSERT_EQ(UI_ERROR_NONE, rc);
    }
    rc = ui_stepper_base_destroy(stepper);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  g_malloc_fail_countdown = -1;

  PASS();
}

SUITE(ui_stepper_base_suite) {
  RUN_TEST(test_stepper_invalid_args);
  RUN_TEST(test_stepper_lifecycle_and_steps);
  RUN_TEST(test_stepper_error_branches);
  RUN_TEST(test_stepper_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_stepper_base_suite);
  GREATEST_MAIN_END();
}
