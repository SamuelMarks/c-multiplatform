#ifndef CMP_ATOMIC_H
#define CMP_ATOMIC_H

/**
 * @file cmp_atomic.h
 * @brief Cross-platform atomic operations for LibCMPC.
 *
 * Provides C89 compatible macros/inline functions for basic atomics,
 * using compiler intrinsics or Windows Interlocked functions.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#endif
/* clang-format on */

/**
 * @brief Atomically increment a 32-bit integer.
 * @param ptr Pointer to the integer to increment.
 * @return The new value after incrementing.
 */
static CMP_INLINE cmp_u32 cmp_atomic_inc_u32(volatile cmp_u32 *ptr) {
#if defined(_WIN32)
  return (cmp_u32)InterlockedIncrement((volatile LONG *)ptr);
#elif defined(__GNUC__) || defined(__clang__)
  return __sync_add_and_fetch(ptr, 1);
#else
#error "Unsupported compiler for atomic increment."
#endif
}

/**
 * @brief Atomically decrement a 32-bit integer.
 * @param ptr Pointer to the integer to decrement.
 * @return The new value after decrementing.
 */
static CMP_INLINE cmp_u32 cmp_atomic_dec_u32(volatile cmp_u32 *ptr) {
#if defined(_WIN32)
  return (cmp_u32)InterlockedDecrement((volatile LONG *)ptr);
#elif defined(__GNUC__) || defined(__clang__)
  return __sync_sub_and_fetch(ptr, 1);
#else
#error "Unsupported compiler for atomic decrement."
#endif
}

/**
 * @brief Atomically compare and swap a 32-bit integer.
 * @param ptr Pointer to the integer.
 * @param expected The expected value.
 * @param new_value The new value to set if the current value equals expected.
 * @return The initial value of the integer. If it equals `expected`, the swap
 * succeeded.
 */
static CMP_INLINE cmp_u32 cmp_atomic_cas_u32(volatile cmp_u32 *ptr,
                                             cmp_u32 expected,
                                             cmp_u32 new_value) {
#if defined(_WIN32)
  return (cmp_u32)InterlockedCompareExchange((volatile LONG *)ptr,
                                             (LONG)new_value, (LONG)expected);
#elif defined(__GNUC__) || defined(__clang__)
  return __sync_val_compare_and_swap(ptr, expected, new_value);
#else
#error "Unsupported compiler for atomic CAS."
#endif
}

/**
 * @brief Atomically exchange a pointer value.
 * @param ptr Pointer to the pointer to exchange.
 * @param new_value The new pointer value.
 * @return The old pointer value.
 */
static CMP_INLINE void *cmp_atomic_exchange_ptr(void *volatile *ptr,
                                                void *new_value) {
#if defined(_WIN32)
  return InterlockedExchangePointer(ptr, new_value);
#elif defined(__GNUC__) || defined(__clang__)
  return __sync_lock_test_and_set(ptr, new_value);
#else
#error "Unsupported compiler for atomic pointer exchange."
#endif
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_ATOMIC_H */