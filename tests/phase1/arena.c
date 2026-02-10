#include "m3/m3_arena.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct M3ArenaBlockInternal {
  struct M3ArenaBlockInternal *next;
  m3_usize capacity;
  m3_usize offset;
  unsigned char data[1];
} M3ArenaBlockInternal;

typedef struct TestAllocCtx {
  int fail_alloc;
  int fail_free;
  int alloc_calls;
  int free_calls;
} TestAllocCtx;

static int M3_CALL test_alloc(void *ctx, m3_usize size, void **out_ptr) {
  TestAllocCtx *state;
  void *mem;

  if (out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestAllocCtx *)ctx;
  if (state != NULL && state->fail_alloc) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  mem = malloc((size_t)size);
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  if (state != NULL) {
    state->alloc_calls += 1;
  }

  *out_ptr = mem;
  return M3_OK;
}

static int M3_CALL test_realloc(void *ctx, void *ptr, m3_usize size,
                                void **out_ptr) {
  TestAllocCtx *state;
  void *mem;

  if (out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestAllocCtx *)ctx;
  if (state != NULL && state->fail_alloc) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  mem = realloc(ptr, (size_t)size);
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  if (state != NULL) {
    state->alloc_calls += 1;
  }

  *out_ptr = mem;
  return M3_OK;
}

static int M3_CALL test_free(void *ctx, void *ptr) {
  TestAllocCtx *state;

  state = (TestAllocCtx *)ctx;
  if (state != NULL && state->fail_free) {
    return M3_ERR_IO;
  }

  free(ptr);

  if (state != NULL) {
    state->free_calls += 1;
  }

  return M3_OK;
}

