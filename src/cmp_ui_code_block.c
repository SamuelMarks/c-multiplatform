/* clang-format off */
#include "cmp_ui_code_block.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Code Block widget.
 */
struct cmp_ui_code_block {
  /** @brief The root node of the code block */
  cmp_ui_node_t *node_root;
  /** @brief The text node containing the code block text */
  cmp_ui_node_t *node_text;
  /** @brief The raw string of the code */
  char *code;
  /** @brief The programming language hint */
  char *language;
};

/**
 * @brief cmp_ui_code_block_create
 *
 * @param out_block Pointer to store the created code block.
 * @param code The source code text.
 * @param language The programming language.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_code_block_create(cmp_ui_code_block_t **out_block, const char *code,
                             const char *language) {
  int rc = CMP_SUCCESS;
  cmp_ui_code_block_t *block = NULL;
  size_t len;

  if (!out_block) {
    LOG_DEBUG("cmp_ui_code_block_create: out_block is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_code_block_t), (void **)&block);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_code_block_create: OOM\n");
    return CMP_ERROR_OOM;
  }
  memset(block, 0, sizeof(cmp_ui_code_block_t));

  block->code = NULL;
  if (code) {
    len = strlen(code);
    rc = CMP_MALLOC(len + 1, (void **)&(block->code));
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_code_block_create: OOM code\n");
      rc = CMP_FREE(block);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_code_block_create: CMP_FREE block failed\n");
      }
      return CMP_ERROR_OOM;
    }
    memcpy(block->code, code, len + 1);
  }

  block->language = NULL;
  if (language) {
    len = strlen(language);
    rc = CMP_MALLOC(len + 1, (void **)&(block->language));
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_code_block_create: OOM language\n");
      if (block->code) {
        int free_rc = CMP_FREE(block->code);
        if (free_rc != CMP_SUCCESS)
          LOG_DEBUG("cmp_ui_code_block_create: CMP_FREE code failed\n");
      }
      rc = CMP_FREE(block);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_code_block_create: CMP_FREE block failed\n");
      }
      return CMP_ERROR_OOM;
    }
    memcpy(block->language, language, len + 1);
  }

  rc = cmp_ui_box_create(&block->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_code_block_create: cmp_ui_box_create failed\n");
    if (block->code) {
      int free_rc = CMP_FREE(block->code);
      if (free_rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_code_block_create: CMP_FREE code failed\n");
    }
    if (block->language) {
      int free_rc = CMP_FREE(block->language);
      if (free_rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_code_block_create: CMP_FREE language failed\n");
    }
    rc = CMP_FREE(block);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_code_block_create: CMP_FREE block failed\n");
    return CMP_ERROR_GENERAL;
  }

  rc =
      cmp_ui_text_create(&block->node_text, block->code ? block->code : "", -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_code_block_create: cmp_ui_text_create failed\n");
    if (block->code) {
      int free_rc = CMP_FREE(block->code);
      if (free_rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_code_block_create: CMP_FREE code failed\n");
    }
    if (block->language) {
      int free_rc = CMP_FREE(block->language);
      if (free_rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_code_block_create: CMP_FREE language failed\n");
    }
    rc = cmp_ui_node_destroy(block->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_code_block_create: cmp_ui_node_destroy failed\n");

    rc = CMP_FREE(block);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_code_block_create: CMP_FREE block failed\n");
    return CMP_ERROR_GENERAL;
  }

  rc = cmp_ui_node_add_child(block->node_root, block->node_text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_code_block_create: cmp_ui_node_add_child failed\n");
  }

  *out_block = block;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_code_block_destroy
 *
 * @param block The code block component.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_code_block_destroy(cmp_ui_code_block_t *block) {
  int rc = CMP_SUCCESS;
  if (!block) {
    LOG_DEBUG("cmp_ui_code_block_destroy: block is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (block->code) {
    rc = CMP_FREE(block->code);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_code_block_destroy: CMP_FREE code failed\n");
    }
  }
  if (block->language) {
    rc = CMP_FREE(block->language);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_code_block_destroy: CMP_FREE language failed\n");
    }
  }
  if (block->node_root) {
    rc = cmp_ui_node_destroy(block->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_code_block_destroy: cmp_ui_node_destroy failed\n");
    }
  }
  rc = CMP_FREE(block);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_code_block_destroy: CMP_FREE block failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_code_block_get_node
 *
 * @param block The code block component.
 * @param out_node Pointer to store the underlying UI node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_code_block_get_node(cmp_ui_code_block_t *block,
                               cmp_ui_node_t **out_node) {
  if (!block || !out_node) {
    LOG_DEBUG("cmp_ui_code_block_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = block->node_root;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_code_block_set_code
 *
 * @param block The code block component.
 * @param code The new code string.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_code_block_set_code(cmp_ui_code_block_t *block, const char *code) {
  int rc = CMP_SUCCESS;
  size_t len;

  if (!block) {
    LOG_DEBUG("cmp_ui_code_block_set_code: block is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (block->code) {
    rc = CMP_FREE(block->code);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_code_block_set_code: CMP_FREE old code failed\n");
    }
    block->code = NULL;
  }

  if (code) {
    len = strlen(code);
    rc = CMP_MALLOC(len + 1, (void **)&(block->code));
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_code_block_set_code: OOM\n");
      return CMP_ERROR_OOM;
    }
    memcpy(block->code, code, len + 1);

    if (block->node_text && block->node_text->properties) {
      rc = CMP_FREE(block->node_text->properties);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_code_block_set_code: CMP_FREE properties failed\n");
    }
    if (block->node_text) {
      rc = CMP_MALLOC(len + 1, (void **)&block->node_text->properties);
      if (rc == CMP_SUCCESS) {
        memcpy(block->node_text->properties, block->code, len + 1);
      } else {
        LOG_DEBUG("cmp_ui_code_block_set_code: OOM properties\n");
      }
    }

  } else {
    if (block->node_text && block->node_text->properties) {
      rc = CMP_FREE(block->node_text->properties);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_code_block_set_code: CMP_FREE properties failed\n");
    }
    if (block->node_text) {
      rc = CMP_MALLOC(1, (void **)&block->node_text->properties);
      if (rc == CMP_SUCCESS) {
        ((char *)block->node_text->properties)[0] = '\0';
      } else {
        LOG_DEBUG("cmp_ui_code_block_set_code: OOM properties\n");
      }
    }
  }

  return CMP_SUCCESS;
}
