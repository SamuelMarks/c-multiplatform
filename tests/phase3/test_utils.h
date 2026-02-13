#ifndef CMP_TEST_UTILS_H
#define CMP_TEST_UTILS_H

#include <stdio.h>

#include "cmpc/cmp_core.h"

#define CMP_TEST_ASSERT(condition)                                              \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "TEST FAIL %s:%d: %s\n", __FILE__, __LINE__,             \
              #condition);                                                     \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define CMP_TEST_EXPECT(expr, expected)                                         \
  do {                                                                         \
    int cmp_test_rc = (expr);                                                   \
    if (cmp_test_rc != (expected)) {                                            \
      fprintf(stderr, "TEST FAIL %s:%d: expected %d got %d\n", __FILE__,       \
              __LINE__, (expected), cmp_test_rc);                               \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define CMP_TEST_OK(expr) CMP_TEST_EXPECT((expr), CMP_OK)

#endif /* CMP_TEST_UTILS_H */
