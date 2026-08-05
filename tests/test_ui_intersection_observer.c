#include "ui_intersection_observer.h"
#include <stdio.h>
#include <stdlib.h>

extern int g_malloc_fail_countdown;
static int g_cb_called = 0;

static ui_error_t
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
  ui_error_t rc;

  printf("Testing invalid arguments...\n");
  if (ui_intersection_observer_create(NULL, 0, NULL, 0, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_intersection_observer_destroy(NULL);

  {
    struct ui_intersection_observer *empty_obs = NULL;
    ui_intersection_observer_create(NULL, 0, NULL, 0, &empty_obs);
    ui_intersection_observer_destroy(empty_obs);
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target);

  rc = ui_intersection_observer_create(root, 10, thresholds, 3, &obs);

  if (ui_intersection_observer_observe(NULL, target) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_intersection_observer_observe(obs, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_intersection_observer_observe(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_intersection_observer_unobserve(NULL, target) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_intersection_observer_unobserve(obs, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
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

  ui_intersection_observer_subscribe(obs, intersection_cb, NULL);
  ui_intersection_observer_observe(obs, target);
  ui_intersection_observer_observe(obs, target); /* double */

  g_cb_called = 0;
  ui_intersection_observer_evaluate(obs);
  if (g_cb_called == 0)
    return 1;

  ui_intersection_observer_unobserve(obs, target);
  if (ui_intersection_observer_unobserve(obs, target) != UI_ERROR_NOT_FOUND)
    return 1;

  ui_intersection_observer_observe(obs, target);
  ui_intersection_observer_disconnect(obs);

  ui_intersection_observer_destroy(obs);
  (void)ui_dom_node_destroy(target);
  (void)ui_dom_node_destroy(root);
  return 0;
}

static int test_capacity_and_unobserve(void) {
  struct ui_intersection_observer *obs = NULL;
  struct ui_dom_node *nodes[8];
  int i;
  ui_error_t rc;

  ui_intersection_observer_create(NULL, 0, NULL, 0, &obs);
  for (i = 0; i < 8; i++)
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &nodes[i]);

  for (i = 0; i < 4; i++)
    ui_intersection_observer_observe(obs, nodes[i]);

  g_malloc_fail_countdown = 0;
  rc = ui_intersection_observer_observe(obs, nodes[4]);
  g_malloc_fail_countdown = -1;

  for (i = 4; i < 8; i++)
    ui_intersection_observer_observe(obs, nodes[i]);

  ui_intersection_observer_unobserve(obs, nodes[2]);
  ui_intersection_observer_subscribe(obs, intersection_cb, NULL);

  g_malloc_fail_countdown = 0;
  rc = ui_intersection_observer_evaluate(obs);
  g_malloc_fail_countdown = -1;

  ui_intersection_observer_evaluate(obs);

  for (i = 0; i < 8; i++)
    ui_intersection_observer_unobserve(obs, nodes[i]);

  ui_intersection_observer_evaluate(obs);
  ui_intersection_observer_observe(obs, nodes[0]);
  ui_intersection_observer_subscribe(obs, NULL, NULL);
  ui_intersection_observer_evaluate(obs);

  for (i = 0; i < 8; i++)
    (void)ui_dom_node_destroy(nodes[i]);
  ui_intersection_observer_destroy(obs);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_intersection_observer *obs = NULL;
  ui_error_t rc;

  g_malloc_fail_countdown = 0;
  rc = ui_intersection_observer_create(NULL, 0, NULL, 0, &obs);
  g_malloc_fail_countdown = 1;
  rc = ui_intersection_observer_create(NULL, 0, NULL, 0, &obs);

  {
    float thresholds[] = {1.0f};
    g_malloc_fail_countdown = 1;
    rc = ui_intersection_observer_create(NULL, 0, thresholds, 1, &obs);
    g_malloc_fail_countdown = 2;
    rc = ui_intersection_observer_create(NULL, 0, thresholds, 1, &obs);
  }

  g_malloc_fail_countdown = -1;
  rc = ui_intersection_observer_create(NULL, 0, NULL, 0, &obs);
  {
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
    ui_intersection_observer_observe(obs, target5);
    g_malloc_fail_countdown = -1;
    ui_intersection_observer_observe(obs, target5);
    ui_intersection_observer_observe(obs, target6);

    ui_intersection_observer_subscribe(obs, intersection_cb, NULL);
    ui_intersection_observer_unobserve(obs, target1);
    ui_intersection_observer_unobserve(obs, target2);
    ui_intersection_observer_unobserve(obs, target3);
    ui_intersection_observer_unobserve(obs, target4);
    ui_intersection_observer_unobserve(obs, target5);
    ui_intersection_observer_unobserve(obs, target6);
    ui_intersection_observer_evaluate(obs);

    ui_intersection_observer_subscribe(obs, NULL, NULL);
    ui_intersection_observer_evaluate(obs);

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

    ui_intersection_observer_evaluate(obs);

    (void)ui_dom_node_destroy(target1);
    (void)ui_dom_node_destroy(target2);
    (void)ui_dom_node_destroy(target3);
    (void)ui_dom_node_destroy(target4);
    (void)ui_dom_node_destroy(target5);
    (void)ui_dom_node_destroy(target6);
    ui_intersection_observer_destroy(obs);
  }

  {
    struct ui_dom_node *root = NULL;
    struct ui_dom_node *target = NULL;
    float thresholds[] = {1.5f};

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target);

    ui_intersection_observer_create(root, 0, thresholds, 1, &obs);
    ui_intersection_observer_subscribe(obs, intersection_cb, NULL);
    ui_intersection_observer_observe(obs, target);
    ui_intersection_observer_evaluate(obs);

    ui_intersection_observer_destroy(obs);
    ui_intersection_observer_create(root, 0, NULL, 0, &obs);
    ui_intersection_observer_subscribe(obs, intersection_cb, NULL);
    ui_intersection_observer_observe(obs, target);

    ui_intersection_observer_evaluate(obs);
    ui_intersection_observer_evaluate(obs);

    ui_intersection_observer_destroy(obs);
    (void)ui_dom_node_destroy(target);
    (void)ui_dom_node_destroy(root);
  }

  return 0;
}

int main(void) {
  if (run_normal_tests() != 0)
    return 1;
  if (run_oom_tests() != 0)
    return 1;
  if (test_capacity_and_unobserve() != 0)
    return 1;
  return 0;
}
