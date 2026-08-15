/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int g_malloc_fail_countdown;
extern int g_malloc_called;

#define OMIT_MAIN 1

/* Basic */
#define example_basic_main example_basic_main
#include "../examples/basic/main.c"
#undef example_basic_main

/* window_basic */
#define example_window_main example_window_main
#include "../examples/window_basic/main.c"
#undef example_window_main

/* responsive_mosaic */
#define example_responsive_main example_responsive_main
#include "../examples/responsive_mosaic/main.c"
#undef example_responsive_main

/* unstyled_components */
#define example_unstyled_main example_unstyled_main
#include "../examples/unstyled_components/main.c"
#undef example_unstyled_main

/* auth_flow */
#define example_auth_flow_main example_auth_flow_main
#include "../examples/auth_flow/main.c"
#undef example_auth_flow_main

#undef OMIT_MAIN

void test_oom_example(int (*main_func)(void), const char* name) {
    int i;
    for (i = 1; i < 200; i++) {
        g_malloc_called = 0;
        g_malloc_fail_countdown = i;
        main_func();
        if (g_malloc_fail_countdown > 0) {
            break;
        }
    }
    g_malloc_fail_countdown = 0;
}

int main(void) {
    printf("Testing examples with OOM...\n");
    test_oom_example(example_basic_main, "basic");
    test_oom_example(example_window_main, "window_basic");
    test_oom_example(example_responsive_main, "responsive_mosaic");
    test_oom_example(example_unstyled_main, "unstyled_components");
    /* Auth flow includes multiple files, might need to compile them together */
    /* test_oom_example(example_auth_flow_main, "auth_flow"); */
    return 0;
}
/* clang-format on */

int g_mock_gles2_destroy_fail = 0;
int g_mock_gles2_flush_fail = 0;
int g_mock_lock_contention = 0;
int g_mock_strcpy_fail = 0;
int g_mock_thread_fail = 0;
int g_ui_timer_clock_gettime_fail = 0;
int g_mock_cg_fail = 0;
int g_mock_cf_fail = 0;
