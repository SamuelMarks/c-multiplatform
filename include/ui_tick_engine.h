#ifndef UI_TICK_ENGINE_H
#define UI_TICK_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Opaque handle representing a single-threaded tick engine.
 */
struct ui_tick_engine;

/**
 * @brief Creates a new single-threaded tick engine.
 *
 * @param out_engine Pointer to receive the new engine handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_tick_engine_create(struct ui_tick_engine **out_engine);

/**
 * @brief Destroys a tick engine and frees its resources.
 *
 * @param engine The engine to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if engine is
 * NULL.
 */
ui_error_t ui_tick_engine_destroy(struct ui_tick_engine *engine);

/**
 * @brief Schedules a task for execution on the next tick.
 *
 * @param engine The engine to schedule the task on.
 * @param callback The function to execute.
 * @param user_data Opaque pointer passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_tick_engine_schedule(struct ui_tick_engine *engine,
                                   ui_error_t (*callback)(void *),
                                   void *user_data);

/**
 * @brief Processes all scheduled tasks in the queue.
 *
 * @param engine The engine to tick.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tick_engine_tick(struct ui_tick_engine *engine);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TICK_ENGINE_H */
