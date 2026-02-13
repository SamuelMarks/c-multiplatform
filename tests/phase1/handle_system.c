#include "cmpc/cmp_object.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct CMPHandleSlotInternal {
  CMPObjectHeader *obj;
  cmp_u32 generation;
  cmp_u32 next_free;
} CMPHandleSlotInternal;

typedef struct CMPHandleSystemImplInternal {
  CMPAllocator allocator;
  cmp_usize capacity;
  cmp_usize live_count;
  CMPHandleSlotInternal *slots;
  cmp_u32 free_head;
} CMPHandleSystemImplInternal;

typedef struct TestObject {
  CMPObjectHeader header;
  CMPHandleSystem *sys;
  int destroy_calls;
  int destroy_should_fail;
} TestObject;

static int CMP_CALL test_retain(void *obj) {
  return cmp_object_retain((CMPObjectHeader *)obj);
}

static int CMP_CALL test_release(void *obj) {
  return cmp_object_release((CMPObjectHeader *)obj);
}

static int CMP_CALL test_destroy(void *obj) {
  TestObject *test_obj;
  CMPBool valid;
  int rc;

  if (obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  test_obj = (TestObject *)obj;
  test_obj->destroy_calls += 1;

  if (test_obj->destroy_should_fail) {
    return CMP_ERR_UNKNOWN;
  }

  if (test_obj->sys == NULL) {
    return CMP_OK;
  }

  rc = cmp_handle_is_valid(test_obj->header.handle, &valid);
  if (rc != CMP_OK) {
    return rc;
  }

  if (!valid) {
    return CMP_OK;
  }

  rc = test_obj->sys->vtable->unregister_object(test_obj->sys->ctx,
                                                test_obj->header.handle);
  if (rc == CMP_ERR_NOT_FOUND) {
    return CMP_OK;
  }

  return rc;
}

static int CMP_CALL test_get_type_id(void *obj, cmp_u32 *out_type_id) {
  TestObject *test_obj;

  if (obj == NULL || out_type_id == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  test_obj = (TestObject *)obj;
  *out_type_id = test_obj->header.type_id;
  return CMP_OK;
}

static const CMPObjectVTable test_vtable = {test_retain, test_release,
                                           test_destroy, test_get_type_id};

static const CMPObjectVTable test_vtable_no_release = {
    test_retain, NULL, test_destroy, test_get_type_id};

static const CMPObjectVTable test_vtable_no_retain = {
    NULL, test_release, test_destroy, test_get_type_id};

static const CMPObjectVTable test_vtable_no_destroy = {test_retain, test_release,
                                                      NULL, test_get_type_id};

static int init_test_object(TestObject *obj, CMPHandleSystem *sys,
                            cmp_u32 type_id) {
  int rc;

  memset(obj, 0, sizeof(*obj));
  obj->sys = sys;
  obj->destroy_calls = 0;
  obj->destroy_should_fail = 0;
  rc = cmp_object_header_init(&obj->header, type_id, 0, &test_vtable);
  return rc;
}

static int CMP_CALL fail_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(size);

  if (out_ptr != NULL) {
    *out_ptr = NULL;
  }
  return CMP_ERR_OUT_OF_MEMORY;
}

static int CMP_CALL fail_realloc(void *ctx, void *ptr, cmp_usize size,
                                void **out_ptr) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(ptr);
  CMP_UNUSED(size);

  if (out_ptr != NULL) {
    *out_ptr = NULL;
  }
  return CMP_ERR_OUT_OF_MEMORY;
}

static int CMP_CALL fail_free(void *ctx, void *ptr) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(ptr);
  return CMP_OK;
}

typedef struct FreeFailCtx {
  int free_calls;
  int fail_on_call;
} FreeFailCtx;

static int CMP_CALL ok_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  CMP_UNUSED(ctx);

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_ptr = malloc((size_t)size);
  if (*out_ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  return CMP_OK;
}

