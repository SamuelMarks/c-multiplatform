/**
 * @file ui_property_grid_base.c
 * @brief ui_property_grid_base.c implementation.
 */
/*
 * \file ui_property_grid_base.c
 * \brief Implementation of the UI Property Grid Base component.
 */

/* clang-format off */
#include "ui_property_grid_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <string.h>
/* clang-format on */

/** @def UI_PROPERTY_GRID_MAX_ROWS
 * @brief Maximum number of rows in the property grid
 */
#define UI_PROPERTY_GRID_MAX_ROWS 128
/** @def UI_PROPERTY_GRID_MAX_GROUPS
 * @brief Maximum number of groups in the property grid
 */
#define UI_PROPERTY_GRID_MAX_GROUPS 32

/**
 * @struct ui_property_group_state
 * \brief Represents the state of a property group.
 */
struct ui_property_group_state {
  const char *group_id;   /**< The identifier of the group */
  ui_bool_t is_collapsed; /**< True if the group is collapsed */
};

/**
 * @struct ui_property_grid_base
 * \brief Internal structure representing a Property Grid component.
 */
struct ui_property_grid_base {
  struct ui_arena *arena; /**< Memory arena */

  struct ui_property_row rows[UI_PROPERTY_GRID_MAX_ROWS]; /**< List of rows */
  int num_rows; /**< Current row count */

  struct ui_property_group_state
      groups[UI_PROPERTY_GRID_MAX_GROUPS]; /**< List of groups */
  int num_groups;                          /**< Current group count */

  ui_property_editor_factory_fn
      editor_factory;      /**< Custom editor factory callback */
  void *factory_user_data; /**< Opaque user data for the factory */

  const char *current_filter; /**< Current search filter string */

  ui_signal_t *value_changed_signal; /**< Signal emitted on value change */
};

/**
 * \brief Evaluates equality of two pointer signal payloads.
 *
 * \param a First payload.
 * \param b Second payload.
 * \param out_equal Pointer to receive the equality result.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief pointer_equality.
 * @param a Parameter a.
 * @param b Parameter b.
 * @param out_equal Parameter out_equal.
 * @return Return value.
 */
