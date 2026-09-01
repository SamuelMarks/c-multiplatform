/* clang-format off */
#include "ui_fab_base.h"

#include "../include/ui_ripple_base.h"
extern int g_mock_ripple_fail;
#define ui_ripple_config_init(cfg) (g_mock_ripple_fail ? UI_ERROR_INVALID_ARGUMENT : (ui_ripple_config_init)(cfg))
#include "../src/ui_fab_base.c"
#undef ui_ripple_config_init

#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
int g_mock_ripple_fail = 0;

static int run_normal_tests(void) {
  struct ui_fab_base *fab = NULL;
  struct ui_button_base *main_btn = NULL;
  struct ui_button_base *action_btn = NULL;
  struct ui_button_base *retrieved_btn = NULL;
  struct ui_ripple_state *rstate = NULL;
  enum ui_fab_state state;
  float progress;
  size_t count;
  ui_error_t rc;

  printf("Testing ui_fab_base_create...\n");
  if (ui_fab_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_fab_base_create(&fab);
  if (rc != UI_ERROR_NONE || !fab) {
    printf("Failed to create fab.\n");
    return 1;
  }

  printf("Testing getters and setters...\n");
  if (ui_fab_base_set_main_button(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_set_main_button(fab, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_button_base_create(&main_btn);
  if (ui_fab_base_set_main_button(fab, main_btn) != UI_ERROR_NONE)
    return 1;

  if (ui_fab_base_get_main_button(NULL, &retrieved_btn) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_get_main_button(fab, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_fab_base_get_main_button(fab, &retrieved_btn);
  if (retrieved_btn != main_btn)
    return 1;

  /* Replace main button */
  ui_button_base_create(&main_btn);
  ui_fab_base_set_main_button(
      fab, main_btn); /* old button is destroyed internally */

  printf("Testing actions...\n");
  if (ui_fab_base_add_action(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_add_action(fab, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_button_base_create(&action_btn);
  if (ui_fab_base_add_action(fab, action_btn) != UI_ERROR_NONE)
    return 1;

  if (ui_fab_base_get_action_count(NULL, &count) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_get_action_count(fab, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_fab_base_get_action_count(fab, &count);
  if (count != 1)
    return 1;

  if (ui_fab_base_get_action(NULL, 0, &retrieved_btn) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_get_action(fab, 0, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_get_action(fab, 1, &retrieved_btn) != UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  ui_fab_base_get_action(fab, 0, &retrieved_btn);
  if (retrieved_btn != action_btn)
    return 1;

  /* Add more actions to test capacity growth */
  ui_button_base_create(&action_btn);
  ui_fab_base_add_action(fab, action_btn);
  ui_button_base_create(&action_btn);
  ui_fab_base_add_action(fab, action_btn);

  ui_fab_base_get_action_count(fab, &count);
  if (count != 3)
    return 1;

  printf("Testing toggle and states...\n");
  if (ui_fab_base_toggle(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_get_state(NULL, &state) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_get_state(fab, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_fab_base_get_state(fab, &state);
  if (state != UI_FAB_STATE_COLLAPSED)
    return 1;

  ui_fab_base_toggle(fab);
  ui_fab_base_get_state(fab, &state);
  if (state != UI_FAB_STATE_EXPANDING)
    return 1;

  ui_fab_base_toggle(fab);
  ui_fab_base_get_state(fab, &state);
  if (state != UI_FAB_STATE_COLLAPSING)
    return 1;

  ui_fab_base_toggle(fab);
  ui_fab_base_get_state(fab, &state);
  if (state != UI_FAB_STATE_EXPANDING)
    return 1;

  if (ui_fab_base_tick(NULL, 16.0f) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Tick animation forward */
  ui_fab_base_tick(fab, 100.0f); /* 50% */
  if (ui_fab_base_get_expansion_progress(NULL, &progress) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_get_expansion_progress(fab, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_fab_base_get_expansion_progress(fab, &progress);
  if (progress < 0.49f || progress > 0.51f)
    return 1;

  ui_fab_base_tick(fab, 100.0f); /* 100% */
  ui_fab_base_get_expansion_progress(fab, &progress);
  if (progress != 1.0f)
    return 1;

  ui_fab_base_get_state(fab, &state);
  if (state != UI_FAB_STATE_EXPANDED)
    return 1;

  /* Toggle again to collapse */
  ui_fab_base_toggle(fab);
  ui_fab_base_get_state(fab, &state);
  if (state != UI_FAB_STATE_COLLAPSING)
    return 1;

  ui_fab_base_tick(fab, 50.0f); /* partially collapse */
  ui_fab_base_get_expansion_progress(fab, &progress);
  if (progress <= 0.0f || progress >= 1.0f)
    return 1;

  ui_fab_base_tick(fab, 250.0f); /* Fully collapse */
  ui_fab_base_get_state(fab, &state);
  if (state != UI_FAB_STATE_COLLAPSED)
    return 1;
  ui_fab_base_get_expansion_progress(fab, &progress);
  if (progress != 0.0f)
    return 1;

  printf("Testing ripple state...\n");
  if (ui_fab_base_start_ripple(NULL, 0.0f, 0.0f) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_get_ripple_state(NULL, &rstate) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_get_ripple_state(fab, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_fab_base_start_ripple(fab, 10.0f, 15.0f);
  ui_fab_base_get_ripple_state(fab, &rstate);
  if (!rstate->active)
    return 1;
  if (rstate->center_x != 10.0f || rstate->center_y != 15.0f)
    return 1;

  ui_fab_base_tick(fab, 50.0f);
  if (rstate->opacity == 1.0f)
    return 1; /* Should have animated somewhat */

  ui_fab_base_tick(fab, 5000.0f);
  ui_fab_base_get_ripple_state(fab, &rstate);
  if (rstate->active)
    return 1; /* Should be completed */

  {
    ui_error_t rc_cleanup = ui_fab_base_destroy(fab);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_fab_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int run_oom_tests(void) {
  struct ui_fab_base *fab = NULL;
  struct ui_button_base *action_btn = NULL;
  ui_error_t rc;

  printf("Testing OOM on create...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_fab_base_create(&fab);
  g_malloc_fail_countdown = -1;

  g_mock_ripple_fail = 1;
  if (ui_fab_base_create(&fab) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  g_mock_ripple_fail = 0;

  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  rc = ui_fab_base_create(&fab);
  if (rc != UI_ERROR_NONE)
    return 1;

  printf("Testing OOM on add action...\n");
  ui_button_base_create(&action_btn);
  g_malloc_fail_countdown = 0;
  rc = ui_fab_base_add_action(fab, action_btn);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    {
      ui_error_t rc_cleanup = ui_button_base_destroy(action_btn);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_fab_base_destroy(fab);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    return 1;
  }

  /* Since we failed to add it, we must destroy it ourselves if not adopted */
  {
    ui_error_t rc_cleanup = ui_button_base_destroy(action_btn);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_fab_base_destroy(fab);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int run_coverage_tests(void);

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0)
    return 1;

  if (run_coverage_tests() != 0) {
    printf("Coverage tests failed.\n");
    return 1;
  }

  if (0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All test_ui_fab_base passed.\n");
  return 0;
}

/* Hack to reach unreachable branch in ui_fab_base_start_ripple */
struct ui_fab_base_hack {
  struct ui_button_base *main_button;
  struct ui_button_base **action_buttons;
  size_t action_count;
  size_t action_capacity;
  enum ui_fab_state state;
  float expansion_progress;
  struct ui_ripple_config ripple_cfg;
  struct ui_ripple_state ripple_state;
  struct ui_signal *disabled_signal;
  struct ui_signal *text_signal;
};

static int run_coverage_tests(void) {
  struct ui_fab_base *fab = NULL;
  struct ui_fab_base_hack *hack = NULL;
  struct ui_signal *dummy_sig = (struct ui_signal *)0x1234;

  printf("Running extra coverage tests...\n");
  ui_fab_base_create(&fab);

  if (ui_fab_base_bind_disabled(NULL, dummy_sig) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_bind_disabled(fab, dummy_sig) != UI_ERROR_NONE)
    return 1;

  if (ui_fab_base_bind_text(NULL, dummy_sig) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_fab_base_bind_text(fab, dummy_sig) != UI_ERROR_NONE)
    return 1;

  hack = (struct ui_fab_base_hack *)fab;
  hack->ripple_cfg.center_origin = 1;
  ui_fab_base_start_ripple(fab, 10.0f, 15.0f);

  if (hack->ripple_state.center_x != 0.0f ||
      hack->ripple_state.center_y != 0.0f) {
    printf("Ripple center origin branch not taken properly\n");
    return 1;
  }

  {
    ui_error_t rc_cleanup = ui_fab_base_destroy(fab);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}
