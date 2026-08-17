#ifndef UI_SECTION_INDEX_BASE_H
#define UI_SECTION_INDEX_BASE_H

/**
 * \file ui_section_index_base.h
 * \brief UI Section Index Base component.
 *
 * This file contains definitions for a vertical alphabet/section scroller,
 * typically displayed on the right edge of long contact lists on mobile OSes.
 */

/** \brief Forward declaration of ui_computed */
struct ui_computed;

/** \brief Forward declaration of ui_signal */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

/**
 * @brief Opaque handle representing a section index component.
 */
struct ui_section_index_base;

/**
 * @brief Creates a new unstyled section index base component.
 *
 * @param out_index Pointer to receive the allocated section index base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_section_index_base_create(struct ui_section_index_base **out_index);

/**
 * @brief Destroys a section index component.
 *
 * @param index The section index to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_section_index_base_destroy(struct ui_section_index_base *index);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param index The section index.
 * @param out_component Pointer to receive the underlying component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_section_index_base_get_component(struct ui_section_index_base *index,
                                    struct ui_component **out_component);

/**
 * @brief Sets the sections (labels) for the index.
 *
 * This will create child nodes for each label.
 *
 * @param index The section index.
 * @param sections Array of string labels (e.g., "A", "B").
 * @param count Number of sections in the array.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_section_index_base_set_sections(struct ui_section_index_base *index,
                                   const char **sections, size_t count);

/**
 * @brief Highlights the active section index visually.
 *
 * @param index The section index.
 * @param active_idx The index in the array to set active, or -1 to clear.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_section_index_base_set_active_section(struct ui_section_index_base *index,
                                         int active_idx);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget component.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_section_index_base_bind_data(struct ui_section_index_base *widget,
                                           struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SECTION_INDEX_BASE_H */
