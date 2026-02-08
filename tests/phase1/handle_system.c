#include "test_utils.h"
#include "m3/m3_object.h"

#include <string.h>
#include <stdlib.h>

typedef struct M3HandleSlotInternal {
    M3ObjectHeader *obj;
    m3_u32 generation;
    m3_u32 next_free;
} M3HandleSlotInternal;

typedef struct M3HandleSystemImplInternal {
    M3Allocator allocator;
    m3_usize capacity;
    m3_usize live_count;
    M3HandleSlotInternal *slots;
    m3_u32 free_head;
} M3HandleSystemImplInternal;

typedef struct TestObject {
    M3ObjectHeader header;
    M3HandleSystem *sys;
    int destroy_calls;
    int destroy_should_fail;
} TestObject;

static int M3_CALL test_retain(void *obj)
{
    return m3_object_retain((M3ObjectHeader *)obj);
}

static int M3_CALL test_release(void *obj)
{
    return m3_object_release((M3ObjectHeader *)obj);
}

static int M3_CALL test_destroy(void *obj)
{
    TestObject *test_obj;
    M3Bool valid;
    int rc;

    if (obj == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    test_obj = (TestObject *)obj;
    test_obj->destroy_calls += 1;

    if (test_obj->destroy_should_fail) {
        return M3_ERR_UNKNOWN;
    }

    if (test_obj->sys == NULL) {
        return M3_OK;
    }

    rc = m3_handle_is_valid(test_obj->header.handle, &valid);
    if (rc != M3_OK) {
        return rc;
    }

    if (!valid) {
        return M3_OK;
    }

    rc = test_obj->sys->vtable->unregister_object(test_obj->sys->ctx, test_obj->header.handle);
    if (rc == M3_ERR_NOT_FOUND) {
        return M3_OK;
    }

    return rc;
}

static int M3_CALL test_get_type_id(void *obj, m3_u32 *out_type_id)
{
    TestObject *test_obj;

    if (obj == NULL || out_type_id == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    test_obj = (TestObject *)obj;
    *out_type_id = test_obj->header.type_id;
    return M3_OK;
}

static const M3ObjectVTable test_vtable = {
    test_retain,
    test_release,
    test_destroy,
    test_get_type_id
};

static const M3ObjectVTable test_vtable_no_release = {
    test_retain,
    NULL,
    test_destroy,
    test_get_type_id
};

static const M3ObjectVTable test_vtable_no_retain = {
    NULL,
    test_release,
    test_destroy,
    test_get_type_id
};

static const M3ObjectVTable test_vtable_no_destroy = {
    test_retain,
    test_release,
    NULL,
    test_get_type_id
};

static int init_test_object(TestObject *obj, M3HandleSystem *sys, m3_u32 type_id)
{
    int rc;

    memset(obj, 0, sizeof(*obj));
    obj->sys = sys;
    obj->destroy_calls = 0;
    obj->destroy_should_fail = 0;
    rc = m3_object_header_init(&obj->header, type_id, 0, &test_vtable);
    return rc;
}

static int M3_CALL fail_alloc(void *ctx, m3_usize size, void **out_ptr)
{
    M3_UNUSED(ctx);
    M3_UNUSED(size);

    if (out_ptr != NULL) {
        *out_ptr = NULL;
    }
    return M3_ERR_OUT_OF_MEMORY;
}

static int M3_CALL fail_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr)
{
    M3_UNUSED(ctx);
    M3_UNUSED(ptr);
    M3_UNUSED(size);

    if (out_ptr != NULL) {
        *out_ptr = NULL;
    }
    return M3_ERR_OUT_OF_MEMORY;
}

static int M3_CALL fail_free(void *ctx, void *ptr)
{
    M3_UNUSED(ctx);
    M3_UNUSED(ptr);
    return M3_OK;
}

typedef struct FreeFailCtx {
    int free_calls;
    int fail_on_call;
} FreeFailCtx;

static int M3_CALL ok_alloc(void *ctx, m3_usize size, void **out_ptr)
{
    M3_UNUSED(ctx);

    if (out_ptr == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_ptr = NULL;
    if (size == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_ptr = malloc((size_t)size);
    if (*out_ptr == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }
    return M3_OK;
}

static int M3_CALL ok_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr)
{
    void *mem;

    M3_UNUSED(ctx);

    if (out_ptr == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_ptr = NULL;
    if (size == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    mem = realloc(ptr, (size_t)size);
    if (mem == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }
    *out_ptr = mem;
    return M3_OK;
}

static int M3_CALL free_fail_on_call(void *ctx, void *ptr)
{
    FreeFailCtx *state;
    int result;

    state = (FreeFailCtx *)ctx;
    result = M3_OK;
    if (state != NULL) {
        state->free_calls += 1;
        if (state->fail_on_call > 0 && state->free_calls == state->fail_on_call) {
            result = M3_ERR_IO;
        }
    }

    free(ptr);
    return result;
}

int main(void)
{
    M3HandleSystem sys;
    M3HandleSystem sys_small;
    M3HandleSystem fail_sys;
    M3HandleSystem free_fail_sys;
    M3HandleSystem overflow_sys;
    M3HandleSystemImplInternal impl;
    M3Allocator bad_alloc;
    M3Allocator fail_allocator;
    M3Allocator free_fail_allocator;
    FreeFailCtx free_fail_ctx;
    M3Handle handle;
    M3Handle obj2_handle;
    M3Bool valid;
    TestObject obj;
    TestObject obj2;
    TestObject obj3;
    TestObject obj_fail;
    M3ObjectHeader header;
    void *resolved;
    m3_u32 type_id;
    m3_u32 max_u32;
    m3_usize max_usize;
    M3HandleSystemImplInternal *sys_impl;
    m3_u32 index;

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

    M3_TEST_EXPECT(m3_handle_is_valid(handle, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_handle_is_valid(handle, &valid));
    M3_TEST_ASSERT(valid == M3_FALSE);

    M3_TEST_EXPECT(m3_object_header_init(NULL, 1, 0, &test_vtable), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_object_header_init(&header, 1, 0, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_object_header_init(&header, 1, 0, &test_vtable_no_release), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(init_test_object(&obj, NULL, 42));
    M3_TEST_ASSERT(obj.header.ref_count == 1);
    M3_TEST_OK(m3_handle_is_valid(obj.header.handle, &valid));
    M3_TEST_ASSERT(valid == M3_FALSE);

    M3_TEST_EXPECT(m3_object_retain(NULL), M3_ERR_INVALID_ARGUMENT);
    obj.header.ref_count = 0;
    M3_TEST_EXPECT(m3_object_retain(&obj.header), M3_ERR_STATE);
    max_u32 = (m3_u32)~(m3_u32)0;
    obj.header.ref_count = max_u32;
    M3_TEST_EXPECT(m3_object_retain(&obj.header), M3_ERR_OVERFLOW);
    obj.header.ref_count = 1;
    M3_TEST_OK(m3_object_retain(&obj.header));
    M3_TEST_ASSERT(obj.header.ref_count == 2);
    obj.header.ref_count = 1;

    M3_TEST_EXPECT(m3_object_release(NULL), M3_ERR_INVALID_ARGUMENT);
    obj.header.ref_count = 0;
    M3_TEST_EXPECT(m3_object_release(&obj.header), M3_ERR_STATE);

    obj.header.ref_count = 1;
    obj.header.vtable = &test_vtable_no_destroy;
    M3_TEST_EXPECT(m3_object_release(&obj.header), M3_ERR_STATE);
    obj.header.vtable = NULL;
    M3_TEST_EXPECT(m3_object_release(&obj.header), M3_ERR_STATE);
    obj.header.vtable = &test_vtable;

    M3_TEST_OK(init_test_object(&obj_fail, NULL, 7));
    obj_fail.destroy_should_fail = 1;
    M3_TEST_EXPECT(m3_object_release(&obj_fail.header), M3_ERR_UNKNOWN);
    M3_TEST_ASSERT(obj_fail.header.ref_count == 1);

    M3_TEST_OK(init_test_object(&obj2, NULL, 9));
    M3_TEST_OK(m3_object_release(&obj2.header));
    M3_TEST_ASSERT(obj2.header.ref_count == 0);
    M3_TEST_ASSERT(obj2.destroy_calls == 1);

    M3_TEST_EXPECT(m3_object_get_type_id(NULL, &type_id), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_object_get_type_id(&obj.header, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_object_get_type_id(&obj.header, &type_id));
    M3_TEST_ASSERT(type_id == obj.header.type_id);

    M3_TEST_EXPECT(m3_handle_system_default_create(0, NULL, &sys), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_handle_system_default_create(1, NULL, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_handle_system_default_create(1, &bad_alloc, &sys), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_handle_system_default_destroy(NULL), M3_ERR_INVALID_ARGUMENT);

#ifdef M3_TESTING
    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
    M3_TEST_EXPECT(m3_handle_system_default_create(1, NULL, &sys), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));
#endif

    fail_allocator.ctx = NULL;
    fail_allocator.alloc = fail_alloc;
    fail_allocator.realloc = fail_realloc;
    fail_allocator.free = fail_free;
    M3_TEST_EXPECT(m3_handle_system_default_create(1, &fail_allocator, &fail_sys), M3_ERR_OUT_OF_MEMORY);

    max_usize = (m3_usize)~(m3_usize)0;
    M3_TEST_EXPECT(m3_handle_system_default_create(max_usize, NULL, &overflow_sys), M3_ERR_OVERFLOW);

    M3_TEST_OK(m3_handle_system_default_create(2, NULL, &sys));
    M3_TEST_ASSERT(sys.vtable != NULL);

    M3_TEST_EXPECT(sys.vtable->init(NULL, 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sys.vtable->shutdown(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sys.vtable->unregister_object(NULL, handle), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sys.vtable->retain(NULL, handle), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sys.vtable->release(NULL, handle), M3_ERR_INVALID_ARGUMENT);

    memset(&impl, 0, sizeof(impl));
    M3_TEST_EXPECT(sys.vtable->init(&impl, 0), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sys.vtable->shutdown(&impl), M3_ERR_STATE);
    M3_TEST_EXPECT(sys.vtable->resolve(&impl, handle, &resolved), M3_ERR_STATE);
    M3_TEST_EXPECT(sys.vtable->unregister_object(&impl, handle), M3_ERR_STATE);

    impl.slots = (M3HandleSlotInternal *)1;
    M3_TEST_EXPECT(sys.vtable->init(&impl, 1), M3_ERR_STATE);
    impl.slots = NULL;

    impl.allocator.alloc = NULL;
    impl.allocator.realloc = NULL;
    impl.allocator.free = NULL;
    M3_TEST_EXPECT(sys.vtable->init(&impl, 1), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_get_default_allocator(&impl.allocator));
    impl.allocator = fail_allocator;
    impl.slots = NULL;
    M3_TEST_EXPECT(sys.vtable->init(&impl, 1), M3_ERR_OUT_OF_MEMORY);
    memset(&impl, 0, sizeof(impl));
    M3_TEST_OK(m3_get_default_allocator(&impl.allocator));

    max_u32 = (m3_u32)~(m3_u32)0;
    max_usize = (m3_usize)~(m3_usize)0;
    if ((m3_usize)max_u32 < max_usize) {
        M3_TEST_EXPECT(sys.vtable->init(&impl, (m3_usize)max_u32 + 1), M3_ERR_RANGE);
    }

    M3_TEST_EXPECT(sys.vtable->init(&impl, max_usize), M3_ERR_OVERFLOW);

    memset(&impl, 0, sizeof(impl));
    free_fail_ctx.free_calls = 0;
    free_fail_ctx.fail_on_call = 1;
    free_fail_allocator.ctx = &free_fail_ctx;
    free_fail_allocator.alloc = ok_alloc;
    free_fail_allocator.realloc = ok_realloc;
    free_fail_allocator.free = free_fail_on_call;
    impl.allocator = free_fail_allocator;
    M3_TEST_OK(sys.vtable->init(&impl, 1));
    M3_TEST_EXPECT(sys.vtable->shutdown(&impl), M3_ERR_IO);
    impl.slots = NULL;

    handle.id = 999;
    handle.generation = 1;
    M3_TEST_EXPECT(sys.vtable->resolve(sys.ctx, handle, &resolved), M3_ERR_NOT_FOUND);
    M3_TEST_EXPECT(sys.vtable->unregister_object(sys.ctx, handle), M3_ERR_NOT_FOUND);

    M3_TEST_OK(init_test_object(&obj, &sys, 100));
#ifdef M3_TESTING
    M3_TEST_OK(m3_object_test_set_handle_is_valid_fail(M3_TRUE));
    handle.id = 1;
    handle.generation = 1;
    M3_TEST_EXPECT(sys.vtable->resolve(sys.ctx, handle, &resolved), M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(sys.vtable->unregister_object(sys.ctx, handle), M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(sys.vtable->register_object(sys.ctx, &obj.header), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_object_test_set_handle_is_valid_fail(M3_FALSE));
#endif
    obj.header.vtable = &test_vtable_no_release;
    M3_TEST_EXPECT(sys.vtable->register_object(NULL, &obj.header), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sys.vtable->register_object(&impl, &obj.header), M3_ERR_STATE);
    M3_TEST_EXPECT(sys.vtable->register_object(sys.ctx, &obj.header), M3_ERR_INVALID_ARGUMENT);

    obj.header.vtable = &test_vtable;
    M3_TEST_OK(sys.vtable->register_object(sys.ctx, &obj.header));
    handle = obj.header.handle;
    M3_TEST_OK(m3_handle_is_valid(handle, &valid));
    M3_TEST_ASSERT(valid == M3_TRUE);

    sys_impl = (M3HandleSystemImplInternal *)sys.ctx;
    sys_impl->live_count = 0;
    M3_TEST_EXPECT(sys.vtable->unregister_object(sys.ctx, handle), M3_ERR_STATE);
    sys_impl->live_count = 1;

    M3_TEST_EXPECT(sys.vtable->register_object(sys.ctx, &obj.header), M3_ERR_STATE);

    M3_TEST_EXPECT(sys.vtable->resolve(NULL, handle, &resolved), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sys.vtable->resolve(sys.ctx, handle, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(sys.vtable->resolve(sys.ctx, handle, &resolved));
    M3_TEST_ASSERT(resolved == &obj);

    M3_TEST_OK(sys.vtable->retain(sys.ctx, handle));
    M3_TEST_ASSERT(obj.header.ref_count == 2);

    obj.header.vtable = &test_vtable_no_release;
    M3_TEST_EXPECT(sys.vtable->release(sys.ctx, handle), M3_ERR_UNSUPPORTED);
    obj.header.vtable = &test_vtable;

    obj.header.vtable = &test_vtable_no_retain;
    M3_TEST_EXPECT(sys.vtable->retain(sys.ctx, handle), M3_ERR_UNSUPPORTED);
    obj.header.vtable = &test_vtable;

    M3_TEST_OK(sys.vtable->release(sys.ctx, handle));
    M3_TEST_ASSERT(obj.header.ref_count == 1);

    M3_TEST_OK(sys.vtable->release(sys.ctx, handle));
    M3_TEST_ASSERT(obj.destroy_calls == 1);

    M3_TEST_OK(m3_handle_is_valid(obj.header.handle, &valid));
    M3_TEST_ASSERT(valid == M3_FALSE);

    M3_TEST_EXPECT(sys.vtable->resolve(sys.ctx, handle, &resolved), M3_ERR_NOT_FOUND);
    M3_TEST_EXPECT(sys.vtable->release(sys.ctx, handle), M3_ERR_NOT_FOUND);

    M3_TEST_OK(init_test_object(&obj2, &sys, 101));
    M3_TEST_OK(sys.vtable->register_object(sys.ctx, &obj2.header));
    M3_TEST_EXPECT(sys.vtable->resolve(sys.ctx, handle, &resolved), M3_ERR_NOT_FOUND);

    obj2_handle = obj2.header.handle;
    M3_TEST_OK(sys.vtable->unregister_object(sys.ctx, obj2_handle));
    M3_TEST_EXPECT(sys.vtable->unregister_object(sys.ctx, obj2_handle), M3_ERR_NOT_FOUND);

    M3_TEST_OK(init_test_object(&obj3, &sys, 102));
    M3_TEST_OK(sys.vtable->register_object(sys.ctx, &obj3.header));
    sys_impl = (M3HandleSystemImplInternal *)sys.ctx;
    index = obj3.header.handle.id - 1;
    sys_impl->slots[index].generation = max_u32;
    obj3.header.handle.generation = max_u32;
    M3_TEST_OK(sys.vtable->unregister_object(sys.ctx, obj3.header.handle));
    M3_TEST_ASSERT(sys_impl->slots[index].generation == 1);

    handle.id = 0;
    handle.generation = 0;
    M3_TEST_EXPECT(sys.vtable->retain(sys.ctx, handle), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sys.vtable->release(sys.ctx, handle), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sys.vtable->unregister_object(sys.ctx, handle), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_handle_system_default_destroy(&sys));
    M3_TEST_EXPECT(m3_handle_system_default_destroy(&sys), M3_ERR_STATE);

    M3_TEST_OK(m3_handle_system_default_create(1, NULL, &sys_small));
    M3_TEST_OK(init_test_object(&obj, &sys_small, 200));
    M3_TEST_OK(sys_small.vtable->register_object(sys_small.ctx, &obj.header));

    M3_TEST_OK(init_test_object(&obj2, &sys_small, 201));
    M3_TEST_EXPECT(sys_small.vtable->register_object(sys_small.ctx, &obj2.header), M3_ERR_BUSY);

    M3_TEST_EXPECT(m3_handle_system_default_destroy(&sys_small), M3_ERR_BUSY);

    M3_TEST_OK(sys_small.vtable->unregister_object(sys_small.ctx, obj.header.handle));

    M3_TEST_OK(m3_handle_system_default_destroy(&sys_small));

    free_fail_ctx.free_calls = 0;
    free_fail_ctx.fail_on_call = 2;
    free_fail_allocator.ctx = &free_fail_ctx;
    free_fail_allocator.alloc = ok_alloc;
    free_fail_allocator.realloc = ok_realloc;
    free_fail_allocator.free = free_fail_on_call;
    M3_TEST_OK(m3_handle_system_default_create(1, &free_fail_allocator, &free_fail_sys));
    M3_TEST_EXPECT(m3_handle_system_default_destroy(&free_fail_sys), M3_ERR_IO);
    free_fail_sys.ctx = NULL;
    free_fail_sys.vtable = NULL;

    return 0;
}
