#ifndef UI_SECTION_INDEX_BASE_H
#define UI_SECTION_INDEX_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_section_index_base;

/**
 * @brief Creates a new unstyled section index base component.
 *
 * This represents a vertical alphabet/section scroller, typical on
 * right-edges of long contact lists on mobile OSes (like Cupertino).
 *
 * @param out_index Pointer to receive the allocated section index base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_section_index_base_create(struct ui_section_index_base **out_index);

/**
 * @brief Destroys a section index component.
 *
 * @param index The section index to destroy.
 */
void ui_section_index_base_destroy(struct ui_section_index_base *index);

/**
 * @brief Gets the underlying component.
 *
 * @param index The section index.
 * @return The underlying component.
 */
enum ui_error
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
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_section_index_base_set_sections(struct ui_section_index_base *index,
                                   const char **sections, size_t count);

/**
 * @brief Highlights the active section index visually.
 *
 * @param index The section index.
 * @param active_idx The index in the array to set active, or -1 to clear.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_section_index_base_set_active_section(struct ui_section_index_base *index,
                                         int active_idx);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_section_index_base_bind_data(struct ui_section_index_base *widget,
                                struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SECTION_INDEX_BASE_H */
