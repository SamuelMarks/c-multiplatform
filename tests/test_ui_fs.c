/* clang-format off */
#include "../include/ui_fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#else
#include <unistd.h>
#endif

extern int g_malloc_fail_countdown;
#ifdef UI_TEST_MOCK_ALLOC
extern int g_ui_fs_fseek_fail;
extern int g_ui_fs_ftell_fail;
extern int g_ui_fs_fread_fail;
extern int g_ui_fs_fwrite_fail;
#endif
/* clang-format on */

#define EXPECT(cond) failed |= !(cond)

static int run_test(const char *name, int (*test_fn)(void)) {
  int rc;
  printf("Running %s... ", name);
  rc = test_fn();
  printf(rc == 0 ? "PASS\n" : "FAIL\n");
  return rc;
}

static int test_fs_read_write(void) {
  int failed = 0;
  enum ui_error err;
  const char *path =
      "/preload.txt"; /* Should exist in Wasm due to --preload-file, fallback to
                         write for native */
  void *data = NULL;
  size_t size = 0;

#if !defined(__EMSCRIPTEN__)
  path = "test_fs_temp.txt";
  err = ui_fs_write_file(path, "test_data", 9);
  EXPECT(err == UI_ERROR_NONE);
#endif

  err = ui_fs_read_file(path, &data, &size);
  EXPECT(err == UI_ERROR_NONE);
  EXPECT(size != 0);
  EXPECT(data != NULL);

  free(data); /* JS _malloced buffer or native UI_MALLOC */

#if !defined(__EMSCRIPTEN__)
  remove(path);
#endif

  return failed;
}

static int test_fs_opfs_write(void) {
  int failed = 0;
  enum ui_error err;
  const char *path = "opfs_test.txt";
  const char *data = "opfs_data";

  err = ui_fs_write_file_opfs_sync(path, data, 9);
#if defined(__EMSCRIPTEN__)
  /* Expected to fail with unsupported when run on main thread without Worker
   * config */
  EXPECT(err == UI_ERROR_UNSUPPORTED);
#else
  EXPECT(err == UI_ERROR_NONE);
  remove(path);
#endif
  return failed;
}

static int test_fs_errors(void) {
  int failed = 0;
  void *data;
  size_t size;

  EXPECT(ui_fs_read_file(NULL, &data, &size) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_fs_read_file("some_path", NULL, &size) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_fs_read_file("some_path", &data, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  EXPECT(ui_fs_write_file(NULL, "data", 4) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_fs_write_file("path", NULL, 4) == UI_ERROR_INVALID_ARGUMENT);

  EXPECT(ui_fs_write_file_opfs_sync(NULL, "data", 4) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_fs_write_file_opfs_sync("path", NULL, 4) ==
         UI_ERROR_INVALID_ARGUMENT);

  EXPECT(ui_fs_read_file("this_file_does_not_exist_ever_9999.txt", &data,
                         &size) == UI_ERROR_IO_FAILED);
  EXPECT(ui_fs_write_file("/this_path_is_invalid_for_writing/fail.txt", "data",
                          4) == UI_ERROR_IO_FAILED);

  /* Try to trigger failure using a directory path for read and write (may fail
   * fread or sz < 0 or fwrite) */
  EXPECT(ui_fs_read_file(".", &data, &size) == UI_ERROR_IO_FAILED);
  EXPECT(ui_fs_write_file(".", "data", 4) == UI_ERROR_IO_FAILED);

#ifdef UI_TEST_MOCK_ALLOC
  {
    /* Test OOM during read */
    const char *path = "test_fs_temp2.txt";
    ui_fs_write_file(path, "data", 4);

    g_malloc_fail_countdown = 0;
    EXPECT(ui_fs_read_file(path, &data, &size) == UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;
    remove(path);
  }
#endif

#ifdef UI_TEST_MOCK_ALLOC
  {
    const char *path = "test_fs_mock.txt";
    ui_fs_write_file(path, "data", 4);

    g_ui_fs_fseek_fail = 1;
    EXPECT(ui_fs_read_file(path, &data, &size) == UI_ERROR_IO_FAILED);
    g_ui_fs_fseek_fail = 0;

    g_ui_fs_fseek_fail = 2;
    EXPECT(ui_fs_read_file(path, &data, &size) == UI_ERROR_IO_FAILED);
    g_ui_fs_fseek_fail = 0;

    g_ui_fs_ftell_fail = 1;
    EXPECT(ui_fs_read_file(path, &data, &size) == UI_ERROR_IO_FAILED);
    g_ui_fs_ftell_fail = 0;

    g_ui_fs_ftell_fail = 2;
    EXPECT(ui_fs_read_file(path, &data, &size) == UI_ERROR_IO_FAILED);
    g_ui_fs_ftell_fail = 0;

    g_ui_fs_fread_fail = 1;
    EXPECT(ui_fs_read_file(path, &data, &size) == UI_ERROR_IO_FAILED);
    g_ui_fs_fread_fail = 0;

    g_ui_fs_fwrite_fail = 1;
    EXPECT(ui_fs_write_file(path, "data", 4) == UI_ERROR_IO_FAILED);
    g_ui_fs_fwrite_fail = 0;

    remove(path);
  }
#endif

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= run_test("test_fs_read_write", test_fs_read_write);
  failed |= run_test("test_fs_opfs_write", test_fs_opfs_write);
  failed |= run_test("test_fs_errors", test_fs_errors);
  return failed == 0 ? 0 : 1;
}
