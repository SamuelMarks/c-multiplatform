/**
 * @file ui_canonical_layout_base.c
 * @brief Implementation of the canonical layout.
 */

/* clang-format off */
#include "ui_canonical_layout_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct ui_canonical_layout_base
 * @struct ui_canonical_layout_base
 * @brief Internal implementation of the canonical layout.
 */
struct ui_canonical_layout_base {
  /* @brief Pointer to the arena used for allocation. */
  struct ui_arena *arena; /**< arena */
  /* @brief Current layout size class. */
  enum ui_window_size_class size_class; /**< size_class */
  /* @brief Signal dispatched on size class change. */
  ui_signal_t *layout_changed_signal; /**< layout_changed_signal */

  /* @brief Central body component. */
  struct ui_component *body; /**< body */
  /* @brief Left/start side pane. */
  struct ui_component *leading_pane; /**< leading_pane */
  /* @brief Right/end side pane. */
  struct ui_component *trailing_pane; /**< trailing_pane */
  /* @brief Bottom bar component. */
  struct ui_component *bottom_bar; /**< bottom_bar */
};

/**
 * @brief size_class_equality.
 * @param a Parameter a.
 * @param b Parameter b.
 * @param out_equal Parameter out_equal.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t size_class_equality(union ui_signal_payload a,
                                      union ui_signal_payload b,
                                      ui_bool_t *out_equal) {
  /* Assumes out_equal is valid since called internally by ui_signal */
  *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_canonical_layout_base_create.
 * @param arena Parameter arena.
 * @param config Parameter config.
 * @param out_layout Parameter out_layout.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_canonical_layout_base_create(struct ui_arena *arena,
                                const struct ui_canonical_layout_config *config,
                                struct ui_canonical_layout_base **out_layout) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !config || !out_layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_canonical_layout_base), 8, &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  *out_layout = (struct ui_canonical_layout_base *)ptr;
  (*out_layout)->arena = arena;
  (*out_layout)->size_class = config->initial_size_class;
  (*out_layout)->body = NULL;
  (*out_layout)->leading_pane = NULL;
  (*out_layout)->trailing_pane = NULL;
  (*out_layout)->bottom_bar = NULL;

  initial_payload.int_val = (ui_int32)config->initial_size_class;
  err = ui_signal_create(
      arena, initial_payload, UI_SIGNAL_TYPE_INT32, size_class_equality, NULL,
      UI_SIGNAL_MODE_SINGLE_THREADED, &(*out_layout)->layout_changed_signal);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_canonical_layout_base_destroy.
 * @param layout Parameter layout.
 * @return UI_ERROR_NONE on success.
 */
#ifdef UI_TEST_MOCK_ALLOC
int g_canonical_mock_fail = 0;
extern ui_error_t ui_component_destroy(struct ui_component *c);
static ui_error_t mock_component_destroy_canonical(struct ui_component *c) {
  if (g_canonical_mock_fail == 20)
    return UI_ERROR_UNKNOWN;
  return ui_component_destroy(c);
}
#define ui_component_destroy mock_component_destroy_canonical
#endif

ui_error_t
ui_canonical_layout_base_destroy(struct ui_canonical_layout_base *layout) {
  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t rc_cleanup = ui_signal_destroy(layout->layout_changed_signal);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_canonical_layout_base_set_size_class.
 * @param layout Parameter layout.
 * @param size_class Parameter size_class.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_canonical_layout_base_set_size_class(struct ui_canonical_layout_base *layout,
                                        enum ui_window_size_class size_class) {
  ui_error_t err;
  union ui_signal_payload payload;

  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  layout->size_class = size_class;
  payload.int_val = (ui_int32)size_class;

  err = ui_signal_set(layout->layout_changed_signal, payload);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_canonical_layout_base_get_size_class.
 * @param layout Parameter layout.
 * @param out_size_class Parameter out_size_class.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_canonical_layout_base_get_size_class(
    const struct ui_canonical_layout_base *layout,
    enum ui_window_size_class *out_size_class) {
  if (!layout || !out_size_class) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_size_class = layout->size_class;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_canonical_layout_base_set_body.
 * @param layout Parameter layout.
 * @param body Parameter body.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_canonical_layout_base_set_body(struct ui_canonical_layout_base *layout,
                                  struct ui_component *body) {
  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  layout->body = body;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_canonical_layout_base_set_leading_pane.
 * @param layout Parameter layout.
 * @param leading_pane Parameter leading_pane.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_canonical_layout_base_set_leading_pane(
    struct ui_canonical_layout_base *layout,
    struct ui_component *leading_pane) {
  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  layout->leading_pane = leading_pane;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_canonical_layout_base_set_trailing_pane.
 * @param layout Parameter layout.
 * @param trailing_pane Parameter trailing_pane.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_canonical_layout_base_set_trailing_pane(
    struct ui_canonical_layout_base *layout,
    struct ui_component *trailing_pane) {
  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  layout->trailing_pane = trailing_pane;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_canonical_layout_base_set_bottom_bar.
 * @param layout Parameter layout.
 * @param bottom_bar Parameter bottom_bar.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_canonical_layout_base_set_bottom_bar(struct ui_canonical_layout_base *layout,
                                        struct ui_component *bottom_bar) {
  if (!layout) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  layout->bottom_bar = bottom_bar;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_canonical_layout_base_get_layout_changed_signal.
 * @param layout Parameter layout.
 * @param out_signal Parameter out_signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_canonical_layout_base_get_layout_changed_signal(
    struct ui_canonical_layout_base *layout, ui_signal_t **out_signal) {
  if (!layout || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = layout->layout_changed_signal;
  return UI_ERROR_NONE;
}
