#ifndef M3_TEST_UTILS_H
#define M3_TEST_UTILS_H

#include <stdio.h>

#include "m3/m3_core.h"

#define M3_TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "TEST FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

#define M3_TEST_EXPECT(expr, expected) \
    do { \
        int m3_test_rc = (expr); \
        if (m3_test_rc != (expected)) { \
            fprintf(stderr, "TEST FAIL %s:%d: expected %d got %d\n", __FILE__, __LINE__, (expected), m3_test_rc); \
            return 1; \
        } \
    } while (0)

#define M3_TEST_OK(expr) M3_TEST_EXPECT((expr), M3_OK)

#endif /* M3_TEST_UTILS_H */
