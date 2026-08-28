/**
 * @file ui_ribbon_base.c
 * @brief ui_ribbon_base.c implementation.
 */
/*
 * \file ui_ribbon_base.c
 * \brief Implementation of the UI Ribbon Base component.
 */

/* clang-format off */
#include "ui_ribbon_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/* \brief Maximum number of ribbon groups */
/** @def UI_RIBBON_MAX_GROUPS
 * @brief Maximum groups
 */
#define UI_RIBBON_MAX_GROUPS 32
/* \brief Maximum number of contextual tabs */
/** @def UI_RIBBON_MAX_CONTEXTUAL_TABS
 * @brief Maximum contextual tabs
 */
#define UI_RIBBON_MAX_CONTEXTUAL_TABS 16

/**
 * @struct ui_ribbon_group_state
 * \brief Internal state of a specific ribbon group.
 */
struct ui_ribbon_group_state {
  struct ui_ribbon_group_config config; /**< Group configuration */
  enum ui_ribbon_group_collapse_state
      current_state; /**< Current collapse state */
};

/**
 * @struct ui_ribbon_base
 * \brief Internal structure representing the ribbon base component.
 */
struct ui_ribbon_base {
  struct ui_arena *arena; /**< Memory arena */

  struct ui_ribbon_group_state
      groups[UI_RIBBON_MAX_GROUPS]; /**< Configured groups */
  int num_groups;                   /**< Current number of groups */

  struct ui_ribbon_contextual_tab_config
      contextual_tabs[UI_RIBBON_MAX_CONTEXTUAL_TABS]; /**< Configured contextual
                                                         tabs */
  int num_contextual_tabs; /**< Current number of contextual tabs */

  ui_signal_t *group_state_changed_signal; /**< Signal for state changes */
};

/**
 * \brief Evaluates equality of two integer signal payloads (group IDs).
 *
 * \param a First payload.
 * \param b Second payload.
 * \param out_equal Pointer to receive the equality result.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief group_id_equality.
 * @param a Parameter a.
 * @param b Parameter b.
 * @param out_equal Parameter out_equal.
 * @return Return value.
 */
static ui_error_t group_id_equality(union ui_signal_payload a,
                                    union ui_signal_payload b,
                                    ui_bool_t *out_equal) {
  *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a ribbon base component.
 *
 * \param arena The memory arena to use for allocation.
 * \param out_ribbon Pointer to receive the created component handle.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_ribbon_base_create(struct ui_arena *arena,
                                 struct ui_ribbon_base **out_ribbon) {

  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !out_ribbon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t _ign_rc =
        ui_arena_alloc(arena, sizeof(struct ui_ribbon_base), 8, &ptr);
    (void)_ign_rc;
  }

  *out_ribbon = (struct ui_ribbon_base *)ptr;
  (*out_ribbon)->arena = arena;
  (*out_ribbon)->num_groups = 0;
  (*out_ribbon)->num_contextual_tabs = 0;

  initial_payload.int_val = -1; /* -1 means no group has changed yet */
  {
    ui_error_t _ign_rc = ui_signal_create(
        arena, initial_payload, UI_SIGNAL_TYPE_INT32, group_id_equality, NULL,
        UI_SIGNAL_MODE_SINGLE_THREADED,
        &(*out_ribbon)->group_state_changed_signal);
    (void)_ign_rc;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a ribbon base component.
 *
 * \param ribbon The component to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_ribbon_base_destroy(struct ui_ribbon_base *ribbon) {
  if (!ribbon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_signal_destroy(ribbon->group_state_changed_signal);

  return UI_ERROR_NONE;
}

/**
 * \brief Adds a command group configuration to the ribbon for width-based
 * collapse calculations.
 *
 * \param ribbon The component.
 * \param config The group configuration to add.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
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

/**
 * \brief Helper to find a group by ID.
 *
 * \param ribbon The component.
 * \param group_id The group ID to find.
 * \param out_state Pointer to receive the group state.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief find_group_state.
 * @param ribbon Parameter ribbon.
 * @param group_id Parameter group_id.
 * @param out_state Parameter out_state.
 * @return Return value.
 */
static ui_error_t find_group_state(const struct ui_ribbon_base *ribbon,
                                   int group_id,
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

/**
 * \brief Helper to sort indices by group priority (ascending) for overflow
 * calculation.
 *
 * \param ribbon The component.
 * \param indices Array of indices to sort.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief sort_indices_by_priority.
 * @param ribbon Parameter ribbon.
 * @param indices Parameter indices.
 * @return Return value.
 */
static ui_error_t sort_indices_by_priority(const struct ui_ribbon_base *ribbon,
                                           int *indices) {
  int i, j, temp;
  for (i = 0; i < ribbon->num_groups - 1; i++) {
    for (j = 0; j < ribbon->num_groups - i - 1; j++) {
      /* Compare priorities. Wait, if we want lower priority to collapse first,
         we should process lower priority first. So ascending sort by priority.
         Assuming lower int = lower priority (0 is lower priority than 1).
         If priorities are equal, what? Preserve order or something.
      */
      if (ribbon->groups[indices[j]].config.priority >
          ribbon->groups[indices[j + 1]].config.priority) {
        temp = indices[j];
        indices[j] = indices[j + 1];
        indices[j + 1] = temp;
      }
    }
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Calculates and updates the collapse states for all groups based on the
 * available width. This should be called by the layout pass during window
 * resize.
 *
 * \param ribbon The component.
 * \param available_width The total physical width available to the ribbon.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_ribbon_base_recalculate_overflow(struct ui_ribbon_base *ribbon,
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
  for (i = 0; i < ribbon->num_groups && current_width > available_width; ++i) {
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

/**
 * \brief Retrieves the calculated collapse state for a specific group.
 *
 * \param ribbon The component.
 * \param group_id The ID of the group.
 * \param out_state Pointer to receive the collapse state.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
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

/**
 * \brief Retrieves the signal emitted when a group's collapse state changes.
 * The payload contains the int group_id that changed.
 *
 * \param ribbon The component.
 * \param out_signal Pointer to receive the signal handle.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_ribbon_base_get_group_state_changed_signal(struct ui_ribbon_base *ribbon,
                                              ui_signal_t **out_signal) {
  if (!ribbon || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = ribbon->group_state_changed_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the active status of a contextual tab.
 *
 * \param ribbon The component.
 * \param tab_id The contextual tab ID.
 * \param is_active True to show, false to hide.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
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

/**
 * \brief Retrieves the active status of a contextual tab.
 *
 * \param ribbon The component.
 * \param tab_id The contextual tab ID.
 * \param out_is_active Pointer to receive the boolean status.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
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
