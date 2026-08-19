/**
 * @file ui_property_grid_base.h
 * @brief Base property grid component for editing object fields and settings.
 */

#ifndef UI_PROPERTY_GRID_BASE_H
#define UI_PROPERTY_GRID_BASE_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_component.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ui_property_grid_base
 * @brief Opaque handle for the Property Grid component.
 */
struct ui_property_grid_base;

/**
 * @enum ui_property_value_type
 * @brief Identifies the expected abstract type of a property value, allowing
 * the grid to automatically delegate to the correct inline editor (e.g. text
 * input vs color picker).
 */
enum ui_property_value_type {
  /** @brief A string property value. */
  UI_PROPERTY_VALUE_TYPE_STRING = 0,
  /** @brief An integer property value. */
  UI_PROPERTY_VALUE_TYPE_INT = 1,
  /** @brief A floating-point property value. */
  UI_PROPERTY_VALUE_TYPE_FLOAT = 2,
  /** @brief A boolean property value. */
  UI_PROPERTY_VALUE_TYPE_BOOLEAN = 3,
  /** @brief A color property value. */
  UI_PROPERTY_VALUE_TYPE_COLOR = 4,
  /** @brief An enum property value. */
  UI_PROPERTY_VALUE_TYPE_ENUM = 5,
  /** @brief A custom property value. */
  UI_PROPERTY_VALUE_TYPE_CUSTOM = 6
};

/**
 * @struct ui_property_row
 * @brief Abstract definition of a single row in the property grid.
 */
struct ui_property_row {
  /** @brief The unique ID of the property row. */
  const char *id;
  /** @brief The display label for the property. */
  const char *label;
  /** @brief Optional group this property belongs to. */
  const char *group_id;
  /** @brief The abstract value type for the property. */
  enum ui_property_value_type type;
  /** @brief True if the property is read-only. */
  ui_bool_t is_read_only;
};

/**
 * @brief Callback signature for providing custom component editors for specific
 * properties.
 *
 * @param row The property row requesting an editor.
 * @param user_data Opaque data provided when setting the factory.
 * @param out_editor_component Pointer to receive the instanced editor
 * component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_property_editor_factory_fn)(
    const struct ui_property_row *row, void *user_data,
    struct ui_component **out_editor_component);

/**
 * @brief Creates a Property Grid base component.
 *
 * @param arena The memory arena to use for allocation.
 * @param out_grid Pointer to receive the created component handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_property_grid_base_create(struct ui_arena *arena,
                             struct ui_property_grid_base **out_grid);

/**
 * @brief Destroys a Property Grid base component.
 *
 * @param grid The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_property_grid_base_destroy(struct ui_property_grid_base *grid);

/**
 * @brief Registers a property row in the grid data model.
 *
 * @param grid The component.
 * @param row The property row definition.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_property_grid_base_add_property(struct ui_property_grid_base *grid,
                                   const struct ui_property_row *row);

/**
 * @brief Sets a custom factory function for instantiating inline editors for
 * specific properties.
 *
 * @param grid The component.
 * @param factory_fn The callback function.
 * @param user_data Opaque data passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_property_grid_base_set_editor_factory(
    struct ui_property_grid_base *grid,
    ui_property_editor_factory_fn factory_fn, void *user_data);

/**
 * @brief Filters the visible properties based on a search string.
 *
 * @param grid The component.
 * @param search_query The string to filter by (filters against labels).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_property_grid_base_set_filter(struct ui_property_grid_base *grid,
                                            const char *search_query);

/**
 * @brief Toggles the collapsed/expanded state of a specific property group.
 *
 * @param grid The component.
 * @param group_id The identifier of the group.
 * @param is_collapsed True to collapse, false to expand.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_property_grid_base_set_group_collapsed(struct ui_property_grid_base *grid,
                                          const char *group_id,
                                          ui_bool_t is_collapsed);

/**
 * @brief Retrieves the signal emitted when a property's value is modified.
 * The payload is a pointer to the ui_property_row struct that changed.
 *
 * @param grid The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_property_grid_base_get_value_changed_signal(
    struct ui_property_grid_base *grid, ui_signal_t **out_signal);

/**
 * @brief Internal simulation helper (used during interactions or tests to
 * trigger a change).
 *
 * @param grid The component.
 * @param property_id The ID of the property to trigger a change for.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
_ui_property_grid_base_trigger_change(struct ui_property_grid_base *grid,
                                      const char *property_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_PROPERTY_GRID_BASE_H */
