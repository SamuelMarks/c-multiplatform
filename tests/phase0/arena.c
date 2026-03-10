/* clang-format off */
#include "cmpc/cmp_arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static void test_arena_basic(void) {
  CMPArena arena;
  void *ptr1, *ptr2, *ptr3;
  CMPArenaStats stats;

  memset(&arena, 0, sizeof(arena));

  if (cmp_arena_init(&arena, NULL, 1024) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };

  if (cmp_arena_alloc(&arena, 100, 8, &ptr1) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  if (cmp_arena_alloc(&arena, 200, 16, &ptr2) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };

  if (cmp_arena_get_stats(&arena, &stats) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  if (stats.block_count != 1) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };

  if (cmp_arena_reset(&arena) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };

  if (cmp_arena_alloc(&arena, 2048, 8, &ptr3) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };

  if (cmp_arena_get_stats(&arena, &stats) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  if (stats.block_count != 2) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };

  if (cmp_arena_shutdown(&arena) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
}

static void test_arena_edge_cases(void) {
  CMPArena arena;
  void *ptr;
  CMPArenaStats stats;

  memset(&arena, 0, sizeof(arena));

  if (cmp_arena_init(NULL, NULL, 1024) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  if (cmp_arena_init(&arena, NULL, 1024) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };

  if (cmp_arena_alloc(NULL, 100, 8, &ptr) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  if (cmp_arena_alloc(&arena, 100, 8, NULL) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  if (cmp_arena_alloc(&arena, 0, 8, &ptr) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  if (cmp_arena_alloc(&arena, 100, 3, &ptr) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  }; /* Invalid alignment */

  if (cmp_arena_reset(NULL) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  if (cmp_arena_shutdown(NULL) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };

  if (cmp_arena_get_stats(NULL, &stats) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  if (cmp_arena_get_stats(&arena, NULL) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };

  if (cmp_arena_shutdown(&arena) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
}

static void test_arena_testing_hooks(void) {
  CMPArena arena;
  void *ptr;

  memset(&arena, 0, sizeof(arena));

  if (cmp_arena_init(&arena, NULL, 1024) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };

  cmp_arena_test_set_force_align_fail(1);
  if (cmp_arena_alloc(&arena, 100, 8, &ptr) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  cmp_arena_test_set_force_align_fail(0);

  cmp_arena_test_set_force_align_fail(2);
  /* This needs to trigger a new block allocation. 100 is less than 1024, so it
     succeeds first check. But if we ask for 2000, the first check succeeds
     (offset=0, capacity=1024), then remaining < size, so it allocates a new
     block. Then the second check should fail! */
  if (cmp_arena_alloc(&arena, 2000, 8, &ptr) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  cmp_arena_test_set_force_align_fail(0);

  cmp_arena_test_set_force_align_fail(3);
  if (cmp_arena_alloc(&arena, 2000, 8, &ptr) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  cmp_arena_test_set_force_align_fail(0);

  cmp_arena_test_set_force_add_overflow(CMP_TRUE);
  if (cmp_arena_alloc(&arena, 100, 8, &ptr) == CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
  cmp_arena_test_set_force_add_overflow(CMP_FALSE);

  if (cmp_arena_shutdown(&arena) != CMP_OK) {
    fprintf(stderr, "Failed at line %d\n", __LINE__);
    exit(1);
  };
}

int main(void) {
  test_arena_basic();
  test_arena_edge_cases();
  test_arena_testing_hooks();
  fprintf(stderr, "arena tests passed\n");
  return 0;
}
