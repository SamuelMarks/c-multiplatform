/* clang-format off */
#include "ui_file_uploader_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t dummy_on_change(union ui_signal_payload value,
                                  void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_on_touched(void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  return UI_ERROR_NONE;
}

static int run_coverage_tests(void) {
  struct ui_file_uploader_base uploader;
  struct ui_control_value_accessor cva;
  union ui_signal_payload payload;
  int touched_called = 0;
  int change_called = 0;

  if (ui_file_uploader_init(&uploader, 2, 0, 0, 100, 100, &cva) !=
      UI_ERROR_NONE)
    return 1;

  if (cva.register_on_change)
    cva.register_on_change(&uploader, dummy_on_change, &change_called);
  if (cva.register_on_touched)
    cva.register_on_touched(&uploader, dummy_on_touched, &touched_called);
  if (cva.set_disabled_state)
    cva.set_disabled_state(&uploader, UI_TRUE);

  if (ui_file_uploader_on_drag_enter(&uploader) != UI_ERROR_NONE)
    return 2;
  if (uploader.state != UI_FILE_UPLOADER_STATE_IDLE)
    return 3;
  if (ui_file_uploader_on_drag_leave(&uploader) != UI_ERROR_NONE)
    return 4;
  if (ui_file_uploader_drop_file(&uploader, "test.txt") != UI_ERROR_NONE)
    return 5;

  if (cva.set_disabled_state)
    cva.set_disabled_state(&uploader, UI_FALSE);

  ui_file_uploader_on_drag_enter(&uploader);
  touched_called = 0;
  FILE *fp2 = fopen("test.txt", "wb");
  if (fp2) {
    fwrite("1", 1, 1, fp2);
    fclose(fp2);
  }
  ui_file_uploader_drop_file(&uploader, "test.txt");

  /* Create a file so it can be read and deleted */
  FILE *fptest = fopen("test_coverage.txt", "wb");
  if (fptest) {
    fwrite("123", 1, 3, fptest);
    fclose(fptest);
  }

  ui_file_uploader_drop_file(&uploader, "C:\\test_coverage.txt");

  /* Test disabled state read */
  uploader.is_disabled = UI_TRUE;
  ui_file_uploader_read_files(&uploader);
  uploader.is_disabled = UI_FALSE;

  /* Actually read it */
  ui_file_uploader_read_files(&uploader);
  /* Read again to trigger 'Already read' */
  ui_file_uploader_read_files(&uploader);

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_mock_io_fail;
  extern void C_MULTIPLATFORM_FREE(void *);
  if (uploader.files[0].data)
    C_MULTIPLATFORM_FREE(uploader.files[0].data);
  uploader.files[0].data = NULL; /* reset to read again */
  g_mock_io_fail = 1;            /* UI_FSEEK */
  ui_file_uploader_read_files(&uploader);
  if (uploader.files[0].data)
    C_MULTIPLATFORM_FREE(uploader.files[0].data);
  uploader.files[0].data = NULL;
  g_mock_io_fail = 2; /* UI_FTELL */
  ui_file_uploader_read_files(&uploader);
  if (uploader.files[0].data)
    C_MULTIPLATFORM_FREE(uploader.files[0].data);
  uploader.files[0].data = NULL;
  g_mock_io_fail = 3; /* UI_FREAD */
  ui_file_uploader_read_files(&uploader);
  if (uploader.files[0].data)
    C_MULTIPLATFORM_FREE(uploader.files[0].data);
  uploader.files[0].data = NULL;
  g_mock_io_fail = 4; /* UI_FSEEK_SET_FAIL */
  ui_file_uploader_read_files(&uploader);
  g_mock_io_fail = 0;

  if (uploader.files[0].data)
    C_MULTIPLATFORM_FREE(uploader.files[0].data);
  uploader.files[0].data = NULL;
  g_malloc_fail_countdown = 0; /* Fail MALLOC in read_files */
  ui_file_uploader_read_files(&uploader);
  g_malloc_fail_countdown = -1;

  /* Now read completely successfully to trigger on_change_cb */
  {
    int i;
    for (i = 0; i < uploader.file_count; i++) {
      if (uploader.files[i].data) {
#ifdef UI_TEST_MOCK_ALLOC
        C_MULTIPLATFORM_FREE(uploader.files[i].data);
#else
        free(uploader.files[i].data);
#endif
      }
    }
  }
  uploader.file_count = 0; /* Clear list */
  ui_file_uploader_drop_file(&uploader, "test.txt");
  ui_file_uploader_read_files(&uploader);

#endif

  if (uploader.files[0].data) {
#ifdef UI_TEST_MOCK_ALLOC
    C_MULTIPLATFORM_FREE(uploader.files[0].data);
#else
    free(uploader.files[0].data);
#endif
  }

  uploader.files[0].data = (unsigned char *)malloc(1);
  uploader.files[0].data[0] = '\0';
  uploader.files[1].data = NULL;
  uploader.file_count = 2;

  payload.ptr_val = NULL;
  if (cva.write_value) {
    cva.write_value(&uploader, payload);
  }

  /* Trigger write_value with non-NULL ptr_val */
  payload.ptr_val = (void *)1;
  if (cva.write_value) {
    cva.write_value(&uploader, payload);
  }

  ui_file_uploader_destroy(&uploader);
  remove("test_coverage.txt");
  remove("test.txt");

  /* Test 0 file size */
  FILE *fp_empty = fopen("empty_test.txt", "wb");
  if (fp_empty) {
    fclose(fp_empty);
  }
  ui_file_uploader_init(&uploader, 2, 0, 0, 100, 100, NULL);
  ui_file_uploader_drop_file(&uploader, "empty_test.txt");
  ui_file_uploader_drop_file(&uploader,
                             "some/fake/path/file.txt"); /* test path slash */
  ui_file_uploader_read_files(&uploader);
  ui_file_uploader_destroy(&uploader);
  remove("empty_test.txt");

  if (ui_file_uploader_init(&uploader, 1, 0, 0, 100, 100, NULL) !=
      UI_ERROR_NONE) {
    ui_file_uploader_destroy(&uploader);
    if (ui_file_uploader_init(&uploader, 1, 0, 0, 100, 100, NULL) !=
        UI_ERROR_NONE)
      return 6;
  }
  ui_file_uploader_drop_file(&uploader, "does_not_exist_xyz.txt");
  ui_file_uploader_destroy(&uploader);

  return 0;
}

