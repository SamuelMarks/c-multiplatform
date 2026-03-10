/* clang-format off */
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_shm.h"
#include "test_utils.h"
/* clang-format on */

extern int CMP_CALL cmp_core_test_set_default_allocator_fail(CMPBool fail);

int test_shm(void) {
  CMPSharedMemory *shm = NULL;
  CMPSharedMemory *shm_open = NULL;
  void *pn = NULL;

  /* Test invalid args for create */
  CMP_TEST_EXPECT(cmp_shm_create(NULL, 1024, &shm), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_shm_create("/test_shm", 0, &shm),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_shm_create("/test_shm", 1024, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test invalid args for open */
  CMP_TEST_EXPECT(cmp_shm_open(NULL, 1024, &shm), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_shm_open("/test_shm", 0, &shm), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_shm_open("/test_shm", 1024, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Open non-existent */
  CMP_TEST_EXPECT(cmp_shm_open("/does_not_exist_12345", 1024, &shm_open),
                  CMP_ERR_NOT_FOUND);

  /* Test alloc failure */
  cmp_core_test_set_default_allocator_fail(CMP_TRUE);
  CMP_TEST_EXPECT(cmp_shm_create("/test_shm", 1024, &shm), CMP_ERR_UNKNOWN);
  CMP_TEST_EXPECT(cmp_shm_open("/test_shm", 1024, &shm), CMP_ERR_UNKNOWN);
  /* we cannot easily test cmp_shm_close failure on get_default_allocator
     because it needs a valid shm first, but we cannot create one while failing.
     So we toggle. */
  cmp_core_test_set_default_allocator_fail(CMP_FALSE);

  /* Create and close */
  int rc = cmp_shm_create("/test_shm_cmp_xyz123", 1024, &shm);
  CMP_TEST_EXPECT(rc, CMP_OK);
  if (rc == CMP_OK) {
    void *p = NULL;
    cmp_shm_get_ptr(shm, &p);
    CMP_TEST_EXPECT(p != NULL, 1);

    /* Open existing */
    CMP_TEST_OK(cmp_shm_open("/test_shm_cmp_xyz123", 1024, &shm_open));
    if (shm_open) {
      void *po = NULL;
      cmp_shm_get_ptr(shm_open, &po);
      CMP_TEST_EXPECT(po != NULL, 1);

      /* Test close with failing allocator fallback handling (though impossible
       * to hit normally unless customized) */
      cmp_core_test_set_default_allocator_fail(CMP_TRUE);
      CMP_TEST_EXPECT(cmp_shm_close(shm_open), CMP_ERR_UNKNOWN);
      cmp_core_test_set_default_allocator_fail(CMP_FALSE);

      CMP_TEST_OK(cmp_shm_close(shm_open));
    }

    CMP_TEST_OK(cmp_shm_close(shm));
    CMP_TEST_OK(cmp_shm_unlink("/test_shm_cmp_xyz123"));
  }

  /* Test invalid args for close and get_ptr */
  CMP_TEST_EXPECT(cmp_shm_close(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_shm_get_ptr(NULL, &pn) != 0, 1);

  /* Test invalid args for unlink */
  CMP_TEST_EXPECT(cmp_shm_unlink(NULL), CMP_ERR_INVALID_ARGUMENT);

  /* Create huge to fail mapping */
#if defined(_WIN32)
  /* A huge size will fail mapping */
  CMP_TEST_EXPECT(
      cmp_shm_create("/fail_huge", (cmp_usize)0xFFFFFFFFFFFFFFFFULL, &shm),
      CMP_ERR_UNKNOWN);
#endif

  return 0;
}

int main(void) {
  if (test_shm() != 0)
    return 1;
  return 0;
}
