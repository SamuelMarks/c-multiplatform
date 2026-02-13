#include "cmpc/cmp_arena.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct CMPArenaBlockInternal {
  struct CMPArenaBlockInternal *next;
  cmp_usize capacity;
  cmp_usize offset;
  unsigned char data[1];
} CMPArenaBlockInternal;

typedef struct TestAllocCtx {
  int fail_alloc;
  int fail_free;
  int alloc_calls;
  int free_calls;
} TestAllocCtx;

static int CMP_CALL test_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  TestAllocCtx *state;
  void *mem;

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestAllocCtx *)ctx;
  if (state != NULL && state->fail_alloc) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  mem = malloc((size_t)size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  if (state != NULL) {
    state->alloc_calls += 1;
  }

  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL test_realloc(void *ctx, void *ptr, cmp_usize size,
                                void **out_ptr) {
  TestAllocCtx *state;
  void *mem;

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestAllocCtx *)ctx;
  if (state != NULL && state->fail_alloc) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  mem = realloc(ptr, (size_t)size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  if (state != NULL) {
    state->alloc_calls += 1;
  }

  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL test_free(void *ctx, void *ptr) {
  TestAllocCtx *state;

  state = (TestAllocCtx *)ctx;
  if (state != NULL && state->fail_free) {
    return CMP_ERR_IO;
  }

  free(ptr);

  if (state != NULL) {
    state->free_calls += 1;
  }

  return CMP_OK;
}

int main(void) {
  CMPAllocator bad_alloc;
  CMPArena arena;
  CMPArena arena_uninit;
  CMPArena small_arena;
  CMPArena fail_arena;
  CMPArena oom_arena;
  CMPArena corrupt_arena;
  CMPArena stats_cap_arena;
  CMPArena stats_used_arena;
  CMPArena align_overflow_arena;
  CMPArena shutdown_fail_arena;
  CMPArenaStats stats;
  CMPAllocator test_allocator;
  TestAllocCtx test_ctx;
  void *ptr;
  void *ptr2;
  cmp_usize max_size;
  CMPArenaBlockInternal *block;

  memset(&arena, 0, sizeof(arena));
  memset(&arena_uninit, 0, sizeof(arena_uninit));
  memset(&small_arena, 0, sizeof(small_arena));
  memset(&fail_arena, 0, sizeof(fail_arena));
  memset(&oom_arena, 0, sizeof(oom_arena));
  memset(&corrupt_arena, 0, sizeof(corrupt_arena));
  memset(&stats_cap_arena, 0, sizeof(stats_cap_arena));
  memset(&stats_used_arena, 0, sizeof(stats_used_arena));
  memset(&align_overflow_arena, 0, sizeof(align_overflow_arena));
  memset(&shutdown_fail_arena, 0, sizeof(shutdown_fail_arena));

  ptr = NULL;
  ptr2 = NULL;
  max_size = (cmp_usize) ~(cmp_usize)0;

  CMP_TEST_EXPECT(cmp_arena_init(NULL, NULL, 0), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_arena_reset(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_arena_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);

  memset(&bad_alloc, 0, sizeof(bad_alloc));
  CMP_TEST_EXPECT(cmp_arena_init(&arena, &bad_alloc, 64),
                 CMP_ERR_INVALID_ARGUMENT);

#ifdef CMP_TESTING
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_arena_init(&arena, NULL, 0), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));
#endif

  CMP_TEST_OK(cmp_arena_init(&arena, NULL, 0));
  CMP_TEST_ASSERT(arena.block_size != 0);
  CMP_TEST_EXPECT(cmp_arena_init(&arena, NULL, 0), CMP_ERR_STATE);

  CMP_TEST_EXPECT(cmp_arena_get_stats(NULL, &stats), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_arena_get_stats(&arena, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_arena_get_stats(&arena_uninit, &stats), CMP_ERR_STATE);

  CMP_TEST_OK(cmp_arena_get_stats(&arena, &stats));
  CMP_TEST_ASSERT(stats.block_count == 1);
  CMP_TEST_ASSERT(stats.total_capacity >= arena.block_size);
  CMP_TEST_ASSERT(stats.total_used == 0);

  CMP_TEST_EXPECT(cmp_arena_alloc(NULL, 8, 4, &ptr), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_arena_alloc(&arena, 0, 4, &ptr), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_arena_alloc(&arena, 8, 0, &ptr), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_arena_alloc(&arena, 8, 3, &ptr), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_arena_alloc(&arena, 8, 4, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_arena_alloc(&arena_uninit, 8, 4, &ptr), CMP_ERR_STATE);

  CMP_TEST_OK(cmp_arena_alloc(&arena, 16, 8, &ptr));
  CMP_TEST_ASSERT(ptr != NULL);
  CMP_TEST_ASSERT(((cmp_usize)ptr % 8) == 0);

  CMP_TEST_OK(cmp_arena_shutdown(&arena));

  CMP_TEST_OK(cmp_arena_init(&corrupt_arena, NULL, 16));
  block = (CMPArenaBlockInternal *)corrupt_arena.current;
  block->capacity = 8;
  block->offset = 32;
  CMP_TEST_OK(cmp_arena_alloc(&corrupt_arena, 8, 1, &ptr));
  CMP_TEST_OK(cmp_arena_shutdown(&corrupt_arena));

  CMP_TEST_OK(cmp_arena_init(&align_overflow_arena, NULL, 16));
  block = (CMPArenaBlockInternal *)align_overflow_arena.current;
  block->offset = max_size;
  CMP_TEST_EXPECT(cmp_arena_alloc(&align_overflow_arena, 8, 8, &ptr),
                 CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_arena_shutdown(&align_overflow_arena));

#ifdef CMP_TESTING
  CMP_TEST_OK(cmp_arena_init(&arena, NULL, 16));
  CMP_TEST_OK(cmp_arena_alloc(&arena, 8, 8, &ptr));
  CMP_TEST_OK(cmp_arena_test_set_force_align_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_arena_alloc(&arena, 32, 8, &ptr2), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_arena_test_set_force_align_fail(CMP_FALSE));
  CMP_TEST_OK(cmp_arena_shutdown(&arena));

  CMP_TEST_OK(cmp_arena_init(&arena, NULL, 16));
  CMP_TEST_OK(cmp_arena_test_set_force_add_overflow(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_arena_alloc(&arena, 8, 1, &ptr), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_arena_test_set_force_add_overflow(CMP_FALSE));
  CMP_TEST_OK(cmp_arena_shutdown(&arena));
#endif

  CMP_TEST_OK(cmp_arena_init(&small_arena, NULL, 32));
  CMP_TEST_OK(cmp_arena_alloc(&small_arena, 24, 8, &ptr));
  CMP_TEST_OK(cmp_arena_alloc(&small_arena, 16, 8, &ptr2));
  CMP_TEST_OK(cmp_arena_get_stats(&small_arena, &stats));
  CMP_TEST_ASSERT(stats.block_count == 2);

  CMP_TEST_OK(cmp_arena_reset(&small_arena));
  CMP_TEST_OK(cmp_arena_get_stats(&small_arena, &stats));
  CMP_TEST_ASSERT(stats.block_count == 1);
  CMP_TEST_ASSERT(stats.total_used == 0);

  CMP_TEST_OK(cmp_arena_shutdown(&small_arena));

  CMP_TEST_OK(cmp_arena_init(&stats_cap_arena, NULL, 16));
  CMP_TEST_OK(cmp_arena_alloc(&stats_cap_arena, 16, 1, &ptr));
  CMP_TEST_OK(cmp_arena_alloc(&stats_cap_arena, 16, 1, &ptr));
  block = (CMPArenaBlockInternal *)stats_cap_arena.head;
  block->capacity = max_size;
  CMP_TEST_EXPECT(cmp_arena_get_stats(&stats_cap_arena, &stats), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_arena_shutdown(&stats_cap_arena));

  CMP_TEST_OK(cmp_arena_init(&stats_used_arena, NULL, 16));
  CMP_TEST_OK(cmp_arena_alloc(&stats_used_arena, 16, 1, &ptr));
  CMP_TEST_OK(cmp_arena_alloc(&stats_used_arena, 16, 1, &ptr));
  block = (CMPArenaBlockInternal *)stats_used_arena.head;
  block->offset = max_size;
  CMP_TEST_EXPECT(cmp_arena_get_stats(&stats_used_arena, &stats),
                 CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_arena_shutdown(&stats_used_arena));

  CMP_TEST_EXPECT(cmp_arena_reset(&arena_uninit), CMP_ERR_STATE);

  memset(&test_ctx, 0, sizeof(test_ctx));
  test_allocator.ctx = &test_ctx;
  test_allocator.alloc = test_alloc;
  test_allocator.realloc = test_realloc;
  test_allocator.free = test_free;

  test_ctx.fail_alloc = 1;
  CMP_TEST_EXPECT(cmp_arena_init(&fail_arena, &test_allocator, 16),
                 CMP_ERR_OUT_OF_MEMORY);
  test_ctx.fail_alloc = 0;

  CMP_TEST_OK(cmp_arena_init(&fail_arena, &test_allocator, 16));
  CMP_TEST_OK(cmp_arena_alloc(&fail_arena, 16, 1, &ptr));
  CMP_TEST_OK(cmp_arena_alloc(&fail_arena, 16, 1, &ptr));
  test_ctx.fail_free = 1;
  CMP_TEST_EXPECT(cmp_arena_reset(&fail_arena), CMP_ERR_IO);
  test_ctx.fail_free = 0;
  CMP_TEST_OK(cmp_arena_shutdown(&fail_arena));

  CMP_TEST_OK(cmp_arena_init(&shutdown_fail_arena, &test_allocator, 16));
  CMP_TEST_OK(cmp_arena_alloc(&shutdown_fail_arena, 16, 1, &ptr));
  test_ctx.fail_free = 1;
  CMP_TEST_EXPECT(cmp_arena_shutdown(&shutdown_fail_arena), CMP_ERR_IO);
  test_ctx.fail_free = 0;
  CMP_TEST_OK(cmp_arena_shutdown(&shutdown_fail_arena));

  CMP_TEST_OK(cmp_arena_init(&oom_arena, &test_allocator, 16));
  CMP_TEST_OK(cmp_arena_alloc(&oom_arena, 16, 1, &ptr));
  test_ctx.fail_alloc = 1;
  CMP_TEST_EXPECT(cmp_arena_alloc(&oom_arena, 64, 1, &ptr), CMP_ERR_OUT_OF_MEMORY);
  test_ctx.fail_alloc = 0;
  CMP_TEST_OK(cmp_arena_shutdown(&oom_arena));

  CMP_TEST_OK(cmp_arena_init(&arena, NULL, 16));
  CMP_TEST_EXPECT(cmp_arena_alloc(&arena, max_size, 1, &ptr), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_arena_shutdown(&arena));

  CMP_TEST_OK(cmp_arena_shutdown(&arena_uninit));

  return 0;
}
