/* clang-format off */
#include "../include/ui_fs.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_ui_fs_fseek_fail = 0;
int g_ui_fs_ftell_fail = 0;
int g_ui_fs_fread_fail = 0;
int g_ui_fs_fwrite_fail = 0;

/*
 * @brief mock_fseek.
 * @param stream Parameter stream.
 * @param offset Parameter offset.
 * @param whence Parameter whence.
 * @return Return value.
 */
static int mock_fseek(FILE *stream, long offset, int whence) {
  if (g_ui_fs_fseek_fail > 0) {
    g_ui_fs_fseek_fail--;
    if (g_ui_fs_fseek_fail == 0)
      return -1;
  }
  return fseek(stream, offset, whence);
}
/*
 * @brief mock_ftell.
 * @param stream Parameter stream.
 * @return Return value.
 */
static long mock_ftell(FILE *stream) {
  if (g_ui_fs_ftell_fail == 1)
    return -1;
  if (g_ui_fs_ftell_fail == 2)
    return 0x40000000; /* Trigger huge file size check */
  return ftell(stream);
}
/*
 * @brief mock_fread.
 * @param ptr Parameter ptr.
 * @param size Parameter size.
 * @param nmemb Parameter nmemb.
 * @param stream Parameter stream.
 * @return Return value.
 */
static size_t mock_fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  if (g_ui_fs_fread_fail)
    return 0;
  return fread(ptr, size, nmemb, stream);
}
/*
 * @brief mock_fwrite.
 * @param ptr Parameter ptr.
 * @param size Parameter size.
 * @param nmemb Parameter nmemb.
 * @param stream Parameter stream.
 * @return Return value.
 */
static size_t mock_fwrite(const void *ptr, size_t size, size_t nmemb,
                          FILE *stream) {
  if (g_ui_fs_fwrite_fail)
    return 0;
  return fwrite(ptr, size, nmemb, stream);
}
/** @cond */
#define UI_FSEEK mock_fseek
/** @endcond */
/** @cond */
#define UI_FTELL mock_ftell
/** @endcond */
/** @cond */
#define UI_FREAD mock_fread
/** @endcond */
/** @cond */
#define UI_FWRITE mock_fwrite
/** @endcond */
#else
/** @cond */
#define UI_FSEEK fseek
/** @endcond */
/** @cond */
#define UI_FTELL ftell
/** @endcond */
/** @cond */
#define UI_FREAD fread
/** @endcond */
/** @cond */
#define UI_FWRITE fwrite
/** @endcond */
#endif

#if defined(__EMSCRIPTEN__)
EM_JS(int, fs_read_file_js,
      (const char *path_cstr, int *out_ptr, int *out_size), {
        const path = UTF8ToString(path_cstr);
        try {
          const data = FS.readFile(path);
          const length = data.length;
          const ptr = _malloc(length);
          if (!ptr)
            return 1; /* OOM */
          HEAPU8.set(data, ptr);
          setValue(out_ptr, ptr, "i32");
          setValue(out_size, length, "i32");
          return 0; /* Success */
        } catch (e) {
          console.error("FS.readFile failed", e);
          return 2; /* IO Error */
        }
      })

EM_JS(int, fs_write_file_js,
      (const char *path_cstr, const void *data, int size), {
        const path = UTF8ToString(path_cstr);
        try {
          const u8 = new Uint8Array(HEAPU8.buffer, data, size);
          FS.writeFile(path, u8);
          return 0; /* Success */
        } catch (e) {
          console.error("FS.writeFile failed", e);
          return 2; /* IO Error */
        }
      })
#endif

/*
 * @brief ui_fs_read_file.
 * @param path Parameter path.
 * @param out_data Parameter out_data.
 * @param out_size Parameter out_size.
 * @return Return value.
 */
