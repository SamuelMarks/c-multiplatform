/* clang-format off */
#include "ui_ribbon_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#define UI_RIBBON_MAX_GROUPS 32
#define UI_RIBBON_MAX_CONTEXTUAL_TABS 16

struct ui_ribbon_group_state {
  struct ui_ribbon_group_config config;
  enum ui_ribbon_group_collapse_state current_state;
};

/** \brief ui_ribbon_base */
struct ui_ribbon_base {
  struct ui_arena *arena;

  struct ui_ribbon_group_state groups[UI_RIBBON_MAX_GROUPS];
  int num_groups;

  /** \brief ui_ribbon_contextual_tab_config */
  struct ui_ribbon_contextual_tab_config
      contextual_tabs[UI_RIBBON_MAX_CONTEXTUAL_TABS];
  int num_contextual_tabs;

  ui_signal_t *group_state_changed_signal;
};

static enum ui_error group_id_equality(union ui_signal_payload a,
                                       union ui_signal_payload b,
                                       ui_bool_t *out_equal) {
  *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

enum ui_error ui_ribbon_base_create(struct ui_arena *arena,
                                    struct ui_ribbon_base **out_ribbon) {

  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !out_ribbon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_arena_alloc(arena, sizeof(struct ui_ribbon_base), 8, &ptr);

  *out_ribbon = (struct ui_ribbon_base *)ptr;
  (*out_ribbon)->arena = arena;
  (*out_ribbon)->num_groups = 0;
  (*out_ribbon)->num_contextual_tabs = 0;

  initial_payload.int_val = -1; /* -1 means no group has changed yet */
  (void)ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_INT32,
                         group_id_equality, NULL,
                         UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_ribbon)->group_state_changed_signal);

  return UI_ERROR_NONE;
}

enum ui_error ui_ribbon_base_destroy(struct ui_ribbon_base *ribbon) {
  if (!ribbon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_signal_destroy(ribbon->group_state_changed_signal);

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_ribbon_base_add_group_config(struct ui_ribbon_base *ribbon,
                                const struct ui_ribbon_group_config *config) {
  struct ui_ribbon_group_state *state;

  if (!ribbon || !config) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ribbon->num_groups >= UI_RIBBON_MAX_GROUPS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  state = &ribbon->groups[ribbon->num_groups++];
  state->config = *config;
  state->current_state = UI_RIBBON_GROUP_COLLAPSE_STATE_NORMAL;

  return UI_ERROR_NONE;
}

/* Helper to find a group by ID */
static enum ui_error
find_group_state(const struct ui_ribbon_base *ribbon, int group_id,
                 struct ui_ribbon_group_state **out_state) {
  int i;
  *out_state = NULL;
  for (i = 0; i < ribbon->num_groups; ++i) {
    if (ribbon->groups[i].config.group_id == group_id) {
      *out_state = (struct ui_ribbon_group_state *)&ribbon->groups[i];
      return UI_ERROR_NONE;
    }
  }
  return UI_ERROR_NONE;
}

/* Helper to sort indices by group priority (ascending) for overflow calculation
 */
static enum ui_error
sort_indices_by_priority(const struct ui_ribbon_base *ribbon, int *indices) {
  int i, j, temp;
  for (i = 0; i < ribbon->num_groups - 1; i++) {
    for (j = 0; j < ribbon->num_groups - i - 1; j++) {
      temp = indices[j];
      indices[j] = indices[j + 1];
      indices[j + 1] = temp;
    }
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_ribbon_base_recalculate_overflow(struct ui_ribbon_base *ribbon,
                                                  int available_width) {
  int indices[UI_RIBBON_MAX_GROUPS];
  int i;
  int current_width = 0;

  union ui_signal_payload payload;

  if (!ribbon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ribbon->num_groups == 0) {
    return UI_ERROR_NONE;
  }

  /* Initialize indices and assume all are normal initially to find max width */
  for (i = 0; i < ribbon->num_groups; ++i) {
    indices[i] = i;
    current_width += ribbon->groups[i].config.min_width_normal;
    /* Reset state internally before calculating */
    ribbon->groups[i].current_state = UI_RIBBON_GROUP_COLLAPSE_STATE_NORMAL;
  }

  (void)sort_indices_by_priority(ribbon, indices);

  /* Step 1: Collapse low-priority groups to COMPACT if we overflow */
  for (i = 0; i < ribbon->num_groups && current_width > available_width; ++i) {
    struct ui_ribbon_group_state *gs = &ribbon->groups[indices[i]];
    current_width -=
        (gs->config.min_width_normal - gs->config.min_width_compact);
    gs->current_state = UI_RIBBON_GROUP_COLLAPSE_STATE_COMPACT;

    payload.int_val = gs->config.group_id;
    (void)ui_signal_set(ribbon->group_state_changed_signal, payload);
  }

  /* Step 2: If we still overflow, collapse low-priority groups entirely */
  for (i = 0; current_width > available_width; ++i) {
    struct ui_ribbon_group_state *gs = &ribbon->groups[indices[i]];
    /* Assuming a completely collapsed dropdown button takes ~32px */
    int collapsed_width = 32;
    current_width -= (gs->config.min_width_compact - collapsed_width);
    gs->current_state = UI_RIBBON_GROUP_COLLAPSE_STATE_COLLAPSED;

    payload.int_val = gs->config.group_id;
    (void)ui_signal_set(ribbon->group_state_changed_signal, payload);
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_ribbon_base_get_group_state(const struct ui_ribbon_base *ribbon,
                               int group_id,
                               enum ui_ribbon_group_collapse_state *out_state) {
  struct ui_ribbon_group_state *gs;

  if (!ribbon || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)find_group_state(ribbon, group_id, &gs);
  if (!gs) {
    return UI_ERROR_NOT_FOUND;
  }

  *out_state = gs->current_state;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_ribbon_base_get_group_state_changed_signal(struct ui_ribbon_base *ribbon,
                                              ui_signal_t **out_signal) {
  if (!ribbon || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = ribbon->group_state_changed_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_ribbon_base_set_contextual_tab_active(struct ui_ribbon_base *ribbon,
                                         int tab_id, ui_bool_t is_active) {
  int i;

  if (!ribbon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Find existing */
  for (i = 0; i < ribbon->num_contextual_tabs; ++i) {
    if (ribbon->contextual_tabs[i].tab_id == tab_id) {
      ribbon->contextual_tabs[i].is_active = is_active;
      return UI_ERROR_NONE;
    }
  }

  /* Add new */
  if (ribbon->num_contextual_tabs >= UI_RIBBON_MAX_CONTEXTUAL_TABS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  ribbon->contextual_tabs[ribbon->num_contextual_tabs].tab_id = tab_id;
  ribbon->contextual_tabs[ribbon->num_contextual_tabs].is_active = is_active;
  ribbon->num_contextual_tabs++;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_ribbon_base_get_contextual_tab_active(const struct ui_ribbon_base *ribbon,
                                         int tab_id, ui_bool_t *out_is_active) {
  int i;

  if (!ribbon || !out_is_active) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_active = UI_FALSE;

  for (i = 0; i < ribbon->num_contextual_tabs; ++i) {
    if (ribbon->contextual_tabs[i].tab_id == tab_id) {
      *out_is_active = ribbon->contextual_tabs[i].is_active;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NOT_FOUND;
}
