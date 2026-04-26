/* clang-format off */
#include "cmp_ui_chip.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Chip widget.
 */
struct cmp_ui_chip {
  /** @brief The root node of the chip */
  cmp_ui_node_t *node_root;
  /** @brief The text node containing the chip label */
  cmp_ui_node_t *node_text;
  /** @brief The raw string of the label */
  char *text;
};

/**
 * @brief Allocates and initializes a UI Chip widget.
 *
 * @param out_chip Pointer to store the created chip.
 * @param text The text to display.
 * @param bg_color Background color.
 * @param text_color Text color.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_ui_chip_create(cmp_ui_chip_t **out_chip, const char *text,
                       uint32_t bg_color, uint32_t text_color) {
  cmp_ui_chip_t *chip = NULL;
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
    return CMP_ERROR_OOM;
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
        int destroy_rc = cmp_string_destroy(&translated);
        if (destroy_rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_chip_create: cmp_string_destroy failed\n");
        }
      }
      rc = CMP_FREE(chip);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_chip_create: CMP_FREE failed\n");
      }
      return CMP_ERROR_OOM;
    }
    memcpy(chip->text, final_text, len + 1);
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
      int free_rc = CMP_FREE(chip->text);
      if (free_rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_chip_create: CMP_FREE text failed\n");
    }
    rc = CMP_FREE(chip);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_chip_create: CMP_FREE chip failed\n");
    return CMP_ERROR_GENERAL;
  }

  chip->node_root->bg_color = bg_color;

  rc = cmp_ui_text_create(&chip->node_text, chip->text ? chip->text : "", -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_chip_create: cmp_ui_text_create failed\n");
    if (chip->text) {
      int free_rc = CMP_FREE(chip->text);
      if (free_rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_chip_create: CMP_FREE text failed\n");
    }
    rc = cmp_ui_node_destroy(chip->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_create: cmp_ui_node_destroy failed\n");
    }
    rc = CMP_FREE(chip);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_chip_create: CMP_FREE chip failed\n");
    return CMP_ERROR_GENERAL;
  }

  chip->node_text->text_color = text_color;

  rc = cmp_ui_node_add_child(chip->node_root, chip->node_text);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_chip_create: cmp_ui_node_add_child failed\n");
  }

  *out_chip = chip;
  return CMP_SUCCESS;
}

/**
 * @brief Destroys a UI Chip widget and frees its resources.
 *
 * @param chip The chip component.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
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
 * @brief Retrieves the root UI node for the given chip.
 *
 * @param chip The chip component.
 * @param out_node Pointer to store the underlying UI node.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
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
 * @brief Updates the text displayed by the UI chip.
 *
 * @param chip The chip component.
 * @param text The new text to display.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
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
        int destroy_rc = cmp_string_destroy(&translated);
        if (destroy_rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_chip_set_text: cmp_string_destroy failed\n");
        }
      }
      return CMP_ERROR_OOM;
    }
    memcpy(chip->text, final_text, len + 1);

    if (chip->node_text && chip->node_text->properties) {
      rc = CMP_FREE(chip->node_text->properties);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_chip_set_text: CMP_FREE properties failed\n");
    }
    if (chip->node_text) {
      rc = CMP_MALLOC(len + 1, (void **)&chip->node_text->properties);
      if (rc == CMP_SUCCESS) {
        memcpy(chip->node_text->properties, chip->text, len + 1);
      } else {
        LOG_DEBUG("cmp_ui_chip_set_text: OOM properties\n");
      }
    }
  } else {
    if (chip->node_text && chip->node_text->properties) {
      rc = CMP_FREE(chip->node_text->properties);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_chip_set_text: CMP_FREE properties failed\n");
    }
    if (chip->node_text) {
      rc = CMP_MALLOC(1, (void **)&chip->node_text->properties);
      if (rc == CMP_SUCCESS) {
        ((char *)chip->node_text->properties)[0] = '\0';
      } else {
        LOG_DEBUG("cmp_ui_chip_set_text: OOM properties\n");
      }
    }
  }

  if (translated.data) {
    rc = cmp_string_destroy(&translated);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_chip_set_text: cmp_string_destroy failed\n");
    }
  }

  return CMP_SUCCESS;
}