#include "cmpc/cmp_core.h"
#include "test_utils.h"

int main(void) {
  CMPAllocator alloc;
  void *ptr;
  void *ptr2;
  cmp_usize max_size;

  ptr = NULL;
  ptr2 = NULL;

  CMP_TEST_EXPECT(cmp_get_default_allocator(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));
  CMP_TEST_ASSERT(alloc.alloc != NULL);
  CMP_TEST_ASSERT(alloc.realloc != NULL);
  CMP_TEST_ASSERT(alloc.free != NULL);

  CMP_TEST_EXPECT(alloc.alloc(alloc.ctx, 0, &ptr), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(alloc.alloc(alloc.ctx, 8, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(alloc.alloc(alloc.ctx, 16, &ptr));
  CMP_TEST_ASSERT(ptr != NULL);

  CMP_TEST_EXPECT(alloc.realloc(alloc.ctx, ptr, 0, &ptr2),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(alloc.realloc(alloc.ctx, ptr, 32, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(alloc.realloc(alloc.ctx, ptr, 32, &ptr2));
  CMP_TEST_ASSERT(ptr2 != NULL);

  CMP_TEST_OK(alloc.free(alloc.ctx, ptr2));
  CMP_TEST_OK(alloc.free(alloc.ctx, NULL));

  CMP_TEST_EXPECT(alloc.realloc(alloc.ctx, NULL, 0, &ptr),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(alloc.realloc(alloc.ctx, NULL, 8, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  ptr = NULL;
  CMP_TEST_OK(alloc.realloc(alloc.ctx, NULL, 8, &ptr));
  CMP_TEST_ASSERT(ptr != NULL);
  CMP_TEST_OK(alloc.free(alloc.ctx, ptr));

  max_size = (cmp_usize) ~(cmp_usize)0;
  ptr = NULL;
  CMP_TEST_EXPECT(alloc.alloc(alloc.ctx, max_size, &ptr), CMP_ERR_OUT_OF_MEMORY);
  CMP_TEST_ASSERT(ptr == NULL);

  ptr = NULL;
  CMP_TEST_EXPECT(alloc.realloc(alloc.ctx, NULL, max_size, &ptr),
                 CMP_ERR_OUT_OF_MEMORY);
  CMP_TEST_ASSERT(ptr == NULL);

  return 0;
}
