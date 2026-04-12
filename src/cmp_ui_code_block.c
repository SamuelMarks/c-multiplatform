/* clang-format off */
#include "cmp_ui_code_block.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_code_block {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *code;
  char *language;
};

int cmp_ui_code_block_create(cmp_ui_code_block_t **out_block, const char *code,
                             const char *language) {
  cmp_ui_code_block_t *block;
  int err;
  size_t len;

  if (!out_block) {
    return CMP_ERROR_INVALID_ARG;
  }

  block = (cmp_ui_code_block_t *)malloc(sizeof(cmp_ui_code_block_t));
  if (!block) {
    return CMP_ERROR_OOM;
  }

  block->code = NULL;
  if (code) {
    len = strlen(code);
    block->code = (char *)malloc(len + 1);
    if (block->code) {
      memcpy(block->code, code, len + 1);
    }
  }

  block->language = NULL;
  if (language) {
    len = strlen(language);
    block->language = (char *)malloc(len + 1);
    if (block->language) {
      memcpy(block->language, language, len + 1);
    }
  }

  err = cmp_ui_box_create(&block->node_root);
  if (err != 0) {
    free(block->code);
    free(block->language);
    free(block);
    return err;
  }

  err =
      cmp_ui_text_create(&block->node_text, block->code ? block->code : "", -1);
  if (err != 0) {
    free(block->code);
    free(block->language);
    free(block);
    return err;
  }

  cmp_ui_node_add_child(block->node_root, block->node_text);

  *out_block = block;
  return 0;
}

int cmp_ui_code_block_destroy(cmp_ui_code_block_t *block) {
  if (!block) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(block->code);
  free(block->language);
  free(block);
  return 0;
}

int cmp_ui_code_block_get_node(cmp_ui_code_block_t *block,
                               cmp_ui_node_t **out_node) {
  if (!block || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = block->node_root;
  return 0;
}

int cmp_ui_code_block_set_code(cmp_ui_code_block_t *block, const char *code) {
  size_t len;

  if (!block) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (block->code) {
    free(block->code);
    block->code = NULL;
  }

  if (code) {
    len = strlen(code);
    block->code = (char *)malloc(len + 1);
    if (!block->code) {
      return CMP_ERROR_OOM;
    }
    memcpy(block->code, code, len + 1);
  }

  return 0;
}
