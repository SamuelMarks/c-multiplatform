#ifndef UI_ENGINE_H
#define UI_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Opaque handle representing the root UI engine context.
 */
struct ui_engine;

/**
 * @brief Configuration struct for initializing the engine.
 */
struct ui_engine_config {
  int num_threads; /**< Number of threads for the background pool. 0 for
                      single-threaded fallback. */
};

/**
 * @brief Initializes the UI engine and returns the root context handle.
 *
 * @param config Pointer to the configuration struct.
 * @param out_engine Pointer to receive the initialized engine handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_engine_create(const struct ui_engine_config *config,
                               struct ui_engine **out_engine);

/**
 * @brief Destroys the UI engine and frees all globally allocated resources.
 *
 * @param engine The engine handle to destroy.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_engine_destroy(struct ui_engine *engine);

/**
 * @brief Runs a single iteration of the engine's main loop.
 *
 * @param engine The engine handle.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_engine_tick(struct ui_engine *engine);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ENGINE_H */
