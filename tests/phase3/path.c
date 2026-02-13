#include "cmpc/cmp_path.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
  cmp_usize alloc_calls;
  cmp_usize realloc_calls;
  cmp_usize free_calls;
  cmp_usize fail_alloc_on;
  cmp_usize fail_realloc_on;
  cmp_usize fail_free_on;
} TestAllocator;

static int test_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  TestAllocator *alloc;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on != 0 && alloc->alloc_calls == alloc->fail_alloc_on) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  return CMP_OK;
}

static int test_realloc(void *ctx, void *ptr, cmp_usize size, void **out_ptr) {
  TestAllocator *alloc;
  void *mem;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->realloc_calls += 1;
  if (alloc->fail_realloc_on != 0 &&
      alloc->realloc_calls == alloc->fail_realloc_on) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  mem = realloc(ptr, size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  *out_ptr = mem;
  return CMP_OK;
}

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on != 0 && alloc->free_calls == alloc->fail_free_on) {
    return CMP_ERR_UNKNOWN;
  }

  free(ptr);
  return CMP_OK;
}

static void test_allocator_init(TestAllocator *alloc) {
  alloc->alloc_calls = 0;
  alloc->realloc_calls = 0;
  alloc->free_calls = 0;
  alloc->fail_alloc_on = 0;
  alloc->fail_realloc_on = 0;
  alloc->fail_free_on = 0;
}

