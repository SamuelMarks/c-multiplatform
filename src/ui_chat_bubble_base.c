/* clang-format off */
#include "ui_chat_bubble_base.h"
#include "ui_internal_mem.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

struct ui_chat_bubble_base {
  struct ui_arena *arena;
  struct ui_chat_bubble_config config;
  ui_signal_t *config_signal;
};

static ui_error_t int_equality(union ui_signal_payload a,
                               union ui_signal_payload b,
                               ui_bool_t *out_equal) {
  *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

static ui_error_t pack_config(const struct ui_chat_bubble_config *config,
                              ui_int32 *out_val) {
  if ((int)config->tail_placement > (int)UI_CHAT_BUBBLE_TAIL_TOP_RIGHT ||
      (int)config->tail_placement < (int)UI_CHAT_BUBBLE_TAIL_NONE)
    return UI_ERROR_INVALID_ARGUMENT;

  if ((int)config->group_position > (int)UI_CHAT_BUBBLE_GROUP_LAST ||
      (int)config->group_position < (int)UI_CHAT_BUBBLE_GROUP_SINGLE)
    return UI_ERROR_INVALID_ARGUMENT;

  /* Pack tail placement and group position into a single 32-bit int */
  *out_val = ((ui_int32)config->tail_placement << 16) |
             (ui_int32)config->group_position;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_chat_bubble_base_create(struct ui_arena *arena,
                           const struct ui_chat_bubble_config *config,
                           struct ui_chat_bubble_base **out_bubble) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !config || !out_bubble) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_chat_bubble_base), 8, &ptr);
#ifdef UI_TEST_MOCK_ALLOC
  {
    void *dummy = C_MULTIPLATFORM_MALLOC(1);
    if (!dummy) {
      err = UI_ERROR_OUT_OF_MEMORY;
    } else {
      C_MULTIPLATFORM_FREE(dummy);
    }
  }
#endif
  if (err != UI_ERROR_NONE)
    return err;

  *out_bubble = (struct ui_chat_bubble_base *)ptr;
  (*out_bubble)->arena = arena;
  (*out_bubble)->config = *config;

  {
    ui_int32 packed = 0;
    err = pack_config(config, &packed);
    if (err != UI_ERROR_NONE)
      return err;
    initial_payload.int_val = packed;
  }
  err = ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_INT32,
                         int_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_bubble)->config_signal);
#ifdef UI_TEST_MOCK_ALLOC
  {
    void *dummy = C_MULTIPLATFORM_MALLOC(1);
    if (!dummy) {
      err = UI_ERROR_OUT_OF_MEMORY;
    } else {
      C_MULTIPLATFORM_FREE(dummy);
    }
  }
#endif
  if (err != UI_ERROR_NONE)
    return err;

  return UI_ERROR_NONE;
}

ui_error_t ui_chat_bubble_base_destroy(struct ui_chat_bubble_base *bubble) {
  if (!bubble)
    return UI_ERROR_INVALID_ARGUMENT;
  (void)ui_signal_destroy(bubble->config_signal);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_chat_bubble_base_set_config(struct ui_chat_bubble_base *bubble,
                               const struct ui_chat_bubble_config *config) {
  union ui_signal_payload payload;

  if (!bubble || !config)
    return UI_ERROR_INVALID_ARGUMENT;

  bubble->config = *config;

  {
    ui_int32 packed = 0;
    ui_error_t err = pack_config(config, &packed);
    if (err != UI_ERROR_NONE)
      return err;
    payload.int_val = packed;
  }
  return ui_signal_set(bubble->config_signal, payload);
}

/** \brief ui_error */
ui_error_t
ui_chat_bubble_base_get_config_signal(struct ui_chat_bubble_base *bubble,
                                      ui_signal_t **out_signal) {
  if (!bubble || !out_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_signal = bubble->config_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_chat_bubble_base_calculate_text_bounds */
ui_error_t ui_chat_bubble_base_calculate_text_bounds(
    const struct ui_chat_bubble_base *bubble,
    const struct ui_dom_rect *raw_bounds, struct ui_dom_rect *out_text_bounds) {

  /* Hardcoded assumption for standard tail geometry (e.g. 10px tail width) */
  const double tail_width = 10.0;
  const double standard_padding = 12.0;

  if (!bubble || !raw_bounds || !out_text_bounds) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Start with standard interior padding */
  *out_text_bounds = *raw_bounds;
  out_text_bounds->x += standard_padding;
  out_text_bounds->y += standard_padding;
  out_text_bounds->width -= (standard_padding * 2);
  out_text_bounds->height -= (standard_padding * 2);

  /* Adjust bounds based on tail placement to avoid text overlapping the tail */
  switch (bubble->config.tail_placement) {
  case UI_CHAT_BUBBLE_TAIL_BOTTOM_LEFT:
  case UI_CHAT_BUBBLE_TAIL_TOP_LEFT:
    out_text_bounds->x += tail_width;
    out_text_bounds->width -= tail_width;
    break;
  case UI_CHAT_BUBBLE_TAIL_BOTTOM_RIGHT:
  case UI_CHAT_BUBBLE_TAIL_TOP_RIGHT:
    out_text_bounds->width -= tail_width;
    break;
  case UI_CHAT_BUBBLE_TAIL_NONE:
  default:
    /* No geometric adjustment needed for missing tail */
    break;
  }

  /* Ensure we don't return negative dimensions if bounds were extremely small
   */
  if (out_text_bounds->width < 0.0)
    out_text_bounds->width = 0.0;
  if (out_text_bounds->height < 0.0)
    out_text_bounds->height = 0.0;

  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
void ui_chat_bubble_base_mock_config(struct ui_chat_bubble_base *bubble,
                                     int tail_placement);
void ui_chat_bubble_base_mock_config(struct ui_chat_bubble_base *bubble,
                                     int tail_placement) {
  if (bubble) {
    bubble->config.tail_placement =
        (enum ui_chat_bubble_tail_placement)tail_placement;
  }
}
#endif
