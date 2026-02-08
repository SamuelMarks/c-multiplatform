#include "test_utils.h"
#include "m3/m3_core.h"

int main(void)
{
    M3Allocator alloc;
    void *ptr;
    void *ptr2;
    m3_usize max_size;

    ptr = NULL;
    ptr2 = NULL;

    M3_TEST_EXPECT(m3_get_default_allocator(NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_get_default_allocator(&alloc));
    M3_TEST_ASSERT(alloc.alloc != NULL);
    M3_TEST_ASSERT(alloc.realloc != NULL);
    M3_TEST_ASSERT(alloc.free != NULL);

    M3_TEST_EXPECT(alloc.alloc(alloc.ctx, 0, &ptr), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(alloc.alloc(alloc.ctx, 8, NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(alloc.alloc(alloc.ctx, 16, &ptr));
    M3_TEST_ASSERT(ptr != NULL);

    M3_TEST_EXPECT(alloc.realloc(alloc.ctx, ptr, 0, &ptr2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(alloc.realloc(alloc.ctx, ptr, 32, NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(alloc.realloc(alloc.ctx, ptr, 32, &ptr2));
    M3_TEST_ASSERT(ptr2 != NULL);

    M3_TEST_OK(alloc.free(alloc.ctx, ptr2));
    M3_TEST_OK(alloc.free(alloc.ctx, NULL));

    M3_TEST_EXPECT(alloc.realloc(alloc.ctx, NULL, 0, &ptr), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(alloc.realloc(alloc.ctx, NULL, 8, NULL), M3_ERR_INVALID_ARGUMENT);

    ptr = NULL;
    M3_TEST_OK(alloc.realloc(alloc.ctx, NULL, 8, &ptr));
    M3_TEST_ASSERT(ptr != NULL);
    M3_TEST_OK(alloc.free(alloc.ctx, ptr));

    max_size = (m3_usize)~(m3_usize)0;
    ptr = NULL;
    M3_TEST_EXPECT(alloc.alloc(alloc.ctx, max_size, &ptr), M3_ERR_OUT_OF_MEMORY);
    M3_TEST_ASSERT(ptr == NULL);

    ptr = NULL;
    M3_TEST_EXPECT(alloc.realloc(alloc.ctx, NULL, max_size, &ptr), M3_ERR_OUT_OF_MEMORY);
    M3_TEST_ASSERT(ptr == NULL);

    return 0;
}
