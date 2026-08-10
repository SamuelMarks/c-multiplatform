/* clang-format off */
#include "ui_search_bar_base.h"
#include "ui_internal_mem.h"
/* clang-format on */

static ui_error_t trigger_cva_change(struct ui_search_bar_base *search_bar) {
  if (search_bar && search_bar->cva_on_change) {
    union ui_signal_payload payload;
    payload.ptr_val = search_bar->query;
    return search_bar->cva_on_change(payload,
                                     search_bar->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t trigger_cva_touched(struct ui_search_bar_base *search_bar) {
  if (search_bar && search_bar->cva_on_touched) {
    return search_bar->cva_on_touched(search_bar->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t search_bar_cva_write_value(void *component,
                                             union ui_signal_payload value) {
  struct ui_search_bar_base *search_bar =
      (struct ui_search_bar_base *)component;
  const char *str;

  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  str = (const char *)value.ptr_val;
  if (!str) {
    str = "";
  }

  return ui_search_bar_base_set_query(search_bar, str);
}

static ui_error_t search_bar_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_search_bar_base *search_bar =
      (struct ui_search_bar_base *)component;
  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  search_bar->cva_on_change = callback;
  search_bar->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t search_bar_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_search_bar_base *search_bar =
      (struct ui_search_bar_base *)component;
  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  search_bar->cva_on_touched = callback;
  search_bar->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t search_bar_cva_set_disabled_state(void *component,
                                                    int is_disabled) {
  struct ui_search_bar_base *search_bar =
      (struct ui_search_bar_base *)component;
  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  search_bar->is_disabled = is_disabled;
  return UI_ERROR_NONE;
}

ui_error_t ui_search_bar_base_init(struct ui_search_bar_base *search_bar,
                                   struct ui_component *component,
                                   struct ui_control_value_accessor *out_cva) {
  if (!search_bar || !component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  search_bar->component = component;
  search_bar->query = NULL;
  search_bar->leading_icon = NULL;
  search_bar->trailing_icon = NULL;
  search_bar->is_loading = 0;

  search_bar->cva_on_change = NULL;
  search_bar->cva_on_change_user_data = NULL;
  search_bar->cva_on_touched = NULL;
  search_bar->cva_on_touched_user_data = NULL;
  search_bar->is_disabled = 0;

  if (out_cva) {
    out_cva->write_value = search_bar_cva_write_value;
    out_cva->register_on_change = search_bar_cva_register_on_change;
    out_cva->register_on_touched = search_bar_cva_register_on_touched;
    out_cva->set_disabled_state = search_bar_cva_set_disabled_state;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_search_bar_base_set_query(struct ui_search_bar_base *search_bar,
                                        const char *query) {
  size_t len;

  if (!search_bar || !query) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (search_bar->query) {
    C_MULTIPLATFORM_FREE(search_bar->query);
    search_bar->query = NULL;
  }

  len = strlen(query);
  search_bar->query = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!search_bar->query) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  UI_STRCPY(search_bar->query, len + 1, query);

  {
    ui_error_t tc_rc = trigger_cva_change(search_bar);
    (void)tc_rc;
  }
  {
    ui_error_t tt_rc = trigger_cva_touched(search_bar);
    (void)tt_rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_search_bar_base_set_loading(struct ui_search_bar_base *search_bar,
                                          int is_loading) {
  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  search_bar->is_loading = is_loading;
  return UI_ERROR_NONE;
}

ui_error_t ui_search_bar_base_cleanup(struct ui_search_bar_base *search_bar) {
  if (!search_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (search_bar->query) {
    C_MULTIPLATFORM_FREE(search_bar->query);
    search_bar->query = NULL;
  }

  search_bar->leading_icon = NULL;
  search_bar->trailing_icon = NULL;
  search_bar->is_loading = 0;
  return UI_ERROR_NONE;
}
