/* clang-format off */
#include "../include/ui_effect.h"
#include "../include/ui_types.h"
#include "../include/ui_error.h"
#include "../include/ui_arena.h"
#include "../include/ui_reactor.h"
#include "ui_internal_mem.h"
#include "ui_reactive_graph.h"
/* clang-format on */

struct ui_effect {
  ui_effect_fn effect_fn;
  void *user_data;
  struct ui_reactor *target_reactor;
  struct ui_arena *arena;
  struct ui_reactive_node self_node;
};

static ui_error_t ui_effect_evaluate(void *user_data) {
  ui_effect_t *eff = (ui_effect_t *)user_data;
  struct ui_reactive_node *prev_node = NULL;
  ui_error_t rc = UI_ERROR_NONE;

  if (eff && eff->effect_fn) {
    rc = ui_reactive_graph_set_current_node(&eff->self_node, &prev_node);
    if (rc != UI_ERROR_NONE)
      return rc;

    rc = eff->effect_fn(eff->user_data);
    if (rc != UI_ERROR_NONE)
      return rc;

    rc = ui_reactive_graph_set_current_node(prev_node, NULL);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return rc;
}

static ui_error_t ui_effect_on_notify(void *user_data) {
  ui_effect_t *eff = (ui_effect_t *)user_data;

  if (eff->target_reactor) {
    ui_error_t sched_rc =
        ui_reactor_schedule(eff->target_reactor, ui_effect_evaluate, eff);
    if (sched_rc != UI_ERROR_NONE)
      return sched_rc;
    return ui_reactor_wake(eff->target_reactor);
  }

  /* Fallback to synchronous if scheduling fails or no target_reactor is
   * provided */
  return ui_effect_evaluate(eff);
}

ui_error_t ui_effect_create(struct ui_arena *arena, ui_effect_fn effect_fn,
                            void *user_data, struct ui_reactor *target_reactor,
                            ui_effect_t **out_effect) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_effect_t *eff = NULL;

  if (!out_effect || !effect_fn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (arena) {
    void *ptr = NULL;
    rc = ui_arena_alloc(arena, sizeof(ui_effect_t), sizeof(void *), &ptr);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    eff = (ui_effect_t *)ptr;
  } else {
    eff = (ui_effect_t *)C_MULTIPLATFORM_MALLOC(sizeof(ui_effect_t));
  }

  if (!eff) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  eff->effect_fn = effect_fn;
  eff->user_data = user_data;
  eff->target_reactor = target_reactor;
  eff->arena = arena;
  eff->self_node.notify_fn = ui_effect_on_notify;
  eff->self_node.user_data = eff;

  /* Run once to establish initial dependencies */
  rc = ui_effect_on_notify(eff);
  if (rc != UI_ERROR_NONE)
    return rc;

  *out_effect = eff;
  return UI_ERROR_NONE;
}

ui_error_t ui_effect_destroy(ui_effect_t *effect) {
  if (!effect) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!effect->arena) {
    C_MULTIPLATFORM_FREE(effect);
  }

  return UI_ERROR_NONE;
}
