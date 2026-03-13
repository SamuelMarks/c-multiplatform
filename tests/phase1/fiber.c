/* clang-format off */
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_fiber.h"
#include "test_utils.h"
/* clang-format on */

int test_fiber_scheduler(void) {
  CMPAllocator alloc;
  CMPFiberScheduler *scheduler = NULL;
  CMP_TEST_OK(cmp_get_default_allocator(&alloc));

  CMP_TEST_OK(cmp_fiber_scheduler_create(&alloc, &scheduler));
  CMP_TEST_OK(cmp_fiber_scheduler_destroy(&alloc, scheduler));

  return 0;
}

int main(void) {
  if (test_fiber_scheduler() != 0)
    return 1;
  return 0;
}
