/**
 * @file test_example_unstyled_components.c
 * @brief Unit test and OOM stress testing for unstyled components example.
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CI_TEST_RUN
#define CI_TEST_RUN 1
#endif

#define OMIT_MAIN 1
#include "../examples/unstyled_components/main.c"
#undef OMIT_MAIN

extern int g_malloc_fail_countdown;
extern int g_malloc_called;
extern int g_mock_gles2_destroy_fail;
extern int g_mock_gles2_flush_fail;
extern int g_mock_lock_contention;
extern int g_mock_strcpy_fail;
extern int g_mock_thread_fail;
extern int g_ui_timer_clock_gettime_fail;
extern int g_mock_cg_fail;
extern int g_mock_cf_fail;
extern int g_mock_cf_string_create_fail;
extern int g_mock_dlopen_fail;

/**
 * @brief Test entry point for unstyled components example execution and OOM simulation.
 * @return 0 on success, non-zero on failure.
 */
int main(void) {
    int i;
    int rc;
    printf("Running OOM loop for unstyled_components...\n");
    for (i = 1; i < 5; i++) {
        g_malloc_called = 0;
        g_malloc_fail_countdown = i;
        rc = example_unstyled_main();
        if (rc != 0) {
            /* Expected failure on simulated memory exhaustion */
        }
        if (g_malloc_fail_countdown > 0) {
            break;
        }
    }
    g_malloc_fail_countdown = -1;
    /* Run once successfully */
    rc = example_unstyled_main();
    if (rc != 0) {
        return rc;
    }
    return 0;
}
/* clang-format on */
