/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include "cmp_apz.h"
#include <stdlib.h>
/* clang-format on */

TEST test_apz_lifecycle(void) {
    cmp_apz_t *apz = NULL;
    ASSERT_EQ(CMP_SUCCESS, cmp_apz_create(&apz));
    ASSERT_NEQ(NULL, apz);
    
    ASSERT_EQ(CMP_SUCCESS, cmp_apz_inject_gesture(apz, 10.0f, -5.0f, 1.2f));
    
    float mat[16];
    ASSERT_EQ(CMP_SUCCESS, cmp_apz_get_transform(apz, mat));
    
    ASSERT_EQ(CMP_SUCCESS, cmp_apz_destroy(apz));
    PASS();
}

SUITE(suite_apz) {
    RUN_TEST(test_apz_lifecycle);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite_apz);
    GREATEST_MAIN_END();
}
