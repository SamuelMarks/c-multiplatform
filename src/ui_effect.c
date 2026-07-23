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

static enum ui_error ui_effect_evaluate(void *user_data) {
  ui_effect_t *eff = (ui_effect_t *)user_data;
  struct ui_reactive_node *prev_node = NULL;
  enum ui_error rc = UI_ERROR_NONE;

  if (eff && eff->effect_fn) {
    rc = ui_reactive_graph_set_current_node(&eff->self_node, &prev_node);

    rc = eff->effect_fn(eff->user_data);

    ui_reactive_graph_set_current_node(prev_node, NULL);
  }
  return rc;
}

static enum ui_error ui_effect_on_notify(void *user_data) {
  ui_effect_t *eff = (ui_effect_t *)user_data;

  if (eff->target_reactor) {
    if (ui_reactor_schedule(eff->target_reactor, ui_effect_evaluate, eff) ==
        UI_ERROR_NONE) {
      return ui_reactor_wake(eff->target_reactor);
    }
  }

  /* Fallback to synchronous if scheduling fails or no target_reactor is
   * provided */
  return ui_effect_evaluate(eff);
}

enum ui_error ui_effect_create(struct ui_arena *arena, ui_effect_fn effect_fn,
                               void *user_data,
                               struct ui_reactor *target_reactor,
                               ui_effect_t **out_effect) {
  enum ui_error rc = UI_ERROR_NONE;
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
    eff = (ui_effect_t *)UI_MALLOC(sizeof(ui_effect_t));
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
  ui_effect_on_notify(eff);

  *out_effect = eff;
  return UI_ERROR_NONE;
}

enum ui_error ui_effect_destroy(ui_effect_t *effect) {
  if (!effect) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!effect->arena) {
    UI_FREE(effect);
  }

  return UI_ERROR_NONE;
}