static int run_normal_tests(void) {
  struct ui_file_uploader_base uploader;
  struct ui_drag_drop_context *drag_ctx = NULL;
  ui_error_t rc;
  FILE *fp;

  if (ui_file_uploader_init(NULL, 5, 0, 0, 100, 100, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_file_uploader_init(&uploader, 0, 0, 0, 100, 100, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_file_uploader_destroy(NULL);

  if (ui_file_uploader_on_drag_enter(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_file_uploader_on_drag_leave(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_file_uploader_drop_file(NULL, "test.txt") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_file_uploader_drop_file(&uploader, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_file_uploader_read_files(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_file_uploader_register_dropzone(NULL, drag_ctx) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_file_uploader_init(&uploader, 2, 0, 0, 100, 100, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;
  if (uploader.state != UI_FILE_UPLOADER_STATE_IDLE)
    return 1;

  rc = ui_file_uploader_on_drag_enter(&uploader);
  if (rc != UI_ERROR_NONE || uploader.state != UI_FILE_UPLOADER_STATE_DRAG_OVER)
    return 1;

  rc = ui_file_uploader_on_drag_leave(&uploader);
  if (rc != UI_ERROR_NONE || uploader.state != UI_FILE_UPLOADER_STATE_IDLE)
    return 1;

  fp = fopen("test_upload.txt", "wb");
  if (fp) {
    fwrite("Hello Uploader", 1, 14, fp);
    fclose(fp);
  }

  rc = ui_file_uploader_drop_file(&uploader, "test_upload.txt");
  if (rc != UI_ERROR_NONE || uploader.file_count != 1)
    return 1;

  rc = ui_file_uploader_drop_file(&uploader, "test_upload.txt");
  if (rc != UI_ERROR_NONE || uploader.file_count != 2)
    return 1;

  rc = ui_file_uploader_drop_file(&uploader, "test_upload.txt");
  if (rc != UI_ERROR_QUEUE_FULL || uploader.file_count != 2)
    return 1;

  rc = ui_file_uploader_read_files(&uploader);
  if (rc != UI_ERROR_NONE)
    return 1;
  if (uploader.state != UI_FILE_UPLOADER_STATE_COMPLETE)
    return 1;
  if (uploader.files[0].file_size != 14)
    return 1;
  if (strncmp((const char *)uploader.files[0].data, "Hello Uploader", 14) != 0)
    return 1;

  ui_drag_drop_create(&drag_ctx);
  if (drag_ctx) {
    if (ui_file_uploader_register_dropzone(&uploader, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_file_uploader_register_dropzone(&uploader, drag_ctx) !=
        UI_ERROR_NONE)
      return 1;
    (void)ui_drag_drop_destroy(drag_ctx);
  }

  ui_file_uploader_destroy(&uploader);
  remove("test_upload.txt");

  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_file_uploader_base uploader;
  ui_error_t rc;
  g_malloc_fail_countdown = 0;
  memset(&uploader, 0, sizeof(uploader));
  rc = ui_file_uploader_init(&uploader, 5, 0, 0, 100, 100, NULL);
  g_malloc_fail_countdown = -1;
  ui_file_uploader_destroy(&uploader);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
#endif
  return 0;
}

static ui_error_t mock_touch_fail(void *ud) {
  return UI_ERROR_INVALID_ARGUMENT;
}
static ui_error_t mock_change_fail(union ui_signal_payload v, void *ud) {
  return UI_ERROR_INVALID_ARGUMENT;
}

static void test_failing_callbacks(void) {
  struct ui_file_uploader_base uploader;

  ui_file_uploader_init(&uploader, 2, 0, 0, 100, 100, NULL);
  uploader.on_touched_cb = mock_touch_fail;
  ui_file_uploader_on_drag_enter(&uploader);
  ui_file_uploader_destroy(&uploader);

  ui_file_uploader_init(&uploader, 2, 0, 0, 100, 100, NULL);
  uploader.on_touched_cb = mock_touch_fail;
  ui_file_uploader_drop_file(&uploader, "test.txt");
  ui_file_uploader_destroy(&uploader);

  ui_file_uploader_init(&uploader, 2, 0, 0, 100, 100, NULL);
  uploader.on_change_cb = mock_change_fail;
  ui_file_uploader_drop_file(&uploader, "test.txt");
  ui_file_uploader_destroy(&uploader);

  FILE *fp = fopen("fail_test.txt", "w");
  if (fp) {
    fprintf(fp, "123");
    fclose(fp);
  }

  ui_file_uploader_init(&uploader, 2, 0, 0, 100, 100, NULL);
  ui_file_uploader_drop_file(&uploader, "fail_test.txt");
  uploader.on_change_cb = mock_change_fail;
  ui_file_uploader_read_files(&uploader);
  ui_file_uploader_destroy(&uploader);

  remove("fail_test.txt");
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("run_normal_tests failed\n");
    return 1;
  }
  if (run_oom_tests() != 0) {
    printf("run_oom_tests failed\n");
    return 1;
  }
  test_failing_callbacks();
  int cov_ret = run_coverage_tests();
  if (cov_ret != 0) {
    printf("run_coverage_tests failed: %d\n", cov_ret);
    return 1;
  }
  printf("All ui_file_uploader_base tests passed.\n");
  return 0;
}
