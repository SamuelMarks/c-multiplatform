/* clang-format off */
#include "cmp.h"
#include "greatest.h"

/* clang-format on */

TEST test_arena_init_success(void) {
  cmp_arena_t arena;
  int res = cmp_arena_init(&arena, 1024);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(arena.buffer != NULL);
  ASSERT_EQ_FMT((unsigned long)1024, (unsigned long)(arena.capacity), "%lu");
  ASSERT_EQ_FMT((unsigned long)0, (unsigned long)(arena.offset), "%lu");
  cmp_arena_free(&arena);
  PASS();
}

TEST test_arena_init_zero_size(void) {
  cmp_arena_t arena;
  int res = cmp_arena_init(&arena, 0);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(arena.buffer == NULL);
  ASSERT_EQ_FMT((unsigned long)0, (unsigned long)(arena.capacity), "%lu");
  ASSERT_EQ_FMT((unsigned long)0, (unsigned long)(arena.offset), "%lu");
  PASS();
}

TEST test_arena_init_null(void) {
  int res = cmp_arena_init(NULL, 1024);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");
  PASS();
}

TEST test_arena_alloc_success(void) {
  cmp_arena_t arena;
  void *ptr1 = NULL;
  void *ptr2 = NULL;
  int res;

  cmp_arena_init(&arena, 1024);

  res = cmp_arena_alloc(&arena, 100, &ptr1);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr1 != NULL);
  ASSERT_EQ_FMT((unsigned long)100, (unsigned long)(arena.offset), "%lu");

  res = cmp_arena_alloc(&arena, 50, &ptr2);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr2 != NULL);
  ASSERT_EQ_FMT((unsigned long)150, (unsigned long)(arena.offset), "%lu");

  cmp_arena_free(&arena);
  PASS();
}

TEST test_arena_alloc_zero_size(void) {
  cmp_arena_t arena;
  void *ptr = (void *)1;
  int res;

  cmp_arena_init(&arena, 1024);

  res = cmp_arena_alloc(&arena, 0, &ptr);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr == NULL);

  cmp_arena_free(&arena);
  PASS();
}

TEST test_arena_alloc_null(void) {
  cmp_arena_t arena;
  void *ptr = NULL;
  int res;

  cmp_arena_init(&arena, 1024);

  res = cmp_arena_alloc(NULL, 100, &ptr);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_arena_alloc(&arena, 100, NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  cmp_arena_free(&arena);
  PASS();
}

TEST test_arena_alloc_oom(void) {
  cmp_arena_t arena;
  void *ptr = NULL;
  int res;

  cmp_arena_init(&arena, 100);

  res = cmp_arena_alloc(&arena, 150, &ptr);
  ASSERT_EQ_FMT(CMP_ERROR_OOM, res, "%d");
  ASSERT(ptr == NULL);

  cmp_arena_free(&arena);
  PASS();
}

TEST test_arena_free_null(void) {
  int res = cmp_arena_free(NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");
  PASS();
}

TEST test_pool_init_success(void) {
  cmp_pool_t pool;
  int res = cmp_pool_init(&pool, 32, 10);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(pool.buffer != NULL);
  ASSERT_EQ_FMT((unsigned long)10, (unsigned long)(pool.capacity), "%lu");
  ASSERT_EQ_FMT((unsigned long)32, (unsigned long)(pool.block_size), "%lu");
  cmp_pool_destroy(&pool);
  PASS();
}

TEST test_pool_init_zero(void) {
  cmp_pool_t pool;
  int res = cmp_pool_init(&pool, 0, 0);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(pool.buffer == NULL);
  cmp_pool_destroy(&pool);
  PASS();
}

TEST test_pool_init_null(void) {
  int res = cmp_pool_init(NULL, 32, 10);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");
  PASS();
}

TEST test_pool_init_small_block(void) {
  cmp_pool_t pool;
  int res = cmp_pool_init(&pool, 1, 10);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(pool.buffer != NULL);
  /* The block size should be aligned and at least sizeof(cmp_pool_block_t) */
  ASSERT(pool.block_size >= sizeof(void *));
  cmp_pool_destroy(&pool);
  PASS();
}

TEST test_pool_alloc_free(void) {
  cmp_pool_t pool;
  void *ptr1 = NULL;
  void *ptr2 = NULL;
  int res;

  /* Use block size of 16 to ensure proper alignment */
  cmp_pool_init(&pool, 16, 2);

  res = cmp_pool_alloc(&pool, &ptr1);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr1 != NULL);

  res = cmp_pool_alloc(&pool, &ptr2);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr2 != NULL);

  res = cmp_pool_alloc(&pool, &ptr1); /* Should fail, only 2 blocks */
  ASSERT_EQ_FMT(CMP_ERROR_OOM, res, "%d");

  res = cmp_pool_free(&pool, ptr2);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_pool_alloc(&pool, &ptr1); /* Should succeed now */
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr1 == ptr2); /* Got the same block back */

  cmp_pool_destroy(&pool);
  PASS();
}

