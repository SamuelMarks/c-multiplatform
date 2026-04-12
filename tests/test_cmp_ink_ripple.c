/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include "cmp_ink_ripple.h"
#include <stdlib.h>
/* clang-format on */

TEST test_ink_ripple_lifecycle(void) {
    cmp_ink_ripple_t *ripple = NULL;
    ASSERT_EQ(CMP_SUCCESS, cmp_ink_ripple_create(&ripple));
    ASSERT_NEQ(NULL, ripple);
    
    ASSERT_EQ(CMP_SUCCESS, cmp_ink_ripple_trigger(ripple, 50.0f, 50.0f));
    ASSERT_EQ(CMP_SUCCESS, cmp_ink_ripple_update(ripple, 16.6f));
    
    ASSERT_EQ(CMP_SUCCESS, cmp_ink_ripple_destroy(ripple));
    PASS();
}

SUITE(suite_ink_ripple) {
    RUN_TEST(test_ink_ripple_lifecycle);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite_ink_ripple);
    GREATEST_MAIN_END();
}
