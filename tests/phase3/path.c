#include "m3/m3_path.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
  m3_usize alloc_calls;
  m3_usize realloc_calls;
  m3_usize free_calls;
  m3_usize fail_alloc_on;
  m3_usize fail_realloc_on;
  m3_usize fail_free_on;
} TestAllocator;

static int test_alloc(void *ctx, m3_usize size, void **out_ptr) {
  TestAllocator *alloc;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on != 0 && alloc->alloc_calls == alloc->fail_alloc_on) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  return M3_OK;
}

static int test_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr) {
  TestAllocator *alloc;
  void *mem;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->realloc_calls += 1;
  if (alloc->fail_realloc_on != 0 &&
      alloc->realloc_calls == alloc->fail_realloc_on) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  mem = realloc(ptr, size);
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  *out_ptr = mem;
  return M3_OK;
}

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on != 0 && alloc->free_calls == alloc->fail_free_on) {
    return M3_ERR_UNKNOWN;
  }

  free(ptr);
  return M3_OK;
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
  M3Path path;
  M3Allocator bad_alloc;
  TestAllocator alloc;
  M3Allocator alloc_iface;
  m3_usize max_size;
  m3_usize cmd_size;
  m3_usize overflow_capacity;

  memset(&path, 0, sizeof(path));

  M3_TEST_EXPECT(m3_path_init(NULL, NULL, 0), M3_ERR_INVALID_ARGUMENT);
