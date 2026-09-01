/* clang-format off */
#include "../include/ui_scroll_spy.h"
#include "../include/ui_error.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_signal.h"
#include "../include/ui_effect.h"
#include "../include/ui_intersection_observer.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t failing_effect_fn(void *user_data) {
  ui_signal_t *sig = (ui_signal_t *)user_data;
  union ui_signal_payload val;
  {
    ui_error_t _ign = ui_signal_get(sig, &val);
    (void)_ign;
  }

  printf("failing_effect_fn called with val %d\n", val.int_val);
  if (val.int_val != -1) {
    return UI_ERROR_UNKNOWN;
  }
  return UI_ERROR_NONE;
}

static int test_scroll_spy_lifecycle(void) {
  struct ui_scroll_spy *spy = NULL;
  ui_error_t rc;

  /* Test NULL out_spy */
  rc = ui_scroll_spy_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_scroll_spy_create(&spy);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  rc = ui_scroll_spy_create(&spy);
  if (rc != UI_ERROR_NONE || spy == NULL)
    return 1;

  /* Destroy with NULL */
  rc = ui_scroll_spy_destroy(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    {
      ui_error_t rc_cleanup = ui_scroll_spy_destroy(spy);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    return 1;
  }

  rc = ui_scroll_spy_destroy(spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_scroll_spy_targets(void) {
  struct ui_scroll_spy *spy = NULL;
  struct ui_dom_node *target1 = (struct ui_dom_node *)0x10;
  struct ui_dom_node *target2 = (struct ui_dom_node *)0x20;
  struct ui_dom_node *target3 = (struct ui_dom_node *)0x30;
  ui_error_t rc;
  int i;

  rc = ui_scroll_spy_create(&spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test evaluate before observer is set */
  rc = ui_scroll_spy_evaluate(spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test set_root with NULL spy */
  rc = ui_scroll_spy_set_root(NULL, NULL, 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_scroll_spy_set_root(spy, NULL, -10);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test add_target invalid args */
  rc = ui_scroll_spy_add_target(NULL, target1, 1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_scroll_spy_add_target(spy, NULL, 1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_scroll_spy_add_target(spy, target1, 1);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_scroll_spy_add_target(spy, target2, 2);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test OOM in add_target */
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    {
      ui_error_t _ign = ui_scroll_spy_add_target(spy, target3, 3);
      (void)_ign;
    }
  }
  g_malloc_fail_countdown = -1;

  rc = ui_scroll_spy_evaluate(spy);
  if (rc != UI_ERROR_NONE) {
    printf("Failed at A, rc=%d\n", rc);
    return 1;
  }

  /* Call set_root again to test existing observer destruction */
  rc = ui_scroll_spy_set_root(spy, NULL, -20);
  if (rc != UI_ERROR_NONE) {
    printf("Failed at B, rc=%d\n", rc);
    return 1;
  }

  /* Fill up targets */
  for (i = 2; i < 64; i++) {
    rc = ui_scroll_spy_add_target(spy,
                                  (struct ui_dom_node *)(size_t)(0x100 + i), i);
    if (rc != UI_ERROR_NONE) {
      if (rc == UI_ERROR_OUT_OF_BOUNDS) {
        break; /* Expected when full */
      }
      printf("Failed at C, i=%d, rc=%d\n", i, rc);
      return 1;
    }
  }
  /* Next one should fail out of bounds */
  rc = ui_scroll_spy_add_target(spy, (struct ui_dom_node *)0x999, 64);
  if (rc != UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  /* Test OOM in set_root */
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    {
      ui_error_t _ign = ui_scroll_spy_set_root(spy, NULL, -10);
      (void)_ign;
    }
  }
  g_malloc_fail_countdown = -1;

  /* Force spy->observer = NULL by failing set_root explicitly */
  g_malloc_fail_countdown = 0;
  {
    ui_error_t rc_cleanup = ui_scroll_spy_set_root(spy, NULL, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_malloc_fail_countdown = -1;

  struct ui_scroll_spy_internal {
    void *obs;
  };
  printf("OBS SERVER: %p\n", ((struct ui_scroll_spy_internal *)spy)->obs);

  /* Now add and remove with observer == NULL to hit missing branches */
  {
    ui_error_t rc_cleanup = ui_scroll_spy_add_target(spy, target3, 3);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_scroll_spy_remove_target(spy, target3);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Restore observer so rest of test doesn't crash if it needs it */
  {
    ui_error_t rc_cleanup = ui_scroll_spy_set_root(spy, NULL, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Test remove_target */
  rc = ui_scroll_spy_remove_target(NULL, target1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_scroll_spy_remove_target(spy, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_scroll_spy_remove_target(spy, target1);
  if (rc != UI_ERROR_NONE)
    return 1;
  /* Remove again, should fail */
  rc = ui_scroll_spy_remove_target(spy, target1);
  if (rc != UI_ERROR_NOT_FOUND)
    return 1;

  {
    ui_error_t rc_cleanup = ui_scroll_spy_destroy(spy);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_scroll_spy_signal(void) {
  struct ui_scroll_spy *spy = NULL;
  ui_signal_t *sig = NULL;
  union ui_signal_payload init_val;
  struct ui_dom_node *target1 = (struct ui_dom_node *)0x10;
  ui_error_t rc;

  rc = ui_scroll_spy_create(&spy);
  if (rc != UI_ERROR_NONE)
    return 1;

  init_val.int_val = -1;
  rc = ui_signal_create(NULL, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_scroll_spy_bind_active_section(NULL, sig);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_scroll_spy_bind_active_section(spy, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_scroll_spy_bind_active_section(spy, sig);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_effect_t *eff = NULL;
  rc = ui_effect_create(NULL, failing_effect_fn, sig, NULL, &eff);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Add root, target, and evaluate to trigger on_intersection and hit
   * ui_signal_set */
  rc = ui_scroll_spy_set_root(spy, NULL, 0);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Evaluate with no targets to hit best_id == -1 branch while signal is bound
   */
  rc = ui_scroll_spy_evaluate(spy);
  if (rc != UI_ERROR_NONE) {
    printf("Expected NONE from empty scroll evaluate, got %d\n", rc);
    return 1;
  }

  /* Force observer to evaluate a target not in spy->targets */
  {
    struct ui_dom_node *ghost = (struct ui_dom_node *)0x111;
    struct ui_scroll_spy_internal {
      void *observer;
      void *active_signal;
    };
    struct ui_scroll_spy_internal *internal =
        (struct ui_scroll_spy_internal *)spy;

    void *saved_obs = internal->observer;
    internal->observer = NULL;
    {
      ui_error_t rc_cleanup = ui_scroll_spy_add_target(spy, ghost, 4);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* This removes ghost from spy->targets but NOT from the observer...
     * actually if we added it when observer was NULL, it was NEVER added to the
     * observer! */
    {
      ui_error_t rc_cleanup = ui_scroll_spy_remove_target(spy, ghost);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Wait! If we want it in the observer but not in targets, we must add it
     * when observer is NOT NULL, then remove it when observer IS NULL! */
    internal->observer = saved_obs;
    {
      ui_error_t rc_cleanup = ui_scroll_spy_add_target(spy, ghost, 4);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    internal->observer = NULL;
    {
      ui_error_t rc_cleanup = ui_scroll_spy_remove_target(spy, ghost);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    internal->observer = saved_obs;
    /* Observer still tracks ghost, but spy doesn't know about it. Evaluates it
     * -> hits loop finish AND best_id == -1 with active_signal */
    {
      ui_error_t rc_cleanup = ui_scroll_spy_evaluate(spy);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    /* Clean up observer so it doesn't crash */
    {
      ui_error_t rc_cleanup = ui_intersection_observer_unobserve(
          (struct ui_intersection_observer *)internal->observer, ghost);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  rc = ui_scroll_spy_add_target(spy, target1, 1);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_scroll_spy_evaluate(spy);
  if (rc != UI_ERROR_NONE) {
    printf("Expected NONE from scroll evaluate, got %d\n", rc);
    return 1;
  }

  /* test evaluate invalid args */
  rc = ui_scroll_spy_evaluate(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    ui_error_t rc_cleanup = ui_effect_destroy(eff);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_signal_destroy(sig);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_scroll_spy_destroy(spy);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

int main(void) {
  int failed = 0;
  int f2;
  int f3;
  printf("Running ui_scroll_spy tests...\n");

  failed |= test_scroll_spy_lifecycle();
  if (failed)
    printf("test_scroll_spy_lifecycle failed\n");
  f2 = test_scroll_spy_targets();
  if (f2)
    printf("test_scroll_spy_targets failed\n");
  failed |= f2;
  f3 = test_scroll_spy_signal();
  if (f3)
    printf("test_scroll_spy_signal failed\n");
  failed |= f3;

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
