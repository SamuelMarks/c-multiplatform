/* clang-format off */
#include "ui_sort_header_base.h"
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_sort_header_base {
  int is_multi;
  struct ui_sort_state *states;
  size_t capacity;
  size_t count;
  struct ui_signal *direction_signal;
};

/** \brief ui_error */
ui_error_t
ui_sort_header_base_create(struct ui_sort_header_base **out_sort_header) {
  struct ui_sort_header_base *manager;

  if (!out_sort_header) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  manager = (struct ui_sort_header_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_sort_header_base));
  if (!manager) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  manager->is_multi = 0;
  manager->states = NULL;
  manager->capacity = 0;
  manager->count = 0;

  *out_sort_header = manager;
  return UI_ERROR_NONE;
}

ui_error_t
ui_sort_header_base_destroy(struct ui_sort_header_base *sort_header) {
  if (!sort_header) {
    return UI_ERROR_NONE;
  }

  if (sort_header->states) {
    C_MULTIPLATFORM_FREE(sort_header->states);
  }
  C_MULTIPLATFORM_FREE(sort_header);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_sort_header_base_set_multi_sort(struct ui_sort_header_base *sort_header,
                                   int is_multi) {
  if (!sort_header) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sort_header->is_multi = is_multi;

  if (!is_multi && sort_header->count > 1) {
    /* Keep only the first (most recently added/modified) state */
    sort_header->count = 1;
  }

  return UI_ERROR_NONE;
}

static ui_error_t find_state_index(struct ui_sort_header_base *sort_header,
                                   void *id, int *out_index) {
  size_t i;
  *out_index = -1;
  for (i = 0; i < sort_header->count; ++i) {
    if (sort_header->states[i].id == id) {
      *out_index = (int)i;
      return UI_ERROR_NONE;
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t remove_state_at(struct ui_sort_header_base *sort_header,
                                  int index) {
  size_t i;

  for (i = (size_t)index; i < sort_header->count - 1; ++i) {
    sort_header->states[i] = sort_header->states[i + 1];
  }
  sort_header->count--;

  return UI_ERROR_NONE;
}

static ui_error_t
insert_or_update_state(struct ui_sort_header_base *sort_header, void *id,
                       enum ui_sort_direction direction) {
  int existing_index;

  if (direction == UI_SORT_NONE) {
    ui_error_t find_rc = find_state_index(sort_header, id, &existing_index);
    if (find_rc != UI_ERROR_NONE)
      return find_rc;
    if (existing_index >= 0) {
      return remove_state_at(sort_header, existing_index);
    }
    return UI_ERROR_NONE;
  }

  if (!sort_header->is_multi) {
    sort_header->count = 0; /* Clear previous */
  } else {
    ui_error_t find_rc = find_state_index(sort_header, id, &existing_index);
    if (find_rc != UI_ERROR_NONE)
      return find_rc;
    if (existing_index >= 0) {
      sort_header->states[existing_index].direction = direction;
      return UI_ERROR_NONE;
    }
  }

  /* Need to append */
  if (sort_header->count >= sort_header->capacity) {
    size_t new_cap = sort_header->capacity == 0 ? 4 : sort_header->capacity * 2;
    struct ui_sort_state *new_states =
        (struct ui_sort_state *)C_MULTIPLATFORM_REALLOC(
            sort_header->states, new_cap * sizeof(struct ui_sort_state));
    if (!new_states) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    sort_header->states = new_states;
    sort_header->capacity = new_cap;
  }

  sort_header->states[sort_header->count].id = id;
  sort_header->states[sort_header->count].direction = direction;
  sort_header->count++;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_sort_header_base_toggle(struct ui_sort_header_base *sort_header,
                                      void *id) {
  int index;
  enum ui_sort_direction current_dir = UI_SORT_NONE;
  enum ui_sort_direction next_dir;

  ui_error_t find_rc;
  if (!sort_header) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  find_rc = find_state_index(sort_header, id, &index);
  if (find_rc != UI_ERROR_NONE)
    return find_rc;
  if (index >= 0) {
    current_dir = sort_header->states[index].direction;
  }

  switch (current_dir) {
  case UI_SORT_NONE:
    next_dir = UI_SORT_ASCENDING;
    break;
  case UI_SORT_ASCENDING:
    next_dir = UI_SORT_DESCENDING;
    break;
  case UI_SORT_DESCENDING:
  default:
    next_dir = UI_SORT_NONE;
    break;
  }

  return insert_or_update_state(sort_header, id, next_dir);
}

/** \brief ui_error */
ui_error_t
ui_sort_header_base_set_direction(struct ui_sort_header_base *sort_header,
                                  void *id, enum ui_sort_direction direction) {
  if (!sort_header) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return insert_or_update_state(sort_header, id, direction);
}

/** \brief ui_error */
ui_error_t
ui_sort_header_base_get_direction(const struct ui_sort_header_base *sort_header,
                                  void *id,
                                  enum ui_sort_direction *out_direction) {
  size_t i;

  if (!sort_header || !out_direction) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_direction = UI_SORT_NONE;
  for (i = 0; i < sort_header->count; ++i) {
    if (sort_header->states[i].id == id) {
      *out_direction = sort_header->states[i].direction;
      break;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_sort_header_base_get_active_sorts */
ui_error_t ui_sort_header_base_get_active_sorts(
    const struct ui_sort_header_base *sort_header,
    struct ui_sort_state *out_states, size_t capacity, size_t *out_count) {
  size_t i;
  size_t copy_count;

  if (!sort_header || (!out_states && capacity > 0) || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  copy_count = capacity < sort_header->count ? capacity : sort_header->count;
  *out_count = copy_count;

  for (i = 0; i < copy_count; ++i) {
    out_states[i] = sort_header->states[i];
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_sort_header_base_clear(struct ui_sort_header_base *sort_header) {
  if (!sort_header) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sort_header->count = 0;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_sort_header_base_bind_direction(struct ui_sort_header_base *widget,
                                   struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->direction_signal = signal;
  return UI_ERROR_NONE;
}
