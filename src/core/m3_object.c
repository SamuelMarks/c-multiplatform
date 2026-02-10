#include "m3/m3_object.h"

#include <string.h>

#ifdef M3_TESTING
static M3Bool g_m3_handle_is_valid_fail = M3_FALSE;
#endif

#define M3_HANDLE_INDEX_INVALID ((m3_u32) ~(m3_u32)0)

typedef struct M3HandleSlot {
  M3ObjectHeader *obj;
  m3_u32 generation;
  m3_u32 next_free;
} M3HandleSlot;

typedef struct M3HandleSystemImpl {
  M3Allocator allocator;
  m3_usize capacity;
  m3_usize live_count;
  M3HandleSlot *slots;
  m3_u32 free_head;
} M3HandleSystemImpl;

static m3_u32 m3_u32_max_value(void) { return (m3_u32) ~(m3_u32)0; }

static int m3_handle_system_resolve_impl(M3HandleSystemImpl *impl,
                                         M3Handle handle, void **out_obj) {
  M3HandleSlot *slot;
  M3Bool valid;
  m3_u32 index;
  int rc;

  if (impl == NULL || out_obj == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_obj = NULL;

  if (impl->slots == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_handle_is_valid(handle, &valid);
  if (rc != M3_OK) {
    return rc;
  }
  if (!valid) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  index = handle.id - 1;
  if ((m3_usize)index >= impl->capacity) {
    return M3_ERR_NOT_FOUND;
  }

  slot = &impl->slots[index];
  if (slot->obj == NULL || slot->generation != handle.generation) {
    return M3_ERR_NOT_FOUND;
  }

  *out_obj = slot->obj;
  return M3_OK;
}

static int M3_CALL m3_handle_system_default_init(void *sys, m3_usize capacity) {
  M3HandleSystemImpl *impl;
  M3HandleSlot *slots;
  m3_usize max_value;
  m3_usize i;
  int rc;

  if (sys == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  impl = (M3HandleSystemImpl *)sys;
  if (capacity == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  max_value = (m3_usize) ~(m3_usize)0;
  if (capacity > max_value / sizeof(M3HandleSlot)) {
    return M3_ERR_OVERFLOW;
  }
  if (capacity > (m3_usize)m3_u32_max_value()) {
    return M3_ERR_RANGE;
  }
  if (impl->slots != NULL) {
    return M3_ERR_STATE;
  }
  if (impl->allocator.alloc == NULL || impl->allocator.realloc == NULL ||
      impl->allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = impl->allocator.alloc(impl->allocator.ctx,
                             sizeof(M3HandleSlot) * capacity, (void **)&slots);
  if (rc != M3_OK) {
    return rc;
  }

  impl->slots = slots;
  impl->capacity = capacity;
  impl->live_count = 0;
  impl->free_head = 0;

  for (i = 0; i < capacity; ++i) {
    slots[i].obj = NULL;
    slots[i].generation = 1;
    if (i + 1 < capacity) {
      slots[i].next_free = (m3_u32)(i + 1);
    } else {
      slots[i].next_free = M3_HANDLE_INDEX_INVALID;
    }
  }

  return M3_OK;
}

static int M3_CALL m3_handle_system_default_shutdown(void *sys) {
  M3HandleSystemImpl *impl;
  int rc;

  if (sys == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  impl = (M3HandleSystemImpl *)sys;
  if (impl->slots == NULL) {
    return M3_ERR_STATE;
  }
  if (impl->live_count != 0) {
    return M3_ERR_BUSY;
  }

  rc = impl->allocator.free(impl->allocator.ctx, impl->slots);
  if (rc != M3_OK) {
    return rc;
  }

  impl->slots = NULL;
  impl->capacity = 0;
  impl->free_head = M3_HANDLE_INDEX_INVALID;
  impl->live_count = 0;
  return M3_OK;
}

static int M3_CALL
m3_handle_system_default_register_object(void *sys, M3ObjectHeader *obj) {
  M3HandleSystemImpl *impl;
  M3HandleSlot *slot;
  M3Bool valid;
  m3_u32 index;
  int rc;

  if (sys == NULL || obj == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  impl = (M3HandleSystemImpl *)sys;
  if (impl->slots == NULL) {
    return M3_ERR_STATE;
  }
  if (obj->vtable == NULL || obj->vtable->retain == NULL ||
      obj->vtable->release == NULL || obj->vtable->destroy == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_handle_is_valid(obj->handle, &valid);
  if (rc != M3_OK) {
    return rc;
  }
  if (valid) {
    return M3_ERR_STATE;
  }

  if (impl->free_head == M3_HANDLE_INDEX_INVALID) {
    return M3_ERR_BUSY;
  }

  index = impl->free_head;
  slot = &impl->slots[index];
  impl->free_head = slot->next_free;

  slot->obj = obj;
  obj->handle.id = index + 1;
  obj->handle.generation = slot->generation;
  impl->live_count += 1;

  return M3_OK;
}

static int M3_CALL m3_handle_system_default_unregister_object(void *sys,
                                                              M3Handle handle) {
  M3HandleSystemImpl *impl;
  M3HandleSlot *slot;
  M3ObjectHeader *obj;
  M3Bool valid;
  m3_u32 index;
  int rc;

  if (sys == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  impl = (M3HandleSystemImpl *)sys;
  if (impl->slots == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_handle_is_valid(handle, &valid);
  if (rc != M3_OK) {
    return rc;
  }
  if (!valid) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  index = handle.id - 1;
  if ((m3_usize)index >= impl->capacity) {
    return M3_ERR_NOT_FOUND;
  }

  slot = &impl->slots[index];
  if (slot->obj == NULL || slot->generation != handle.generation) {
    return M3_ERR_NOT_FOUND;
  }

  if (impl->live_count == 0) {
    return M3_ERR_STATE;
  }

  obj = slot->obj;
  slot->obj = NULL;
  slot->generation += 1;
  if (slot->generation == 0) {
    slot->generation = 1;
  }
  slot->next_free = impl->free_head;
  impl->free_head = index;
  impl->live_count -= 1;

  if (obj != NULL) {
    obj->handle.id = 0;
    obj->handle.generation = 0;
  }

  return M3_OK;
}

static int M3_CALL m3_handle_system_default_resolve(void *sys, M3Handle handle,
                                                    void **out_obj) {
  return m3_handle_system_resolve_impl((M3HandleSystemImpl *)sys, handle,
                                       out_obj);
}

static int M3_CALL m3_handle_system_default_retain(void *sys, M3Handle handle) {
  M3ObjectHeader *obj;
  void *resolved;
  int rc;

  if (sys == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_handle_system_resolve_impl((M3HandleSystemImpl *)sys, handle,
                                     &resolved);
  if (rc != M3_OK) {
    return rc;
  }

  obj = (M3ObjectHeader *)resolved;
  if (obj->vtable == NULL || obj->vtable->retain == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  return obj->vtable->retain(obj);
}

static int M3_CALL m3_handle_system_default_release(void *sys,
                                                    M3Handle handle) {
  M3ObjectHeader *obj;
  void *resolved;
  int rc;

  if (sys == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_handle_system_resolve_impl((M3HandleSystemImpl *)sys, handle,
                                     &resolved);
  if (rc != M3_OK) {
    return rc;
  }

  obj = (M3ObjectHeader *)resolved;
  if (obj->vtable == NULL || obj->vtable->release == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  return obj->vtable->release(obj);
}

static const M3HandleSystemVTable m3_handle_system_default_vtable = {
    m3_handle_system_default_init,
    m3_handle_system_default_shutdown,
    m3_handle_system_default_register_object,
    m3_handle_system_default_unregister_object,
    m3_handle_system_default_resolve,
    m3_handle_system_default_retain,
    m3_handle_system_default_release};

int M3_CALL m3_handle_is_valid(M3Handle handle, M3Bool *out_valid) {
  if (out_valid == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (g_m3_handle_is_valid_fail) {
    return M3_ERR_UNKNOWN;
  }
#endif

  if (handle.id == 0 || handle.generation == 0) {
    *out_valid = M3_FALSE;
  } else {
    *out_valid = M3_TRUE;
  }

  return M3_OK;
}

int M3_CALL m3_object_header_init(M3ObjectHeader *obj, m3_u32 type_id,
                                  m3_u32 flags, const M3ObjectVTable *vtable) {
  if (obj == NULL || vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (vtable->retain == NULL || vtable->release == NULL ||
      vtable->destroy == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  obj->handle.id = 0;
  obj->handle.generation = 0;
  obj->type_id = type_id;
  obj->flags = flags;
  obj->ref_count = 1;
  obj->vtable = vtable;
  return M3_OK;
}

int M3_CALL m3_object_retain(M3ObjectHeader *obj) {
  if (obj == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (obj->ref_count == 0) {
    return M3_ERR_STATE;
  }

  if (obj->ref_count == m3_u32_max_value()) {
    return M3_ERR_OVERFLOW;
  }

  obj->ref_count += 1;
  return M3_OK;
}

int M3_CALL m3_object_release(M3ObjectHeader *obj) {
  int rc;

  if (obj == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (obj->ref_count == 0) {
    return M3_ERR_STATE;
  }

  if (obj->ref_count > 1) {
    obj->ref_count -= 1;
    return M3_OK;
  }

  if (obj->vtable == NULL || obj->vtable->destroy == NULL) {
    return M3_ERR_STATE;
  }

  obj->ref_count = 0;
  rc = obj->vtable->destroy(obj);
  if (rc != M3_OK) {
    obj->ref_count = 1;
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_object_get_type_id(const M3ObjectHeader *obj,
                                  m3_u32 *out_type_id) {
  if (obj == NULL || out_type_id == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_type_id = obj->type_id;
  return M3_OK;
}

int M3_CALL m3_handle_system_default_create(m3_usize capacity,
                                            const M3Allocator *allocator,
                                            M3HandleSystem *out_sys) {
  M3Allocator default_alloc;
  M3HandleSystemImpl *impl;
  int rc;

  if (out_sys == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  out_sys->ctx = NULL;
  out_sys->vtable = NULL;

  if (capacity == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (allocator == NULL) {
    rc = m3_get_default_allocator(&default_alloc);
    if (rc != M3_OK) {
      return rc;
    }
    allocator = &default_alloc;
  }

  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = allocator->alloc(allocator->ctx, sizeof(M3HandleSystemImpl),
                        (void **)&impl);
  if (rc != M3_OK) {
    return rc;
  }

  memset(impl, 0, sizeof(M3HandleSystemImpl));
  impl->allocator = *allocator;
  impl->free_head = M3_HANDLE_INDEX_INVALID;

  rc = m3_handle_system_default_init(impl, capacity);
  if (rc != M3_OK) {
    allocator->free(allocator->ctx, impl);
    return rc;
  }

  out_sys->ctx = impl;
  out_sys->vtable = &m3_handle_system_default_vtable;
  return M3_OK;
}

int M3_CALL m3_handle_system_default_destroy(M3HandleSystem *sys) {
  M3HandleSystemImpl *impl;
  M3Allocator allocator;
  int rc;

  if (sys == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (sys->ctx == NULL || sys->vtable == NULL) {
    return M3_ERR_STATE;
  }

  impl = (M3HandleSystemImpl *)sys->ctx;
  rc = sys->vtable->shutdown(impl);
  if (rc != M3_OK) {
    return rc;
  }

  allocator = impl->allocator;
  rc = allocator.free(allocator.ctx, impl);
  if (rc != M3_OK) {
    return rc;
  }

  sys->ctx = NULL;
  sys->vtable = NULL;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_object_test_set_handle_is_valid_fail(M3Bool fail) {
  g_m3_handle_is_valid_fail = fail ? M3_TRUE : M3_FALSE;
  return M3_OK;
}
#endif
