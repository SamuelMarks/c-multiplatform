/* clang-format off */
#include "cmp_ui_chip.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_chip {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *text;
};

/**
 * @brief cmp_ui_chip_create
 *
 * @param out_chip Parameter description.
 * @param text Parameter description.
 * @param bg_color Parameter description.
 * @param text_color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_chip_create(cmp_ui_chip_t **out_chip, const char *text,
                       uint32_t bg_color, uint32_t text_color) {
  cmp_ui_chip_t *chip;
  int rc;
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!out_chip) {
    LOG_DEBUG("cmp_ui_chip_create: out_chip is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_chip_t), (void **)&chip);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_chip_create: OOM\n");
    return rc;
  }
  memset(chip, 0, sizeof(cmp_ui_chip_t));

  if (text) {
    rc = cmp_i18n_translate(text, &translated);
    if (rc == CMP_SUCCESS && translated.data) {
      final_text = translated.data;
    } else if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_create: cmp_i18n_translate failed\n");
    }
  }

  chip->text = NULL;
  if (final_text) {
    len = strlen(final_text);
    rc = CMP_MALLOC(len + 1, (void **)&chip->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_create: OOM for text\n");
      if (translated.data) {
        rc = cmp_string_destroy(&translated);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_chip_create: cmp_string_destroy failed\n");
        }
      }
      CMP_FREE(chip);
      return CMP_ERROR_OOM;
    }
#if defined(_MSC_VER)
    if (memcpy_s(chip->text, len + 1, final_text, len + 1) != 0) {
      LOG_DEBUG("cmp_ui_chip_create: memcpy_s failed\n");
      CMP_FREE(chip->text);
      if (translated.data) {
        rc = cmp_string_destroy(&translated);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_chip_create: cmp_string_destroy failed\n");
        }
      }
      CMP_FREE(chip);
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(chip->text, final_text, len + 1);
#endif
  }

  if (translated.data) {
    rc = cmp_string_destroy(&translated);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_create: cmp_string_destroy failed\n");
    }
  }

  rc = cmp_ui_box_create(&chip->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_chip_create: cmp_ui_box_create failed\n");
    if (chip->text) {
      CMP_FREE(chip->text);
    }
    CMP_FREE(chip);
    return rc;
  }

  chip->node_root->bg_color = bg_color;

  rc = cmp_ui_text_create(&chip->node_text, chip->text ? chip->text : "", -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_chip_create: cmp_ui_text_create failed\n");
    if (chip->text) {
      CMP_FREE(chip->text);
    }
    rc = cmp_ui_node_destroy(chip->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_create: cmp_ui_node_destroy failed\n");
    }
    CMP_FREE(chip);
    return rc; /* original error */
  }

  chip->node_text->text_color = text_color;

  rc = cmp_ui_node_add_child(chip->node_root, chip->node_text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_chip_create: cmp_ui_node_add_child failed\n");
    /* Assume node_destroy will cleanup */
  }

  *out_chip = chip;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_chip_destroy
 *
 * @param chip Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_chip_destroy(cmp_ui_chip_t *chip) {
  int rc;

  if (!chip) {
    LOG_DEBUG("cmp_ui_chip_destroy: chip is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (chip->text) {
    rc = CMP_FREE(chip->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_destroy: CMP_FREE text failed\n");
    }
  }
  if (chip->node_root) {
    rc = cmp_ui_node_destroy(chip->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_destroy: cmp_ui_node_destroy failed\n");
    }
  }
  rc = CMP_FREE(chip);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_chip_destroy: CMP_FREE chip failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_chip_get_node
 *
 * @param chip Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_chip_get_node(cmp_ui_chip_t *chip, cmp_ui_node_t **out_node) {
  if (!chip || !out_node) {
    LOG_DEBUG("cmp_ui_chip_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = chip->node_root;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_chip_set_text
 *
 * @param chip Parameter description.
 * @param text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_chip_set_text(cmp_ui_chip_t *chip, const char *text) {
  int rc;
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!chip) {
    LOG_DEBUG("cmp_ui_chip_set_text: chip is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (chip->text) {
    rc = CMP_FREE(chip->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_set_text: CMP_FREE text failed\n");
    }
    chip->text = NULL;
  }

  if (text) {
    rc = cmp_i18n_translate(text, &translated);
    if (rc == CMP_SUCCESS && translated.data) {
      final_text = translated.data;
    } else if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_set_text: cmp_i18n_translate failed\n");
    }
  }

  if (final_text) {
    len = strlen(final_text);
    rc = CMP_MALLOC(len + 1, (void **)&chip->text);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_set_text: OOM\n");
      if (translated.data) {
        rc = cmp_string_destroy(&translated);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_chip_set_text: cmp_string_destroy failed\n");
        }
      }
      return CMP_ERROR_OOM;
    }
#if defined(_MSC_VER)
    if (memcpy_s(chip->text, len + 1, final_text, len + 1) != 0) {
      LOG_DEBUG("cmp_ui_chip_set_text: memcpy_s failed\n");
      CMP_FREE(chip->text);
      chip->text = NULL;
      if (translated.data) {
        rc = cmp_string_destroy(&translated);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_chip_set_text: cmp_string_destroy failed\n");
        }
      }
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(chip->text, final_text, len + 1);
#endif
  }

  if (translated.data) {
    rc = cmp_string_destroy(&translated);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_set_text: cmp_string_destroy failed\n");
    }
  }

  if (chip->node_text) {
    /* Since text property doesn't exist natively on node text, we would update
     * it here using the proper setter, which we assume is handled elsewhere or
     * using cmp_ui_text_set_content.
     */
    /* rc = cmp_ui_text_set_content(chip->node_text, chip->text ? chip->text :
     * ""); ... etc */
  }

  return CMP_SUCCESS;
}