/* clang-format off */
#include "ui_chips_base.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/**
 * @struct ui_chips_base
 * @brief Internal representation of a chips/tags base component.
 */
struct ui_chips_base {
  /** @brief Array of string tokens/chips. */
  char **tokens;
  /** @brief Current number of tokens. */
  size_t count;
  /** @brief Capacity of the tokens array. */
  size_t capacity;

  /** @brief CVA: Callback fired when value changes. */
  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  /** @brief CVA: User data for on_change callback. */
  void *cva_on_change_user_data;

  /** @brief CVA: Callback fired when input is touched. */
  ui_error_t (*cva_on_touched)(void *user_data);
  /** @brief CVA: User data for on_touched callback. */
  void *cva_on_touched_user_data;

  /** @brief 1 if disabled, 0 otherwise. */
  int is_disabled;
};

/**
 * @brief trigger_cva_change.
 * @param chips Parameter chips.
 * @return Return value.
 */
static ui_error_t trigger_cva_change(struct ui_chips_base *chips) {
  union ui_signal_payload payload;
  if (chips->cva_on_change) {
    /* Pass the token array directly as a pointer payload.
       A robust app might serialize this or rely on custom equality functions,
       but providing the base pointer is standard for array/pointer payloads. */
    payload.ptr_val = chips->tokens;
    return chips->cva_on_change(payload, chips->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief chips_cva_write_value.
 * @param component Parameter component.
 * @param value Parameter value.
 * @return Return value.
 */
static ui_error_t chips_cva_write_value(void *component,
                                        union ui_signal_payload value) {
  struct ui_chips_base *chips = (struct ui_chips_base *)component;
  /* Complex arrays from signals might require deep updates.
     For this base implementation, we do not fully replace the array
     if driven from outside, but we provide the hook. */
  (void)chips;
  (void)value;
  return UI_ERROR_NONE; /* Ignored for now */
}

static ui_error_t chips_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_chips_base *chips = (struct ui_chips_base *)component;
  if (!chips) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  chips->cva_on_change = callback;
  chips->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t chips_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_chips_base *chips = (struct ui_chips_base *)component;
  if (!chips) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  chips->cva_on_touched = callback;
  chips->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief chips_cva_set_disabled_state.
 * @param component Parameter component.
 * @param is_disabled Parameter is_disabled.
 * @return Return value.
 */
static ui_error_t chips_cva_set_disabled_state(void *component,
                                               ui_bool_t is_disabled) {
  struct ui_chips_base *chips = (struct ui_chips_base *)component;
  if (!chips) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  chips->is_disabled = is_disabled;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_chips_base_create.
 * @param out_chips Parameter out_chips.
 * @param out_cva Parameter out_cva.
 * @return Return value.
 */
ui_error_t ui_chips_base_create(struct ui_chips_base **out_chips,
                                struct ui_control_value_accessor *out_cva) {
  struct ui_chips_base *chips;
  if (!out_chips) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  chips = (struct ui_chips_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_chips_base));
  if (!chips) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  chips->tokens = NULL;
  chips->count = 0;
  chips->capacity = 0;
  chips->cva_on_change = NULL;
  chips->cva_on_change_user_data = NULL;
  chips->cva_on_touched = NULL;
  chips->cva_on_touched_user_data = NULL;
  chips->is_disabled = 0;

  if (out_cva) {
    out_cva->write_value = chips_cva_write_value;
    out_cva->register_on_change = chips_cva_register_on_change;
    out_cva->register_on_touched = chips_cva_register_on_touched;
    out_cva->set_disabled_state = chips_cva_set_disabled_state;
  }

  *out_chips = chips;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_chips_base_destroy.
 * @param chips Parameter chips.
 * @return Return value.
 */
ui_error_t ui_chips_base_destroy(struct ui_chips_base *chips) {
  size_t i;
  if (!chips) {
    return UI_ERROR_NONE;
  }
  for (i = 0; i < chips->count; ++i) {
    C_MULTIPLATFORM_FREE(chips->tokens[i]);
  }
  if (chips->tokens) {
    C_MULTIPLATFORM_FREE(chips->tokens);
  }
  C_MULTIPLATFORM_FREE(chips);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_chips_base_add.
 * @param chips Parameter chips.
 * @param token Parameter token.
 * @return Return value.
 */
ui_error_t ui_chips_base_add(struct ui_chips_base *chips, const char *token) {
  char **new_tokens;
  char *new_token;
  size_t len;

  if (!chips || !token) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (chips->count == chips->capacity) {
    size_t new_cap = chips->capacity == 0 ? 4 : chips->capacity * 2;
    new_tokens = (char **)C_MULTIPLATFORM_REALLOC(chips->tokens,
                                                  new_cap * sizeof(char *));
    if (!new_tokens) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    chips->tokens = new_tokens;
    chips->capacity = new_cap;
  }

  len = strlen(token);
  new_token = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!new_token) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  if (UI_STRCPY(new_token, len + 1, token) != 0) {
    C_MULTIPLATFORM_FREE(new_token);
    return UI_ERROR_UNKNOWN;
  }

  chips->tokens[chips->count++] = new_token;
  return trigger_cva_change(chips);
}

/**
 * @brief ui_chips_base_remove.
 * @param chips Parameter chips.
 * @param index Parameter index.
 * @return Return value.
 */
ui_error_t ui_chips_base_remove(struct ui_chips_base *chips, size_t index) {
  size_t i;
  if (!chips) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (index >= chips->count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  C_MULTIPLATFORM_FREE(chips->tokens[index]);
  for (i = index; i < chips->count - 1; ++i) {
    chips->tokens[i] = chips->tokens[i + 1];
  }
  chips->count--;
  return trigger_cva_change(chips);
}

/**
 * @brief ui_chips_base_remove_last.
 * @param chips Parameter chips.
 * @return Return value.
 */
ui_error_t ui_chips_base_remove_last(struct ui_chips_base *chips) {
  if (!chips) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (chips->count == 0) {
    return UI_ERROR_NOT_FOUND;
  }
  return ui_chips_base_remove(chips, chips->count - 1);
}

/**
 * @brief ui_chips_base_get_count.
 * @param chips Parameter chips.
 * @param out_count Parameter out_count.
 * @return Return value.
 */
ui_error_t ui_chips_base_get_count(const struct ui_chips_base *chips,
                                   size_t *out_count) {
  if (!chips || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_count = chips->count;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_chips_base_get_token.
 * @param chips Parameter chips.
 * @param index Parameter index.
 * @param out_token Parameter out_token.
 * @return Return value.
 */
ui_error_t ui_chips_base_get_token(const struct ui_chips_base *chips,
                                   size_t index, const char **out_token) {
  if (!chips || !out_token) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (index >= chips->count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  *out_token = chips->tokens[index];
  return UI_ERROR_NONE;
}

/**
 * @brief ui_chips_base_handle_backspace.
 * @param chips Parameter chips.
 * @param current_input Parameter current_input.
 * @param out_focus_moved_to_last Parameter out_focus_moved_to_last.
 * @return Return value.
 */
ui_error_t ui_chips_base_handle_backspace(struct ui_chips_base *chips,
                                          const char *current_input,
                                          int *out_focus_moved_to_last) {
  if (!chips || !out_focus_moved_to_last) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_focus_moved_to_last = 0;

  /* If the input is empty or null, and there are chips, focus moves to the last
   * chip */
  if (chips->count > 0 && (!current_input || current_input[0] == '\0')) {
    *out_focus_moved_to_last = 1;
  }

  return UI_ERROR_NONE;
}
