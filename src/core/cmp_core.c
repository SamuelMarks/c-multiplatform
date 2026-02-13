#include "cmpc/cmp_core.h"

#include <stdlib.h>

#ifdef CMP_TESTING
static CMPBool g_cmp_default_allocator_fail = CMP_FALSE;
#endif

static cmp_usize cmp_usize_max_value(void) { return (cmp_usize) ~(cmp_usize)0; }

static void *cmp_default_malloc(cmp_usize size) {
  if (size == cmp_usize_max_value()) {
    return NULL;
  }
  return malloc((size_t)size);
}

static void *cmp_default_realloc_impl(void *ptr, cmp_usize size) {
  if (size == cmp_usize_max_value()) {
    return NULL;
  }
  return realloc(ptr, (size_t)size);
}

static int CMP_CALL cmp_default_alloc(void *ctx, cmp_usize size,
                                      void **out_ptr) {
  void *mem;

  CMP_UNUSED(ctx);

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  mem = cmp_default_malloc(size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL cmp_default_realloc(void *ctx, void *ptr, cmp_usize size,
                                        void **out_ptr) {
  void *mem;

  CMP_UNUSED(ctx);

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  mem = cmp_default_realloc_impl(ptr, size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL cmp_default_free(void *ctx, void *ptr) {
  CMP_UNUSED(ctx);

  if (ptr == NULL) {
    return CMP_OK;
  }

  free(ptr);
  return CMP_OK;
}

int CMP_CALL cmp_get_default_allocator(CMPAllocator *out_alloc) {
  if (out_alloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_default_allocator_fail) {
    return CMP_ERR_UNKNOWN;
  }
#endif

  out_alloc->ctx = NULL;
  out_alloc->alloc = cmp_default_alloc;
  out_alloc->realloc = cmp_default_realloc;
  out_alloc->free = cmp_default_free;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_core_test_set_default_allocator_fail(CMPBool fail) {
  g_cmp_default_allocator_fail = fail ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif
