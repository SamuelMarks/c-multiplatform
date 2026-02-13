#include "cmpc/cmp_object.h"

#include <string.h>

#ifdef CMP_TESTING
static CMPBool g_cmp_handle_is_valid_fail = CMP_FALSE;
#endif

#define CMP_HANDLE_INDEX_INVALID ((cmp_u32) ~(cmp_u32)0)

typedef struct CMPHandleSlot {
  CMPObjectHeader *obj;
  cmp_u32 generation;
  cmp_u32 next_free;
} CMPHandleSlot;

typedef struct CMPHandleSystemImpl {
  CMPAllocator allocator;
  cmp_usize capacity;
  cmp_usize live_count;
  CMPHandleSlot *slots;
  cmp_u32 free_head;
} CMPHandleSystemImpl;

static cmp_u32 cmp_u32_max_value(void) { return (cmp_u32) ~(cmp_u32)0; }

static int cmp_handle_system_resolve_impl(CMPHandleSystemImpl *impl,
                                          CMPHandle handle, void **out_obj) {
  CMPHandleSlot *slot;
  CMPBool valid;
  cmp_u32 index;
  int rc;

  if (impl == NULL || out_obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_obj = NULL;

  if (impl->slots == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_handle_is_valid(handle, &valid);
  if (rc != CMP_OK) {
    return rc;
  }
  if (!valid) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  index = handle.id - 1;
  if ((cmp_usize)index >= impl->capacity) {
    return CMP_ERR_NOT_FOUND;
  }

  slot = &impl->slots[index];
  if (slot->obj == NULL || slot->generation != handle.generation) {
    return CMP_ERR_NOT_FOUND;
  }

  *out_obj = slot->obj;
  return CMP_OK;
}

static int CMP_CALL cmp_handle_system_default_init(void *sys,
                                                   cmp_usize capacity) {
  CMPHandleSystemImpl *impl;
  CMPHandleSlot *slots;
  cmp_usize max_value;
  cmp_usize i;
  int rc;

  if (sys == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  impl = (CMPHandleSystemImpl *)sys;
  if (capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  max_value = (cmp_usize) ~(cmp_usize)0;
  if (capacity > max_value / sizeof(CMPHandleSlot)) {
    return CMP_ERR_OVERFLOW;
  }
  if (capacity > (cmp_usize)cmp_u32_max_value()) {
    return CMP_ERR_RANGE;
  }
  if (impl->slots != NULL) {
    return CMP_ERR_STATE;
  }
  if (impl->allocator.alloc == NULL || impl->allocator.realloc == NULL ||
      impl->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = impl->allocator.alloc(impl->allocator.ctx,
                             sizeof(CMPHandleSlot) * capacity, (void **)&slots);
  if (rc != CMP_OK) {
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
      slots[i].next_free = (cmp_u32)(i + 1);
    } else {
      slots[i].next_free = CMP_HANDLE_INDEX_INVALID;
    }
  }

  return CMP_OK;
}

static int CMP_CALL cmp_handle_system_default_shutdown(void *sys) {
  CMPHandleSystemImpl *impl;
  int rc;

  if (sys == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  impl = (CMPHandleSystemImpl *)sys;
  if (impl->slots == NULL) {
    return CMP_ERR_STATE;
  }
  if (impl->live_count != 0) {
    return CMP_ERR_BUSY;
  }

  rc = impl->allocator.free(impl->allocator.ctx, impl->slots);
  if (rc != CMP_OK) {
    return rc;
  }

  impl->slots = NULL;
  impl->capacity = 0;
  impl->free_head = CMP_HANDLE_INDEX_INVALID;
  impl->live_count = 0;
  return CMP_OK;
}

static int CMP_CALL
cmp_handle_system_default_register_object(void *sys, CMPObjectHeader *obj) {
  CMPHandleSystemImpl *impl;
  CMPHandleSlot *slot;
  CMPBool valid;
  cmp_u32 index;
  int rc;

  if (sys == NULL || obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  impl = (CMPHandleSystemImpl *)sys;
  if (impl->slots == NULL) {
    return CMP_ERR_STATE;
  }
  if (obj->vtable == NULL || obj->vtable->retain == NULL ||
      obj->vtable->release == NULL || obj->vtable->destroy == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_handle_is_valid(obj->handle, &valid);
  if (rc != CMP_OK) {
    return rc;
  }
  if (valid) {
    return CMP_ERR_STATE;
  }

  if (impl->free_head == CMP_HANDLE_INDEX_INVALID) {
    return CMP_ERR_BUSY;
  }

  index = impl->free_head;
  slot = &impl->slots[index];
  impl->free_head = slot->next_free;

  slot->obj = obj;
  obj->handle.id = index + 1;
  obj->handle.generation = slot->generation;
  impl->live_count += 1;

  return CMP_OK;
}

static int CMP_CALL
cmp_handle_system_default_unregister_object(void *sys, CMPHandle handle) {
  CMPHandleSystemImpl *impl;
  CMPHandleSlot *slot;
  CMPObjectHeader *obj;
  CMPBool valid;
  cmp_u32 index;
  int rc;

  if (sys == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  impl = (CMPHandleSystemImpl *)sys;
  if (impl->slots == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_handle_is_valid(handle, &valid);
  if (rc != CMP_OK) {
    return rc;
  }
  if (!valid) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  index = handle.id - 1;
  if ((cmp_usize)index >= impl->capacity) {
    return CMP_ERR_NOT_FOUND;
  }

  slot = &impl->slots[index];
  if (slot->obj == NULL || slot->generation != handle.generation) {
    return CMP_ERR_NOT_FOUND;
  }

  if (impl->live_count == 0) {
    return CMP_ERR_STATE;
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

  return CMP_OK;
}

static int CMP_CALL cmp_handle_system_default_resolve(void *sys,
                                                      CMPHandle handle,
                                                      void **out_obj) {
  return cmp_handle_system_resolve_impl((CMPHandleSystemImpl *)sys, handle,
                                        out_obj);
}

static int CMP_CALL cmp_handle_system_default_retain(void *sys,
                                                     CMPHandle handle) {
  CMPObjectHeader *obj;
  void *resolved;
  int rc;

  if (sys == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_handle_system_resolve_impl((CMPHandleSystemImpl *)sys, handle,
                                      &resolved);
  if (rc != CMP_OK) {
    return rc;
  }

  obj = (CMPObjectHeader *)resolved;
  if (obj->vtable == NULL || obj->vtable->retain == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  return obj->vtable->retain(obj);
}

static int CMP_CALL cmp_handle_system_default_release(void *sys,
                                                      CMPHandle handle) {
  CMPObjectHeader *obj;
  void *resolved;
  int rc;

  if (sys == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_handle_system_resolve_impl((CMPHandleSystemImpl *)sys, handle,
                                      &resolved);
  if (rc != CMP_OK) {
    return rc;
  }

  obj = (CMPObjectHeader *)resolved;
  if (obj->vtable == NULL || obj->vtable->release == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  return obj->vtable->release(obj);
}

static const CMPHandleSystemVTable cmp_handle_system_default_vtable = {
    cmp_handle_system_default_init,
    cmp_handle_system_default_shutdown,
    cmp_handle_system_default_register_object,
    cmp_handle_system_default_unregister_object,
    cmp_handle_system_default_resolve,
    cmp_handle_system_default_retain,
    cmp_handle_system_default_release};

int CMP_CALL cmp_handle_is_valid(CMPHandle handle, CMPBool *out_valid) {
  if (out_valid == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_handle_is_valid_fail) {
    return CMP_ERR_UNKNOWN;
  }
#endif

  if (handle.id == 0 || handle.generation == 0) {
    *out_valid = CMP_FALSE;
  } else {
    *out_valid = CMP_TRUE;
  }

  return CMP_OK;
}

int CMP_CALL cmp_object_header_init(CMPObjectHeader *obj, cmp_u32 type_id,
                                    cmp_u32 flags,
                                    const CMPObjectVTable *vtable) {
  if (obj == NULL || vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (vtable->retain == NULL || vtable->release == NULL ||
      vtable->destroy == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  obj->handle.id = 0;
  obj->handle.generation = 0;
  obj->type_id = type_id;
  obj->flags = flags;
  obj->ref_count = 1;
  obj->vtable = vtable;
  return CMP_OK;
}

int CMP_CALL cmp_object_retain(CMPObjectHeader *obj) {
  if (obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (obj->ref_count == 0) {
    return CMP_ERR_STATE;
  }

  if (obj->ref_count == cmp_u32_max_value()) {
    return CMP_ERR_OVERFLOW;
  }

  obj->ref_count += 1;
  return CMP_OK;
}

int CMP_CALL cmp_object_release(CMPObjectHeader *obj) {
  int rc;

  if (obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (obj->ref_count == 0) {
    return CMP_ERR_STATE;
  }

  if (obj->ref_count > 1) {
    obj->ref_count -= 1;
    return CMP_OK;
  }

  if (obj->vtable == NULL || obj->vtable->destroy == NULL) {
    return CMP_ERR_STATE;
  }

  obj->ref_count = 0;
  rc = obj->vtable->destroy(obj);
  if (rc != CMP_OK) {
    obj->ref_count = 1;
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_object_get_type_id(const CMPObjectHeader *obj,
                                    cmp_u32 *out_type_id) {
  if (obj == NULL || out_type_id == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_type_id = obj->type_id;
  return CMP_OK;
}

int CMP_CALL cmp_handle_system_default_create(cmp_usize capacity,
                                              const CMPAllocator *allocator,
                                              CMPHandleSystem *out_sys) {
  CMPAllocator default_alloc;
  CMPHandleSystemImpl *impl;
  int rc;

  if (out_sys == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  out_sys->ctx = NULL;
  out_sys->vtable = NULL;

  if (capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (allocator == NULL) {
    rc = cmp_get_default_allocator(&default_alloc);
    if (rc != CMP_OK) {
      return rc;
    }
    allocator = &default_alloc;
  }

  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = allocator->alloc(allocator->ctx, sizeof(CMPHandleSystemImpl),
                        (void **)&impl);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(impl, 0, sizeof(CMPHandleSystemImpl));
  impl->allocator = *allocator;
  impl->free_head = CMP_HANDLE_INDEX_INVALID;

  rc = cmp_handle_system_default_init(impl, capacity);
  if (rc != CMP_OK) {
    allocator->free(allocator->ctx, impl);
    return rc;
  }

  out_sys->ctx = impl;
  out_sys->vtable = &cmp_handle_system_default_vtable;
  return CMP_OK;
}

int CMP_CALL cmp_handle_system_default_destroy(CMPHandleSystem *sys) {
  CMPHandleSystemImpl *impl;
  CMPAllocator allocator;
  int rc;

  if (sys == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (sys->ctx == NULL || sys->vtable == NULL) {
    return CMP_ERR_STATE;
  }

  impl = (CMPHandleSystemImpl *)sys->ctx;
  rc = sys->vtable->shutdown(impl);
  if (rc != CMP_OK) {
    return rc;
  }

  allocator = impl->allocator;
  rc = allocator.free(allocator.ctx, impl);
  if (rc != CMP_OK) {
    return rc;
  }

  sys->ctx = NULL;
  sys->vtable = NULL;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_object_test_set_handle_is_valid_fail(CMPBool fail) {
  g_cmp_handle_is_valid_fail = fail ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif
