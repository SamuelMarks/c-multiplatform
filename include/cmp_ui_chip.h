#ifndef CMP_UI_CHIP_H
#define CMP_UI_CHIP_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * ile cmp_ui_chip.h
 * rief UI Component for rendering a text chip/tag.
 */

/**
 * rief Opaque handle to a chip UI component.
 */
typedef struct cmp_ui_chip cmp_ui_chip_t;

/**
 * rief Creates a new chip component.
 *
 * \param out_chip Pointer to store the created chip handle.
 * \param text The text to display in the chip.
 * \param bg_color Background color of the chip (ARGB).
 * \param text_color Text color of the chip (ARGB).
 *
eturn 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_chip_create(cmp_ui_chip_t **out_chip, const char *text,
                               uint32_t bg_color, uint32_t text_color);

/**
 * rief Destroys a chip component.
 *
 * \param chip The chip component to destroy.
 *
eturn 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_chip_destroy(cmp_ui_chip_t *chip);

/**
 * rief Retrieves the underlying UI node for the chip.
 *
 * \param chip The chip component.
 * \param out_node Pointer to store the UI node.
 *
eturn 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_chip_get_node(cmp_ui_chip_t *chip, cmp_ui_node_t **out_node);

/**
 * rief Updates the text displayed in the chip.
 *
 * \param chip The chip component.
 * \param text The new text to display.
 *
eturn 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_chip_set_text(cmp_ui_chip_t *chip, const char *text);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_CHIP_H */
