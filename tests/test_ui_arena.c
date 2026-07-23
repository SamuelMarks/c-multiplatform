/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "../include/ui_arena.h"
#include "../include/ui_types.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

int main(void) {
  struct ui_arena *arena = NULL;
  enum ui_error rc;
  void *ptr1 = NULL;
  void *ptr2 = NULL;
  void *ptr3 = NULL;
  int i;
  struct ui_arena_savepoint sps[100];

  /* Test NULL / invalid arguments for ui_arena_create */
  rc = ui_arena_create(0, &arena);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject 0 block size\n");
    return 1;
  }
  rc = ui_arena_create(1024, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject NULL out_arena\n");
    return 1;
  }

  /* Test out of memory on arena create */
  g_malloc_fail_countdown = 0;
  rc = ui_arena_create(1024, &arena);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to return OUT_OF_MEMORY on create\n");
    return 1;
  }
  g_malloc_fail_countdown = -1; /* reset */

  /* Test NULL / invalid arguments for other functions */
  rc = ui_arena_destroy(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject NULL arena on destroy\n");
    return 1;
  }

  rc = ui_arena_reset(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject NULL arena on reset\n");
    return 1;
  }

  rc = ui_arena_alloc(NULL, 100, 8, &ptr1);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject NULL arena on alloc\n");
    return 1;
  }

  rc = ui_arena_create(1024, &arena);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create arena\n");
    return 1;
  }

  /* Empty savepoint test */
  {
    struct ui_arena_savepoint empty_sp;
    if (ui_arena_save(NULL, &empty_sp) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_arena_save(arena, NULL) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    rc = ui_arena_save(arena, &empty_sp);
    if (rc != UI_ERROR_NONE || empty_sp.used != 0 || empty_sp.block != NULL)
      return 1;

    if (ui_arena_restore(NULL, empty_sp) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    rc = ui_arena_restore(arena, empty_sp);
    if (rc != UI_ERROR_NONE)
      return 1;
  }

  rc = ui_arena_alloc(arena, 100, 8, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject NULL out_ptr on alloc\n");
    return 1;
  }

  rc = ui_arena_alloc(arena, 100, 0, &ptr1);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject 0 alignment on alloc\n");
    return 1;
  }

  rc = ui_arena_alloc(arena, 100, 7, &ptr1);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed to reject non-power-of-2 alignment on alloc\n");
    return 1;
  }

  /* Test 4-byte alignment */
  rc = ui_arena_alloc(arena, 100, 4, &ptr1);
  if (rc != UI_ERROR_NONE || !ptr1) {
    printf("Failed to allocate ptr1 (4-byte alignment)\n");
    return 1;
  }
  if ((ui_uintptr)ptr1 % 4 != 0) {
    printf("ptr1 not aligned to 4\n");
    return 1;
  }

  /* Test 8-byte alignment */
  rc = ui_arena_alloc(arena, 100, 8, &ptr1);
  if (rc != UI_ERROR_NONE || !ptr1) {
    printf("Failed to allocate ptr1 (8-byte alignment)\n");
    return 1;
  }
  if ((ui_uintptr)ptr1 % 8 != 0) {
    printf("ptr1 not aligned to 8\n");
    return 1;
  }

  /* Test 16-byte alignment */
  rc = ui_arena_alloc(arena, 200, 16, &ptr2);
  if (rc != UI_ERROR_NONE || !ptr2) {
    printf("Failed to allocate ptr2 (16-byte alignment)\n");
    return 1;
  }
  if ((ui_uintptr)ptr2 % 16 != 0) {
    printf("ptr2 not aligned to 16\n");
    return 1;
  }

  /* Force a new block allocation */
  rc = ui_arena_alloc(arena, 2048, 32, &ptr3);
  if (rc != UI_ERROR_NONE || !ptr3) {
    printf("Failed to allocate ptr3 (large block)\n");
    return 1;
  }

  if ((ui_uintptr)ptr3 % 32 != 0) {
    printf("ptr3 not aligned to 32\n");
    return 1;
  }

  /* Test out of memory on block allocation */
  g_malloc_fail_countdown = 0;
  rc = ui_arena_alloc(arena, 4096, 32, &ptr3);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to return OUT_OF_MEMORY on block alloc\n");
    return 1;
  }
  g_malloc_fail_countdown = -1; /* reset */

  /* Test memory zeroing in debug mode (assumed NDEBUG not defined) */
  rc = ui_arena_alloc(arena, 128, 8, &ptr1);
  if (rc == UI_ERROR_NONE) {
    memset(ptr1, 0xFF, 128); /* write non-zero data */
    ui_arena_reset(arena);
    rc = ui_arena_alloc(arena, 128, 8, &ptr2);
#ifndef NDEBUG
    if (rc == UI_ERROR_NONE) {
      char *bytes = (char *)ptr2;
      for (i = 0; i < 128; i++) {
        if (bytes[i] != 0) {
          printf("Memory was not zeroed on reset!\n");
          return 1;
        }
      }
    }
#endif
  }

  /* Test nested save/restore 100 levels deep */
  ui_arena_reset(arena);
  for (i = 0; i < 100; i++) {
    rc = ui_arena_save(arena, &sps[i]);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to save arena at depth %d\n", i);
      return 1;
    }
    rc = ui_arena_alloc(arena, 16, 8, &ptr1);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to alloc arena at depth %d\n", i);
      return 1;
    }
  }

  for (i = 99; i >= 0; i--) {
    rc = ui_arena_restore(arena, sps[i]);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to restore arena at depth %d\n", i);
      return 1;
    }
  }

  ui_arena_destroy(arena);

  printf("All arena tests passed.\n");
  return 0;
}
