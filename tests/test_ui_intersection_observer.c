/* clang-format off */
#include "ui_intersection_observer.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_cb_called = 0;

static enum ui_error
intersection_cb(struct ui_intersection_observer *observer,
                const struct ui_intersection_observer_entry *entries,
                int entry_count, void *user_data) {
  (void)observer;
  (void)entries;
  (void)user_data;
  g_cb_called += entry_count;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_intersection_observer *obs = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *target = NULL;
  float thresholds[] = {0.0f, 0.5f, 1.0f};
  enum ui_error rc;

  printf("Testing invalid arguments...\n");
  if (ui_intersection_observer_create(NULL, 0, NULL, 0, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_intersection_observer_destroy(NULL);
  if (ui_intersection_observer_observe(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_intersection_observer_unobserve(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_intersection_observer_disconnect(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_intersection_observer_subscribe(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_intersection_observer_evaluate(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_intersection_observer_create(root, 10, thresholds, 3, &obs);
  if (rc != UI_ERROR_NONE || obs == NULL)
    return 1;

  rc = ui_intersection_observer_subscribe(obs, intersection_cb, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_intersection_observer_observe(obs, target);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Double observe should be fine */
  rc = ui_intersection_observer_observe(obs, target);
  if (rc != UI_ERROR_NONE)
    return 1;

  g_cb_called = 0;
  rc = ui_intersection_observer_evaluate(obs);
  if (rc != UI_ERROR_NONE)
    return 1;
  if (g_cb_called == 0)
    return 1; /* Mock evaluates to 1.0, crosses 0.0, 0.5, 1.0 */

  rc = ui_intersection_observer_unobserve(obs, target);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_intersection_observer_unobserve(obs, target);
  if (rc != UI_ERROR_NOT_FOUND)
    return 1;

  rc = ui_intersection_observer_observe(obs, target);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_intersection_observer_disconnect(obs);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_intersection_observer_destroy(obs);
  ui_dom_node_destroy(target);
  ui_dom_node_destroy(root);
  return 0;
}

static int test_capacity_and_unobserve(void) {
  struct ui_intersection_observer *obs = NULL;
  struct ui_dom_node *nodes[8];
  int i;
  enum ui_error rc;

  printf("Testing capacity expansion...\n");
  rc = ui_intersection_observer_create(NULL, 0, NULL, 0, &obs);
  if (rc != UI_ERROR_NONE)
    return 1;

  for (i = 0; i < 8; i++) {
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &nodes[i]);
  }

  /* Initial capacity is 4. Fill to 4 */
  for (i = 0; i < 4; i++) {
    ui_intersection_observer_observe(obs, nodes[i]);
  }

  /* OOM during reallocation */
  g_malloc_fail_countdown = 0;
  rc = ui_intersection_observer_observe(obs, nodes[4]);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  /* Success reallocation */
  for (i = 4; i < 8; i++) {
    ui_intersection_observer_observe(obs, nodes[i]);
  }

  /* Unobserve middle element to hit shifting branch */
  ui_intersection_observer_unobserve(obs, nodes[2]);

  /* Evaluate with enough targets to trigger entries allocation */
  ui_intersection_observer_subscribe(obs, intersection_cb, NULL);

  /* OOM during entries allocation */
  g_malloc_fail_countdown = 0;
  rc = ui_intersection_observer_evaluate(obs);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_intersection_observer_evaluate(obs);

  /* Empty targets early out */
  for (i = 0; i < 8; i++) {
    ui_intersection_observer_unobserve(obs, nodes[i]);
  }

  /* Force evaluate with callback but no targets */
  ui_intersection_observer_evaluate(obs);

  /* Force evaluate with targets but no callback */
  ui_intersection_observer_observe(obs, nodes[0]);
  ui_intersection_observer_subscribe(obs, NULL, NULL);
  ui_intersection_observer_evaluate(obs);

  for (i = 0; i < 8; i++) {
    ui_dom_node_destroy(nodes[i]);
  }
  ui_intersection_observer_destroy(obs);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_intersection_observer *obs = NULL;
  enum ui_error rc;

  printf("Testing OOM...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_intersection_observer_create(NULL, 0, NULL, 0, &obs);
  if (rc != UI_ERROR_OUT_OF_MEMORY || obs != NULL) {
    printf("Failed to catch OOM in create (base).\n");
    return 1;
  }

  g_malloc_fail_countdown = 1;
  rc = ui_intersection_observer_create(NULL, 0, NULL, 0, &obs);
  if (rc != UI_ERROR_OUT_OF_MEMORY || obs != NULL) {
    printf("Failed to catch OOM in create (targets).\n");
    return 1;
  }

  g_malloc_fail_countdown = 0;
  {
    float thresholds[] = {1.0f};

    g_malloc_fail_countdown = 1;
    rc = ui_intersection_observer_create(NULL, 0, thresholds, 1, &obs);
    if (rc != UI_ERROR_OUT_OF_MEMORY || obs != NULL) {
      printf("Failed to catch OOM in create (thresholds).\n");
      return 1;
    }

    /* Allocation order: 1 for obs, 2 for thresholds, 3 for targets */
    g_malloc_fail_countdown = 2;
    rc = ui_intersection_observer_create(NULL, 0, thresholds, 1, &obs);
    if (rc != UI_ERROR_OUT_OF_MEMORY || obs != NULL) {
      printf("Failed to catch OOM in create (targets with thresholds).\n");
      return 1;
    }
  }

  rc = ui_intersection_observer_create(NULL, 0, NULL, 0, &obs);
  if (rc == UI_ERROR_NONE && obs != NULL) {
    struct ui_dom_node *target1 = NULL;
    struct ui_dom_node *target2 = NULL;
    struct ui_dom_node *target3 = NULL;
    struct ui_dom_node *target4 = NULL;
    struct ui_dom_node *target5 = NULL;

    struct ui_dom_node *target6 = NULL;

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target1);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target2);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target3);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target4);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target5);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target6);

    ui_intersection_observer_observe(obs, target1);
    ui_intersection_observer_observe(obs, target2);
    ui_intersection_observer_observe(obs, target3);
    ui_intersection_observer_observe(obs, target4);

    g_malloc_fail_countdown = 0;
    rc = ui_intersection_observer_observe(obs, target5);
    if (rc != UI_ERROR_OUT_OF_MEMORY) {
      printf("Failed to catch OOM in observe.\n");
      return 1;
    }
    g_malloc_fail_countdown = -1;
    ui_intersection_observer_observe(obs, target5);
    ui_intersection_observer_observe(obs, target6);

    /* force evaluate on empty target count (remove callback check bypass) */
    ui_intersection_observer_subscribe(obs, intersection_cb, NULL);
    ui_intersection_observer_unobserve(obs, target1);
    ui_intersection_observer_unobserve(obs, target2);
    ui_intersection_observer_unobserve(obs, target3);
    ui_intersection_observer_unobserve(obs, target4);
    ui_intersection_observer_unobserve(obs, target5);
    ui_intersection_observer_unobserve(obs, target6);
    ui_intersection_observer_evaluate(obs);

    /* force evaluate on un-subscribed */
    ui_intersection_observer_subscribe(obs, NULL, NULL);
    ui_intersection_observer_evaluate(obs);

    /* restore state */
    ui_intersection_observer_observe(obs, target1);
    ui_intersection_observer_observe(obs, target2);
    ui_intersection_observer_observe(obs, target4);
    ui_intersection_observer_observe(obs, target5);
    ui_intersection_observer_observe(obs, target6);

    ui_intersection_observer_subscribe(obs, intersection_cb, NULL);

    ui_intersection_observer_unobserve(obs, target1);
    ui_intersection_observer_unobserve(obs, target2);
    ui_intersection_observer_unobserve(obs, target4);
    ui_intersection_observer_unobserve(obs, target5);
    ui_intersection_observer_unobserve(obs, target6);

    /* evaluate with no targets */
    ui_intersection_observer_evaluate(obs);

    ui_dom_node_destroy(target1);
    ui_dom_node_destroy(target2);
    ui_dom_node_destroy(target3);
    ui_dom_node_destroy(target4);
    ui_dom_node_destroy(target5);
    ui_dom_node_destroy(target6);
    ui_intersection_observer_destroy(obs);
  }

  /* Test threshold hit backwards */
  g_malloc_fail_countdown = -1;
  {
    struct ui_dom_node *root = NULL;
    struct ui_dom_node *target = NULL;
    float thresholds[] = {1.5f}; /* mock hardcodes current_ratio = 1.0f */

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target);

    ui_intersection_observer_create(root, 0, thresholds, 1, &obs);
    ui_intersection_observer_subscribe(obs, intersection_cb, NULL);
    ui_intersection_observer_observe(obs, target);

    /* force evaluate where ratio drops below threshold */
    ui_intersection_observer_evaluate(
        obs); /* Initial eval crosses 1.5 because mock uses 1.0 vs 0.0 */

    ui_intersection_observer_destroy(obs);
    ui_intersection_observer_create(root, 0, NULL, 0, &obs);
    ui_intersection_observer_subscribe(obs, intersection_cb, NULL);
    ui_intersection_observer_observe(obs, target);

    ui_intersection_observer_evaluate(obs); /* Intersecting 0 -> 1 */
    ui_intersection_observer_evaluate(
        obs); /* Intersecting 1 -> 1 (no crossing) */

    ui_intersection_observer_destroy(obs);
    ui_dom_node_destroy(target);
    ui_dom_node_destroy(root);
  }

  return 0;
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  if (test_capacity_and_unobserve() != 0) {
    printf("Capacity tests failed.\n");
    return 1;
  }

  printf("All ui_intersection_observer tests passed.\n");
  return 0;
}
