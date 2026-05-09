#ifndef CMP_PTY_H
#define CMP_PTY_H

/* clang-format off */
#include "cmp_ffi.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file cmp_pty.h
 * @brief PTY (Pseudo-terminal) OS bindings and raw subprocess spawning.
 */

typedef struct cmp_pty cmp_pty_t;

/**
 * @brief Creates a pseudo-terminal instance and spawns a command.
 * @param out_pty Pointer to receive the created instance.
 * @param command The command to execute.
 * @param cols The initial number of columns.
 * @param rows The initial number of rows.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_pty_create(cmp_pty_t **out_pty, const char *command, int cols,
                           int rows);

/**
 * @brief Destroys a pseudo-terminal instance.
 * @param pty The instance to destroy.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_pty_destroy(cmp_pty_t *pty);

/**
 * @brief Resizes the pseudo-terminal.
 * @param pty The PTY instance.
 * @param cols The new number of columns.
 * @param rows The new number of rows.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_pty_resize(cmp_pty_t *pty, int cols, int rows);

/**
 * @brief Writes data to the pseudo-terminal.
 * @param pty The PTY instance.
 * @param data The data to write.
 * @param len The number of bytes to write.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_pty_write(cmp_pty_t *pty, const void *data, unsigned int len);

/**
 * @brief Reads data from the pseudo-terminal.
 * @param pty The PTY instance.
 * @param out_buffer Pointer to a buffer to receive data.
 * @param buffer_size The size of the buffer.
 * @param out_read Pointer to receive the number of bytes read.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_pty_read(cmp_pty_t *pty, void *out_buffer,
                         unsigned int buffer_size, unsigned int *out_read);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_PTY_H */
