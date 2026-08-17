/**
 * @file ui_text_node_base.h
 * @brief Defines the typography and text node component base logic.
 */
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

/**
 * @enum ui_text_node_overflow
 * @brief Describes how text overflow should be handled.
 */
enum ui_text_node_overflow {
  /** @brief Clip the text at the boundary. */
  UI_TEXT_NODE_OVERFLOW_CLIP = 0,
  /** @brief Show an ellipsis when text overflows. */
  UI_TEXT_NODE_OVERFLOW_ELLIPSIS
};

/**
 * @brief Creates a typography/text node component.
 *
 * @param out_node Pointer to receive the text node instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_create(struct ui_text_node_base **out_node);

/**
 * @brief Destroys a text node component.
 *
 * @param node The text node to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_destroy(struct ui_text_node_base *node);

/**
 * @brief Sets the text content for the text node.
 *
 * @param node The text node.
 * @param text The text string.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_set_text(struct ui_text_node_base *node,
                                      const char *text);

/**
 * @brief Gets the text content of the text node.
 *
 * @param node The text node.
 * @param out_text Pointer to receive the text string.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_get_text(const struct ui_text_node_base *node,
                                      const char **out_text);

/**
 * @brief Sets the font manager for the text node.
 *
 * @param node The text node.
 * @param font_manager The font manager.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_text_node_base_set_font_manager(struct ui_text_node_base *node,
                                   struct ui_font_manager *font_manager);

/**
 * @brief Sets the font family for the text node.
 *
 * @param node The text node.
 * @param family The font family string.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_set_font_family(struct ui_text_node_base *node,
                                             const char *family);

/**
 * @brief Sets the font size for the text node.
 *
 * @param node The text node.
 * @param size The font size in pixels.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_set_font_size(struct ui_text_node_base *node,
                                           float size);

/**
 * @brief Sets the maximum width for the text node.
 *
 * @param node The text node.
 * @param max_width The maximum width.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_set_max_width(struct ui_text_node_base *node,
                                           float max_width);

/**
 * @brief Sets the maximum lines for the text node.
 *
 * @param node The text node.
 * @param max_lines The maximum number of lines.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_set_max_lines(struct ui_text_node_base *node,
                                           int max_lines);

/**
 * @brief Sets the overflow behavior for the text node.
 *
 * @param node The text node.
 * @param overflow The overflow behavior.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_set_overflow(struct ui_text_node_base *node,
                                          enum ui_text_node_overflow overflow);

/**
 * @brief Updates the text layout (shapes text, calculates bounding box, applies
 * line clamp).
 *
 * @param node The text node to update.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_update_layout(struct ui_text_node_base *node);

/**
 * @brief Gets the computed layout object.
 *
 * @param node The text node.
 * @param out_layout Pointer to receive the text layout.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_get_layout(struct ui_text_node_base *node,
                                        struct ui_text_layout **out_layout);

/**
 * @brief Gets the component instance.
 *
 * @param node The text node.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_get_component(struct ui_text_node_base *node,
                                           struct ui_component **out_component);

/**
 * @brief Binds the text property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_text_node_base_bind_text(struct ui_text_node_base *widget,
                                       struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TEXT_NODE_BASE_H */
