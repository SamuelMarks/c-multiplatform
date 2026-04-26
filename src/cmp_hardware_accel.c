/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
/* clang-format on */

/**
 * @brief Verifies hardware acceleration backend availability.
 *
 * @param out_is_cpu_fallback Pointer to an integer which receives 1 if the
 *        CPU software fallback is used, or 0 otherwise.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_verify_hardware_acceleration(int *out_is_cpu_fallback) {
  int rc = 0; /* CMP_SUCCESS */
  cmp_gpu_t *gpu = NULL;

  if (!out_is_cpu_fallback) {
    LOG_DEBUG("cmp_verify_hardware_acceleration: invalid argument (out_is_cpu_fallback is NULL)\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_is_cpu_fallback = 0;

  /* Attempt to create the preferred GPU backend (let cmp choose best available) */
  rc = cmp_gpu_create((cmp_gpu_backend_type_t)0, &gpu);
  if (rc != 0 || !gpu) {
    LOG_DEBUG("cmp_verify_hardware_acceleration: Failed to initialize any rendering backend\n");
    /* We can't use Safe CRT for printf to stdout here easily, printf is fine */
    printf("[Hardware] FATAL: Failed to initialize any rendering backend.\n");
    return rc != 0 ? rc : CMP_ERROR_NOT_FOUND;
  }

  /* Verify if the resolved backend is the legacy CPU software rasterizer */
  if (gpu->backend == CMP_BACKEND_CPU_SOFTWARE) {
    *out_is_cpu_fallback = 1;
    printf("[Hardware] WARNING: Hardware Acceleration UNAVAILABLE. Active backend: CPU Software Fallback.\n");
  } else {
    printf("[Hardware] SUCCESS: Hardware Acceleration ACTIVE. Active backend ID: %d\n", (int)gpu->backend);
  }

  /* Destroy the context since this is just a verification pass */
  rc = cmp_gpu_destroy(gpu);
  if (rc != 0) {
    LOG_DEBUG("cmp_verify_hardware_acceleration: Failed to destroy gpu context\n");
  }

  return 0;
}
