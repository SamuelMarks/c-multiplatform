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
 * @return The original value before addition.
 */
enum ui_error ui_atomic_add(ui_atomic_t *target, long value,
                            long *out_old_value);

/**
 * @brief Atomically compares and swaps.
 *
 * @param target Pointer to the atomic variable.
 * @param expected The expected current value.
 * @param new_value The new value to set.
 * @return 1 if successful, 0 otherwise.
 */
enum ui_error ui_atomic_cas(ui_atomic_t *target, long expected, long new_value,
                            int *out_swapped);

/**
 * @brief Atomically loads a value with a full memory barrier.
 *
 * @param target Pointer to the atomic variable.
 * @return The current value.
 */
enum ui_error ui_atomic_load(ui_atomic_t *target, long *out_value);

/**
 * @brief Atomically stores a value with a full memory barrier.
 *
 * @param target Pointer to the atomic variable.
 * @param value The value to store.
 * @return enum ui_error
 */
enum ui_error ui_atomic_store(ui_atomic_t *target, long value);

enum ui_error ui_atomic_ptr_cas(void *volatile *target, void *expected,
                                void *new_value, int *out_actual);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ATOMIC_H */
