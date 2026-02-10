#include "m3/m3_core.h"

#include <stdlib.h>

#ifdef M3_TESTING
static M3Bool g_m3_default_allocator_fail = M3_FALSE;
#endif

static m3_usize m3_usize_max_value(void) { return (m3_usize) ~(m3_usize)0; }

static void *m3_default_malloc(m3_usize size) {
  if (size == m3_usize_max_value()) {
    return NULL;
  }
  return malloc((size_t)size);
}

static void *m3_default_realloc_impl(void *ptr, m3_usize size) {
  if (size == m3_usize_max_value()) {
    return NULL;
  }
  return realloc(ptr, (size_t)size);
}

static int M3_CALL m3_default_alloc(void *ctx, m3_usize size, void **out_ptr) {
  void *mem;

  M3_UNUSED(ctx);

  if (out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  mem = m3_default_malloc(size);
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = mem;
  return M3_OK;
}

static int M3_CALL m3_default_realloc(void *ctx, void *ptr, m3_usize size,
                                      void **out_ptr) {
  void *mem;

  M3_UNUSED(ctx);

  if (out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  mem = m3_default_realloc_impl(ptr, size);
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = mem;
  return M3_OK;
}

static int M3_CALL m3_default_free(void *ctx, void *ptr) {
  M3_UNUSED(ctx);

  if (ptr == NULL) {
    return M3_OK;
  }

  free(ptr);
  return M3_OK;
}

int M3_CALL m3_get_default_allocator(M3Allocator *out_alloc) {
  if (out_alloc == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (g_m3_default_allocator_fail) {
    return M3_ERR_UNKNOWN;
  }
#endif

  out_alloc->ctx = NULL;
  out_alloc->alloc = m3_default_alloc;
  out_alloc->realloc = m3_default_realloc;
  out_alloc->free = m3_default_free;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_core_test_set_default_allocator_fail(M3Bool fail) {
  g_m3_default_allocator_fail = fail ? M3_TRUE : M3_FALSE;
  return M3_OK;
}
#endif
