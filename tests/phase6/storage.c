#include "cmpc/cmp_storage.h"
#include "test_utils.h"

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
  cmp_usize *mutate_target;
  cmp_usize mutate_value;
} TestAlloc;

static int test_alloc_reset(TestAlloc *alloc) {
  if (alloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(alloc, 0, sizeof(*alloc));
  return CMP_OK;
}

static int test_alloc_fn(void *ctx, cmp_usize size, void **out_ptr) {
  TestAlloc *alloc;
  void *ptr;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAlloc *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on_call > 0 &&
      alloc->alloc_calls == alloc->fail_alloc_on_call) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  ptr = malloc((size_t)size);
  if (ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  if (alloc->mutate_on_alloc && alloc->mutate_target != NULL) {
    *alloc->mutate_target = alloc->mutate_value;
    alloc->mutate_on_alloc = 0;
  }

  *out_ptr = ptr;
  return CMP_OK;
}

static int test_realloc_fn(void *ctx, void *ptr, cmp_usize size,
                           void **out_ptr) {
  TestAlloc *alloc;
  void *new_ptr;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAlloc *)ctx;
  alloc->realloc_calls += 1;
  if (alloc->fail_realloc_on_call > 0 &&
      alloc->realloc_calls == alloc->fail_realloc_on_call) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  new_ptr = realloc(ptr, (size_t)size);
  if (new_ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = new_ptr;
  return CMP_OK;
}

static int test_free_fn(void *ctx, void *ptr) {
  TestAlloc *alloc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAlloc *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on_call > 0 &&
      alloc->free_calls == alloc->fail_free_on_call) {
    return CMP_ERR_IO;
  }

  if (ptr != NULL) {
    free(ptr);
  }

  return CMP_OK;
}

typedef struct TestIO {
  cmp_u8 *buffer;
  cmp_usize capacity;
  cmp_usize size;
  int fail_write;
  int fail_read;
  int fail_read_alloc;
} TestIO;

static int test_io_reset(TestIO *io, cmp_u8 *buffer, cmp_usize capacity) {
  if (io == NULL || buffer == NULL || capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  io->buffer = buffer;
  io->capacity = capacity;
  io->size = 0;
  io->fail_write = 0;
  io->fail_read = 0;
  io->fail_read_alloc = 0;
  memset(io->buffer, 0, (size_t)capacity);
  return CMP_OK;
}

static int test_io_write_file(void *io, const char *utf8_path, const void *data,
                              cmp_usize size, CMPBool overwrite) {
  TestIO *state;

  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (data == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (overwrite != CMP_FALSE && overwrite != CMP_TRUE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestIO *)io;
  if (state->fail_write) {
    return CMP_ERR_IO;
  }
  if (size > state->capacity) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  if (size > 0) {
    memcpy(state->buffer, data, (size_t)size);
  }
  state->size = size;
  return CMP_OK;
}

static int test_io_read_file(void *io, const char *utf8_path, void *buffer,
                             cmp_usize buffer_size, cmp_usize *out_read) {
  TestIO *state;

  if (io == NULL || utf8_path == NULL || out_read == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (buffer == NULL && buffer_size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestIO *)io;
  if (state->fail_read) {
    return CMP_ERR_IO;
  }
  if (buffer_size < state->size) {
    return CMP_ERR_RANGE;
  }

  if (state->size > 0) {
    memcpy(buffer, state->buffer, (size_t)state->size);
  }
  *out_read = state->size;
  return CMP_OK;
}

static int test_io_read_file_alloc(void *io, const char *utf8_path,
                                   const CMPAllocator *allocator,
                                   void **out_data, cmp_usize *out_size) {
  TestIO *state;
  int rc;

  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestIO *)io;
  if (state->fail_read_alloc) {
    return CMP_ERR_IO;
  }
  if (state->size == 0) {
    return CMP_ERR_NOT_FOUND;
  }

  rc = allocator->alloc(allocator->ctx, state->size, out_data);
  if (rc != CMP_OK) {
    return rc;
  }

  memcpy(*out_data, state->buffer, (size_t)state->size);
  *out_size = state->size;
  return CMP_OK;
}

static int test_io_file_exists(void *io, const char *utf8_path,
                               CMPBool *out_exists) {
  TestIO *state;

  if (io == NULL || utf8_path == NULL || out_exists == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestIO *)io;
  *out_exists = state->size > 0 ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

static int test_io_delete_file(void *io, const char *utf8_path) {
  TestIO *state;

  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestIO *)io;
  state->size = 0;
  return CMP_OK;
}

static int test_io_stat_file(void *io, const char *utf8_path,
                             CMPFileInfo *out_info) {
  TestIO *state;

  if (io == NULL || utf8_path == NULL || out_info == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestIO *)io;
  if (state->size > (cmp_usize)((cmp_u32) ~(cmp_u32)0)) {
    return CMP_ERR_RANGE;
  }

  out_info->size_bytes = (cmp_u32)state->size;
  out_info->flags = 0u;
  return CMP_OK;
}

static const CMPIOVTable g_test_io_vtable = {
    test_io_read_file,   test_io_read_file_alloc, test_io_write_file,
    test_io_file_exists, test_io_delete_file,     test_io_stat_file};

static int test_write_u32_le(cmp_u8 *dst, cmp_u32 value) {
  if (dst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  dst[0] = (cmp_u8)(value & 0xFFu);
  dst[1] = (cmp_u8)((value >> 8) & 0xFFu);
  dst[2] = (cmp_u8)((value >> 16) & 0xFFu);
  dst[3] = (cmp_u8)((value >> 24) & 0xFFu);
  return CMP_OK;
}

static int test_storage_helpers(void) {
  cmp_usize value;
  cmp_u32 value32;
  cmp_usize max_value;
  cmp_usize max_u32;
  int rc;

  CMP_TEST_EXPECT(cmp_storage_test_mul_overflow(1u, 2u, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_test_add_overflow(1u, 2u, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_test_u32_from_usize(1u, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  rc = cmp_storage_test_mul_overflow(4u, 5u, &value);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(value == 20u);

  rc = cmp_storage_test_add_overflow(7u, 3u, &value);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(value == 10u);

  max_value = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_EXPECT(cmp_storage_test_mul_overflow(max_value, 2u, &value),
                 CMP_ERR_OVERFLOW);
  CMP_TEST_EXPECT(cmp_storage_test_add_overflow(max_value, 1u, &value),
                 CMP_ERR_OVERFLOW);

  rc = cmp_storage_test_u32_from_usize(42u, &value32);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(value32 == 42u);

  max_u32 = (cmp_usize)((cmp_u32) ~(cmp_u32)0);
  if (max_u32 < (cmp_usize) ~(cmp_usize)0) {
    CMP_TEST_EXPECT(cmp_storage_test_u32_from_usize(max_u32 + 1u, &value32),
                   CMP_ERR_RANGE);
  }
  return 0;
}

static int test_storage_config_init(void) {
  CMPStorageConfig config;

  CMP_TEST_EXPECT(cmp_storage_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_storage_config_init(&config));
  CMP_TEST_ASSERT(config.allocator == NULL);
  CMP_TEST_ASSERT(config.entry_capacity ==
                 (cmp_usize)CMP_STORAGE_DEFAULT_CAPACITY);
  return 0;
}

static int test_storage_init_errors(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  cmp_usize max_value;

  memset(&storage, 0, sizeof(storage));

  CMP_TEST_OK(cmp_storage_config_init(&config));
  CMP_TEST_EXPECT(cmp_storage_init(NULL, &config), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_init(&storage, NULL), CMP_ERR_INVALID_ARGUMENT);

  config.entry_capacity = 0;
  CMP_TEST_EXPECT(cmp_storage_init(&storage, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_alloc_reset(&test_alloc));
  allocator.ctx = NULL;
  allocator.alloc = NULL;
  allocator.realloc = NULL;
  allocator.free = NULL;
  config.entry_capacity = 1u;
  config.allocator = &allocator;
  CMP_TEST_EXPECT(cmp_storage_init(&storage, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  config.allocator = NULL;
  CMP_TEST_EXPECT(cmp_storage_init(&storage, &config), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));

  max_value = (cmp_usize) ~(cmp_usize)0;
  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;
  config.allocator = &allocator;
  config.entry_capacity = max_value / sizeof(CMPStorageEntry) + 1u;
  CMP_TEST_EXPECT(cmp_storage_init(&storage, &config), CMP_ERR_OVERFLOW);

  CMP_TEST_OK(test_alloc_reset(&test_alloc));
  test_alloc.fail_alloc_on_call = 1;
  config.entry_capacity = 1u;
  CMP_TEST_EXPECT(cmp_storage_init(&storage, &config), CMP_ERR_OUT_OF_MEMORY);

  storage.entries = (CMPStorageEntry *)1;
  CMP_TEST_EXPECT(cmp_storage_init(&storage, &config), CMP_ERR_STATE);
  storage.entries = NULL;

  CMP_TEST_OK(test_alloc_reset(&test_alloc));
  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;
  config.allocator = &allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));
  CMP_TEST_OK(cmp_storage_shutdown(&storage));

  return 0;
}

static int test_storage_init_shutdown(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  cmp_usize count;
  int rc;

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 2u;

  CMP_TEST_OK(cmp_storage_init(&storage, &config));
  CMP_TEST_OK(cmp_storage_count(&storage, &count));
  CMP_TEST_ASSERT(count == 0u);

  rc = cmp_storage_put(&storage, "alpha", 5u, "one", 3u, CMP_TRUE);
  CMP_TEST_OK(rc);

  CMP_TEST_OK(cmp_storage_shutdown(&storage));
  CMP_TEST_ASSERT(storage.entries == NULL);

  CMP_TEST_EXPECT(cmp_storage_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_shutdown(&storage), CMP_ERR_STATE);

  return 0;
}

static int test_storage_init_custom_allocator(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 1u;

  CMP_TEST_OK(cmp_storage_init(&storage, &config));
  CMP_TEST_OK(cmp_storage_shutdown(&storage));
  return 0;
}

static int test_storage_put_errors(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  cmp_usize max_value;

  CMP_TEST_EXPECT(cmp_storage_put(NULL, "a", 1u, "b", 1u, CMP_FALSE),
                 CMP_ERR_INVALID_ARGUMENT);

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "a", 1u, "b", 1u, CMP_FALSE),
                 CMP_ERR_STATE);

  CMP_TEST_OK(test_alloc_reset(&test_alloc));
  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 2u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));

  CMP_TEST_EXPECT(cmp_storage_put(&storage, "a", 1u, "b", 1u, 2),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "a", 0u, "b", 1u, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "a", 1u, NULL, 1u, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);

  max_value = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "a", max_value, "b", 1u, CMP_TRUE),
                 CMP_ERR_OVERFLOW);

  CMP_TEST_OK(cmp_storage_put(&storage, "alpha", 5u, "one", 3u, CMP_FALSE));

  test_alloc.alloc_calls = 0;
  test_alloc.fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "alpha", 5u, "two", 3u, CMP_TRUE),
                 CMP_ERR_OUT_OF_MEMORY);
  test_alloc.fail_alloc_on_call = 0;

  test_alloc.free_calls = 0;
  test_alloc.fail_free_on_call = 1;
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "alpha", 5u, "two", 3u, CMP_TRUE),
                 CMP_ERR_IO);
  test_alloc.fail_free_on_call = 0;

  storage.entry_capacity = 1u;
  storage.allocator.realloc = NULL;
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "beta", 4u, "two", 3u, CMP_FALSE),
                 CMP_ERR_INVALID_ARGUMENT);
  storage.allocator.realloc = test_realloc_fn;
  storage.entry_capacity = 2u;

  test_alloc.free_calls = 0;
  test_alloc.fail_free_on_call = 1;
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "alpha", 5u, NULL, 0u, CMP_TRUE),
                 CMP_ERR_IO);
  test_alloc.fail_free_on_call = 0;

  CMP_TEST_OK(cmp_storage_shutdown(&storage));
  return 0;
}

