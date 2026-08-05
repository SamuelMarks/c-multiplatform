#ifndef UI_SIGNAL_H
#define UI_SIGNAL_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_arena.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Execution modalities for signals.
 */
enum ui_signal_mode {
  UI_SIGNAL_MODE_SINGLE_THREADED = 0,
  UI_SIGNAL_MODE_MULTI_THREADED = 1
};

/**
 * @brief Types of values a signal can hold.
 */
enum ui_signal_type {
  UI_SIGNAL_TYPE_POINTER = 0,
  UI_SIGNAL_TYPE_INT32 = 1,
  UI_SIGNAL_TYPE_FLOAT32 = 2,
  UI_SIGNAL_TYPE_BOOL = 3
};

/**
 * @brief Tagged union payload avoiding MSVC 2005 alignment quirks.
 * Ensure it uses standard C89 types without complex nested anonymous
 * unions/structs.
 */
union ui_signal_payload {
  void *ptr_val;
  ui_int32 int_val;
  float float_val;
  ui_bool_t bool_val;
};

/**
 * @brief Opaque handle to a signal.
 */
typedef struct ui_signal ui_signal_t;

/**
 * @brief Function pointer for equality check.
 * Must set *out_equal to UI_TRUE or UI_FALSE.
 */
typedef ui_error_t (*ui_equality_fn)(union ui_signal_payload a,
                                     union ui_signal_payload b,
                                     ui_bool_t *out_equal);

/**
 * @brief Function pointer for destructor.
 */
typedef ui_error_t (*ui_destructor_fn)(union ui_signal_payload payload);

/**
 * @brief Function pointer for update.
 */
typedef ui_error_t (*ui_update_fn)(union ui_signal_payload current_value,
                                   union ui_signal_payload *out_value);

/**
 * @brief Creates a new signal.
 *
 * @param arena The arena to allocate from.
 * @param initial_value The initial value of the signal.
 * @param type The type of the signal payload.
 * @param equality_fn The equality function.
 * @param destructor_fn The destructor function.
 * @param mode The signal mode.
 * @param out_signal The pointer to store the created signal.
 * @return ui_error_t
 */
ui_error_t ui_signal_create(struct ui_arena *arena,
                            union ui_signal_payload initial_value,
                            enum ui_signal_type type,
                            ui_equality_fn equality_fn,
                            ui_destructor_fn destructor_fn,
                            enum ui_signal_mode mode, ui_signal_t **out_signal);

/**
 * @brief Gets the current value of the signal.
 *
 * @param signal The signal.
 * @param out_value The pointer to store the current value.
 * @return ui_error_t
 */
ui_error_t ui_signal_get(ui_signal_t *signal,
                         union ui_signal_payload *out_value);

/**
 * @brief Sets the value of the signal.
 *
 * @param signal The signal.
 * @param new_value The new value to set.
 * @return ui_error_t
 */
ui_error_t ui_signal_set(ui_signal_t *signal,
                         union ui_signal_payload new_value);

/**
 * @brief Updates the value of the signal using an update function.
 *
 * @param signal The signal.
 * @param update_fn The update function.
 * @return ui_error_t
 */
ui_error_t ui_signal_update(ui_signal_t *signal, ui_update_fn update_fn);

/**
 * @brief Destroys the signal.
 *
 * @param signal The signal.
 * @return ui_error_t
 */
ui_error_t ui_signal_destroy(ui_signal_t *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SIGNAL_H */