ui_error_t ui_fs_read_file(const char *path, void **out_data,
                           size_t *out_size) {
  if (!path || !out_data || !out_size)
    return UI_ERROR_INVALID_ARGUMENT;

#if defined(__EMSCRIPTEN__)
  {
    int ptr = 0;
    int size = 0;
    int err = fs_read_file_js(path, &ptr, &size);
    if (err == 1)
      return UI_ERROR_OUT_OF_MEMORY;
    if (err == 2)
      return UI_ERROR_IO_FAILED;

    *out_data = (void *)(intptr_t)ptr;
    *out_size = (size_t)size;
    return UI_ERROR_NONE;
  }
#else
  {
    /* Use standard C library for native builds */
    FILE *f = fopen(path, "rb");
    long sz;
    size_t read_bytes;

    if (!f)
      return UI_ERROR_IO_FAILED;

    if (UI_FSEEK(f, 0, SEEK_END) != 0) {
      fclose(f);
      return UI_ERROR_IO_FAILED;
    }
    sz = UI_FTELL(f);
    if (UI_FSEEK(f, 0, SEEK_SET) != 0) {
      fclose(f);
      return UI_ERROR_IO_FAILED;
    }

    if (sz < 0) {
      fclose(f);
      return UI_ERROR_IO_FAILED;
    }

    /* Prevent arbitrarily huge file sizes like LONG_MAX on directories */
    if ((size_t)sz > (size_t)0x3fffffff) {
      fclose(f);
      return UI_ERROR_IO_FAILED;
    }

    *out_data = C_MULTIPLATFORM_MALLOC((size_t)sz + 1);
    if (!*out_data) {
      fclose(f);
      return UI_ERROR_OUT_OF_MEMORY;
    }

    read_bytes = UI_FREAD(*out_data, 1, (size_t)sz, f);
    fclose(f);

    if (read_bytes != (size_t)sz) {
      C_MULTIPLATFORM_FREE(*out_data);
      *out_data = NULL;
      return UI_ERROR_IO_FAILED;
    }

    ((char *)*out_data)[sz] =
        '\0'; /* Null terminate for safety if reading text */
    *out_size = (size_t)sz;
    return UI_ERROR_NONE;
  }
#endif
}

/*
 * @brief ui_fs_write_file.
 * @param path Parameter path.
 * @param data Parameter data.
 * @param size Parameter size.
 * @return Return value.
 */
ui_error_t ui_fs_write_file(const char *path, const void *data, size_t size) {
  if (!path || !data)
    return UI_ERROR_INVALID_ARGUMENT;

#if defined(__EMSCRIPTEN__)
  {
    int err = fs_write_file_js(path, data, (int)size);
    if (err != 0)
      return UI_ERROR_IO_FAILED;
    return UI_ERROR_NONE;
  }
#else
  {
    FILE *f = fopen(path, "wb");
    size_t written;

    if (!f)
      return UI_ERROR_IO_FAILED;

    written = UI_FWRITE(data, 1, size, f);
    fclose(f);

    if (written != size)
      return UI_ERROR_IO_FAILED;
    return UI_ERROR_NONE;
  }
#endif
}

#if defined(__EMSCRIPTEN__)
EM_JS(int, fs_opfs_write_sync_js,
      (const char *path_cstr, const void *data, int size), {
        const path = UTF8ToString(path_cstr);
        const u8 = new Uint8Array(HEAPU8.buffer, data, size);

        try {
          if (!navigator.storage || !navigator.storage.getDirectory) {
            console.error("OPFS not supported");
            return 3; /* Unsupported */
          }

          /* This is a naive */ synchronous wrapper attempt (in a real WebWorker
          /* this would use */ createSyncAccessHandle) Since we cannot block the
          /* main thread synchronously */ in JS easily without Asyncify, we will
          /* simulate the error */ code return for now in non-worker environments
          /* if true sync */ OPFS is requested on main thread.
          console.error("Synchronous OPFS writes must be performed in a "
                        "WebWorker context.");
          return 3;
        } catch (e) {
          console.error("OPFS sync write failed", e);
          return 2;
        }
      })
#endif

/*
 * @brief ui_fs_write_file_opfs_sync.
 * @param path Parameter path.
 * @param data Parameter data.
 * @param size Parameter size.
 * @return Return value.
 */
ui_error_t ui_fs_write_file_opfs_sync(const char *path, const void *data,
                                      size_t size) {
  if (!path || !data)
    return UI_ERROR_INVALID_ARGUMENT;

#if defined(__EMSCRIPTEN__)
  {
    int err = fs_opfs_write_sync_js(path, data, (int)size);
    if (err == 3)
      return UI_ERROR_UNSUPPORTED;
    if (err != 0)
      return UI_ERROR_IO_FAILED;
    return UI_ERROR_NONE;
  }
#else
  /* Fallback to standard write on native */
  return ui_fs_write_file(path, data, size);
#endif
}
