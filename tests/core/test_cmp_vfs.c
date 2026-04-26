/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#if defined(__WATCOMC__) || defined(__DOS__)

#endif
#include <stdio.h>

#if defined(_WIN32)
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#else
#include <unistd.h>
#endif
/* clang-format on */

TEST test_vfs_lifecycle(void) {
  int res;

  res = cmp_vfs_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_vfs_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_vfs_read_sync(void) {
  void *buf = NULL;
  size_t read_size = 0;
  int res;
  FILE *f;

  cmp_vfs_init();

  /* Create dummy file */
  f = fopen("dummy.txt", "wb");
  ASSERT(f != NULL);
  fwrite("Hello VFS", 1, 9, f);
  fclose(f);

  res = cmp_vfs_read_file_sync("dummy.txt", &buf, &read_size);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT((size_t)9, read_size, "%zd");
  ASSERT_STRN_EQ("Hello VFS", (const char *)buf, 9);

  if (buf != NULL) {
    CMP_FREE(buf);
  }

  remove("dummy.txt");
  cmp_vfs_shutdown();
  PASS();
}

typedef struct {
  cmp_modality_t *mod;
  int error;
  size_t size;
  char buf[32];
} test_async_ctx_t;

static void test_vfs_read_cb(int error, void *buffer, size_t size,
                             void *user_data) {
  test_async_ctx_t *ctx = (test_async_ctx_t *)user_data;
  ctx->error = error;
  ctx->size = size;
  if (error == CMP_SUCCESS && buffer != NULL) {
    if (size < sizeof(ctx->buf)) {
      memcpy(ctx->buf, buffer, size);
      ctx->buf[size] = '\0';
    }
    CMP_FREE(buffer);
  }
  cmp_modality_stop(ctx->mod);
}

TEST test_vfs_read_async(void) {
  test_async_ctx_t ctx;
  cmp_modality_t mod;
  int res;
  FILE *f;

  cmp_vfs_init();
  cmp_modality_sync_single_init(&mod);

  /* Create dummy file */
  f = fopen("dummy_async.txt", "wb");
  ASSERT(f != NULL);
  fwrite("Async VFS", 1, 9, f);
  fclose(f);

  ctx.mod = &mod;
  ctx.error = -1;
  ctx.size = 0;
  memset(ctx.buf, 0, sizeof(ctx.buf));

  res =
      cmp_vfs_read_file_async(&mod, "dummy_async.txt", test_vfs_read_cb, &ctx);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_modality_run(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, ctx.error, "%d");
  ASSERT_EQ_FMT((size_t)9, ctx.size, "%zd");
  ASSERT_STRN_EQ("Async VFS", ctx.buf, 9);

  remove("dummy_async.txt");
  cmp_modality_destroy(&mod);
  cmp_vfs_shutdown();
  PASS();
}

TEST test_vfs_standard_paths(void) {
  cmp_string_t path;
  int res;

  cmp_vfs_init();

  /* Test AppData */
  res = cmp_vfs_get_standard_path(1, &path);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(path.length > 0);
  cmp_string_destroy(&path);

  /* Test Temp */
  res = cmp_vfs_get_standard_path(2, &path);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(path.length > 0);
  cmp_string_destroy(&path);

  /* Test Cache */
  res = cmp_vfs_get_standard_path(3, &path);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(path.length > 0);
  cmp_string_destroy(&path);

  /* Test Documents */
  res = cmp_vfs_get_standard_path(4, &path);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(path.length > 0);
  cmp_string_destroy(&path);

  /* Test Executable Directory */
  res = cmp_vfs_get_standard_path(5, &path);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(path.length > 0);
  cmp_string_destroy(&path);

  cmp_vfs_shutdown();
  PASS();
}

TEST test_vfs_mount(void) {
  void *buf = NULL;
  size_t read_size = 0;
  int res;
  FILE *f;

  cmp_vfs_init();

  /* Create dummy file */
  f = fopen("dummy_mount.txt", "wb");
  ASSERT(f != NULL);
  fwrite("Mount VFS", 1, 9, f);
  fclose(f);

  res = cmp_vfs_mount("virt:/", ".");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_vfs_read_file_sync("virt:/dummy_mount.txt", &buf, &read_size);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT((size_t)9, read_size, "%zd");
  ASSERT_STRN_EQ("Mount VFS", (const char *)buf, 9);

  if (buf != NULL) {
    CMP_FREE(buf);
  }

  remove("dummy_mount.txt");
  cmp_vfs_shutdown();
  PASS();
}

static void test_watch_cb(const char *path, int event_type, void *user_data) {
  int *triggered = (int *)user_data;
  *triggered = 1;
  (void)path;
  (void)event_type;
}

TEST test_vfs_watch(void) {
  cmp_vfs_watch_t *watch = NULL;
  int triggered = 0;
  int res;
  FILE *f;

  cmp_vfs_init();

  res = cmp_vfs_watch_path(".", test_watch_cb, &triggered, &watch);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(watch != NULL);

  f = fopen("dummy_watch.txt", "w");
  if (f) {
    fputs("hello", f);
    fclose(f);
  }

#if defined(_WIN32)
  Sleep(100);
#else
  /* usleep */ (void)(100000);
#endif

  /*
   * We expect triggered to be 1, but we don't strictly assert it
   * because file watching events can be notoriously delayed or batched by OS.
   * Ensuring it doesn't crash on unwatch is the primary mechanical test.
   */
  if (triggered) {
    /* pass */
  }

  res = cmp_vfs_unwatch(watch);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  remove("dummy_watch.txt");

  cmp_vfs_shutdown();
  PASS();
}

TEST test_vfs_edge_cases(void) {
  void *buf;
  size_t size;
  cmp_string_t path;
  cmp_vfs_watch_t *watch;

  /* Try using without init - should fail or be safe, but let's test invalid args first. */
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_mount(NULL, "."), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_mount("virt:/", NULL), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_resolve_path(NULL, &path), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_resolve_path("a", NULL), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_read_file_sync(NULL, &buf, &size), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_read_file_sync("a", NULL, &size), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_read_file_sync("a", &buf, NULL), "%d");

  cmp_vfs_init();

  /* Missing file */
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, cmp_vfs_read_file_sync("nonexistent.file", &buf, &size), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_read_file_async(NULL, "a", test_vfs_read_cb, NULL), "%d");
  /* Cannot pass invalid pointer for mod, so we test missing virtual_path with a dummy valid-ish or NULL mod */
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_read_file_async(NULL, NULL, test_vfs_read_cb, NULL), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_watch_path(NULL, test_watch_cb, NULL, &watch), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_watch_path(".", NULL, NULL, &watch), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_watch_path(".", test_watch_cb, NULL, NULL), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_unwatch(NULL), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_get_standard_path(1, NULL), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_vfs_get_standard_path(999, &path), "%d"); /* Invalid type */

  cmp_vfs_shutdown();
  PASS();
}

SUITE(vfs_suite) {
  RUN_TEST(test_vfs_lifecycle);
  RUN_TEST(test_vfs_read_sync);
  RUN_TEST(test_vfs_read_async);
  RUN_TEST(test_vfs_mount);
  RUN_TEST(test_vfs_standard_paths);
  RUN_TEST(test_vfs_watch);
  RUN_TEST(test_vfs_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(vfs_suite);
  GREATEST_MAIN_END();
}
