/* clang-format off */
#include "../include/ui_atomic.h"
/* clang-format on */

#if defined(_MSC_VER)

#ifndef UI_WINAPI
#if defined(_MSC_VER)
#define UI_WINAPI __stdcall
#elif defined(__GNUC__)
#define UI_WINAPI __attribute__((stdcall))
#else
#define UI_WINAPI
#endif
#endif

extern long UI_WINAPI _InterlockedExchangeAdd(long volatile *, long);
extern long UI_WINAPI _InterlockedCompareExchange(long volatile *, long, long);
extern void *UI_WINAPI _InterlockedCompareExchangePointer(void *volatile *,
                                                          void *, void *);
extern long UI_WINAPI _InterlockedExchange(long volatile *, long);

#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedCompareExchangePointer)
#pragma intrinsic(_InterlockedExchange)

enum ui_error ui_atomic_add(ui_atomic_t *target, long value,
                            long *out_old_value) {
  if (!target || !out_old_value)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_old_value = _InterlockedExchangeAdd((volatile long *)target, value);
  return UI_ERROR_NONE;
}

enum ui_error ui_atomic_cas(ui_atomic_t *target, long expected, long new_value,
                            int *out_swapped) {
  if (!target || !out_swapped)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_swapped = _InterlockedCompareExchange((volatile long *)target, new_value,
                                             expected) == expected
                     ? 1
                     : 0;
  return UI_ERROR_NONE;
}

enum ui_error ui_atomic_ptr_cas(void *volatile *target, void *expected,
                                void *new_value, int *out_swapped) {
  if (!target || !out_swapped)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_swapped = _InterlockedCompareExchangePointer(target, new_value,
                                                    expected) == expected
                     ? 1
                     : 0;
  return UI_ERROR_NONE;
}

enum ui_error ui_atomic_load(ui_atomic_t *target, long *out_value) {
  if (!target || !out_value)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_value = _InterlockedCompareExchange((volatile long *)target, 0, 0);
  return UI_ERROR_NONE;
}

enum ui_error ui_atomic_store(ui_atomic_t *target, long value) {
  if (!target)
    return UI_ERROR_INVALID_ARGUMENT;
  _InterlockedExchange((volatile long *)target, value);
  return UI_ERROR_NONE;
}

#elif defined(__GNUC__) || defined(__clang__)

enum ui_error ui_atomic_add(ui_atomic_t *target, long value,
                            long *out_old_value) {
  if (!target)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!out_old_value)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_old_value = __sync_fetch_and_add(target, value);
  return UI_ERROR_NONE;
}

enum ui_error ui_atomic_cas(ui_atomic_t *target, long expected, long new_value,
                            int *out_swapped) {
  if (!target)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!out_swapped)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_swapped =
      __sync_bool_compare_and_swap(target, expected, new_value) ? 1 : 0;
  return UI_ERROR_NONE;
}

enum ui_error ui_atomic_ptr_cas(void *volatile *target, void *expected,
                                void *new_value, int *out_swapped) {
  if (!target)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!out_swapped)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_swapped =
      __sync_bool_compare_and_swap(target, expected, new_value) ? 1 : 0;
  return UI_ERROR_NONE;
}

enum ui_error ui_atomic_load(ui_atomic_t *target, long *out_value) {
  if (!target)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!out_value)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_value = __sync_val_compare_and_swap(target, 0, 0);
  return UI_ERROR_NONE;
}

enum ui_error ui_atomic_store(ui_atomic_t *target, long value) {
  if (!target)
    return UI_ERROR_INVALID_ARGUMENT;
  __sync_lock_test_and_set(target, value);
  return UI_ERROR_NONE;
}

#else
#error "No atomic operations implemented for this compiler."
#endif
