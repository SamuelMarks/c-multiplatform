/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OMIT_MAIN 1
#include "../examples/responsive_mosaic/main.c"
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

int main(void) {
    int i;
    printf("Running OOM loop for responsive_mosaic...\n");
    for (i = 1; i < 5; i++) {
        g_malloc_called = 0;
        g_malloc_fail_countdown = i;
        example_responsive_main();
        if (g_malloc_fail_countdown > 0) {
            break;
        }
    }
    g_malloc_fail_countdown = 0;
    /* Run once successfully */
    example_responsive_main();
    return 0;
}
/* clang-format on */