int main(void) {
  M3Allocator bad_alloc;
  M3Arena arena;
  M3Arena arena_uninit;
  M3Arena small_arena;
  M3Arena fail_arena;
  M3Arena oom_arena;
  M3Arena corrupt_arena;
  M3Arena stats_cap_arena;
  M3Arena stats_used_arena;
  M3Arena align_overflow_arena;
  M3Arena shutdown_fail_arena;
  M3ArenaStats stats;
  M3Allocator test_allocator;
  TestAllocCtx test_ctx;
  void *ptr;
  void *ptr2;
  m3_usize max_size;
  M3ArenaBlockInternal *block;

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
  max_size = (m3_usize) ~(m3_usize)0;

  M3_TEST_EXPECT(m3_arena_init(NULL, NULL, 0), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_arena_reset(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_arena_shutdown(NULL), M3_ERR_INVALID_ARGUMENT);

  memset(&bad_alloc, 0, sizeof(bad_alloc));
  M3_TEST_EXPECT(m3_arena_init(&arena, &bad_alloc, 64),
                 M3_ERR_INVALID_ARGUMENT);

#ifdef M3_TESTING
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_arena_init(&arena, NULL, 0), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));
#endif

  M3_TEST_OK(m3_arena_init(&arena, NULL, 0));
  M3_TEST_ASSERT(arena.block_size != 0);
  M3_TEST_EXPECT(m3_arena_init(&arena, NULL, 0), M3_ERR_STATE);

  M3_TEST_EXPECT(m3_arena_get_stats(NULL, &stats), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_arena_get_stats(&arena, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_arena_get_stats(&arena_uninit, &stats), M3_ERR_STATE);

  M3_TEST_OK(m3_arena_get_stats(&arena, &stats));
  M3_TEST_ASSERT(stats.block_count == 1);
  M3_TEST_ASSERT(stats.total_capacity >= arena.block_size);
  M3_TEST_ASSERT(stats.total_used == 0);

  M3_TEST_EXPECT(m3_arena_alloc(NULL, 8, 4, &ptr), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_arena_alloc(&arena, 0, 4, &ptr), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_arena_alloc(&arena, 8, 0, &ptr), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_arena_alloc(&arena, 8, 3, &ptr), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_arena_alloc(&arena, 8, 4, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_arena_alloc(&arena_uninit, 8, 4, &ptr), M3_ERR_STATE);

  M3_TEST_OK(m3_arena_alloc(&arena, 16, 8, &ptr));
  M3_TEST_ASSERT(ptr != NULL);
  M3_TEST_ASSERT(((m3_usize)ptr % 8) == 0);

  M3_TEST_OK(m3_arena_shutdown(&arena));

  M3_TEST_OK(m3_arena_init(&corrupt_arena, NULL, 16));
  block = (M3ArenaBlockInternal *)corrupt_arena.current;
  block->capacity = 8;
  block->offset = 32;
  M3_TEST_OK(m3_arena_alloc(&corrupt_arena, 8, 1, &ptr));
  M3_TEST_OK(m3_arena_shutdown(&corrupt_arena));

  M3_TEST_OK(m3_arena_init(&align_overflow_arena, NULL, 16));
  block = (M3ArenaBlockInternal *)align_overflow_arena.current;
  block->offset = max_size;
  M3_TEST_EXPECT(m3_arena_alloc(&align_overflow_arena, 8, 8, &ptr),
                 M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_arena_shutdown(&align_overflow_arena));

#ifdef M3_TESTING
  M3_TEST_OK(m3_arena_init(&arena, NULL, 16));
  M3_TEST_OK(m3_arena_alloc(&arena, 8, 8, &ptr));
  M3_TEST_OK(m3_arena_test_set_force_align_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_arena_alloc(&arena, 32, 8, &ptr2), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_arena_test_set_force_align_fail(M3_FALSE));
  M3_TEST_OK(m3_arena_shutdown(&arena));

  M3_TEST_OK(m3_arena_init(&arena, NULL, 16));
  M3_TEST_OK(m3_arena_test_set_force_add_overflow(M3_TRUE));
  M3_TEST_EXPECT(m3_arena_alloc(&arena, 8, 1, &ptr), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_arena_test_set_force_add_overflow(M3_FALSE));
  M3_TEST_OK(m3_arena_shutdown(&arena));
#endif

  M3_TEST_OK(m3_arena_init(&small_arena, NULL, 32));
  M3_TEST_OK(m3_arena_alloc(&small_arena, 24, 8, &ptr));
  M3_TEST_OK(m3_arena_alloc(&small_arena, 16, 8, &ptr2));
  M3_TEST_OK(m3_arena_get_stats(&small_arena, &stats));
  M3_TEST_ASSERT(stats.block_count == 2);

  M3_TEST_OK(m3_arena_reset(&small_arena));
  M3_TEST_OK(m3_arena_get_stats(&small_arena, &stats));
  M3_TEST_ASSERT(stats.block_count == 1);
  M3_TEST_ASSERT(stats.total_used == 0);

  M3_TEST_OK(m3_arena_shutdown(&small_arena));

  M3_TEST_OK(m3_arena_init(&stats_cap_arena, NULL, 16));
  M3_TEST_OK(m3_arena_alloc(&stats_cap_arena, 16, 1, &ptr));
  M3_TEST_OK(m3_arena_alloc(&stats_cap_arena, 16, 1, &ptr));
  block = (M3ArenaBlockInternal *)stats_cap_arena.head;
  block->capacity = max_size;
  M3_TEST_EXPECT(m3_arena_get_stats(&stats_cap_arena, &stats), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_arena_shutdown(&stats_cap_arena));

  M3_TEST_OK(m3_arena_init(&stats_used_arena, NULL, 16));
  M3_TEST_OK(m3_arena_alloc(&stats_used_arena, 16, 1, &ptr));
  M3_TEST_OK(m3_arena_alloc(&stats_used_arena, 16, 1, &ptr));
  block = (M3ArenaBlockInternal *)stats_used_arena.head;
  block->offset = max_size;
  M3_TEST_EXPECT(m3_arena_get_stats(&stats_used_arena, &stats),
                 M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_arena_shutdown(&stats_used_arena));

  M3_TEST_EXPECT(m3_arena_reset(&arena_uninit), M3_ERR_STATE);

  memset(&test_ctx, 0, sizeof(test_ctx));
  test_allocator.ctx = &test_ctx;
  test_allocator.alloc = test_alloc;
  test_allocator.realloc = test_realloc;
  test_allocator.free = test_free;

  test_ctx.fail_alloc = 1;
  M3_TEST_EXPECT(m3_arena_init(&fail_arena, &test_allocator, 16),
                 M3_ERR_OUT_OF_MEMORY);
  test_ctx.fail_alloc = 0;

  M3_TEST_OK(m3_arena_init(&fail_arena, &test_allocator, 16));
  M3_TEST_OK(m3_arena_alloc(&fail_arena, 16, 1, &ptr));
  M3_TEST_OK(m3_arena_alloc(&fail_arena, 16, 1, &ptr));
  test_ctx.fail_free = 1;
  M3_TEST_EXPECT(m3_arena_reset(&fail_arena), M3_ERR_IO);
  test_ctx.fail_free = 0;
  M3_TEST_OK(m3_arena_shutdown(&fail_arena));

  M3_TEST_OK(m3_arena_init(&shutdown_fail_arena, &test_allocator, 16));
  M3_TEST_OK(m3_arena_alloc(&shutdown_fail_arena, 16, 1, &ptr));
  test_ctx.fail_free = 1;
  M3_TEST_EXPECT(m3_arena_shutdown(&shutdown_fail_arena), M3_ERR_IO);
  test_ctx.fail_free = 0;
  M3_TEST_OK(m3_arena_shutdown(&shutdown_fail_arena));

  M3_TEST_OK(m3_arena_init(&oom_arena, &test_allocator, 16));
  M3_TEST_OK(m3_arena_alloc(&oom_arena, 16, 1, &ptr));
  test_ctx.fail_alloc = 1;
  M3_TEST_EXPECT(m3_arena_alloc(&oom_arena, 64, 1, &ptr), M3_ERR_OUT_OF_MEMORY);
  test_ctx.fail_alloc = 0;
  M3_TEST_OK(m3_arena_shutdown(&oom_arena));

  M3_TEST_OK(m3_arena_init(&arena, NULL, 16));
  M3_TEST_EXPECT(m3_arena_alloc(&arena, max_size, 1, &ptr), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_arena_shutdown(&arena));

  M3_TEST_OK(m3_arena_shutdown(&arena_uninit));

  return 0;
}
