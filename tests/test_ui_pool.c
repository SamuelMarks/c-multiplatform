/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include "../include/ui_pool.h"
#include "../include/ui_types.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

int main(void) {
  struct ui_pool *pool = NULL;
  ui_error_t rc;
  size_t free_count = 0;
  size_t total_capacity = 0;
  void *ptr1 = NULL;
  void *ptr2 = NULL;
  void *ptr3 = NULL;
  void *ptr1_recycled = NULL;
  void *ptrs[100];
  int i;
  size_t sizes[] = {16, 32, 64, 128};
  int size_idx;

  /* Test NULL / invalid arguments for ui_pool_create */
  rc = ui_pool_create(0, 10, &pool);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject 0 element size\n");
    return 1;
  }
  rc = ui_pool_create(16, 0, &pool);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject 0 chunk capacity\n");
    return 1;
  }
  rc = ui_pool_create(16, 10, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject NULL out_pool\n");
    return 1;
  }

  /* Test pool initialization with variable element sizes */
  for (size_idx = 0; size_idx < 4; size_idx++) {
    rc = ui_pool_create(sizes[size_idx], 10, &pool);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to create pool with size %lu\n",
             (unsigned long)sizes[size_idx]);
      return 1;
    }
    {
      ui_error_t rc_cleanup = ui_pool_destroy(pool);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  /* Test OOM on create */
  g_malloc_fail_countdown = 0;
  rc = ui_pool_create(16, 10, &pool);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to handle OOM on create\n");
    return 1;
  }
  g_malloc_fail_countdown = -1;

  rc = ui_pool_create(16, 2, &pool);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create pool\n");
    return 1;
  }

  /* Initially no chunks allocated, capacity is 0 */
  rc = ui_pool_get_metrics(pool, &free_count, &total_capacity);
  if (rc != UI_ERROR_NONE || free_count != 0 || total_capacity != 0) {
    printf("Initial metrics incorrect: %lu, %lu\n", (unsigned long)free_count,
           (unsigned long)total_capacity);
    return 1;
  }

  /* Test alloc (triggers first chunk allocation) */
  rc = ui_pool_alloc(pool, &ptr1);
  if (rc != UI_ERROR_NONE || !ptr1) {
    printf("Failed to alloc ptr1\n");
    return 1;
  }

  {
    ui_error_t rc_cleanup =
        ui_pool_get_metrics(pool, &free_count, &total_capacity);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (free_count != 1 || total_capacity != 2) {
    printf("Metrics incorrect after alloc: %lu, %lu\n",
           (unsigned long)free_count, (unsigned long)total_capacity);
    return 1;
  }

  /* Exhaust first chunk */
  rc = ui_pool_alloc(pool, &ptr2);
  if (rc != UI_ERROR_NONE || !ptr2) {
    printf("Failed to alloc ptr2\n");
    return 1;
  }

  {
    ui_error_t rc_cleanup =
        ui_pool_get_metrics(pool, &free_count, &total_capacity);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (free_count != 0 || total_capacity != 2) {
    printf("Metrics incorrect after exhaustion: %lu, %lu\n",
           (unsigned long)free_count, (unsigned long)total_capacity);
    return 1;
  }

  /* Dynamic chunk allocation when capacity is exceeded */
  rc = ui_pool_alloc(pool, &ptr3);
  if (rc != UI_ERROR_NONE || !ptr3) {
    printf("Failed to alloc ptr3 (dynamic chunk)\n");
    return 1;
  }

  {
    ui_error_t rc_cleanup =
        ui_pool_get_metrics(pool, &free_count, &total_capacity);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (free_count != 1 || total_capacity != 4) {
    printf("Metrics incorrect after dynamic chunk alloc: %lu, %lu\n",
           (unsigned long)free_count, (unsigned long)total_capacity);
    return 1;
  }

  /* Object recycling */
  rc = ui_pool_free(pool, ptr1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to free ptr1\n");
    return 1;
  }

  {
    ui_error_t rc_cleanup =
        ui_pool_get_metrics(pool, &free_count, &total_capacity);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (free_count != 2 || total_capacity != 4) {
    printf("Metrics incorrect after free: %lu, %lu\n",
           (unsigned long)free_count, (unsigned long)total_capacity);
    return 1;
  }

  rc = ui_pool_alloc(pool, &ptr1_recycled);
  if (rc != UI_ERROR_NONE || ptr1_recycled != ptr1) {
    printf("Object not recycled properly. Expected %p, got %p\n", ptr1,
           ptr1_recycled);
    return 1;
  }

  {
    ui_error_t rc_cleanup = ui_pool_destroy(pool);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Fragmentation metrics: Verify pool does not permanently fragment on random
   * frees */
  rc = ui_pool_create(32, 10, &pool);
  for (i = 0; i < 50; i++) {
    {
      ui_error_t rc_cleanup = ui_pool_alloc(pool, &ptrs[i]);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_pool_get_metrics(pool, &free_count, &total_capacity);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (free_count != 0 || total_capacity != 50) {
    printf("Metrics incorrect after 50 allocs: %lu, %lu\n",
           (unsigned long)free_count, (unsigned long)total_capacity);
    return 1;
  }

  /* Free every other element */
  for (i = 0; i < 50; i += 2) {
    {
      ui_error_t rc_cleanup = ui_pool_free(pool, ptrs[i]);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_pool_get_metrics(pool, &free_count, &total_capacity);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (free_count != 25 || total_capacity != 50) {
    printf("Metrics incorrect after freeing half: %lu, %lu\n",
           (unsigned long)free_count, (unsigned long)total_capacity);
    return 1;
  }

  /* Alloc 25 more elements, capacity should not increase */
  for (i = 0; i < 50; i += 2) {
    {
      ui_error_t rc_cleanup = ui_pool_alloc(pool, &ptrs[i]);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_pool_get_metrics(pool, &free_count, &total_capacity);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (free_count != 0 || total_capacity != 50) {
    printf("Metrics incorrect after re-allocating 25: %lu, %lu\n",
           (unsigned long)free_count, (unsigned long)total_capacity);
    return 1;
  }

  {
    ui_error_t rc_cleanup = ui_pool_destroy(pool);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Test NULL passing */
  if (ui_pool_alloc(NULL, &ptr1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    ui_error_t rc_cleanup = ui_pool_create(16, 10, &pool);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (ui_pool_alloc(pool, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pool_free(NULL, ptr1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pool_free(pool, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pool_get_metrics(NULL, &free_count, &total_capacity) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pool_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Missing branches for get_metrics */
  if (ui_pool_get_metrics(pool, NULL, NULL) != UI_ERROR_NONE)
    return 1;
  if (ui_pool_get_metrics(pool, &free_count, NULL) != UI_ERROR_NONE)
    return 1;
  if (ui_pool_get_metrics(pool, NULL, &total_capacity) != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_pool_destroy(pool);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Test OOM on dynamic chunk alloc */
  {
    ui_error_t rc_cleanup = ui_pool_create(16, 2, &pool);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_pool_alloc(pool, &ptr1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_pool_alloc(pool, &ptr2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_malloc_fail_countdown = 0;
  if (ui_pool_alloc(pool, &ptr3) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;
  {
    ui_error_t rc_cleanup = ui_pool_destroy(pool);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Test pool element size auto-alignment */
  rc = ui_pool_create(1, 10, &pool); /* Will force element size up to at least
                                        sizeof(ui_pool_free_node) */
  if (rc != UI_ERROR_NONE)
    return 1;
  {
    ui_error_t rc_cleanup = ui_pool_destroy(pool);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  rc = ui_pool_create(sizeof(void *) + 1, 10,
                      &pool); /* Will force alignment to pointer size */
  if (rc != UI_ERROR_NONE)
    return 1;
  {
    ui_error_t rc_cleanup = ui_pool_destroy(pool);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  printf("All pool tests passed.\n");
  return 0;
}