int main(void) {
  CMPPath path;
  CMPAllocator bad_alloc;
  TestAllocator alloc;
  CMPAllocator alloc_iface;
  cmp_usize max_size;
  cmp_usize cmd_size;
  cmp_usize overflow_capacity;

  memset(&path, 0, sizeof(path));

  CMP_TEST_EXPECT(cmp_path_move_to(&path, 0.0f, 0.0f), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_path_line_to(&path, 0.0f, 0.0f), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_path_quad_to(&path, 0.0f, 0.0f, 0.0f, 0.0f), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_path_cubic_to(&path, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
                 CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_path_close(&path), CMP_ERR_STATE);

  CMP_TEST_EXPECT(cmp_path_init(NULL, NULL, 0), CMP_ERR_INVALID_ARGUMENT);
#ifdef CMP_TESTING
  {
    cmp_usize out_value;
    CMPBool has_current;
    CMPPath bad_path;
    CMPPathCmd dummy_cmd;
    cmp_usize max_value;

    CMP_TEST_EXPECT(cmp_path_test_add_overflow(1, 2, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_path_test_add_overflow(1, 2, &out_value));
    CMP_TEST_ASSERT(out_value == 3);
    max_value = (cmp_usize) ~(cmp_usize)0;
    CMP_TEST_EXPECT(cmp_path_test_add_overflow(max_value, 1, &out_value),
                   CMP_ERR_OVERFLOW);
    CMP_TEST_EXPECT(cmp_path_test_mul_overflow(2, 3, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_path_test_mul_overflow(2, 3, &out_value));
    CMP_TEST_ASSERT(out_value == 6);
    CMP_TEST_EXPECT(cmp_path_test_has_current(&path, NULL),
                   CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_path_test_has_current(&path, &has_current), CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_path_test_reserve(&path, 1), CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_path_test_append(&path, NULL), CMP_ERR_INVALID_ARGUMENT);

    memset(&bad_path, 0, sizeof(bad_path));
    bad_path.commands = &dummy_cmd;
    bad_path.capacity = 1;
    bad_path.count = 2;
    CMP_TEST_EXPECT(cmp_path_test_has_current(&bad_path, &has_current),
                   CMP_ERR_STATE);
  }
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_path_init(&path, NULL, 1), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));
#endif

  bad_alloc.ctx = NULL;
  bad_alloc.alloc = NULL;
  bad_alloc.realloc = NULL;
  bad_alloc.free = NULL;
  CMP_TEST_EXPECT(cmp_path_init(&path, &bad_alloc, 1), CMP_ERR_INVALID_ARGUMENT);

  test_allocator_init(&alloc);
  alloc_iface.ctx = &alloc;
  alloc_iface.alloc = test_alloc;
  alloc_iface.realloc = test_realloc;
  alloc_iface.free = test_free;

  alloc.fail_alloc_on = 1;
  CMP_TEST_EXPECT(cmp_path_init(&path, &alloc_iface, 1), CMP_ERR_OUT_OF_MEMORY);
  alloc.fail_alloc_on = 0;

  max_size = (cmp_usize) ~(cmp_usize)0;
  cmd_size = (cmp_usize)sizeof(CMPPathCmd);
  overflow_capacity = max_size / cmd_size + 1;
  CMP_TEST_EXPECT(cmp_path_init(&path, &alloc_iface, overflow_capacity),
                 CMP_ERR_OVERFLOW);

#ifdef CMP_TESTING
  {
    CMPPath reserve_path;
    CMPPath overflow_path;
    CMPPath zero_path;
    CMPPathCmd dummy_cmd;
    cmp_usize max_value;

    max_value = (cmp_usize) ~(cmp_usize)0;

    memset(&overflow_path, 0, sizeof(overflow_path));
    overflow_path.commands = &dummy_cmd;
    overflow_path.capacity = max_value;
    overflow_path.count = max_value;
    overflow_path.allocator = alloc_iface;
    CMP_TEST_EXPECT(cmp_path_test_reserve(&overflow_path, 1), CMP_ERR_OVERFLOW);

    memset(&zero_path, 0, sizeof(zero_path));
    zero_path.commands = &dummy_cmd;
    zero_path.capacity = 0;
    zero_path.count = 0;
    zero_path.allocator = alloc_iface;
    CMP_TEST_EXPECT(cmp_path_test_reserve(&zero_path, 1), CMP_ERR_STATE);

    memset(&reserve_path, 0, sizeof(reserve_path));
    CMP_TEST_OK(cmp_path_init(&reserve_path, &alloc_iface, 1));
    CMP_TEST_OK(cmp_path_move_to(&reserve_path, 0.0f, 0.0f));
    CMP_TEST_OK(cmp_path_test_reserve(&reserve_path, 4));
    CMP_TEST_OK(cmp_path_shutdown(&reserve_path));

    memset(&overflow_path, 0, sizeof(overflow_path));
    overflow_path.commands = &dummy_cmd;
    overflow_path.capacity = max_value;
    overflow_path.count = max_value;
    overflow_path.allocator = alloc_iface;
    CMP_TEST_OK(cmp_path_test_set_force_reserve(CMP_TRUE));
    CMP_TEST_EXPECT(cmp_path_test_reserve(&overflow_path, 0), CMP_ERR_OVERFLOW);
    CMP_TEST_OK(cmp_path_test_set_force_reserve(CMP_FALSE));
  }
#endif

  memset(&path, 0, sizeof(path));
  CMP_TEST_OK(cmp_path_init(&path, &alloc_iface, 0));
  CMP_TEST_ASSERT(path.capacity >= 1);
  CMP_TEST_OK(cmp_path_shutdown(&path));

  memset(&path, 0, sizeof(path));
  CMP_TEST_OK(cmp_path_init(&path, &alloc_iface, 1));
  CMP_TEST_EXPECT(cmp_path_init(&path, &alloc_iface, 1), CMP_ERR_STATE);
  CMP_TEST_OK(cmp_path_shutdown(&path));

  CMP_TEST_EXPECT(cmp_path_reset(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_path_reset(&path), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_path_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_path_shutdown(&path), CMP_ERR_STATE);

  memset(&path, 0, sizeof(path));
  CMP_TEST_OK(cmp_path_init(&path, &alloc_iface, 1));
  CMP_TEST_EXPECT(cmp_path_quad_to(&path, 0.0f, 0.0f, 0.0f, 0.0f), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_path_cubic_to(&path, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
                 CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_path_line_to(&path, 1.0f, 2.0f), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_path_close(&path), CMP_ERR_STATE);

  CMP_TEST_OK(cmp_path_move_to(&path, 1.0f, 2.0f));
  CMP_TEST_OK(cmp_path_line_to(&path, 3.0f, 4.0f));
  CMP_TEST_OK(cmp_path_quad_to(&path, 5.0f, 6.0f, 7.0f, 8.0f));
  CMP_TEST_OK(cmp_path_cubic_to(&path, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f));
  CMP_TEST_OK(cmp_path_close(&path));
  CMP_TEST_EXPECT(cmp_path_line_to(&path, 15.0f, 16.0f), CMP_ERR_STATE);

  CMP_TEST_ASSERT(path.count == 5);
  CMP_TEST_ASSERT(path.commands[0].type == CMP_PATH_CMD_MOVE_TO);
  CMP_TEST_ASSERT(path.commands[0].data.move_to.x == 1.0f);
  CMP_TEST_ASSERT(path.commands[0].data.move_to.y == 2.0f);
  CMP_TEST_ASSERT(path.commands[1].type == CMP_PATH_CMD_LINE_TO);
  CMP_TEST_ASSERT(path.commands[1].data.line_to.x == 3.0f);
  CMP_TEST_ASSERT(path.commands[1].data.line_to.y == 4.0f);
  CMP_TEST_ASSERT(path.commands[2].type == CMP_PATH_CMD_QUAD_TO);
  CMP_TEST_ASSERT(path.commands[2].data.quad_to.cx == 5.0f);
  CMP_TEST_ASSERT(path.commands[2].data.quad_to.cy == 6.0f);
  CMP_TEST_ASSERT(path.commands[2].data.quad_to.x == 7.0f);
  CMP_TEST_ASSERT(path.commands[2].data.quad_to.y == 8.0f);
  CMP_TEST_ASSERT(path.commands[3].type == CMP_PATH_CMD_CUBIC_TO);
  CMP_TEST_ASSERT(path.commands[3].data.cubic_to.cx1 == 9.0f);
  CMP_TEST_ASSERT(path.commands[3].data.cubic_to.cy1 == 10.0f);
  CMP_TEST_ASSERT(path.commands[3].data.cubic_to.cx2 == 11.0f);
  CMP_TEST_ASSERT(path.commands[3].data.cubic_to.cy2 == 12.0f);
  CMP_TEST_ASSERT(path.commands[3].data.cubic_to.x == 13.0f);
  CMP_TEST_ASSERT(path.commands[3].data.cubic_to.y == 14.0f);
  CMP_TEST_ASSERT(path.commands[4].type == CMP_PATH_CMD_CLOSE);

  CMP_TEST_OK(cmp_path_reset(&path));
  CMP_TEST_ASSERT(path.count == 0);

  CMP_TEST_OK(cmp_path_move_to(&path, 0.0f, 0.0f));
  CMP_TEST_OK(cmp_path_test_reserve(&path, 4));

  path.count = max_size / 2 + 1;
  path.capacity = path.count;
  CMP_TEST_EXPECT(cmp_path_move_to(&path, 0.0f, 0.0f), CMP_ERR_OVERFLOW);

  CMP_TEST_OK(cmp_path_shutdown(&path));

  memset(&path, 0, sizeof(path));
  test_allocator_init(&alloc);
  alloc_iface.ctx = &alloc;
  alloc_iface.alloc = test_alloc;
  alloc_iface.realloc = test_realloc;
  alloc_iface.free = test_free;
  CMP_TEST_OK(cmp_path_init(&path, &alloc_iface, 1));
  CMP_TEST_OK(cmp_path_move_to(&path, 0.0f, 0.0f));
  alloc.fail_realloc_on = 1;
  CMP_TEST_EXPECT(cmp_path_line_to(&path, 1.0f, 1.0f), CMP_ERR_OUT_OF_MEMORY);
  alloc.fail_realloc_on = 0;
  CMP_TEST_OK(cmp_path_shutdown(&path));

  memset(&path, 0, sizeof(path));
  test_allocator_init(&alloc);
  alloc_iface.ctx = &alloc;
  alloc_iface.alloc = test_alloc;
  alloc_iface.realloc = test_realloc;
  alloc_iface.free = test_free;
  CMP_TEST_OK(cmp_path_init(&path, &alloc_iface, 1));
  alloc.fail_free_on = 1;
  CMP_TEST_EXPECT(cmp_path_shutdown(&path), CMP_ERR_UNKNOWN);

  return 0;
}
