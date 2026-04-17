/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
/* clang-format on */

int cmp_verify_hardware_acceleration(void) {
  int rc = CMP_SUCCESS;
  cmp_gpu_t *gpu = NULL;
  int is_cpu_fallback = 0;

  /* Attempt to create the preferred GPU backend (let cmp choose best available)
   */
  rc = cmp_gpu_create((cmp_gpu_backend_type_t)0, &gpu);
  if (rc != CMP_SUCCESS || !gpu) {
    LOG_DEBUG("Error in cmp_verify_hardware_acceleration: Failed to initialize "
              "any rendering backend\n");
    printf("[Hardware] FATAL: Failed to initialize any rendering backend.\n");
    return rc;
  }

  /* Verify if the resolved backend is the legacy CPU software rasterizer */
  if (gpu->backend == CMP_BACKEND_CPU_SOFTWARE) {
    is_cpu_fallback = 1;
    printf("[Hardware] WARNING: Hardware Acceleration UNAVAILABLE. Active "
           "backend: CPU Software Fallback.\n");
  } else {
    printf("[Hardware] SUCCESS: Hardware Acceleration ACTIVE. Active backend "
           "ID: %d\n",
           (int)gpu->backend);
  }

  /* Destroy the context since this is just a verification pass */
  rc = cmp_gpu_destroy(gpu);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_verify_hardware_acceleration: Failed to destroy "
              "gpu context\n");
  }

  return is_cpu_fallback;
}
