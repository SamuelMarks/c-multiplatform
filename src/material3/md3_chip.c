/**
 * @file md3_chip.c
 * @brief Material 3 Chip components implementation wrapping ui_chips_base.
 */

/* clang-format off */
#include "material3/md3_chip.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t md3_chip_create(struct ui_engine *engine, enum md3_chip_type type,
                           struct md3_chip **out_chip,
                           struct ui_control_value_accessor *out_cva) {
  struct md3_chip *chip;
  ui_error_t rc;

  if (!engine || !out_chip || (unsigned)type >= MD3_CHIP_TYPE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  chip = (struct md3_chip *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_chip));
  if (!chip) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(chip, 0, sizeof(struct md3_chip));
  chip->type = type;

  rc = ui_chips_base_create(&chip->base, out_cva);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(chip);
    return rc;
  }

  *out_chip = chip;
  return UI_ERROR_NONE;
}

ui_error_t md3_chip_destroy(struct md3_chip *chip) {
  ui_error_t rc;

  if (!chip) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_chips_base_destroy(chip->base);
  C_MULTIPLATFORM_FREE(chip);
  return rc;
}

ui_error_t md3_chip_add(struct md3_chip *chip, const char *label) {
  if (!chip || !label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_chips_base_add(chip->base, label);
}

ui_error_t md3_chip_remove(struct md3_chip *chip, size_t index) {
  if (!chip) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_chips_base_remove(chip->base, index);
}

ui_error_t md3_chip_get_count(const struct md3_chip *chip, size_t *out_count) {
  if (!chip || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_chips_base_get_count(chip->base, out_count);
}

ui_error_t md3_chip_get_label(const struct md3_chip *chip, size_t index,
                              const char **out_label) {
  if (!chip || !out_label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_chips_base_get_token(chip->base, index, out_label);
}

ui_error_t md3_chip_set_elevated(struct md3_chip *chip, int elevated) {
  if (!chip) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  chip->elevated = elevated ? 1 : 0;
  return UI_ERROR_NONE;
}
