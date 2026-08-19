/**
 * @file ui_effect.h
 * @brief Side effect management (used by the reactive graph).
 */

#ifndef UI_EFFECT_H
#define UI_EFFECT_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_arena.h"
#include "ui_reactor.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle to an effect.
 */
typedef struct ui_effect ui_effect_t;

/**
 * @brief Function pointer for a side effect.
 */
typedef ui_error_t (*ui_effect_fn)(void *user_data);

/**
 * @brief Creates a new effect.
 *
 * @param arena The arena to allocate from.
 * @param effect_fn The side effect function.
 * @param user_data User data passed to the effect function.
 * @param target_reactor The target reactor for the effect execution.
 * @param out_effect The pointer to store the created effect.
 * @return ui_error_t
 */
ui_error_t ui_effect_create(struct ui_arena *arena, ui_effect_fn effect_fn,
                            void *user_data, struct ui_reactor *target_reactor,
                            ui_effect_t **out_effect);

/**
 * @brief Destroys the effect.
 *
 * @param effect The effect.
 * @return ui_error_t
 */
ui_error_t ui_effect_destroy(ui_effect_t *effect);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_EFFECT_H */
