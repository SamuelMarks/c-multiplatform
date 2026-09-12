/**
 * @file ui_component_registry.h
 * @brief Component registry for mapping schema type strings to factory and
 * lifecycle vtables.
 */

#ifndef UI_COMPONENT_REGISTRY_H
#define UI_COMPONENT_REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include "ui_dom_node.h"
#include "ui_control_value_accessor.h"
#include <stddef.h>
/* clang-format on */

struct ui_component_registry;

/**
 * @brief Factory function pointer to allocate and create a widget instance and
 * its DOM root.
 *
 * @param out_instance Pointer to receive the component instance pointer.
 * @param out_dom_node Pointer to receive the associated DOM node.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_component_factory_fn)(
    void **out_instance, struct ui_dom_node **out_dom_node);

/**
 * @brief Function pointer to retrieve a Control Value Accessor for a widget
 * instance.
 *
 * @param instance The component instance.
 * @param out_cva Pointer to store the populated CVA interface.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_SUPPORTED if not a form
 * control.
 */
typedef ui_error_t (*ui_component_cva_fn)(
    void *instance, struct ui_control_value_accessor *out_cva);

/**
 * @brief Function pointer to apply a static or coerced property by name.
 *
 * @param instance The component instance.
 * @param key The property key.
 * @param value The property value string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_component_prop_fn)(void *instance, const char *key,
                                           const char *value);

/**
 * @brief Function pointer to attach an event handler to a widget instance.
 *
 * @param instance The component instance.
 * @param event_name The event hook name (e.g. on_click, on_change).
 * @param callback The callback function to invoke.
 * @param user_data User context to pass to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_component_event_fn)(
    void *instance, const char *event_name,
    ui_error_t (*callback)(void *user_data), void *user_data);

/**
 * @brief Function pointer to destroy a widget instance.
 *
 * @param instance The component instance to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_component_destroy_fn)(void *instance);

/**
 * @brief Function pointer to append a child DOM node to a container component.
 *
 * @param instance The component instance.
 * @param child_node The child DOM node to append.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_component_append_fn)(void *instance,
                                             struct ui_dom_node *child_node);

/**
 * @struct ui_component_vtable
 * @brief Lifecycle and binding vtable for a registered component type.
 */
struct ui_component_vtable {
  ui_component_factory_fn factory;     /**< Widget allocation factory */
  ui_component_cva_fn get_cva;         /**< Optional CVA getter */
  ui_component_prop_fn set_prop;       /**< Property setter */
  ui_component_event_fn attach_event;  /**< Event hook attacher */
  ui_component_destroy_fn destroy;     /**< Widget destructor */
  ui_component_append_fn append_child; /**< Child node appender */
};

/**
 * @brief Creates a new component registry.
 *
 * @param out_registry Pointer to receive the allocated registry.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_component_registry_create(struct ui_component_registry **out_registry);

/**
 * @brief Destroys a component registry and frees its registrations.
 *
 * @param registry The registry to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_component_registry_destroy(struct ui_component_registry *registry);

/**
 * @brief Registers a component type with its associated vtable.
 *
 * @param registry The registry to register into.
 * @param type_name The schema type name (e.g. "ui_button_base", "row").
 * @param vtable The vtable defining creation and manipulation operations.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_component_registry_register(
    struct ui_component_registry *registry, const char *type_name,
    const struct ui_component_vtable *vtable);

/**
 * @brief Looks up a registered component type vtable.
 *
 * @param registry The registry to query.
 * @param type_name The schema type name to find.
 * @param out_vtable Pointer to receive the found vtable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not registered.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_component_registry_lookup(
    const struct ui_component_registry *registry, const char *type_name,
    const struct ui_component_vtable **out_vtable);

/**
 * @brief Registers all default standard engine widgets and layout primitives.
 * (ui_button_base, ui_input_base, ui_card_base, ui_checkbox_base,
 * ui_label_base, ui_slider_base, row, column, grid, container).
 *
 * @param registry The registry to populate with defaults.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_component_registry_register_defaults(struct ui_component_registry *registry);

/**
 * @brief Retrieves the global default component registry instance.
 *
 * @param out_registry Pointer to receive the shared default registry.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_component_registry_get_default(struct ui_component_registry **out_registry);

/**
 * @brief Shuts down and frees the global default component registry instance.
 *
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_component_registry_shutdown_default(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_COMPONENT_REGISTRY_H */