static int test_storage_access_errors(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  CMPBool exists;
  cmp_usize value_size;
  char buffer[8];

  memset(&storage, 0, sizeof(storage));

  CMP_TEST_EXPECT(
      cmp_storage_get(NULL, "a", 1u, buffer, sizeof(buffer), &value_size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_storage_get(&storage, "a", 1u, buffer, sizeof(buffer), &value_size),
      CMP_ERR_STATE);
  CMP_TEST_EXPECT(
      cmp_storage_get(&storage, "a", 1u, buffer, sizeof(buffer), NULL),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_storage_contains(NULL, "a", 1u, &exists),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_contains(&storage, "a", 1u, &exists), CMP_ERR_STATE);

  CMP_TEST_EXPECT(cmp_storage_remove(NULL, "a", 1u), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_remove(&storage, "a", 1u), CMP_ERR_STATE);

  CMP_TEST_EXPECT(cmp_storage_clear(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_clear(&storage), CMP_ERR_STATE);

  CMP_TEST_EXPECT(cmp_storage_count(NULL, &value_size), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_count(&storage, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_count(&storage, &value_size), CMP_ERR_STATE);

  CMP_TEST_OK(test_alloc_reset(&test_alloc));
  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));

  CMP_TEST_EXPECT(
      cmp_storage_get(&storage, NULL, 1u, buffer, sizeof(buffer), &value_size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_get(&storage, "a", 1u, NULL, 1u, &value_size),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_contains(&storage, NULL, 1u, &exists),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_contains(&storage, "a", 1u, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_remove(&storage, NULL, 1u),
                 CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_storage_shutdown(&storage));
  return 0;
}

static int test_storage_release_errors(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));
  CMP_TEST_OK(cmp_storage_put(&storage, "alpha", 5u, "one", 3u, CMP_FALSE));

  test_alloc.free_calls = 0;
  test_alloc.fail_free_on_call = 2;
  CMP_TEST_EXPECT(cmp_storage_clear(&storage), CMP_ERR_IO);
  test_alloc.fail_free_on_call = 0;

  CMP_TEST_OK(cmp_storage_put(&storage, "alpha", 5u, "one", 3u, CMP_FALSE));
  storage.allocator.free = NULL;
  CMP_TEST_EXPECT(cmp_storage_shutdown(&storage), CMP_ERR_INVALID_ARGUMENT);
  storage.allocator.free = test_free_fn;

  test_alloc.free_calls = 0;
  test_alloc.fail_free_on_call = 3;
  CMP_TEST_EXPECT(cmp_storage_shutdown(&storage), CMP_ERR_IO);
  test_alloc.fail_free_on_call = 0;

  return 0;
}

static int test_storage_grow_overflow(void) {
  CMPStorage storage;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  cmp_usize max_value;
  int rc;

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  storage.allocator = allocator;
  storage.entries = (CMPStorageEntry *)1;
  max_value = (cmp_usize) ~(cmp_usize)0;
  storage.entry_capacity = max_value / sizeof(CMPStorageEntry) + 1u;

  rc = cmp_storage_test_grow(&storage, storage.entry_capacity);
  CMP_TEST_EXPECT(rc, CMP_ERR_OVERFLOW);
  return 0;
}

static int test_storage_put_get_remove(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  CMPBool exists;
  cmp_usize value_size;
  cmp_usize count;
  char buffer[16];

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 2u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));

  CMP_TEST_OK(cmp_storage_put(&storage, "alpha", 5u, "one", 3u, CMP_FALSE));
  CMP_TEST_OK(cmp_storage_put(&storage, "beta", 4u, "two", 3u, CMP_FALSE));

  CMP_TEST_OK(cmp_storage_get(&storage, "alpha", 5u, buffer, sizeof(buffer),
                            &value_size));
  CMP_TEST_ASSERT(value_size == 3u);
  CMP_TEST_ASSERT(memcmp(buffer, "one", 3u) == 0);

  CMP_TEST_OK(cmp_storage_contains(&storage, "alpha", 5u, &exists));
  CMP_TEST_ASSERT(exists == CMP_TRUE);
  CMP_TEST_OK(cmp_storage_contains(&storage, "missing", 7u, &exists));
  CMP_TEST_ASSERT(exists == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_storage_get(&storage, "missing", 7u, buffer, sizeof(buffer),
                                &value_size),
                 CMP_ERR_NOT_FOUND);
  CMP_TEST_ASSERT(value_size == 0u);

  CMP_TEST_EXPECT(cmp_storage_get(&storage, "alpha", 5u, buffer, 2u, &value_size),
                 CMP_ERR_RANGE);
  CMP_TEST_ASSERT(value_size == 3u);

  CMP_TEST_OK(cmp_storage_count(&storage, &count));
  CMP_TEST_ASSERT(count == 2u);

  CMP_TEST_OK(cmp_storage_remove(&storage, "alpha", 5u));
  CMP_TEST_OK(cmp_storage_count(&storage, &count));
  CMP_TEST_ASSERT(count == 1u);
  CMP_TEST_EXPECT(cmp_storage_remove(&storage, "alpha", 5u), CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(cmp_storage_clear(&storage));
  CMP_TEST_OK(cmp_storage_count(&storage, &count));
  CMP_TEST_ASSERT(count == 0u);

  CMP_TEST_OK(cmp_storage_shutdown(&storage));
  return 0;
}

