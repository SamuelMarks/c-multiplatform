/* clang-format off */
#include "cmpc/cmp_deadlock.h"
#include "test_utils.h"
/* clang-format on */

extern int CMP_CALL cmp_core_test_set_default_allocator_fail(CMPBool fail);

static int CMP_CALL fail_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  (void)ctx;
  (void)size;
  *out_ptr = NULL;
  return CMP_ERR_OUT_OF_MEMORY;
}

static int CMP_CALL fake_free(void *ctx, void *ptr) {
  (void)ctx;
  (void)ptr;
  return CMP_OK;
}

int test_deadlock(void) {
  CMPAllocator alloc;
  CMP_TEST_OK(cmp_get_default_allocator(&alloc));

  CMP_TEST_EXPECT(cmp_deadlock_detector_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMPAllocator bad_alloc = alloc;
  bad_alloc.alloc = NULL;
  CMP_TEST_EXPECT(cmp_deadlock_detector_init(&bad_alloc),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_deadlock_detector_init(&alloc));

  CMPMutex *m1 = NULL, *m2 = NULL;
  CMP_TEST_OK(cmp_mutex_create(&alloc, &m1));
  CMP_TEST_OK(cmp_mutex_create(&alloc, &m2));

  cmp_deadlock_check_before_lock(NULL, "test");
  cmp_deadlock_record_lock(NULL);
  cmp_deadlock_record_unlock(NULL);

  CMP_TEST_OK(cmp_deadlock_check_before_lock(m1, "test"));
  cmp_deadlock_record_lock(m1);

  CMP_TEST_OK(cmp_deadlock_check_before_lock(m2, "test"));
  cmp_deadlock_record_lock(m2);

  /* Simulate cycle */
  /* Thread 1 owns m1 and wants m2. Let's say we are Thread 1. */
  cmp_deadlock_check_before_lock(m1, "test"); /* Should be fine, we own it */

  /* Out of memory in lock record */
  CMPAllocator fail_a = {.alloc = fail_alloc, .free = fake_free, .ctx = NULL};
  cmp_deadlock_detector_destroy();
  cmp_deadlock_detector_init(&fail_a);
  cmp_deadlock_record_lock(m1); /* Will fail silently */

  cmp_deadlock_detector_destroy();
  cmp_deadlock_detector_init(&alloc);

  cmp_deadlock_record_unlock(m2);
  cmp_deadlock_record_unlock(m1);
  cmp_deadlock_record_unlock(m1); /* Unlock non-existent */

  cmp_deadlock_detector_destroy();

  CMP_TEST_OK(cmp_mutex_destroy(&alloc, m1));
  CMP_TEST_OK(cmp_mutex_destroy(&alloc, m2));

  return 0;
}

int main(void) {
  if (test_deadlock() != 0)
    return 1;
  return 0;
}