static int CMP_CALL ok_realloc(void *ctx, void *ptr, cmp_usize size,
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

  mem = realloc(ptr, (size_t)size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL free_fail_on_call(void *ctx, void *ptr) {
  FreeFailCtx *state;
  int result;

  state = (FreeFailCtx *)ctx;
  result = CMP_OK;
  if (state != NULL) {
    state->free_calls += 1;
    if (state->fail_on_call > 0 && state->free_calls == state->fail_on_call) {
      result = CMP_ERR_IO;
    }
  }

  free(ptr);
  return result;
}

int main(void) {
  CMPHandleSystem sys;
  CMPHandleSystem sys_small;
  CMPHandleSystem fail_sys;
  CMPHandleSystem free_fail_sys;
  CMPHandleSystem overflow_sys;
  CMPHandleSystemImplInternal impl;
  CMPAllocator bad_alloc;
  CMPAllocator fail_allocator;
  CMPAllocator free_fail_allocator;
  FreeFailCtx free_fail_ctx;
  CMPHandle handle;
  CMPHandle obj2_handle;
  CMPBool valid;
  TestObject obj;
  TestObject obj2;
  TestObject obj3;
  TestObject obj_fail;
  CMPObjectHeader header;
  void *resolved;
  cmp_u32 type_id;
  cmp_u32 max_u32;
  cmp_usize max_usize;
  CMPHandleSystemImplInternal *sys_impl;
  cmp_u32 index;

  memset(&sys, 0, sizeof(sys));
  memset(&sys_small, 0, sizeof(sys_small));
  memset(&fail_sys, 0, sizeof(fail_sys));
  memset(&free_fail_sys, 0, sizeof(free_fail_sys));
  memset(&overflow_sys, 0, sizeof(overflow_sys));
  memset(&bad_alloc, 0, sizeof(bad_alloc));
  memset(&header, 0, sizeof(header));
  memset(&free_fail_ctx, 0, sizeof(free_fail_ctx));

  handle.id = 0;
  handle.generation = 0;

  CMP_TEST_EXPECT(cmp_handle_is_valid(handle, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_handle_is_valid(handle, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_object_header_init(NULL, 1, 0, &test_vtable),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_object_header_init(&header, 1, 0, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_object_header_init(&header, 1, 0, &test_vtable_no_release),
                 CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(init_test_object(&obj, NULL, 42));
  CMP_TEST_ASSERT(obj.header.ref_count == 1);
  CMP_TEST_OK(cmp_handle_is_valid(obj.header.handle, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_object_retain(NULL), CMP_ERR_INVALID_ARGUMENT);
  obj.header.ref_count = 0;
  CMP_TEST_EXPECT(cmp_object_retain(&obj.header), CMP_ERR_STATE);
  max_u32 = (cmp_u32) ~(cmp_u32)0;
  obj.header.ref_count = max_u32;
  CMP_TEST_EXPECT(cmp_object_retain(&obj.header), CMP_ERR_OVERFLOW);
  obj.header.ref_count = 1;
  CMP_TEST_OK(cmp_object_retain(&obj.header));
  CMP_TEST_ASSERT(obj.header.ref_count == 2);
  obj.header.ref_count = 1;

  CMP_TEST_EXPECT(cmp_object_release(NULL), CMP_ERR_INVALID_ARGUMENT);
  obj.header.ref_count = 0;
  CMP_TEST_EXPECT(cmp_object_release(&obj.header), CMP_ERR_STATE);

  obj.header.ref_count = 1;
  obj.header.vtable = &test_vtable_no_destroy;
  CMP_TEST_EXPECT(cmp_object_release(&obj.header), CMP_ERR_STATE);
  obj.header.vtable = NULL;
  CMP_TEST_EXPECT(cmp_object_release(&obj.header), CMP_ERR_STATE);
  obj.header.vtable = &test_vtable;

  CMP_TEST_OK(init_test_object(&obj_fail, NULL, 7));
  obj_fail.destroy_should_fail = 1;
  CMP_TEST_EXPECT(cmp_object_release(&obj_fail.header), CMP_ERR_UNKNOWN);
  CMP_TEST_ASSERT(obj_fail.header.ref_count == 1);

  CMP_TEST_OK(init_test_object(&obj2, NULL, 9));
  CMP_TEST_OK(cmp_object_release(&obj2.header));
  CMP_TEST_ASSERT(obj2.header.ref_count == 0);
  CMP_TEST_ASSERT(obj2.destroy_calls == 1);

  CMP_TEST_EXPECT(cmp_object_get_type_id(NULL, &type_id),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_object_get_type_id(&obj.header, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_object_get_type_id(&obj.header, &type_id));
  CMP_TEST_ASSERT(type_id == obj.header.type_id);

  CMP_TEST_EXPECT(cmp_handle_system_default_create(0, NULL, &sys),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_handle_system_default_create(1, NULL, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_handle_system_default_create(1, &bad_alloc, &sys),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_handle_system_default_destroy(NULL),
                 CMP_ERR_INVALID_ARGUMENT);

#ifdef CMP_TESTING
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_handle_system_default_create(1, NULL, &sys),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));
#endif

  fail_allocator.ctx = NULL;
  fail_allocator.alloc = fail_alloc;
  fail_allocator.realloc = fail_realloc;
  fail_allocator.free = fail_free;
  CMP_TEST_EXPECT(cmp_handle_system_default_create(1, &fail_allocator, &fail_sys),
                 CMP_ERR_OUT_OF_MEMORY);

  max_usize = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_EXPECT(
      cmp_handle_system_default_create(max_usize, NULL, &overflow_sys),
      CMP_ERR_OVERFLOW);

  CMP_TEST_OK(cmp_handle_system_default_create(2, NULL, &sys));
  CMP_TEST_ASSERT(sys.vtable != NULL);

  CMP_TEST_EXPECT(sys.vtable->init(NULL, 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sys.vtable->shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sys.vtable->unregister_object(NULL, handle),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sys.vtable->retain(NULL, handle), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sys.vtable->release(NULL, handle), CMP_ERR_INVALID_ARGUMENT);

  memset(&impl, 0, sizeof(impl));
  CMP_TEST_EXPECT(sys.vtable->init(&impl, 0), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sys.vtable->shutdown(&impl), CMP_ERR_STATE);
  CMP_TEST_EXPECT(sys.vtable->resolve(&impl, handle, &resolved), CMP_ERR_STATE);
  CMP_TEST_EXPECT(sys.vtable->unregister_object(&impl, handle), CMP_ERR_STATE);

  impl.slots = (CMPHandleSlotInternal *)1;
  CMP_TEST_EXPECT(sys.vtable->init(&impl, 1), CMP_ERR_STATE);
  impl.slots = NULL;

  impl.allocator.alloc = NULL;
  impl.allocator.realloc = NULL;
  impl.allocator.free = NULL;
  CMP_TEST_EXPECT(sys.vtable->init(&impl, 1), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_get_default_allocator(&impl.allocator));
  impl.allocator = fail_allocator;
  impl.slots = NULL;
  CMP_TEST_EXPECT(sys.vtable->init(&impl, 1), CMP_ERR_OUT_OF_MEMORY);
  memset(&impl, 0, sizeof(impl));
  CMP_TEST_OK(cmp_get_default_allocator(&impl.allocator));

  max_u32 = (cmp_u32) ~(cmp_u32)0;
  max_usize = (cmp_usize) ~(cmp_usize)0;
  if ((cmp_usize)max_u32 < max_usize) {
    CMP_TEST_EXPECT(sys.vtable->init(&impl, (cmp_usize)max_u32 + 1),
                   CMP_ERR_RANGE);
  }

  CMP_TEST_EXPECT(sys.vtable->init(&impl, max_usize), CMP_ERR_OVERFLOW);

  memset(&impl, 0, sizeof(impl));
  free_fail_ctx.free_calls = 0;
  free_fail_ctx.fail_on_call = 1;
  free_fail_allocator.ctx = &free_fail_ctx;
  free_fail_allocator.alloc = ok_alloc;
  free_fail_allocator.realloc = ok_realloc;
  free_fail_allocator.free = free_fail_on_call;
  impl.allocator = free_fail_allocator;
  CMP_TEST_OK(sys.vtable->init(&impl, 1));
  CMP_TEST_EXPECT(sys.vtable->shutdown(&impl), CMP_ERR_IO);
  impl.slots = NULL;

  handle.id = 999;
  handle.generation = 1;
  CMP_TEST_EXPECT(sys.vtable->resolve(sys.ctx, handle, &resolved),
                 CMP_ERR_NOT_FOUND);
  CMP_TEST_EXPECT(sys.vtable->unregister_object(sys.ctx, handle),
                 CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(init_test_object(&obj, &sys, 100));
#ifdef CMP_TESTING
  CMP_TEST_OK(cmp_object_test_set_handle_is_valid_fail(CMP_TRUE));
  handle.id = 1;
  handle.generation = 1;
  CMP_TEST_EXPECT(sys.vtable->resolve(sys.ctx, handle, &resolved),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_EXPECT(sys.vtable->unregister_object(sys.ctx, handle),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_EXPECT(sys.vtable->register_object(sys.ctx, &obj.header),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_object_test_set_handle_is_valid_fail(CMP_FALSE));
#endif
  obj.header.vtable = &test_vtable_no_release;
  CMP_TEST_EXPECT(sys.vtable->register_object(NULL, &obj.header),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sys.vtable->register_object(&impl, &obj.header), CMP_ERR_STATE);
  CMP_TEST_EXPECT(sys.vtable->register_object(sys.ctx, &obj.header),
                 CMP_ERR_INVALID_ARGUMENT);

  obj.header.vtable = &test_vtable;
  CMP_TEST_OK(sys.vtable->register_object(sys.ctx, &obj.header));
  handle = obj.header.handle;
  CMP_TEST_OK(cmp_handle_is_valid(handle, &valid));
  CMP_TEST_ASSERT(valid == CMP_TRUE);

  sys_impl = (CMPHandleSystemImplInternal *)sys.ctx;
  sys_impl->live_count = 0;
  CMP_TEST_EXPECT(sys.vtable->unregister_object(sys.ctx, handle), CMP_ERR_STATE);
  sys_impl->live_count = 1;

  CMP_TEST_EXPECT(sys.vtable->register_object(sys.ctx, &obj.header),
                 CMP_ERR_STATE);

  CMP_TEST_EXPECT(sys.vtable->resolve(NULL, handle, &resolved),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sys.vtable->resolve(sys.ctx, handle, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(sys.vtable->resolve(sys.ctx, handle, &resolved));
  CMP_TEST_ASSERT(resolved == &obj);

  CMP_TEST_OK(sys.vtable->retain(sys.ctx, handle));
  CMP_TEST_ASSERT(obj.header.ref_count == 2);

  obj.header.vtable = &test_vtable_no_release;
  CMP_TEST_EXPECT(sys.vtable->release(sys.ctx, handle), CMP_ERR_UNSUPPORTED);
  obj.header.vtable = &test_vtable;

  obj.header.vtable = &test_vtable_no_retain;
  CMP_TEST_EXPECT(sys.vtable->retain(sys.ctx, handle), CMP_ERR_UNSUPPORTED);
  obj.header.vtable = &test_vtable;

  CMP_TEST_OK(sys.vtable->release(sys.ctx, handle));
  CMP_TEST_ASSERT(obj.header.ref_count == 1);

  CMP_TEST_OK(sys.vtable->release(sys.ctx, handle));
  CMP_TEST_ASSERT(obj.destroy_calls == 1);

  CMP_TEST_OK(cmp_handle_is_valid(obj.header.handle, &valid));
  CMP_TEST_ASSERT(valid == CMP_FALSE);

  CMP_TEST_EXPECT(sys.vtable->resolve(sys.ctx, handle, &resolved),
                 CMP_ERR_NOT_FOUND);
  CMP_TEST_EXPECT(sys.vtable->release(sys.ctx, handle), CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(init_test_object(&obj2, &sys, 101));
  CMP_TEST_OK(sys.vtable->register_object(sys.ctx, &obj2.header));
  CMP_TEST_EXPECT(sys.vtable->resolve(sys.ctx, handle, &resolved),
                 CMP_ERR_NOT_FOUND);

  obj2_handle = obj2.header.handle;
  CMP_TEST_OK(sys.vtable->unregister_object(sys.ctx, obj2_handle));
  CMP_TEST_EXPECT(sys.vtable->unregister_object(sys.ctx, obj2_handle),
                 CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(init_test_object(&obj3, &sys, 102));
  CMP_TEST_OK(sys.vtable->register_object(sys.ctx, &obj3.header));
  sys_impl = (CMPHandleSystemImplInternal *)sys.ctx;
  index = obj3.header.handle.id - 1;
  sys_impl->slots[index].generation = max_u32;
  obj3.header.handle.generation = max_u32;
  CMP_TEST_OK(sys.vtable->unregister_object(sys.ctx, obj3.header.handle));
  CMP_TEST_ASSERT(sys_impl->slots[index].generation == 1);

  handle.id = 0;
  handle.generation = 0;
  CMP_TEST_EXPECT(sys.vtable->retain(sys.ctx, handle), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sys.vtable->release(sys.ctx, handle), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(sys.vtable->unregister_object(sys.ctx, handle),
                 CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_handle_system_default_destroy(&sys));
  CMP_TEST_EXPECT(cmp_handle_system_default_destroy(&sys), CMP_ERR_STATE);

  CMP_TEST_OK(cmp_handle_system_default_create(1, NULL, &sys_small));
  CMP_TEST_OK(init_test_object(&obj, &sys_small, 200));
  CMP_TEST_OK(sys_small.vtable->register_object(sys_small.ctx, &obj.header));

  CMP_TEST_OK(init_test_object(&obj2, &sys_small, 201));
  CMP_TEST_EXPECT(sys_small.vtable->register_object(sys_small.ctx, &obj2.header),
                 CMP_ERR_BUSY);

  CMP_TEST_EXPECT(cmp_handle_system_default_destroy(&sys_small), CMP_ERR_BUSY);

  CMP_TEST_OK(
      sys_small.vtable->unregister_object(sys_small.ctx, obj.header.handle));

  CMP_TEST_OK(cmp_handle_system_default_destroy(&sys_small));

  free_fail_ctx.free_calls = 0;
  free_fail_ctx.fail_on_call = 2;
  free_fail_allocator.ctx = &free_fail_ctx;
  free_fail_allocator.alloc = ok_alloc;
  free_fail_allocator.realloc = ok_realloc;
  free_fail_allocator.free = free_fail_on_call;
  CMP_TEST_OK(
      cmp_handle_system_default_create(1, &free_fail_allocator, &free_fail_sys));
  CMP_TEST_EXPECT(cmp_handle_system_default_destroy(&free_fail_sys), CMP_ERR_IO);
  free_fail_sys.ctx = NULL;
  free_fail_sys.vtable = NULL;

  return 0;
}