static int test_storage_overwrite_and_grow(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  cmp_usize count;
  cmp_usize value_size;
  char buffer[8];
  cmp_usize max_value;

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));

  CMP_TEST_OK(cmp_storage_put(&storage, "dup", 3u, "one", 3u, CMP_FALSE));
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "dup", 3u, "two", 3u, CMP_FALSE),
                 CMP_ERR_BUSY);

  CMP_TEST_OK(cmp_storage_put(&storage, "dup", 3u, "two", 3u, CMP_TRUE));
  CMP_TEST_OK(
      cmp_storage_get(&storage, "dup", 3u, buffer, sizeof(buffer), &value_size));
  CMP_TEST_ASSERT(value_size == 3u);
  CMP_TEST_ASSERT(memcmp(buffer, "two", 3u) == 0);

  CMP_TEST_OK(cmp_storage_put(&storage, "dup", 3u, NULL, 0u, CMP_TRUE));
  CMP_TEST_OK(
      cmp_storage_get(&storage, "dup", 3u, buffer, sizeof(buffer), &value_size));
  CMP_TEST_ASSERT(value_size == 0u);

  CMP_TEST_OK(cmp_storage_put(&storage, "grow", 4u, "x", 1u, CMP_FALSE));
  CMP_TEST_OK(cmp_storage_count(&storage, &count));
  CMP_TEST_ASSERT(count == 2u);
  CMP_TEST_ASSERT(storage.entry_capacity >= 2u);

  test_alloc.realloc_calls = 0;
  test_alloc.fail_realloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "fail", 4u, "y", 1u, CMP_FALSE),
                 CMP_ERR_OUT_OF_MEMORY);
  test_alloc.fail_realloc_on_call = 0;

  max_value = (cmp_usize) ~(cmp_usize)0;
  storage.entry_capacity = max_value / 2u + 1u;
  CMP_TEST_EXPECT(cmp_storage_test_grow(&storage, storage.entry_capacity + 1u),
                 CMP_ERR_OVERFLOW);
  storage.entry_capacity = 2u;

  CMP_TEST_OK(cmp_storage_shutdown(&storage));
  return 0;
}

static int test_storage_alloc_failures(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 2u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));

  test_alloc.alloc_calls = 0;
  test_alloc.fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "key", 3u, NULL, 0u, CMP_FALSE),
                 CMP_ERR_OUT_OF_MEMORY);
  test_alloc.fail_alloc_on_call = 0;

  test_alloc.alloc_calls = 0;
  test_alloc.alloc_calls = 0;
  test_alloc.alloc_calls = 0;
  test_alloc.alloc_calls = 0;
  test_alloc.alloc_calls = 0;
  test_alloc.fail_alloc_on_call = 2;
  CMP_TEST_EXPECT(cmp_storage_put(&storage, "key", 3u, "val", 3u, CMP_FALSE),
                 CMP_ERR_OUT_OF_MEMORY);
  test_alloc.fail_alloc_on_call = 0;

  CMP_TEST_OK(cmp_storage_shutdown(&storage));
  return 0;
}

static int test_storage_free_failures(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  int rc;

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 2u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));

  CMP_TEST_OK(cmp_storage_put(&storage, "free", 4u, "data", 4u, CMP_FALSE));

  test_alloc.free_calls = 0;
  test_alloc.free_calls = 0;
  test_alloc.fail_free_on_call = 1;
  rc = cmp_storage_remove(&storage, "free", 4u);
  CMP_TEST_ASSERT(rc != CMP_OK);
  test_alloc.fail_free_on_call = 0;

  CMP_TEST_OK(cmp_storage_put(&storage, "free", 4u, "data", 4u, CMP_FALSE));
  test_alloc.free_calls = 0;
  test_alloc.fail_free_on_call = 1;
  rc = cmp_storage_shutdown(&storage);
  CMP_TEST_ASSERT(rc != CMP_OK);

  memset(&storage, 0, sizeof(storage));
  return 0;
}

