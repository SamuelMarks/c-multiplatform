#ifndef UI_TEST_MOCK_MEM_H
#define UI_TEST_MOCK_MEM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_malloc_called;
extern int g_ui_timer_clock_gettime_fail;
extern int g_ui_clipboard_force_fallback;
extern int g_mock_io_fail;
extern int g_mock_thread_fail;

extern int g_mock_gles2_create_fail;
extern int g_mock_gles2_destroy_fail;
extern int g_mock_gles2_flush_fail;
extern int g_native_init_fail;
extern int g_gles_init_fail;
extern int g_mock_append_child_fail;
extern int g_mock_strcpy_fail;
extern int g_mock_lock_contention;
extern int g_mock_cg_fail;

void *ui_mock_malloc(size_t size);
void *ui_mock_realloc(void *ptr, size_t size);
void ui_mock_free(void *ptr);
char *ui_mock_strdup(const char *src);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TEST_MOCK_MEM_H */
