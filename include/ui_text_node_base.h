#ifndef UI_TEXT_NODE_BASE_H
#define UI_TEXT_NODE_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_text_layout.h"
#include "ui_font_manager.h"
/* clang-format on */

struct ui_text_node_base;

/** \brief ui_text_node_overflow */
enum ui_text_node_overflow {
  UI_TEXT_NODE_OVERFLOW_CLIP = 0,
  UI_TEXT_NODE_OVERFLOW_ELLIPSIS
};

/**
 * @brief Creates a typography/text node component.
 */
enum ui_error ui_text_node_base_create(struct ui_text_node_base **out_node);

/**
 * @brief Destroys a text node component.
 */
void ui_text_node_base_destroy(struct ui_text_node_base *node);

enum ui_error ui_text_node_base_set_text(struct ui_text_node_base *node,
                                         const char *text);
enum ui_error ui_text_node_base_get_text(const struct ui_text_node_base *node,
                                         const char **out_text);

/** \brief ui_error */
enum ui_error
ui_text_node_base_set_font_manager(struct ui_text_node_base *node,
                                   struct ui_font_manager *font_manager);
enum ui_error ui_text_node_base_set_font_family(struct ui_text_node_base *node,
                                                const char *family);
enum ui_error ui_text_node_base_set_font_size(struct ui_text_node_base *node,
                                              float size);

enum ui_error ui_text_node_base_set_max_width(struct ui_text_node_base *node,
                                              float max_width);
enum ui_error ui_text_node_base_set_max_lines(struct ui_text_node_base *node,
                                              int max_lines);
/** \brief ui_error */
enum ui_error
ui_text_node_base_set_overflow(struct ui_text_node_base *node,
                               enum ui_text_node_overflow overflow);

/**
 * @brief Updates the text layout (shapes text, calculates bounding box, applies
 * line clamp).
 */
enum ui_error ui_text_node_base_update_layout(struct ui_text_node_base *node);

/**
 * @brief Gets the computed layout object.
 */
enum ui_error ui_text_node_base_get_layout(struct ui_text_node_base *node,
                                           struct ui_text_layout **out_layout);

/**
 * @brief Gets the component instance.
 */
enum ui_error
ui_text_node_base_get_component(struct ui_text_node_base *node,
                                struct ui_component **out_component);

/**
 * @brief Binds the text property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_text_node_base_bind_text(struct ui_text_node_base *widget,
                                          struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TEXT_NODE_BASE_H */
