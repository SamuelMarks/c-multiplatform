#ifndef CMP_TREE_SITTER_H
#define CMP_TREE_SITTER_H

/* clang-format off */
#include "cmp_ffi.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_tree_sitter.h
 * @brief Tree-sitter base engine wrapper.
 */

typedef struct cmp_tree_sitter cmp_tree_sitter_t;
typedef struct cmp_tree_node cmp_tree_node_t;

/**
 * @brief Creates a Tree-sitter parser instance.
 * @param out_ts Pointer to receive the instance.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_tree_sitter_create(cmp_tree_sitter_t **out_ts);

/**
 * @brief Destroys a Tree-sitter parser instance.
 * @param ts The instance to destroy.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_tree_sitter_destroy(cmp_tree_sitter_t *ts);

/**
 * @brief Parses source code into a syntax tree representation.
 * @param ts The parser instance.
 * @param language The language identifier (e.g., "c", "python").
 * @param source_code The source code string.
 * @param out_root Pointer to receive the root node.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_tree_sitter_parse(cmp_tree_sitter_t *ts, const char *language,
                                  const char *source_code,
                                  cmp_tree_node_t **out_root);

/**
 * @brief Retrieves the type of a syntax node.
 * @param node The syntax node.
 * @param out_type Pointer to receive the type string. Must be freed.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_tree_sitter_node_get_type(cmp_tree_node_t *node,
                                          char **out_type);

/**
 * @brief Frees a syntax tree node and its children.
 * @param node The node to free.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_tree_sitter_free_node(cmp_tree_node_t *node);

/**
 * @brief Frees a string returned by the Tree-sitter wrapper.
 * @param str The string to free.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_tree_sitter_free_string(char *str);

#ifdef __cplusplus
}
#endif

#endif /* CMP_TREE_SITTER_H */
