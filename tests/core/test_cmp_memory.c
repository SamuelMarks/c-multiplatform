/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_arena_init_success(void) {
  cmp_arena_t arena;
  int res = cmp_arena_init(&arena, 1024);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(arena.buffer != NULL);
  ASSERT_EQ_FMT((size_t)1024, arena.capacity, "%zd");
  ASSERT_EQ_FMT((size_t)0, arena.offset, "%zd");
  cmp_arena_free(&arena);
  PASS();
}

TEST test_arena_init_zero_size(void) {
  cmp_arena_t arena;
  int res = cmp_arena_init(&arena, 0);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(arena.buffer == NULL);
  ASSERT_EQ_FMT((size_t)0, arena.capacity, "%zd");
  ASSERT_EQ_FMT((size_t)0, arena.offset, "%zd");
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
  ASSERT_EQ_FMT((size_t)100, arena.offset, "%zd");

  res = cmp_arena_alloc(&arena, 50, &ptr2);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(ptr2 != NULL);
  ASSERT_EQ_FMT((size_t)150, arena.offset, "%zd");

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

TEST test_pool_init_success(void) {
  cmp_pool_t pool;
  int res = cmp_pool_init(&pool, 32, 10);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(pool.buffer != NULL);
  ASSERT_EQ_FMT((size_t)10, pool.capacity, "%zd");
  ASSERT_EQ_FMT((size_t)32, pool.block_size, "%zd");
  cmp_pool_destroy(&pool);
  PASS();
}

TEST test_pool_alloc_free(void) {
  cmp_pool_t pool;
  void *ptr1 = NULL;
  void *ptr2 = NULL;
  int res;

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
  PASS();
}

TEST test_mem_tracking_not_found(void) {
  int res;
  int dummy = 0;
  res = CMP_FREE(&dummy);
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");
  PASS();
}

SUITE(mem_suite) {
  RUN_TEST(test_mem_tracking);
  RUN_TEST(test_mem_tracking_null);
  RUN_TEST(test_mem_tracking_not_found);
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
  RUN_TEST(test_arena_alloc_success);
  RUN_TEST(test_arena_alloc_oom);
  RUN_TEST(test_arena_massive_reallocation);
}

SUITE(pool_suite) {
  RUN_TEST(test_pool_init_success);
  RUN_TEST(test_pool_alloc_free);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(mem_suite);
  RUN_SUITE(arena_suite);
  RUN_SUITE(pool_suite);
  GREATEST_MAIN_END();
}