#ifdef M3_TESTING
  {
    m3_usize out_value;
    M3Bool has_current;

    M3_TEST_EXPECT(m3_path_test_add_overflow(1, 2, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_path_test_add_overflow(1, 2, &out_value));
    M3_TEST_ASSERT(out_value == 3);
    M3_TEST_EXPECT(m3_path_test_mul_overflow(2, 3, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_path_test_mul_overflow(2, 3, &out_value));
    M3_TEST_ASSERT(out_value == 6);
    M3_TEST_EXPECT(m3_path_test_has_current(&path, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_path_test_has_current(&path, &has_current), M3_ERR_STATE);
  }
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_path_init(&path, NULL, 1), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));
#endif

  bad_alloc.ctx = NULL;
  bad_alloc.alloc = NULL;
  bad_alloc.realloc = NULL;
  bad_alloc.free = NULL;
  M3_TEST_EXPECT(m3_path_init(&path, &bad_alloc, 1), M3_ERR_INVALID_ARGUMENT);

  test_allocator_init(&alloc);
  alloc_iface.ctx = &alloc;
  alloc_iface.alloc = test_alloc;
  alloc_iface.realloc = test_realloc;
  alloc_iface.free = test_free;

  alloc.fail_alloc_on = 1;
  M3_TEST_EXPECT(m3_path_init(&path, &alloc_iface, 1), M3_ERR_OUT_OF_MEMORY);
  alloc.fail_alloc_on = 0;

  max_size = (m3_usize) ~(m3_usize)0;
  cmd_size = (m3_usize)sizeof(M3PathCmd);
  overflow_capacity = max_size / cmd_size + 1;
  M3_TEST_EXPECT(m3_path_init(&path, &alloc_iface, overflow_capacity),
                 M3_ERR_OVERFLOW);

  memset(&path, 0, sizeof(path));
  M3_TEST_OK(m3_path_init(&path, &alloc_iface, 0));
  M3_TEST_ASSERT(path.capacity >= 1);
  M3_TEST_OK(m3_path_shutdown(&path));

  memset(&path, 0, sizeof(path));
  M3_TEST_OK(m3_path_init(&path, &alloc_iface, 1));
  M3_TEST_EXPECT(m3_path_init(&path, &alloc_iface, 1), M3_ERR_STATE);
  M3_TEST_OK(m3_path_shutdown(&path));

  M3_TEST_EXPECT(m3_path_reset(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_path_reset(&path), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_path_shutdown(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_path_shutdown(&path), M3_ERR_STATE);

  memset(&path, 0, sizeof(path));
  M3_TEST_OK(m3_path_init(&path, &alloc_iface, 1));
  M3_TEST_EXPECT(m3_path_line_to(&path, 1.0f, 2.0f), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_path_close(&path), M3_ERR_STATE);

  M3_TEST_OK(m3_path_move_to(&path, 1.0f, 2.0f));
  M3_TEST_OK(m3_path_line_to(&path, 3.0f, 4.0f));
  M3_TEST_OK(m3_path_quad_to(&path, 5.0f, 6.0f, 7.0f, 8.0f));
  M3_TEST_OK(m3_path_cubic_to(&path, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f));
  M3_TEST_OK(m3_path_close(&path));
  M3_TEST_EXPECT(m3_path_line_to(&path, 15.0f, 16.0f), M3_ERR_STATE);

  M3_TEST_ASSERT(path.count == 5);
  M3_TEST_ASSERT(path.commands[0].type == M3_PATH_CMD_MOVE_TO);
  M3_TEST_ASSERT(path.commands[0].data.move_to.x == 1.0f);
  M3_TEST_ASSERT(path.commands[0].data.move_to.y == 2.0f);
  M3_TEST_ASSERT(path.commands[1].type == M3_PATH_CMD_LINE_TO);
  M3_TEST_ASSERT(path.commands[1].data.line_to.x == 3.0f);
  M3_TEST_ASSERT(path.commands[1].data.line_to.y == 4.0f);
  M3_TEST_ASSERT(path.commands[2].type == M3_PATH_CMD_QUAD_TO);
  M3_TEST_ASSERT(path.commands[2].data.quad_to.cx == 5.0f);
  M3_TEST_ASSERT(path.commands[2].data.quad_to.cy == 6.0f);
  M3_TEST_ASSERT(path.commands[2].data.quad_to.x == 7.0f);
  M3_TEST_ASSERT(path.commands[2].data.quad_to.y == 8.0f);
  M3_TEST_ASSERT(path.commands[3].type == M3_PATH_CMD_CUBIC_TO);
  M3_TEST_ASSERT(path.commands[3].data.cubic_to.cx1 == 9.0f);
  M3_TEST_ASSERT(path.commands[3].data.cubic_to.cy1 == 10.0f);
  M3_TEST_ASSERT(path.commands[3].data.cubic_to.cx2 == 11.0f);
  M3_TEST_ASSERT(path.commands[3].data.cubic_to.cy2 == 12.0f);
  M3_TEST_ASSERT(path.commands[3].data.cubic_to.x == 13.0f);
  M3_TEST_ASSERT(path.commands[3].data.cubic_to.y == 14.0f);
  M3_TEST_ASSERT(path.commands[4].type == M3_PATH_CMD_CLOSE);

  M3_TEST_OK(m3_path_reset(&path));
  M3_TEST_ASSERT(path.count == 0);

  path.count = max_size / 2 + 1;
  path.capacity = path.count;
  M3_TEST_EXPECT(m3_path_move_to(&path, 0.0f, 0.0f), M3_ERR_OVERFLOW);

  M3_TEST_OK(m3_path_shutdown(&path));

  memset(&path, 0, sizeof(path));
  test_allocator_init(&alloc);
  alloc_iface.ctx = &alloc;
  alloc_iface.alloc = test_alloc;
  alloc_iface.realloc = test_realloc;
  alloc_iface.free = test_free;
  M3_TEST_OK(m3_path_init(&path, &alloc_iface, 1));
  M3_TEST_OK(m3_path_move_to(&path, 0.0f, 0.0f));
  alloc.fail_realloc_on = 1;
  M3_TEST_EXPECT(m3_path_line_to(&path, 1.0f, 1.0f), M3_ERR_OUT_OF_MEMORY);
  alloc.fail_realloc_on = 0;
  M3_TEST_OK(m3_path_shutdown(&path));

  memset(&path, 0, sizeof(path));
  test_allocator_init(&alloc);
  alloc_iface.ctx = &alloc;
  alloc_iface.alloc = test_alloc;
  alloc_iface.realloc = test_realloc;
  alloc_iface.free = test_free;
  M3_TEST_OK(m3_path_init(&path, &alloc_iface, 1));
  alloc.fail_free_on = 1;
  M3_TEST_EXPECT(m3_path_shutdown(&path), M3_ERR_UNKNOWN);

  return 0;
}
