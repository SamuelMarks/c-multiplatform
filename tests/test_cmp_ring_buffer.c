/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_ring_buffer_lifecycle(void) {
  cmp_ring_buffer_t rb;
  int dummy1 = 1;
  int dummy2 = 2;
  int dummy3 = 3;
  void *out;
  int res;

  /* Init */
  res = cmp_ring_buffer_init(&rb, 3);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT((size_t)3, rb.capacity, "%zd");

  /* Empty Pop */
  res = cmp_ring_buffer_pop(&rb, &out);
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");

  /* Push 1 */
  res = cmp_ring_buffer_push(&rb, &dummy1);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Push 2 */
  res = cmp_ring_buffer_push(&rb, &dummy2);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Push 3 (Should fail, capacity is 3 but one slot is kept open to distinguish
   * full vs empty) */
  res = cmp_ring_buffer_push(&rb, &dummy3);
  ASSERT_EQ_FMT(CMP_ERROR_BOUNDS, res, "%d");

  /* Pop 1 */
  res = cmp_ring_buffer_pop(&rb, &out);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(out == &dummy1);

  /* Push 3 */
  res = cmp_ring_buffer_push(&rb, &dummy3);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Pop 2 */
  res = cmp_ring_buffer_pop(&rb, &out);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(out == &dummy2);

  /* Pop 3 */
  res = cmp_ring_buffer_pop(&rb, &out);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(out == &dummy3);

  /* Empty Pop */
  res = cmp_ring_buffer_pop(&rb, &out);
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");

  /* Destroy */
  res = cmp_ring_buffer_destroy(&rb);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(rb.buffer == NULL);

  PASS();
}

SUITE(ring_buffer_suite) { RUN_TEST(test_ring_buffer_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ring_buffer_suite);
  GREATEST_MAIN_END();
}