static int test_storage_save_load_roundtrip(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  TestIO test_io;
  CMPIO io;
  cmp_u8 buffer[256];
  cmp_usize value_size;
  char value[8];

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));
  CMP_TEST_OK(test_io_reset(&test_io, buffer, sizeof(buffer)));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  io.ctx = &test_io;
  io.vtable = &g_test_io_vtable;

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 2u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));

  CMP_TEST_OK(cmp_storage_put(&storage, "a", 1u, "1", 1u, CMP_FALSE));
  CMP_TEST_OK(cmp_storage_put(&storage, "b", 1u, "2", 1u, CMP_FALSE));

  CMP_TEST_OK(cmp_storage_save(&storage, &io, "mem"));
  CMP_TEST_ASSERT(test_io.size > 0u);
  CMP_TEST_ASSERT(test_io.buffer[0] == 'M');
  CMP_TEST_ASSERT(test_io.buffer[1] == '3');
  CMP_TEST_ASSERT(test_io.buffer[2] == 'S');
  CMP_TEST_ASSERT(test_io.buffer[3] == 'T');

  CMP_TEST_OK(cmp_storage_clear(&storage));
  CMP_TEST_OK(cmp_storage_load(&storage, &io, "mem"));

  CMP_TEST_OK(
      cmp_storage_get(&storage, "a", 1u, value, sizeof(value), &value_size));
  CMP_TEST_ASSERT(value_size == 1u);
  CMP_TEST_ASSERT(value[0] == '1');

  CMP_TEST_OK(
      cmp_storage_get(&storage, "b", 1u, value, sizeof(value), &value_size));
  CMP_TEST_ASSERT(value_size == 1u);
  CMP_TEST_ASSERT(value[0] == '2');

  CMP_TEST_OK(cmp_storage_shutdown(&storage));
  return 0;
}