static ui_error_t pointer_equality(union ui_signal_payload a,
                                   union ui_signal_payload b,
                                   ui_bool_t *out_equal) {
  *out_equal = (a.ptr_val == b.ptr_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a Property Grid base component.
 *
 * \param arena The memory arena to use for allocation.
 * \param out_grid Pointer to receive the created component handle.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_property_grid_base_create(struct ui_arena *arena,
                             struct ui_property_grid_base **out_grid) {
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !out_grid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t rc_cleanup =
        ui_arena_alloc(arena, sizeof(struct ui_property_grid_base), 8, &ptr);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  *out_grid = (struct ui_property_grid_base *)ptr;
  (*out_grid)->arena = arena;
  (*out_grid)->num_rows = 0;
  (*out_grid)->num_groups = 0;
  (*out_grid)->editor_factory = NULL;
  (*out_grid)->factory_user_data = NULL;
  (*out_grid)->current_filter = NULL;

  initial_payload.ptr_val = NULL;
  {
    ui_error_t rc_cleanup = ui_signal_create(
        arena, initial_payload, UI_SIGNAL_TYPE_POINTER, pointer_equality, NULL,
        UI_SIGNAL_MODE_SINGLE_THREADED, &(*out_grid)->value_changed_signal);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a Property Grid base component.
 *
 * \param grid The component to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_property_grid_base_destroy(struct ui_property_grid_base *grid) {
  if (!grid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t rc_cleanup = ui_signal_destroy(grid->value_changed_signal);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets an existing group state or creates a new one.
 *
 * \param grid The component.
 * \param group_id The group identifier.
 * \param out_group Pointer to receive the group state.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t
/**
 * @brief get_or_create_group.
 * @param grid Parameter grid.
 * @param group_id Parameter group_id.
 * @param out_group Parameter out_group.
 * @return Return value.
 */
get_or_create_group(struct ui_property_grid_base *grid, const char *group_id,
                    struct ui_property_group_state **out_group) {
  int i;
  *out_group = NULL;

  for (i = 0; i < grid->num_groups; ++i) {
    if (strcmp(grid->groups[i].group_id, group_id) == 0) {
      *out_group = &grid->groups[i];
      return UI_ERROR_NONE;
    }
  }

  if (grid->num_groups >= UI_PROPERTY_GRID_MAX_GROUPS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  grid->groups[grid->num_groups].group_id = group_id;
  grid->groups[grid->num_groups].is_collapsed = UI_FALSE;
  *out_group = &grid->groups[grid->num_groups++];
  return UI_ERROR_NONE;
}

/**
 * \brief Registers a property row in the grid data model.
 *
 * \param grid The component.
 * \param row The property row definition.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_property_grid_base_add_property(struct ui_property_grid_base *grid,
                                   const struct ui_property_row *row) {
  if (!grid || !row || !row->id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (grid->num_rows >= UI_PROPERTY_GRID_MAX_ROWS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (row->group_id) {
    struct ui_property_group_state *group = NULL;
    {
      ui_error_t goc_rc = get_or_create_group(grid, row->group_id, &group);
      if (goc_rc != UI_ERROR_NONE)
        return goc_rc;
    }
  }

  grid->rows[grid->num_rows++] = *row;

  return UI_ERROR_NONE;
}

/**
 * \brief Sets a custom factory function for instantiating inline editors.
 *
 * \param grid The component.
 * \param factory_fn The callback function.
 * \param user_data Opaque data passed to the callback.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_property_grid_base_set_editor_factory(
    struct ui_property_grid_base *grid,
    ui_property_editor_factory_fn factory_fn, void *user_data) {
  if (!grid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  grid->editor_factory = factory_fn;
  grid->factory_user_data = user_data;

  return UI_ERROR_NONE;
}

/**
 * \brief Filters the visible properties based on a search string.
 *
 * \param grid The component.
 * \param search_query The string to filter by.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_property_grid_base_set_filter(struct ui_property_grid_base *grid,
                                            const char *search_query) {
  if (!grid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  grid->current_filter = search_query;
  return UI_ERROR_NONE;
}

/**
 * \brief Toggles the collapsed/expanded state of a specific property group.
 *
 * \param grid The component.
 * \param group_id The identifier of the group.
 * \param is_collapsed True to collapse, false to expand.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_property_grid_base_set_group_collapsed(struct ui_property_grid_base *grid,
                                          const char *group_id,
                                          ui_bool_t is_collapsed) {
  struct ui_property_group_state *group;

  if (!grid || !group_id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t goc_rc = get_or_create_group(grid, group_id, &group);
    if (goc_rc != UI_ERROR_NONE)
      return goc_rc;
  }

  group->is_collapsed = is_collapsed;

  return UI_ERROR_NONE;
}

/**
 * \brief Retrieves the signal emitted when a property's value is modified.
 *
 * \param grid The component.
 * \param out_signal Pointer to receive the signal handle.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_property_grid_base_get_value_changed_signal(
    struct ui_property_grid_base *grid, ui_signal_t **out_signal) {
  if (!grid || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = grid->value_changed_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Internal simulation helper.
 *
 * \param grid The component.
 * \param property_id The ID of the property to trigger a change for.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
_ui_property_grid_base_trigger_change(struct ui_property_grid_base *grid,
                                      const char *property_id) {
  int i;
  union ui_signal_payload payload;

  if (!grid || !property_id)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < grid->num_rows; ++i) {
    if (strcmp(grid->rows[i].id, property_id) == 0) {
      payload.ptr_val = &grid->rows[i];
      return ui_signal_set(grid->value_changed_signal, payload);
    }
  }

  return UI_ERROR_NOT_FOUND;
}
