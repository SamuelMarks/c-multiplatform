/* clang-format off */
#include "cmp_ui_chip.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_chip {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *text;
};

int cmp_ui_chip_create(cmp_ui_chip_t **out_chip, const char *text,
                       uint32_t bg_color, uint32_t text_color) {
  cmp_ui_chip_t *chip;
  int err;
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!out_chip) {
    return CMP_ERROR_INVALID_ARG;
  }

  chip = (cmp_ui_chip_t *)malloc(sizeof(cmp_ui_chip_t));
  if (!chip) {
    return CMP_ERROR_OOM;
  }

  if (text && cmp_i18n_translate(text, &translated) == 0 && translated.data) {
    final_text = translated.data;
  }

  chip->text = NULL;
  if (final_text) {
    len = strlen(final_text);
    chip->text = (char *)malloc(len + 1);
    if (chip->text) {
      memcpy(chip->text, final_text, len + 1);
    }
  }

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  err = cmp_ui_box_create(&chip->node_root);
  if (err != 0) {
    free(chip->text);
    free(chip);
    return err;
  }

  chip->node_root->bg_color = bg_color;

  err = cmp_ui_text_create(&chip->node_text, chip->text ? chip->text : "", -1);
  if (err != 0) {
    free(chip->text);
    free(chip);
    return err;
  }

  chip->node_text->text_color = text_color;

  cmp_ui_node_add_child(chip->node_root, chip->node_text);

  *out_chip = chip;
  return 0;
}

int cmp_ui_chip_destroy(cmp_ui_chip_t *chip) {
  if (!chip) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(chip->text);
  free(chip);
  return 0;
}

int cmp_ui_chip_get_node(cmp_ui_chip_t *chip, cmp_ui_node_t **out_node) {
  if (!chip || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = chip->node_root;
  return 0;
}

int cmp_ui_chip_set_text(cmp_ui_chip_t *chip, const char *text) {
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_text = text;

  if (!chip) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (chip->text) {
    free(chip->text);
    chip->text = NULL;
  }

  if (text && cmp_i18n_translate(text, &translated) == 0 && translated.data) {
    final_text = translated.data;
  }

  if (final_text) {
    len = strlen(final_text);
    chip->text = (char *)malloc(len + 1);
    if (!chip->text) {
      if (translated.data) {
        cmp_string_destroy(&translated);
      }
      return CMP_ERROR_OOM;
    }
    memcpy(chip->text, final_text, len + 1);
  }

  if (translated.data) {
    cmp_string_destroy(&translated);
  }

  return 0;
}
