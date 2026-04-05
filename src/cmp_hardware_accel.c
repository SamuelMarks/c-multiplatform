/* clang-format off */
#include "cmp.h"
#include "cmp.h"
#include <stdio.h>
/* clang-format on */

int cmp_verify_hardware_acceleration(void) {
  cmp_gpu_t *gpu = NULL;
  int is_cpu_fallback = 0;
  int result;

  /* Attempt to create the preferred GPU backend (let cmp choose best available)
   */
  result = cmp_gpu_create((cmp_gpu_backend_type_t)0, &gpu);
  if (result != CMP_SUCCESS || !gpu) {
    printf("[Hardware] FATAL: Failed to initialize any rendering backend.\n");
    return result;
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
  cmp_gpu_destroy(gpu);

  return is_cpu_fallback;
}