static int test_storage_save_errors(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  TestIO test_io;
  CMPIO io;
  cmp_u8 buffer[256];
  cmp_usize max_u32;
  char *saved_key;
  cmp_usize saved_key_len;
  cmp_usize saved_value_len;

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));
  CMP_TEST_OK(test_io_reset(&test_io, buffer, sizeof(buffer)));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  io.ctx = &test_io;
  io.vtable = &g_test_io_vtable;

  CMP_TEST_EXPECT(cmp_storage_save(NULL, &io, "mem"), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_save(&storage, NULL, "mem"),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"), CMP_ERR_STATE);

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));

  io.vtable = NULL;
  CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"),
                 CMP_ERR_INVALID_ARGUMENT);
  io.vtable = &g_test_io_vtable;

  storage.allocator.alloc = NULL;
  CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"),
                 CMP_ERR_INVALID_ARGUMENT);
  storage.allocator.alloc = test_alloc_fn;

  CMP_TEST_OK(cmp_storage_put(&storage, "k", 1u, "v", 1u, CMP_FALSE));
  saved_key = storage.entries[0].key;
  saved_key_len = storage.entries[0].key_len;
  saved_value_len = storage.entries[0].value_len;
  storage.entries[0].key = NULL;
  CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"), CMP_ERR_STATE);
  storage.entries[0].key = saved_key;

  max_u32 = (cmp_usize)((cmp_u32) ~(cmp_u32)0);
  if (max_u32 < (cmp_usize) ~(cmp_usize)0) {
    storage.entry_count = max_u32 + 1u;
    CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"), CMP_ERR_RANGE);
    storage.entry_count = 1u;

    storage.entries[0].key_len = max_u32 + 1u;
    CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"), CMP_ERR_RANGE);
    storage.entries[0].key_len = saved_key_len;

    storage.entries[0].value_len = max_u32 + 1u;
    CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"), CMP_ERR_RANGE);
    storage.entries[0].value_len = saved_value_len;
  }

  if (max_u32 < (cmp_usize) ~(cmp_usize)0) {
    test_alloc.mutate_on_alloc = 1;
    test_alloc.mutate_target = &storage.entries[0].key_len;
    test_alloc.mutate_value = max_u32 + 1u;
    CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"), CMP_ERR_RANGE);
    storage.entries[0].key_len = saved_key_len;
    test_alloc.mutate_target = NULL;

    test_alloc.mutate_on_alloc = 1;
    test_alloc.mutate_target = &storage.entries[0].value_len;
    test_alloc.mutate_value = max_u32 + 1u;
    CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"), CMP_ERR_RANGE);
    storage.entries[0].value_len = saved_value_len;
    test_alloc.mutate_target = NULL;
  }

  test_alloc.alloc_calls = 0;
  test_alloc.fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"), CMP_ERR_OUT_OF_MEMORY);
  test_alloc.fail_alloc_on_call = 0;

  test_io.fail_write = 1;
  CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"), CMP_ERR_IO);
  test_io.fail_write = 0;

  test_alloc.free_calls = 0;
  test_alloc.fail_free_on_call = 1;
  CMP_TEST_EXPECT(cmp_storage_save(&storage, &io, "mem"), CMP_ERR_IO);
  test_alloc.fail_free_on_call = 0;

  CMP_TEST_OK(cmp_storage_shutdown(&storage));
  return 0;
}

