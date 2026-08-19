#ifndef UI_FS_H
#define UI_FS_H

/**
 * @file ui_fs.h
 * @brief File system operations.
 *
 * Provides cross-platform file reading and writing utilities.
 */

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reads a file into memory.
 *
 * @param path The path of the file to read.
 * @param out_data Pointer to receive the allocated data buffer. Caller must
 * free this memory.
 * @param out_size Pointer to receive the size of the read data.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fs_read_file(const char *path, void **out_data, size_t *out_size);

/**
 * @brief Writes data to a file.
 *
 * @param path The path of the file to write to.
 * @param data The data buffer to write.
 * @param size The size of the data in bytes.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fs_write_file(const char *path, const void *data, size_t size);

/**
 * @brief Writes a file synchronously using OPFS in WebAssembly (requires
 * WebWorker context). Falls back to standard synchronous write on native
 * platforms.
 *
 * @param path The file path.
 * @param data The binary data to write.
 * @param size The size of the data in bytes.
 * @return UI_ERROR_NONE on success, UI_ERROR_UNSUPPORTED if OPFS is not
 * available or called on main thread without support.
 */
ui_error_t ui_fs_write_file_opfs_sync(const char *path, const void *data,
                                      size_t size);

#ifdef __cplusplus
}
#endif
#endif /* UI_FS_H */
