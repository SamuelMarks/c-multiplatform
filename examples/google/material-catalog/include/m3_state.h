/* clang-format off */
#ifndef M3_STATE_H
#define M3_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmp.h>
#include "material_catalog.h"

/* Forward declarations */
typedef struct m3_state_dep_t m3_state_dep_t;
typedef struct m3_debounce_task_t m3_debounce_task_t;

/**
 * \brief Callback function type for state updates.
 * \param node The UI node that needs updating.
 * \param state_variable The new value of the state.
 * \param user_data Optional user context.
 */
typedef void (*m3_state_update_cb)(cmp_ui_node_t *node, void *state_variable, void *user_data);

/**
 * \struct m3_state_dep_t
 * \brief Represents a dependency binding between a state variable and a UI node.
 */
struct m3_state_dep_t {
    cmp_ui_node_t *node;
    void *state_variable;
    m3_state_update_cb update_cb;
    void *user_data;
    m3_state_dep_t *next;
};

/**
 * \struct m3_debounce_task_t
 * \brief Represents a delayed task that can be reset if called repeatedly.
 */
struct m3_debounce_task_t {
    int id;
    uint32_t target_time_ms;
    void (*callback)(material_catalog_state_t *state, void *user_data);
    void *user_data;
    m3_debounce_task_t *next;
};

/**
 * \struct m3_state_engine_t
 * \brief Global state engine context for managing bindings and debouncing.
 */
typedef struct m3_state_engine_t {
    m3_state_dep_t *dependencies;
    m3_debounce_task_t *debounce_queue;
} m3_state_engine_t;

/**
 * \brief Initializes the state and reactivity engine.
 * \param engine Pointer to the state engine structure.
 * \return 0 on success, non-zero on error.
 */
int m3_state_engine_init(m3_state_engine_t *engine);

/**
 * \brief Cleans up the state engine, freeing associated memory.
 * \param engine Pointer to the state engine structure.
 * \return 0 on success, non-zero on error.
 */
int m3_state_engine_cleanup(m3_state_engine_t *engine);

/**
 * \brief Binds a UI node to a specific state variable.
 * \param engine Pointer to the state engine structure.
 * \param state The global material catalog state (used for memory allocation).
 * \param node The UI node to bind.
 * \param state_variable Pointer to the observed state variable.
 * \param update_cb Callback invoked when the state changes.
 * \param user_data Optional context passed to the callback.
 * \return 0 on success, non-zero on error.
 */
int m3_state_bind_node(m3_state_engine_t *engine, material_catalog_state_t *state, cmp_ui_node_t *node, void *state_variable, m3_state_update_cb update_cb, void *user_data);

/**
 * \brief Notifies the engine that a state variable has changed, triggering bound callbacks.
 * \param engine Pointer to the state engine structure.
 * \param state_variable Pointer to the state variable that changed.
 * \return 0 on success, non-zero on error.
 */
int m3_state_notify_changed(m3_state_engine_t *engine, void *state_variable);

/**
 * \brief Invalidates a specific UI node, propagating dirty flags instead of a full clear.
 * \param state Pointer to the material catalog state.
 * \param node The node to invalidate.
 */
void m3_invalidate_ui(material_catalog_state_t *state, cmp_ui_node_t *node);

/**
 * \brief Schedules a debounced task. Subsequent calls with the same ID reset the delay timer.
 * \param engine Pointer to the state engine structure.
 * \param state The global material catalog state.
 * \param id A unique identifier for this debounced task.
 * \param delay_ms The delay in milliseconds before execution.
 * \param callback The function to execute.
 * \param user_data Optional context passed to the callback.
 * \return 0 on success, non-zero on error.
 */
int m3_debounce_task(m3_state_engine_t *engine, material_catalog_state_t *state, int id, uint32_t delay_ms, void (*callback)(material_catalog_state_t *state, void *user_data), void *user_data);

/**
 * \brief Processes the debounce queue. Should be called each frame.
 * \param engine Pointer to the state engine structure.
 * \param state The global material catalog state.
 * \param current_time_ms The current time in milliseconds.
 */
void m3_process_debounce_queue(m3_state_engine_t *engine, material_catalog_state_t *state, uint32_t current_time_ms);

#ifdef __cplusplus
}
#endif

#endif /* M3_STATE_H */
/* clang-format on */