/* clang-format off */
#include "ui_stepper_base.h"
#include "ui_error.h"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
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

#define ASSERT_INT_EQ(expr, expected)                                          \
  do {                                                                         \
    int val = (expr);                                                          \
    if (val != (expected)) {                                                   \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             val);                                                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static int test_ui_stepper_base_create_destroy(void) {
  struct ui_stepper_base *stepper = NULL;

  ASSERT_EQ(ui_stepper_base_create(NULL), UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_stepper_base_create(&stepper));
  if (!stepper)
    return 1;

  (void)ui_stepper_base_destroy(stepper);
  (void)ui_stepper_base_destroy(NULL);
  return 0;
}

static int test_ui_stepper_base_get_component(void) {
  struct ui_stepper_base *stepper = NULL;
  struct ui_component *comp = NULL;

  ASSERT_SUCCESS(ui_stepper_base_create(&stepper));
  ASSERT_EQ(ui_stepper_base_get_component(NULL, &comp),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_stepper_base_get_component(stepper, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_stepper_base_get_component(stepper, &comp));

  (void)ui_stepper_base_destroy(stepper);
  return 0;
}

static int test_ui_stepper_base_mode(void) {
  struct ui_stepper_base *stepper = NULL;

  ASSERT_SUCCESS(ui_stepper_base_create(&stepper));
  ASSERT_EQ(ui_stepper_base_set_mode(NULL, UI_STEPPER_MODE_LINEAR),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_stepper_base_set_mode(stepper, UI_STEPPER_MODE_NON_LINEAR));
  ASSERT_SUCCESS(ui_stepper_base_set_mode(stepper, UI_STEPPER_MODE_LINEAR));

  (void)ui_stepper_base_destroy(stepper);
  return 0;
}

static int mock_validator_allow(struct ui_stepper_base *stepper, int step_index,
                                void *user_data) {
  (void)stepper;
  (void)step_index;
  (void)user_data;
  return 1;
}

static int mock_validator_deny(struct ui_stepper_base *stepper, int step_index,
                               void *user_data) {
  (void)stepper;
  (void)step_index;
  (void)user_data;
  return 0;
}

static int test_ui_stepper_base_add_and_navigate(void) {
  struct ui_stepper_base *stepper = NULL;
  struct ui_dom_node *h1 = NULL, *c1 = NULL, *h2 = NULL, *c2 = NULL;
  int index;
  enum ui_stepper_step_state state;

  ASSERT_SUCCESS(ui_stepper_base_create(&stepper));

  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h1));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h2));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c2));

  ASSERT_EQ(ui_stepper_base_add_step(NULL, "s1", h1, c1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_stepper_base_add_step(stepper, NULL, h1, c1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_stepper_base_add_step(stepper, "s1", NULL, c1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_stepper_base_add_step(stepper, "s1", h1, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_stepper_base_add_step(stepper, "s1", h1, c1));
  ASSERT_SUCCESS(ui_stepper_base_add_step(stepper, "s2", h2, c2));

  /* Add 3 more steps to hit capacity expansion branch */
  for (index = 0; index < 3; index++) {
    struct ui_dom_node *hn = NULL, *cn = NULL;
    char buf[16];
    sprintf(buf, "s%d", index + 3);
    ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &hn));
    ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cn));
    ASSERT_SUCCESS(ui_stepper_base_add_step(stepper, buf, hn, cn));
  }

  ASSERT_EQ(ui_stepper_base_get_active_index(NULL, &index),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_stepper_base_get_active_index(stepper, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_stepper_base_get_active_index(stepper, &index));
  ASSERT_INT_EQ(index, 0);

  ASSERT_EQ(ui_stepper_base_next_step(NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_stepper_base_prev_step(NULL), UI_ERROR_INVALID_ARGUMENT);

  /* Linear progression - validator allows */
  ASSERT_SUCCESS(
      ui_stepper_base_set_validate_hook(stepper, mock_validator_allow, NULL));
  ASSERT_EQ(ui_stepper_base_next_step(NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_stepper_base_next_step(stepper));
  ASSERT_SUCCESS(ui_stepper_base_get_active_index(stepper, &index));
  ASSERT_INT_EQ(index, 1);

  /* Prev step */
  ASSERT_EQ(ui_stepper_base_prev_step(NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_stepper_base_prev_step(stepper));
  ASSERT_SUCCESS(ui_stepper_base_get_active_index(stepper, &index));
  ASSERT_INT_EQ(index, 0);

  /* Jump to index */
  ASSERT_EQ(ui_stepper_base_set_active_index(NULL, 1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_stepper_base_set_active_index(stepper, 1));
  ASSERT_SUCCESS(ui_stepper_base_get_active_index(stepper, &index));
  ASSERT_INT_EQ(index, 1);

  /* Deny */
  ASSERT_SUCCESS(
      ui_stepper_base_set_validate_hook(stepper, mock_validator_deny, NULL));
  ASSERT_SUCCESS(ui_stepper_base_prev_step(stepper)); /* back to 0 */
  ASSERT_EQ(ui_stepper_base_next_step(stepper), UI_ERROR_UNKNOWN);
  ASSERT_SUCCESS(ui_stepper_base_get_active_index(stepper, &index));
  ASSERT_INT_EQ(index, 0);

  /* State getting/setting */
  ASSERT_EQ(ui_stepper_base_get_step_state(NULL, 0, &state),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_stepper_base_get_step_state(stepper, 0, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_stepper_base_get_step_state(stepper, -1, &state),
            UI_ERROR_OUT_OF_BOUNDS);
  ASSERT_EQ(ui_stepper_base_get_step_state(stepper, 999, &state),
            UI_ERROR_OUT_OF_BOUNDS);

  /* Test out of bounds set_step_state */
  ASSERT_EQ(
      ui_stepper_base_set_step_state(NULL, 0, UI_STEPPER_STEP_STATE_COMPLETED),
      UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_stepper_base_set_step_state(stepper, -1,
                                           UI_STEPPER_STEP_STATE_COMPLETED),
            UI_ERROR_OUT_OF_BOUNDS);
  ASSERT_EQ(ui_stepper_base_set_step_state(stepper, 999,
                                           UI_STEPPER_STEP_STATE_COMPLETED),
            UI_ERROR_OUT_OF_BOUNDS);

  ASSERT_SUCCESS(ui_stepper_base_set_step_state(
      stepper, 1, UI_STEPPER_STEP_STATE_COMPLETED));
  ASSERT_SUCCESS(ui_stepper_base_get_step_state(stepper, 1, &state));
  ASSERT_INT_EQ(state, UI_STEPPER_STEP_STATE_COMPLETED);

  ASSERT_SUCCESS(ui_stepper_base_get_step_state(stepper, 0, &state));
  ASSERT_INT_EQ(state, UI_STEPPER_STEP_STATE_ACTIVE);

  ASSERT_SUCCESS(
      ui_stepper_base_set_step_state(stepper, 1, UI_STEPPER_STEP_STATE_ERROR));
  ASSERT_SUCCESS(ui_stepper_base_get_step_state(stepper, 1, &state));
  ASSERT_INT_EQ(state, UI_STEPPER_STEP_STATE_ERROR);

  ASSERT_SUCCESS(ui_stepper_base_set_step_state(
      stepper, 1, (enum ui_stepper_step_state)999));
  ASSERT_SUCCESS(ui_stepper_base_get_step_state(stepper, 1, &state));
  ASSERT_INT_EQ(state, (enum ui_stepper_step_state)999);

  ASSERT_SUCCESS(
      ui_stepper_base_set_step_state(stepper, 1, UI_STEPPER_STEP_STATE_ACTIVE));
  ASSERT_SUCCESS(ui_stepper_base_get_step_state(stepper, 1, &state));
  ASSERT_INT_EQ(state, UI_STEPPER_STEP_STATE_ACTIVE);

  ASSERT_SUCCESS(ui_stepper_base_set_step_state(stepper, 1,
                                                UI_STEPPER_STEP_STATE_DEFAULT));
  ASSERT_SUCCESS(ui_stepper_base_get_step_state(stepper, 1, &state));
  ASSERT_INT_EQ(state, UI_STEPPER_STEP_STATE_DEFAULT);

  (void)ui_stepper_base_destroy(stepper);
  return 0;
}

static int test_ui_stepper_base_bindings(void) {
  struct ui_stepper_base *stepper = NULL;

  ASSERT_SUCCESS(ui_stepper_base_create(&stepper));
  ASSERT_EQ(ui_stepper_base_bind_active_index(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_stepper_base_bind_active_index(stepper, NULL));

  ASSERT_EQ(ui_stepper_base_set_validate_hook(NULL, NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  (void)ui_stepper_base_destroy(stepper);
  return 0;
}

static int test_ui_stepper_base_allocation_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_stepper_base *stepper = NULL;
  struct ui_dom_node *h1 = NULL, *c1 = NULL;
  int i;
  ui_error_t err;

  /* Fail split button struct alloc */
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(ui_stepper_base_create(&stepper), UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  for (i = 1; i < 2000; ++i) {
    g_malloc_fail_countdown = i;
    err = ui_stepper_base_create(&stepper);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_NONE) {
      (void)ui_stepper_base_destroy(stepper);
      break;
    }
  }

  ASSERT_SUCCESS(ui_stepper_base_create(&stepper));

  for (i = 0; i < 5; ++i) {
    ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h1));
    ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1));

    g_malloc_fail_countdown = i;
    err = ui_stepper_base_add_step(stepper, "s1", h1, c1);
    g_malloc_fail_countdown = -1;

    if (err == UI_ERROR_NONE) {
      break;
    }

    ASSERT_EQ(err, UI_ERROR_OUT_OF_MEMORY);
    (void)ui_dom_node_destroy(h1);
    (void)ui_dom_node_destroy(c1);
  }

  (void)ui_stepper_base_destroy(stepper);
#endif
  return 0;
}

static int test_ui_stepper_base_edge_cases(void) {
  struct ui_stepper_base *stepper = NULL;
  struct ui_dom_node *h1 = NULL, *c1 = NULL, *h2 = NULL, *c2 = NULL;

  ASSERT_SUCCESS(ui_stepper_base_create(&stepper));

  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h1));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h2));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c2));

  ASSERT_SUCCESS(ui_stepper_base_add_step(stepper, "s1", h1, c1));
  ASSERT_SUCCESS(ui_stepper_base_add_step(stepper, "s2", h2, c2));

  /* Test out of bounds set_active_index */
  ASSERT_EQ(ui_stepper_base_set_active_index(stepper, -1),
            UI_ERROR_OUT_OF_BOUNDS);
  ASSERT_EQ(ui_stepper_base_set_active_index(stepper, 5),
            UI_ERROR_OUT_OF_BOUNDS);

  /* Test prevailing past boundaries */
  ASSERT_SUCCESS(ui_stepper_base_set_active_index(stepper, 0));
  ASSERT_SUCCESS(
      ui_stepper_base_set_active_index(stepper, 0)); /* Should return NONE */
  ASSERT_EQ(ui_stepper_base_prev_step(stepper), UI_ERROR_OUT_OF_BOUNDS);

  ASSERT_SUCCESS(ui_stepper_base_set_active_index(stepper, 1));
  ASSERT_SUCCESS(
      ui_stepper_base_set_active_index(stepper, 0)); /* Test going back */
  ASSERT_SUCCESS(ui_stepper_base_set_active_index(stepper, 1)); /* Reset to 1 */
  ASSERT_EQ(ui_stepper_base_next_step(stepper), UI_ERROR_OUT_OF_BOUNDS);

  /* Test non-linear mode */
  ASSERT_SUCCESS(ui_stepper_base_set_mode(stepper, UI_STEPPER_MODE_NON_LINEAR));
  ASSERT_SUCCESS(ui_stepper_base_set_active_index(stepper, 0));
  ASSERT_SUCCESS(ui_stepper_base_set_active_index(stepper, 1));

  (void)ui_stepper_base_destroy(stepper);
  return 0;
}

int main(void) {
  if (test_ui_stepper_base_create_destroy())
    return 1;
  if (test_ui_stepper_base_get_component())
    return 1;
  if (test_ui_stepper_base_mode())
    return 1;
  if (test_ui_stepper_base_add_and_navigate())
    return 1;
  if (test_ui_stepper_base_bindings())
    return 1;
  if (test_ui_stepper_base_allocation_failures())
    return 1;
  if (test_ui_stepper_base_edge_cases())
    return 1;
  return 0;
}
