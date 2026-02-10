#include "test_utils.h"
#include "m3/m3_storage.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAlloc {
    int fail_alloc_on_call;
    int fail_realloc_on_call;
    int fail_free_on_call;
    int alloc_calls;
    int realloc_calls;
    int free_calls;
    int mutate_on_alloc;
    m3_usize *mutate_target;
    m3_usize mutate_value;
} TestAlloc;

static int test_alloc_reset(TestAlloc *alloc)
{
    if (alloc == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(alloc, 0, sizeof(*alloc));
    return M3_OK;
}

static int test_alloc_fn(void *ctx, m3_usize size, void **out_ptr)
{
    TestAlloc *alloc;
    void *ptr;

    if (ctx == NULL || out_ptr == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    alloc = (TestAlloc *)ctx;
    alloc->alloc_calls += 1;
    if (alloc->fail_alloc_on_call > 0 && alloc->alloc_calls == alloc->fail_alloc_on_call) {
        return M3_ERR_OUT_OF_MEMORY;
    }
    if (size == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    ptr = malloc((size_t)size);
    if (ptr == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    if (alloc->mutate_on_alloc && alloc->mutate_target != NULL) {
        *alloc->mutate_target = alloc->mutate_value;
        alloc->mutate_on_alloc = 0;
    }

    *out_ptr = ptr;
    return M3_OK;
}

static int test_realloc_fn(void *ctx, void *ptr, m3_usize size, void **out_ptr)
{
    TestAlloc *alloc;
    void *new_ptr;

    if (ctx == NULL || out_ptr == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    alloc = (TestAlloc *)ctx;
    alloc->realloc_calls += 1;
    if (alloc->fail_realloc_on_call > 0 && alloc->realloc_calls == alloc->fail_realloc_on_call) {
        return M3_ERR_OUT_OF_MEMORY;
    }
    if (size == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    new_ptr = realloc(ptr, (size_t)size);
    if (new_ptr == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    *out_ptr = new_ptr;
    return M3_OK;
}

static int test_free_fn(void *ctx, void *ptr)
{
    TestAlloc *alloc;

    if (ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    alloc = (TestAlloc *)ctx;
    alloc->free_calls += 1;
    if (alloc->fail_free_on_call > 0 && alloc->free_calls == alloc->fail_free_on_call) {
        return M3_ERR_IO;
    }

    if (ptr != NULL) {
        free(ptr);
    }

    return M3_OK;
}

typedef struct TestIO {
    m3_u8 *buffer;
    m3_usize capacity;
    m3_usize size;
    int fail_write;
    int fail_read;
    int fail_read_alloc;
} TestIO;

static int test_io_reset(TestIO *io, m3_u8 *buffer, m3_usize capacity)
{
    if (io == NULL || buffer == NULL || capacity == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    io->buffer = buffer;
    io->capacity = capacity;
    io->size = 0;
    io->fail_write = 0;
    io->fail_read = 0;
    io->fail_read_alloc = 0;
    memset(io->buffer, 0, (size_t)capacity);
    return M3_OK;
}

static int test_io_write_file(void *io, const char *utf8_path, const void *data, m3_usize size, M3Bool overwrite)
{
    TestIO *state;

    if (io == NULL || utf8_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (data == NULL && size != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (overwrite != M3_FALSE && overwrite != M3_TRUE) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestIO *)io;
    if (state->fail_write) {
        return M3_ERR_IO;
    }
    if (size > state->capacity) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    if (size > 0) {
        memcpy(state->buffer, data, (size_t)size);
    }
    state->size = size;
    return M3_OK;
}

static int test_io_read_file(void *io, const char *utf8_path, void *buffer, m3_usize buffer_size, m3_usize *out_read)
{
    TestIO *state;

    if (io == NULL || utf8_path == NULL || out_read == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (buffer == NULL && buffer_size != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestIO *)io;
    if (state->fail_read) {
        return M3_ERR_IO;
    }
    if (buffer_size < state->size) {
        return M3_ERR_RANGE;
    }

    if (state->size > 0) {
        memcpy(buffer, state->buffer, (size_t)state->size);
    }
    *out_read = state->size;
    return M3_OK;
}

static int test_io_read_file_alloc(void *io, const char *utf8_path, const M3Allocator *allocator, void **out_data, m3_usize *out_size)
{
    TestIO *state;
    int rc;

    if (io == NULL || utf8_path == NULL || allocator == NULL || out_data == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestIO *)io;
    if (state->fail_read_alloc) {
        return M3_ERR_IO;
    }
    if (state->size == 0) {
        return M3_ERR_NOT_FOUND;
    }

    rc = allocator->alloc(allocator->ctx, state->size, out_data);
    if (rc != M3_OK) {
        return rc;
    }

    memcpy(*out_data, state->buffer, (size_t)state->size);
    *out_size = state->size;
    return M3_OK;
}

static int test_io_file_exists(void *io, const char *utf8_path, M3Bool *out_exists)
{
    TestIO *state;

    if (io == NULL || utf8_path == NULL || out_exists == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestIO *)io;
    *out_exists = state->size > 0 ? M3_TRUE : M3_FALSE;
    return M3_OK;
}

static int test_io_delete_file(void *io, const char *utf8_path)
{
    TestIO *state;

    if (io == NULL || utf8_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestIO *)io;
    state->size = 0;
    return M3_OK;
}

static int test_io_stat_file(void *io, const char *utf8_path, M3FileInfo *out_info)
{
    TestIO *state;

    if (io == NULL || utf8_path == NULL || out_info == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestIO *)io;
    if (state->size > (m3_usize)((m3_u32)~(m3_u32)0)) {
        return M3_ERR_RANGE;
    }

    out_info->size_bytes = (m3_u32)state->size;
    out_info->flags = 0u;
    return M3_OK;
}

static const M3IOVTable g_test_io_vtable = {
    test_io_read_file,
    test_io_read_file_alloc,
    test_io_write_file,
    test_io_file_exists,
    test_io_delete_file,
    test_io_stat_file
};

static int test_write_u32_le(m3_u8 *dst, m3_u32 value)
{
    if (dst == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dst[0] = (m3_u8)(value & 0xFFu);
    dst[1] = (m3_u8)((value >> 8) & 0xFFu);
    dst[2] = (m3_u8)((value >> 16) & 0xFFu);
    dst[3] = (m3_u8)((value >> 24) & 0xFFu);
    return M3_OK;
}

static int test_storage_helpers(void)
{
    m3_usize value;
    m3_u32 value32;
    m3_usize max_value;
    m3_usize max_u32;
    int rc;

    M3_TEST_EXPECT(m3_storage_test_mul_overflow(1u, 2u, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_test_add_overflow(1u, 2u, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_test_u32_from_usize(1u, NULL), M3_ERR_INVALID_ARGUMENT);

    rc = m3_storage_test_mul_overflow(4u, 5u, &value);
    M3_TEST_OK(rc);
    M3_TEST_ASSERT(value == 20u);

    rc = m3_storage_test_add_overflow(7u, 3u, &value);
    M3_TEST_OK(rc);
    M3_TEST_ASSERT(value == 10u);

    max_value = (m3_usize)~(m3_usize)0;
    M3_TEST_EXPECT(m3_storage_test_mul_overflow(max_value, 2u, &value), M3_ERR_OVERFLOW);
    M3_TEST_EXPECT(m3_storage_test_add_overflow(max_value, 1u, &value), M3_ERR_OVERFLOW);

    rc = m3_storage_test_u32_from_usize(42u, &value32);
    M3_TEST_OK(rc);
    M3_TEST_ASSERT(value32 == 42u);

    max_u32 = (m3_usize)((m3_u32)~(m3_u32)0);
    if (max_u32 < (m3_usize)~(m3_usize)0) {
        M3_TEST_EXPECT(m3_storage_test_u32_from_usize(max_u32 + 1u, &value32), M3_ERR_RANGE);
    }
    return 0;
}

static int test_storage_config_init(void)
{
    M3StorageConfig config;

    M3_TEST_EXPECT(m3_storage_config_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_storage_config_init(&config));
    M3_TEST_ASSERT(config.allocator == NULL);
    M3_TEST_ASSERT(config.entry_capacity == (m3_usize)M3_STORAGE_DEFAULT_CAPACITY);
    return 0;
}

static int test_storage_init_errors(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;
    m3_usize max_value;

    memset(&storage, 0, sizeof(storage));

    M3_TEST_OK(m3_storage_config_init(&config));
    M3_TEST_EXPECT(m3_storage_init(NULL, &config), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_init(&storage, NULL), M3_ERR_INVALID_ARGUMENT);

    config.entry_capacity = 0;
    M3_TEST_EXPECT(m3_storage_init(&storage, &config), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(test_alloc_reset(&test_alloc));
    allocator.ctx = NULL;
    allocator.alloc = NULL;
    allocator.realloc = NULL;
    allocator.free = NULL;
    config.entry_capacity = 1u;
    config.allocator = &allocator;
    M3_TEST_EXPECT(m3_storage_init(&storage, &config), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
    config.allocator = NULL;
    M3_TEST_EXPECT(m3_storage_init(&storage, &config), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));

    max_value = (m3_usize)~(m3_usize)0;
    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;
    config.allocator = &allocator;
    config.entry_capacity = max_value / sizeof(M3StorageEntry) + 1u;
    M3_TEST_EXPECT(m3_storage_init(&storage, &config), M3_ERR_OVERFLOW);

    M3_TEST_OK(test_alloc_reset(&test_alloc));
    test_alloc.fail_alloc_on_call = 1;
    config.entry_capacity = 1u;
    M3_TEST_EXPECT(m3_storage_init(&storage, &config), M3_ERR_OUT_OF_MEMORY);

    storage.entries = (M3StorageEntry *)1;
    M3_TEST_EXPECT(m3_storage_init(&storage, &config), M3_ERR_STATE);
    storage.entries = NULL;

    return 0;
}

static int test_storage_init_shutdown(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;
    m3_usize count;
    int rc;

    memset(&storage, 0, sizeof(storage));
    M3_TEST_OK(test_alloc_reset(&test_alloc));

    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 2u;

    M3_TEST_OK(m3_storage_init(&storage, &config));
    M3_TEST_OK(m3_storage_count(&storage, &count));
    M3_TEST_ASSERT(count == 0u);

    rc = m3_storage_put(&storage, "alpha", 5u, "one", 3u, M3_TRUE);
    M3_TEST_OK(rc);

    M3_TEST_OK(m3_storage_shutdown(&storage));
    M3_TEST_ASSERT(storage.entries == NULL);

    M3_TEST_EXPECT(m3_storage_shutdown(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_shutdown(&storage), M3_ERR_STATE);

    return 0;
}

static int test_storage_put_errors(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;
    m3_usize max_value;

    M3_TEST_EXPECT(m3_storage_put(NULL, "a", 1u, "b", 1u, M3_FALSE), M3_ERR_INVALID_ARGUMENT);

    memset(&storage, 0, sizeof(storage));
    M3_TEST_EXPECT(m3_storage_put(&storage, "a", 1u, "b", 1u, M3_FALSE), M3_ERR_STATE);

    M3_TEST_OK(test_alloc_reset(&test_alloc));
    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 2u;
    M3_TEST_OK(m3_storage_init(&storage, &config));

    M3_TEST_EXPECT(m3_storage_put(&storage, "a", 1u, "b", 1u, 2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_put(&storage, "a", 0u, "b", 1u, M3_TRUE), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_put(&storage, "a", 1u, NULL, 1u, M3_TRUE), M3_ERR_INVALID_ARGUMENT);

    max_value = (m3_usize)~(m3_usize)0;
    M3_TEST_EXPECT(m3_storage_put(&storage, "a", max_value, "b", 1u, M3_TRUE), M3_ERR_OVERFLOW);

    M3_TEST_OK(m3_storage_put(&storage, "alpha", 5u, "one", 3u, M3_FALSE));

    test_alloc.alloc_calls = 0;
    test_alloc.fail_alloc_on_call = 1;
    M3_TEST_EXPECT(m3_storage_put(&storage, "alpha", 5u, "two", 3u, M3_TRUE), M3_ERR_OUT_OF_MEMORY);
    test_alloc.fail_alloc_on_call = 0;

    test_alloc.free_calls = 0;
    test_alloc.fail_free_on_call = 1;
    M3_TEST_EXPECT(m3_storage_put(&storage, "alpha", 5u, "two", 3u, M3_TRUE), M3_ERR_IO);
    test_alloc.fail_free_on_call = 0;

    storage.entry_capacity = 1u;
    storage.allocator.realloc = NULL;
    M3_TEST_EXPECT(m3_storage_put(&storage, "beta", 4u, "two", 3u, M3_FALSE), M3_ERR_INVALID_ARGUMENT);
    storage.allocator.realloc = test_realloc_fn;
    storage.entry_capacity = 2u;

    test_alloc.free_calls = 0;
    test_alloc.fail_free_on_call = 1;
    M3_TEST_EXPECT(m3_storage_put(&storage, "alpha", 5u, NULL, 0u, M3_TRUE), M3_ERR_IO);
    test_alloc.fail_free_on_call = 0;

    M3_TEST_OK(m3_storage_shutdown(&storage));
    return 0;
}

static int test_storage_access_errors(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;
    M3Bool exists;
    m3_usize value_size;
    char buffer[8];

    memset(&storage, 0, sizeof(storage));

    M3_TEST_EXPECT(m3_storage_get(NULL, "a", 1u, buffer, sizeof(buffer), &value_size), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_get(&storage, "a", 1u, buffer, sizeof(buffer), &value_size), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_storage_get(&storage, "a", 1u, buffer, sizeof(buffer), NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_storage_contains(NULL, "a", 1u, &exists), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_contains(&storage, "a", 1u, &exists), M3_ERR_STATE);

    M3_TEST_EXPECT(m3_storage_remove(NULL, "a", 1u), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_remove(&storage, "a", 1u), M3_ERR_STATE);

    M3_TEST_EXPECT(m3_storage_clear(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_clear(&storage), M3_ERR_STATE);

    M3_TEST_EXPECT(m3_storage_count(NULL, &value_size), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_count(&storage, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_count(&storage, &value_size), M3_ERR_STATE);

    M3_TEST_OK(test_alloc_reset(&test_alloc));
    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 1u;
    M3_TEST_OK(m3_storage_init(&storage, &config));

    M3_TEST_EXPECT(m3_storage_get(&storage, NULL, 1u, buffer, sizeof(buffer), &value_size), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_get(&storage, "a", 1u, NULL, 1u, &value_size), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_contains(&storage, NULL, 1u, &exists), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_contains(&storage, "a", 1u, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_remove(&storage, NULL, 1u), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_storage_shutdown(&storage));
    return 0;
}

static int test_storage_release_errors(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;

    memset(&storage, 0, sizeof(storage));
    M3_TEST_OK(test_alloc_reset(&test_alloc));

    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 1u;
    M3_TEST_OK(m3_storage_init(&storage, &config));
    M3_TEST_OK(m3_storage_put(&storage, "alpha", 5u, "one", 3u, M3_FALSE));

    test_alloc.free_calls = 0;
    test_alloc.fail_free_on_call = 2;
    M3_TEST_EXPECT(m3_storage_clear(&storage), M3_ERR_IO);
    test_alloc.fail_free_on_call = 0;

    M3_TEST_OK(m3_storage_put(&storage, "alpha", 5u, "one", 3u, M3_FALSE));
    storage.allocator.free = NULL;
    M3_TEST_EXPECT(m3_storage_shutdown(&storage), M3_ERR_INVALID_ARGUMENT);
    storage.allocator.free = test_free_fn;

    test_alloc.free_calls = 0;
    test_alloc.fail_free_on_call = 3;
    M3_TEST_EXPECT(m3_storage_shutdown(&storage), M3_ERR_IO);
    test_alloc.fail_free_on_call = 0;

    return 0;
}

static int test_storage_grow_overflow(void)
{
    M3Storage storage;
    M3Allocator allocator;
    TestAlloc test_alloc;
    m3_usize max_value;
    int rc;

    memset(&storage, 0, sizeof(storage));
    M3_TEST_OK(test_alloc_reset(&test_alloc));

    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    storage.allocator = allocator;
    storage.entries = (M3StorageEntry *)1;
    max_value = (m3_usize)~(m3_usize)0;
    storage.entry_capacity = max_value / sizeof(M3StorageEntry) + 1u;

    rc = m3_storage_test_grow(&storage, storage.entry_capacity);
    M3_TEST_EXPECT(rc, M3_ERR_OVERFLOW);
    return 0;
}

static int test_storage_put_get_remove(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;
    M3Bool exists;
    m3_usize value_size;
    m3_usize count;
    char buffer[16];

    memset(&storage, 0, sizeof(storage));
    M3_TEST_OK(test_alloc_reset(&test_alloc));

    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 2u;
    M3_TEST_OK(m3_storage_init(&storage, &config));

    M3_TEST_OK(m3_storage_put(&storage, "alpha", 5u, "one", 3u, M3_FALSE));
    M3_TEST_OK(m3_storage_put(&storage, "beta", 4u, "two", 3u, M3_FALSE));

    M3_TEST_OK(m3_storage_get(&storage, "alpha", 5u, buffer, sizeof(buffer), &value_size));
    M3_TEST_ASSERT(value_size == 3u);
    M3_TEST_ASSERT(memcmp(buffer, "one", 3u) == 0);

    M3_TEST_OK(m3_storage_contains(&storage, "alpha", 5u, &exists));
    M3_TEST_ASSERT(exists == M3_TRUE);
    M3_TEST_OK(m3_storage_contains(&storage, "missing", 7u, &exists));
    M3_TEST_ASSERT(exists == M3_FALSE);

    M3_TEST_EXPECT(m3_storage_get(&storage, "missing", 7u, buffer, sizeof(buffer), &value_size), M3_ERR_NOT_FOUND);
    M3_TEST_ASSERT(value_size == 0u);

    M3_TEST_EXPECT(m3_storage_get(&storage, "alpha", 5u, buffer, 2u, &value_size), M3_ERR_RANGE);
    M3_TEST_ASSERT(value_size == 3u);

    M3_TEST_OK(m3_storage_count(&storage, &count));
    M3_TEST_ASSERT(count == 2u);

    M3_TEST_OK(m3_storage_remove(&storage, "alpha", 5u));
    M3_TEST_OK(m3_storage_count(&storage, &count));
    M3_TEST_ASSERT(count == 1u);
    M3_TEST_EXPECT(m3_storage_remove(&storage, "alpha", 5u), M3_ERR_NOT_FOUND);

    M3_TEST_OK(m3_storage_clear(&storage));
    M3_TEST_OK(m3_storage_count(&storage, &count));
    M3_TEST_ASSERT(count == 0u);

    M3_TEST_OK(m3_storage_shutdown(&storage));
    return 0;
}

static int test_storage_overwrite_and_grow(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;
    m3_usize count;
    m3_usize value_size;
    char buffer[8];
    m3_usize max_value;

    memset(&storage, 0, sizeof(storage));
    M3_TEST_OK(test_alloc_reset(&test_alloc));

    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 1u;
    M3_TEST_OK(m3_storage_init(&storage, &config));

    M3_TEST_OK(m3_storage_put(&storage, "dup", 3u, "one", 3u, M3_FALSE));
    M3_TEST_EXPECT(m3_storage_put(&storage, "dup", 3u, "two", 3u, M3_FALSE), M3_ERR_BUSY);

    M3_TEST_OK(m3_storage_put(&storage, "dup", 3u, "two", 3u, M3_TRUE));
    M3_TEST_OK(m3_storage_get(&storage, "dup", 3u, buffer, sizeof(buffer), &value_size));
    M3_TEST_ASSERT(value_size == 3u);
    M3_TEST_ASSERT(memcmp(buffer, "two", 3u) == 0);

    M3_TEST_OK(m3_storage_put(&storage, "dup", 3u, NULL, 0u, M3_TRUE));
    M3_TEST_OK(m3_storage_get(&storage, "dup", 3u, buffer, sizeof(buffer), &value_size));
    M3_TEST_ASSERT(value_size == 0u);

    M3_TEST_OK(m3_storage_put(&storage, "grow", 4u, "x", 1u, M3_FALSE));
    M3_TEST_OK(m3_storage_count(&storage, &count));
    M3_TEST_ASSERT(count == 2u);
    M3_TEST_ASSERT(storage.entry_capacity >= 2u);

    test_alloc.realloc_calls = 0;
    test_alloc.fail_realloc_on_call = 1;
    M3_TEST_EXPECT(m3_storage_put(&storage, "fail", 4u, "y", 1u, M3_FALSE), M3_ERR_OUT_OF_MEMORY);
    test_alloc.fail_realloc_on_call = 0;

    max_value = (m3_usize)~(m3_usize)0;
    storage.entry_capacity = max_value / 2u + 1u;
    M3_TEST_EXPECT(m3_storage_test_grow(&storage, storage.entry_capacity + 1u), M3_ERR_OVERFLOW);
    storage.entry_capacity = 2u;

    M3_TEST_OK(m3_storage_shutdown(&storage));
    return 0;
}

static int test_storage_alloc_failures(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;

    memset(&storage, 0, sizeof(storage));
    M3_TEST_OK(test_alloc_reset(&test_alloc));

    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 2u;
    M3_TEST_OK(m3_storage_init(&storage, &config));

    test_alloc.alloc_calls = 0;
    test_alloc.fail_alloc_on_call = 1;
    M3_TEST_EXPECT(m3_storage_put(&storage, "key", 3u, NULL, 0u, M3_FALSE), M3_ERR_OUT_OF_MEMORY);
    test_alloc.fail_alloc_on_call = 0;

    test_alloc.alloc_calls = 0;
    test_alloc.alloc_calls = 0;
    test_alloc.alloc_calls = 0;
    test_alloc.alloc_calls = 0;
    test_alloc.alloc_calls = 0;
    test_alloc.fail_alloc_on_call = 2;
    M3_TEST_EXPECT(m3_storage_put(&storage, "key", 3u, "val", 3u, M3_FALSE), M3_ERR_OUT_OF_MEMORY);
    test_alloc.fail_alloc_on_call = 0;

    M3_TEST_OK(m3_storage_shutdown(&storage));
    return 0;
}

static int test_storage_free_failures(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;
    int rc;

    memset(&storage, 0, sizeof(storage));
    M3_TEST_OK(test_alloc_reset(&test_alloc));

    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 2u;
    M3_TEST_OK(m3_storage_init(&storage, &config));

    M3_TEST_OK(m3_storage_put(&storage, "free", 4u, "data", 4u, M3_FALSE));

    test_alloc.free_calls = 0;
    test_alloc.free_calls = 0;
    test_alloc.fail_free_on_call = 1;
    rc = m3_storage_remove(&storage, "free", 4u);
    M3_TEST_ASSERT(rc != M3_OK);
    test_alloc.fail_free_on_call = 0;

    M3_TEST_OK(m3_storage_put(&storage, "free", 4u, "data", 4u, M3_FALSE));
    test_alloc.free_calls = 0;
    test_alloc.fail_free_on_call = 1;
    rc = m3_storage_shutdown(&storage);
    M3_TEST_ASSERT(rc != M3_OK);

    memset(&storage, 0, sizeof(storage));
    return 0;
}

static int test_storage_save_load_roundtrip(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;
    TestIO test_io;
    M3IO io;
    m3_u8 buffer[256];
    m3_usize value_size;
    char value[8];

    memset(&storage, 0, sizeof(storage));
    M3_TEST_OK(test_alloc_reset(&test_alloc));
    M3_TEST_OK(test_io_reset(&test_io, buffer, sizeof(buffer)));

    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    io.ctx = &test_io;
    io.vtable = &g_test_io_vtable;

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 2u;
    M3_TEST_OK(m3_storage_init(&storage, &config));

    M3_TEST_OK(m3_storage_put(&storage, "a", 1u, "1", 1u, M3_FALSE));
    M3_TEST_OK(m3_storage_put(&storage, "b", 1u, "2", 1u, M3_FALSE));

    M3_TEST_OK(m3_storage_save(&storage, &io, "mem"));
    M3_TEST_ASSERT(test_io.size > 0u);
    M3_TEST_ASSERT(test_io.buffer[0] == 'M');
    M3_TEST_ASSERT(test_io.buffer[1] == '3');
    M3_TEST_ASSERT(test_io.buffer[2] == 'S');
    M3_TEST_ASSERT(test_io.buffer[3] == 'T');

    M3_TEST_OK(m3_storage_clear(&storage));
    M3_TEST_OK(m3_storage_load(&storage, &io, "mem"));

    M3_TEST_OK(m3_storage_get(&storage, "a", 1u, value, sizeof(value), &value_size));
    M3_TEST_ASSERT(value_size == 1u);
    M3_TEST_ASSERT(value[0] == '1');

    M3_TEST_OK(m3_storage_get(&storage, "b", 1u, value, sizeof(value), &value_size));
    M3_TEST_ASSERT(value_size == 1u);
    M3_TEST_ASSERT(value[0] == '2');

    M3_TEST_OK(m3_storage_shutdown(&storage));
    return 0;
}

static int test_storage_save_errors(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;
    TestIO test_io;
    M3IO io;
    m3_u8 buffer[256];
    m3_usize max_u32;
    char *saved_key;
    m3_usize saved_key_len;
    m3_usize saved_value_len;

    memset(&storage, 0, sizeof(storage));
    M3_TEST_OK(test_alloc_reset(&test_alloc));
    M3_TEST_OK(test_io_reset(&test_io, buffer, sizeof(buffer)));

    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    io.ctx = &test_io;
    io.vtable = &g_test_io_vtable;

    M3_TEST_EXPECT(m3_storage_save(NULL, &io, "mem"), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_save(&storage, NULL, "mem"), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_save(&storage, &io, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_STATE);

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 1u;
    M3_TEST_OK(m3_storage_init(&storage, &config));

    io.vtable = NULL;
    M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_INVALID_ARGUMENT);
    io.vtable = &g_test_io_vtable;

    storage.allocator.alloc = NULL;
    M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_INVALID_ARGUMENT);
    storage.allocator.alloc = test_alloc_fn;

    M3_TEST_OK(m3_storage_put(&storage, "k", 1u, "v", 1u, M3_FALSE));
    saved_key = storage.entries[0].key;
    saved_key_len = storage.entries[0].key_len;
    saved_value_len = storage.entries[0].value_len;
    storage.entries[0].key = NULL;
    M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_STATE);
    storage.entries[0].key = saved_key;

    max_u32 = (m3_usize)((m3_u32)~(m3_u32)0);
    if (max_u32 < (m3_usize)~(m3_usize)0) {
        storage.entry_count = max_u32 + 1u;
        M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_RANGE);
        storage.entry_count = 1u;

        storage.entries[0].key_len = max_u32 + 1u;
        M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_RANGE);
        storage.entries[0].key_len = saved_key_len;

        storage.entries[0].value_len = max_u32 + 1u;
        M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_RANGE);
        storage.entries[0].value_len = saved_value_len;
    }

    if (max_u32 < (m3_usize)~(m3_usize)0) {
        test_alloc.mutate_on_alloc = 1;
        test_alloc.mutate_target = &storage.entries[0].key_len;
        test_alloc.mutate_value = max_u32 + 1u;
        M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_RANGE);
        storage.entries[0].key_len = saved_key_len;
        test_alloc.mutate_target = NULL;

        test_alloc.mutate_on_alloc = 1;
        test_alloc.mutate_target = &storage.entries[0].value_len;
        test_alloc.mutate_value = max_u32 + 1u;
        M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_RANGE);
        storage.entries[0].value_len = saved_value_len;
        test_alloc.mutate_target = NULL;
    }

    test_alloc.alloc_calls = 0;
    test_alloc.fail_alloc_on_call = 1;
    M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_OUT_OF_MEMORY);
    test_alloc.fail_alloc_on_call = 0;

    test_io.fail_write = 1;
    M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_IO);
    test_io.fail_write = 0;

    test_alloc.free_calls = 0;
    test_alloc.fail_free_on_call = 1;
    M3_TEST_EXPECT(m3_storage_save(&storage, &io, "mem"), M3_ERR_IO);
    test_alloc.fail_free_on_call = 0;

    M3_TEST_OK(m3_storage_shutdown(&storage));
    return 0;
}

static int test_storage_load_errors(void)
{
    M3Storage storage;
    M3StorageConfig config;
    M3Allocator allocator;
    TestAlloc test_alloc;
    TestIO test_io;
    M3IO io;
    m3_u8 buffer[256];
    m3_u8 small[8];
    int rc;

    memset(&storage, 0, sizeof(storage));
    M3_TEST_OK(test_alloc_reset(&test_alloc));
    M3_TEST_OK(test_io_reset(&test_io, buffer, sizeof(buffer)));

    allocator.ctx = &test_alloc;
    allocator.alloc = test_alloc_fn;
    allocator.realloc = test_realloc_fn;
    allocator.free = test_free_fn;

    io.ctx = &test_io;
    io.vtable = &g_test_io_vtable;

    M3_TEST_EXPECT(m3_storage_load(NULL, &io, "mem"), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_load(&storage, NULL, "mem"), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_STATE);

    M3_TEST_OK(m3_storage_config_init(&config));
    config.allocator = &allocator;
    config.entry_capacity = 1u;
    M3_TEST_OK(m3_storage_init(&storage, &config));

    io.vtable = NULL;
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_INVALID_ARGUMENT);
    io.vtable = &g_test_io_vtable;

    storage.allocator.alloc = NULL;
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_INVALID_ARGUMENT);
    storage.allocator.alloc = test_alloc_fn;

    test_io.fail_read_alloc = 1;
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_IO);
    test_io.fail_read_alloc = 0;

    test_io.size = sizeof(small);
    memset(small, 0, sizeof(small));
    memcpy(test_io.buffer, small, sizeof(small));
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_CORRUPT);

    memset(test_io.buffer, 0, sizeof(buffer));
    test_io.size = 12u;
    test_io.buffer[0] = 'X';
    test_io.buffer[1] = '3';
    test_io.buffer[2] = 'S';
    test_io.buffer[3] = 'T';
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 8, 0u));
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_CORRUPT);

    test_io.buffer[0] = 'M';
    test_io.buffer[1] = '3';
    test_io.buffer[2] = 'S';
    test_io.buffer[3] = 'T';
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 4, 2u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 8, 0u));
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_UNSUPPORTED);

    M3_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 8, 1u));
    test_io.size = 12u;
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_CORRUPT);

    test_io.size = 20u;
    memset(test_io.buffer, 0, 20u);
    test_io.buffer[0] = 'M';
    test_io.buffer[1] = '3';
    test_io.buffer[2] = 'S';
    test_io.buffer[3] = 'T';
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 8, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 12, 0u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 16, 0u));
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_CORRUPT);

    test_io.size = 20u;
    memset(test_io.buffer, 0, 20u);
    test_io.buffer[0] = 'M';
    test_io.buffer[1] = '3';
    test_io.buffer[2] = 'S';
    test_io.buffer[3] = 'T';
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 8, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 12, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 16, 5u));
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_CORRUPT);

    test_io.size = 32u;
    memset(test_io.buffer, 0, 32u);
    test_io.buffer[0] = 'M';
    test_io.buffer[1] = '3';
    test_io.buffer[2] = 'S';
    test_io.buffer[3] = 'T';
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 8, 2u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 12, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 16, 1u));
    test_io.buffer[20] = 'a';
    test_io.buffer[21] = 'b';
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 22, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 26, 1u));
    test_io.buffer[30] = 'a';
    test_io.buffer[31] = 'c';
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_BUSY);

    test_alloc.alloc_calls = 0;
    test_alloc.fail_alloc_on_call = 2;
    test_io.size = 12u;
    memset(test_io.buffer, 0, 12u);
    test_io.buffer[0] = 'M';
    test_io.buffer[1] = '3';
    test_io.buffer[2] = 'S';
    test_io.buffer[3] = 'T';
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 8, 0u));
    M3_TEST_EXPECT(m3_storage_load(&storage, &io, "mem"), M3_ERR_OUT_OF_MEMORY);
    test_alloc.fail_alloc_on_call = 0;

    test_io.size = 16u;
    memset(test_io.buffer, 0, 16u);
    test_io.buffer[0] = 'M';
    test_io.buffer[1] = '3';
    test_io.buffer[2] = 'S';
    test_io.buffer[3] = 'T';
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
    M3_TEST_OK(test_write_u32_le(test_io.buffer + 8, 0u));
    test_alloc.free_calls = 0;
    test_alloc.fail_free_on_call = 1;
    rc = m3_storage_load(&storage, &io, "mem");
    M3_TEST_ASSERT(rc != M3_OK);
    test_alloc.fail_free_on_call = 0;

    M3_TEST_OK(m3_storage_shutdown(&storage));
    return 0;
}

int main(void)
{
    M3_TEST_ASSERT(test_storage_helpers() == 0);
    M3_TEST_ASSERT(test_storage_config_init() == 0);
    M3_TEST_ASSERT(test_storage_init_errors() == 0);
    M3_TEST_ASSERT(test_storage_init_shutdown() == 0);
    M3_TEST_ASSERT(test_storage_put_errors() == 0);
    M3_TEST_ASSERT(test_storage_access_errors() == 0);
    M3_TEST_ASSERT(test_storage_release_errors() == 0);
    M3_TEST_ASSERT(test_storage_grow_overflow() == 0);
    M3_TEST_ASSERT(test_storage_put_get_remove() == 0);
    M3_TEST_ASSERT(test_storage_overwrite_and_grow() == 0);
    M3_TEST_ASSERT(test_storage_alloc_failures() == 0);
    M3_TEST_ASSERT(test_storage_free_failures() == 0);
    M3_TEST_ASSERT(test_storage_save_load_roundtrip() == 0);
    M3_TEST_ASSERT(test_storage_save_errors() == 0);
    M3_TEST_ASSERT(test_storage_load_errors() == 0);
    return 0;
}
