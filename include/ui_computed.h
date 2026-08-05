#ifndef UI_COMPUTED_H
#define UI_COMPUTED_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_arena.h"
#include "ui_signal.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle to a computed signal.
 */
typedef struct ui_computed ui_computed_t;

/**
 * @brief Function pointer for computing a new value based on dependencies.
 */
typedef ui_error_t (*ui_compute_fn)(void *user_data,
                                    union ui_signal_payload *out_value);

/**
 * @brief Creates a new computed signal.
 *
 * @param arena The arena to allocate from.
 * @param compute_fn The function that computes the value.
 * @param user_data User data passed to the compute function.
 * @param type The type of the signal payload.
 * @param mode The signal mode.
 * @param out_computed The pointer to store the created computed signal.
 * @return ui_error_t
 */
ui_error_t ui_computed_create(struct ui_arena *arena, ui_compute_fn compute_fn,
                              void *user_data, enum ui_signal_type type,
                              enum ui_signal_mode mode,
                              ui_computed_t **out_computed);

/**
 * @brief Gets the current value of the computed signal.
 *
 * @param computed The computed signal.
 * @param out_value The pointer to store the current value.
 * @return ui_error_t
 */
ui_error_t ui_computed_get(ui_computed_t *computed,
                           union ui_signal_payload *out_value);

/**
 * @brief Destroys the computed signal.
 *
 * @param computed The computed signal.
 * @return ui_error_t
 */
ui_error_t ui_computed_destroy(ui_computed_t *computed);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_COMPUTED_H */