TEST test_pool_alloc_null(void) {
  cmp_pool_t pool;
  void *ptr = NULL;
  int res;

  cmp_pool_init(&pool, 16, 2);

  res = cmp_pool_alloc(NULL, &ptr);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_pool_alloc(&pool, NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  cmp_pool_destroy(&pool);
  PASS();
}

TEST test_pool_alloc_zero_capacity(void) {
  cmp_pool_t pool;
  void *ptr = (void *)1;
  int res;

  cmp_pool_init(&pool, 16, 0);

  res = cmp_pool_alloc(&pool, &ptr);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr == NULL);

  cmp_pool_destroy(&pool);
  PASS();
}

TEST test_pool_free_null(void) {
  cmp_pool_t pool;
  int res;

  cmp_pool_init(&pool, 16, 2);

  res = cmp_pool_free(NULL, (void *)1);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_pool_free(&pool, NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  cmp_pool_destroy(&pool);
  PASS();
}

TEST test_pool_free_bounds_misaligned(void) {
  cmp_pool_t pool;
  void *ptr;
  int res;

  cmp_pool_init(&pool, 16, 2);

  res = cmp_pool_alloc(&pool, &ptr);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Out of bounds below */
  res = cmp_pool_free(&pool, pool.buffer - 1);
  ASSERT_EQ_FMT(CMP_ERROR_BOUNDS, res, "%d");

  /* Out of bounds above */
  res = cmp_pool_free(&pool, pool.buffer + (16 * 2));
  ASSERT_EQ_FMT(CMP_ERROR_BOUNDS, res, "%d");

  /* Misaligned */
  res = cmp_pool_free(&pool, pool.buffer + 1);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  cmp_pool_free(&pool, ptr);
  cmp_pool_destroy(&pool);
  PASS();
}

TEST test_pool_destroy_null(void) {
  int res = cmp_pool_destroy(NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");
  PASS();
}

TEST test_mem_tracking(void) {
  void *ptr1 = NULL;
  void *ptr2 = NULL;
  int res;

  res = CMP_MALLOC(100, &ptr1);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr1 != NULL);

  res = CMP_MALLOC(200, &ptr2);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr2 != NULL);

  /* Should report 2 leaks */
  ASSERT_EQ_FMT(2, cmp_mem_check_leaks(), "%d");

  res = CMP_FREE(ptr1);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Should report 1 leak */
  ASSERT_EQ_FMT(1, cmp_mem_check_leaks(), "%d");

  res = CMP_FREE(ptr2);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Should report 0 leaks */
  ASSERT_EQ_FMT(0, cmp_mem_check_leaks(), "%d");

  PASS();
}

TEST test_mem_tracking_null(void) {
  int res;
  res = CMP_FREE(NULL);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_mem_alloc_tracked(10, "file", 1, NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  PASS();
}

TEST test_mem_tracking_not_found(void) {
  int res;
  int dummy = 0;
  res = CMP_FREE(&dummy);
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
  PASS();
}

TEST test_mem_tracking_zero(void) {
  void *ptr = (void *)1;
  int res;
  res = cmp_mem_alloc_tracked(0, "file", 1, &ptr);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr == NULL);
  PASS();
}

SUITE(mem_suite) {
  RUN_TEST(test_mem_tracking);
  RUN_TEST(test_mem_tracking_null);
  RUN_TEST(test_mem_tracking_not_found);
  RUN_TEST(test_mem_tracking_zero);
}

TEST test_arena_massive_reallocation(void) {
  cmp_arena_t arena;
  void *ptr;
  int res;
  int i;
  int j;

  /* Initialize arena */
  res = cmp_arena_init(&arena, 1024);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  for (i = 0; i < 1000; i++) {
    /* Fill arena */
    for (j = 0; j < 10; j++) {
      res = cmp_arena_alloc(&arena, 100, &ptr);
      ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
      ASSERT(ptr != NULL);
    }

    /* Next allocation should fail */
    res = cmp_arena_alloc(&arena, 100, &ptr);
    ASSERT_EQ_FMT(CMP_ERROR_OOM, res, "%d");

    /* "Free" the arena by resetting its offset */
    arena.offset = 0;
  }

  /* Cleanup */
  cmp_arena_free(&arena);
  PASS();
}

SUITE(arena_suite) {
  RUN_TEST(test_arena_init_success);
  RUN_TEST(test_arena_init_zero_size);
  RUN_TEST(test_arena_init_null);
  RUN_TEST(test_arena_alloc_success);
  RUN_TEST(test_arena_alloc_zero_size);
  RUN_TEST(test_arena_alloc_null);
  RUN_TEST(test_arena_alloc_oom);
  RUN_TEST(test_arena_free_null);
  RUN_TEST(test_arena_massive_reallocation);
}

SUITE(pool_suite) {
  RUN_TEST(test_pool_init_success);
  RUN_TEST(test_pool_init_zero);
  RUN_TEST(test_pool_init_null);
  RUN_TEST(test_pool_init_small_block);
  RUN_TEST(test_pool_alloc_free);
  RUN_TEST(test_pool_alloc_null);
  RUN_TEST(test_pool_alloc_zero_capacity);
  RUN_TEST(test_pool_free_null);
  RUN_TEST(test_pool_free_bounds_misaligned);
  RUN_TEST(test_pool_destroy_null);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(mem_suite);
  RUN_SUITE(arena_suite);
  RUN_SUITE(pool_suite);
  GREATEST_MAIN_END();
}
