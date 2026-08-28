/**
 * @file ui_atomic.h
 * @brief Atomic operations and definitions.
 */
#ifndef UI_ATOMIC_H
#define UI_ATOMIC_H

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Volatile integer type for atomic operations.
 */
typedef volatile long ui_atomic_t;

/**
 * @brief Atomically adds a value.
 *
 * @param target Pointer to the atomic variable.
 * @param value The value to add.
 * @param out_old_value Pointer to receive the original value before addition.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_atomic_add(ui_atomic_t *target, long value, long *out_old_value);

/**
 * @brief Atomically compares and swaps.
 *
 * @param target Pointer to the atomic variable.
 * @param expected The expected current value.
 * @param new_value The new value to set.
 * @param out_swapped Pointer to receive 1 if successful, 0 otherwise.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_atomic_cas(ui_atomic_t *target, long expected, long new_value,
                         int *out_swapped);

/**
 * @brief Atomically loads a value with a full memory barrier.
 *
 * @param target Pointer to the atomic variable.
 * @param out_value Pointer to receive the current value.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_atomic_load(ui_atomic_t *target, long *out_value);

/**
 * @brief Atomically stores a value with a full memory barrier.
 *
 * @param target Pointer to the atomic variable.
 * @param value The value to store.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_atomic_store(ui_atomic_t *target, long value);

/**
 * @brief Atomically compares and swaps a pointer.
 *
 * @param target Pointer to the volatile pointer to swap.
 * @param expected The expected current pointer.
 * @param new_value The new pointer to set.
 * @param out_swapped Pointer to receive 1 if successful, 0 otherwise.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_atomic_ptr_cas(void *volatile *target, void *expected,
                             void *new_value, int *out_swapped);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ATOMIC_H */