static int test_storage_load_errors(void) {
  CMPStorage storage;
  CMPStorageConfig config;
  CMPAllocator allocator;
  TestAlloc test_alloc;
  TestIO test_io;
  CMPIO io;
  cmp_u8 buffer[256];
  cmp_u8 small[8];
  int rc;

  memset(&storage, 0, sizeof(storage));
  CMP_TEST_OK(test_alloc_reset(&test_alloc));
  CMP_TEST_OK(test_io_reset(&test_io, buffer, sizeof(buffer)));

  allocator.ctx = &test_alloc;
  allocator.alloc = test_alloc_fn;
  allocator.realloc = test_realloc_fn;
  allocator.free = test_free_fn;

  io.ctx = &test_io;
  io.vtable = &g_test_io_vtable;

  CMP_TEST_EXPECT(cmp_storage_load(NULL, &io, "mem"), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_load(&storage, NULL, "mem"),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"), CMP_ERR_STATE);

  CMP_TEST_OK(cmp_storage_config_init(&config));
  config.allocator = &allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_storage_init(&storage, &config));

  io.vtable = NULL;
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"),
                 CMP_ERR_INVALID_ARGUMENT);
  io.vtable = &g_test_io_vtable;

  storage.allocator.alloc = NULL;
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"),
                 CMP_ERR_INVALID_ARGUMENT);
  storage.allocator.alloc = test_alloc_fn;

  test_io.fail_read_alloc = 1;
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"), CMP_ERR_IO);
  test_io.fail_read_alloc = 0;

  test_io.size = sizeof(small);
  memset(small, 0, sizeof(small));
  memcpy(test_io.buffer, small, sizeof(small));
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"), CMP_ERR_CORRUPT);

  memset(test_io.buffer, 0, sizeof(buffer));
  test_io.size = 12u;
  test_io.buffer[0] = 'X';
  test_io.buffer[1] = '3';
  test_io.buffer[2] = 'S';
  test_io.buffer[3] = 'T';
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 8, 0u));
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"), CMP_ERR_CORRUPT);

  test_io.buffer[0] = 'M';
  test_io.buffer[1] = '3';
  test_io.buffer[2] = 'S';
  test_io.buffer[3] = 'T';
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 4, 2u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 8, 0u));
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"), CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 8, 1u));
  test_io.size = 12u;
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"), CMP_ERR_CORRUPT);

  test_io.size = 20u;
  memset(test_io.buffer, 0, 20u);
  test_io.buffer[0] = 'M';
  test_io.buffer[1] = '3';
  test_io.buffer[2] = 'S';
  test_io.buffer[3] = 'T';
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 8, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 12, 0u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 16, 0u));
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"), CMP_ERR_CORRUPT);

  test_io.size = 20u;
  memset(test_io.buffer, 0, 20u);
  test_io.buffer[0] = 'M';
  test_io.buffer[1] = '3';
  test_io.buffer[2] = 'S';
  test_io.buffer[3] = 'T';
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 8, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 12, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 16, 5u));
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"), CMP_ERR_CORRUPT);

  test_io.size = 32u;
  memset(test_io.buffer, 0, 32u);
  test_io.buffer[0] = 'M';
  test_io.buffer[1] = '3';
  test_io.buffer[2] = 'S';
  test_io.buffer[3] = 'T';
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 8, 2u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 12, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 16, 1u));
  test_io.buffer[20] = 'a';
  test_io.buffer[21] = 'b';
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 22, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 26, 1u));
  test_io.buffer[30] = 'a';
  test_io.buffer[31] = 'c';
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"), CMP_ERR_BUSY);

  test_alloc.alloc_calls = 0;
  test_alloc.fail_alloc_on_call = 2;
  test_io.size = 12u;
  memset(test_io.buffer, 0, 12u);
  test_io.buffer[0] = 'M';
  test_io.buffer[1] = '3';
  test_io.buffer[2] = 'S';
  test_io.buffer[3] = 'T';
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 8, 0u));
  CMP_TEST_EXPECT(cmp_storage_load(&storage, &io, "mem"), CMP_ERR_OUT_OF_MEMORY);
  test_alloc.fail_alloc_on_call = 0;

  test_io.size = 16u;
  memset(test_io.buffer, 0, 16u);
  test_io.buffer[0] = 'M';
  test_io.buffer[1] = '3';
  test_io.buffer[2] = 'S';
  test_io.buffer[3] = 'T';
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 4, 1u));
  CMP_TEST_OK(test_write_u32_le(test_io.buffer + 8, 0u));
  test_alloc.free_calls = 0;
  test_alloc.fail_free_on_call = 1;
  rc = cmp_storage_load(&storage, &io, "mem");
  CMP_TEST_ASSERT(rc != CMP_OK);
  test_alloc.fail_free_on_call = 0;

  CMP_TEST_OK(cmp_storage_shutdown(&storage));
  return 0;
}

int main(void) {
  CMP_TEST_ASSERT(test_storage_helpers() == 0);
  CMP_TEST_ASSERT(test_storage_config_init() == 0);
  CMP_TEST_ASSERT(test_storage_init_errors() == 0);
  CMP_TEST_ASSERT(test_storage_init_shutdown() == 0);
  CMP_TEST_ASSERT(test_storage_init_custom_allocator() == 0);
  CMP_TEST_ASSERT(test_storage_put_errors() == 0);
  CMP_TEST_ASSERT(test_storage_access_errors() == 0);
  CMP_TEST_ASSERT(test_storage_release_errors() == 0);
  CMP_TEST_ASSERT(test_storage_grow_overflow() == 0);
  CMP_TEST_ASSERT(test_storage_put_get_remove() == 0);
  CMP_TEST_ASSERT(test_storage_overwrite_and_grow() == 0);
  CMP_TEST_ASSERT(test_storage_alloc_failures() == 0);
  CMP_TEST_ASSERT(test_storage_free_failures() == 0);
  CMP_TEST_ASSERT(test_storage_save_load_roundtrip() == 0);
  CMP_TEST_ASSERT(test_storage_save_errors() == 0);
  CMP_TEST_ASSERT(test_storage_load_errors() == 0);
  return 0;
}
