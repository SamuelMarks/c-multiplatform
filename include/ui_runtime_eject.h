/**
 * @file ui_runtime_eject.h
 * @brief Ahead-of-Time (AoT) code generator ejecting UI Schema ASTs to native C
 * code.
 */

#ifndef UI_RUNTIME_EJECT_H
#define UI_RUNTIME_EJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_runtime_schema.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Traverses an AST node tree and emits equivalent C89 source and header
 * code into memory buffers.
 *
 * @param node Root AST node.
 * @param func_name Name of the C mount function to generate.
 * @param c_buf Buffer to receive the generated C source code.
 * @param c_buf_size Size of the C source buffer.
 * @param h_buf Buffer to receive the generated C header code.
 * @param h_buf_size Size of the C header buffer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_runtime_eject_tree_to_c_buffer(
    const struct ui_runtime_node *node, const char *func_name, char *c_buf,
    size_t c_buf_size, char *h_buf, size_t h_buf_size);

/**
 * @brief Ejects an application manifest's routes into generated .c and .h
 * files.
 *
 * @param manifest The application manifest to eject.
 * @param func_name Base function name.
 * @param output_c_path Path of the .c file to write.
 * @param output_h_path Path of the .h file to write.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_runtime_eject_to_c(
    const struct ui_runtime_app_manifest *manifest, const char *func_name,
    const char *output_c_path, const char *output_h_path);

/**
 * @brief Ejects a single widget tree AST node into generated .c and .h files.
 *
 * @param node Root AST node.
 * @param func_name Base function name.
 * @param output_c_path Path of the .c file to write.
 * @param output_h_path Path of the .h file to write.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_runtime_eject_node_to_c(
    const struct ui_runtime_node *node, const char *func_name,
    const char *output_c_path, const char *output_h_path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RUNTIME_EJECT_H */
