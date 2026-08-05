/* clang-format off */
#include "ui_selection_model.h"
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_selection_model {
  int is_multi;
  void **selected_ids;
  int capacity;
  int count;
  ui_selection_model_on_change_t on_change;
  void *on_change_user_data;
};

static ui_error_t trigger_change(struct ui_selection_model *model) {
  if (model && model->on_change) {
    return model->on_change(model, model->on_change_user_data);
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_selection_model_create(struct ui_selection_model **out_model) {
  struct ui_selection_model *model;

  if (!out_model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  model = (struct ui_selection_model *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_selection_model));
  if (!model) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  model->is_multi = 0;
  model->selected_ids = NULL;
  model->capacity = 0;
  model->count = 0;
  model->on_change = NULL;
  model->on_change_user_data = NULL;

  *out_model = model;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_selection_model_set_on_change(struct ui_selection_model *model,
                                 ui_selection_model_on_change_t callback,
                                 void *user_data) {
  if (!model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  model->on_change = callback;
  model->on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

ui_error_t ui_selection_model_destroy(struct ui_selection_model *model) {
  if (!model) {
    return UI_ERROR_NONE;
  }

  if (model->selected_ids) {
    C_MULTIPLATFORM_FREE(model->selected_ids);
  }
  C_MULTIPLATFORM_FREE(model);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_selection_model_set_multi_select(struct ui_selection_model *model,
                                               int is_multi) {
  if (!model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  model->is_multi = is_multi;

  if (!is_multi && model->count > 1) {
    /* Truncate to the first selected item */
    model->count = 1;
    return trigger_change(model);
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_selection_model_select(struct ui_selection_model *model,
                                     void *id) {
  int i;

  if (!model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Check if already selected */
  for (i = 0; i < model->count; ++i) {
    if (model->selected_ids[i] == id) {
      if (!model->is_multi && model->count > 1) {
        /* Fallback cleanup for invalid state */
        model->count = 1;
        model->selected_ids[0] = id;
        return trigger_change(model);
      }
      return UI_ERROR_NONE; /* Already selected */
    }
  }

  if (!model->is_multi) {
    /* Single select: replace selection */
    if (model->capacity < 1) {
      void **new_arr = (void **)C_MULTIPLATFORM_MALLOC(sizeof(void *));
      if (!new_arr) {
        return UI_ERROR_OUT_OF_MEMORY;
      }
      model->selected_ids = new_arr;
      model->capacity = 1;
    }
    model->selected_ids[0] = id;
    model->count = 1;
    return trigger_change(model);
  }

  /* Multi select: append */
  if (model->count >= model->capacity) {
    int new_cap = model->capacity == 0 ? 4 : model->capacity * 2;
    void **new_arr = (void **)C_MULTIPLATFORM_REALLOC(
        model->selected_ids, (size_t)new_cap * sizeof(void *));
    if (!new_arr) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    model->selected_ids = new_arr;
    model->capacity = new_cap;
  }

  model->selected_ids[model->count] = id;
  model->count++;
  return trigger_change(model);
}

ui_error_t ui_selection_model_deselect(struct ui_selection_model *model,
                                       void *id) {
  int i;
  int found_index = -1;

  if (!model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < model->count; ++i) {
    if (model->selected_ids[i] == id) {
      found_index = i;
      break;
    }
  }

  if (found_index >= 0) {
    for (i = found_index; i < model->count - 1; ++i) {
      model->selected_ids[i] = model->selected_ids[i + 1];
    }
    model->count--;
    return trigger_change(model);
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_selection_model_toggle(struct ui_selection_model *model,
                                     void *id) {
  int i;
  int is_selected = 0;

  if (!model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < model->count; ++i) {
    if (model->selected_ids[i] == id) {
      is_selected = 1;
      break;
    }
  }

  if (is_selected) {
    return ui_selection_model_deselect(model, id);
  } else {
    return ui_selection_model_select(model, id);
  }
}

ui_error_t ui_selection_model_clear(struct ui_selection_model *model) {
  if (!model) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (model->count > 0) {
    model->count = 0;
    return trigger_change(model);
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_selection_model_select_all(struct ui_selection_model *model,
                                         void **ids, int count) {
  int i;
  int rc;

  if (!model || (!ids && count > 0) || count < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!model->is_multi) {
    if (count > 0) {
      /* For single select, 'select all' doesn't make sense if count > 1, but we
       * can just select the last to emulate standard behavior or return error.
       * Returning error is safer. */
      if (count == 1) {
        return ui_selection_model_select(model, ids[0]);
      }
      return UI_ERROR_UNSUPPORTED; /* Cannot select multiple in single-select
                                      mode */
    }
    return UI_ERROR_NONE;
  }

  /* Suspend change events or just trigger at the end?
     For simple implementation, we can just let it trigger per item,
     or we can bypass the per-item trigger. Since `ui_selection_model_select`
     triggers it, it's safer to just do that, though slightly noisy.
     We will let it trigger per item. */
  for (i = 0; i < count; ++i) {
    rc = ui_selection_model_select(model, ids[i]);
    if (rc != UI_ERROR_NONE) {
      return (ui_error_t)rc;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_selection_model_is_selected(const struct ui_selection_model *model, void *id,
                               int *out_is_selected) {
  int i;

  if (!model || !out_is_selected) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_selected = 0;
  for (i = 0; i < model->count; ++i) {
    if (model->selected_ids[i] == id) {
      *out_is_selected = 1;
      break;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_selection_model_get_selected_count(const struct ui_selection_model *model,
                                      int *out_count) {
  if (!model || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = model->count;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_selection_model_get_selected(const struct ui_selection_model *model,
                                void **out_ids, int capacity) {
  int i;
  int to_copy;

  if (!model || !out_ids) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  to_copy = capacity < model->count ? capacity : model->count;

  for (i = 0; i < to_copy; ++i) {
    out_ids[i] = model->selected_ids[i];
  }

  return UI_ERROR_NONE;
}
