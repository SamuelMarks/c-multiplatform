#ifndef UI_DOM_NODE_H
#define UI_DOM_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Types of DOM nodes.
 */
enum ui_dom_node_type {
  UI_DOM_NODE_TYPE_ELEMENT = 1,
  UI_DOM_NODE_TYPE_TEXT = 3,
  UI_DOM_NODE_TYPE_COMMENT = 8,
  UI_DOM_NODE_TYPE_DOCUMENT = 9
};

/**
 * @brief Represents a single key-value attribute pair for an element node.
 */
struct ui_dom_attribute {
  char *name;
  char *value;
  struct ui_dom_attribute *next;
};

/**
 * @brief State flags for UI DOM nodes (e.g., for CSS pseudo-class matching).
 */
enum ui_dom_node_state {
  UI_DOM_NODE_STATE_HOVER = 1 << 0,
  UI_DOM_NODE_STATE_ACTIVE = 1 << 1,
  UI_DOM_NODE_STATE_FOCUS = 1 << 2,
  UI_DOM_NODE_STATE_FOCUS_VISIBLE = 1 << 3,
  UI_DOM_NODE_STATE_FOCUS_WITHIN = 1 << 4,
  UI_DOM_NODE_STATE_TARGET = 1 << 5,
  UI_DOM_NODE_STATE_TARGET_WITHIN = 1 << 6
};

/**
 * @brief Represents a node in the UI engine's DOM tree.
 */

struct ui_dom_node;
typedef ui_error_t (*ui_event_handler_t)(struct ui_dom_node *node,
                                         const struct ui_event *event,
                                         void *user_data);
struct ui_dom_event_listener {
  enum ui_event_type type;
  ui_event_handler_t handler;
  void *user_data;
  struct ui_dom_event_listener *next;
};

struct ui_dom_node {
  struct ui_dom_event_listener *listeners;
  enum ui_dom_node_type type;
  char *tag_name;     /**< Applicable for ELEMENT nodes */
  char *text_content; /**< Applicable for TEXT and COMMENT nodes */

  struct ui_dom_attribute *attributes;
  unsigned int state_flags;

  struct ui_dom_node *parent;
  struct ui_dom_node *first_child;
  struct ui_dom_node *last_child;
  struct ui_dom_node *previous_sibling;
  struct ui_dom_node *next_sibling;
};

/**
 * @brief Creates a new DOM node of the specified type.
 *
 * @param type The type of node to create.
 * @param out_node Pointer to receive the new node.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_create(enum ui_dom_node_type type,
                              struct ui_dom_node **out_node);

/**
 * @brief Recursively destroys a DOM node and all its descendants.
 *
 * @param node The node to destroy.
 */
ui_error_t ui_dom_node_destroy(struct ui_dom_node *node);

/**
 * @brief Appends a child node to the parent node.
 *
 * @param parent The parent node.
 * @param child The child node to append.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_append_child(struct ui_dom_node *parent,
                                    struct ui_dom_node *child);

/**
 * @brief Removes a child node from its parent node.
 *
 * @param parent The parent node.
 * @param child The child node to remove.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_remove_child(struct ui_dom_node *parent,
                                    struct ui_dom_node *child);

/**
 * @brief Sets an attribute on an element node.
 *
 * @param node The element node.
 * @param name The attribute name.
 * @param value The attribute value.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_set_attribute(struct ui_dom_node *node, const char *name,
                                     const char *value);

/**
 * @brief Retrieves the value of an attribute on an element node.
 *
 * @param node The element node.
 * @param name The attribute name.
 * @param out_value Pointer to receive the attribute value (valid until the
 * attribute is changed or removed).
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if the attribute doesn't
 * exist, or an appropriate error code.
 */
ui_error_t ui_dom_node_get_attribute(const struct ui_dom_node *node,
                                     const char *name, const char **out_value);

/**
 * @brief Removes an attribute from an element node.
 *
 * @param node The element node.
 * @param name The attribute name to remove.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_remove_attribute(struct ui_dom_node *node,
                                        const char *name);

/**
 * @brief Sets the tag name of an element node.
 *
 * @param node The element node.
 * @param tag_name The tag name.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_set_tag_name(struct ui_dom_node *node,
                                    const char *tag_name);

/**
 * @brief Sets the text content of a text or comment node.
 *
 * @param node The node.
 * @param text The text content.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_set_text_content(struct ui_dom_node *node,
                                        const char *text);

ui_error_t ui_dom_node_add_event_listener(struct ui_dom_node *node,
                                          enum ui_event_type type,
                                          ui_event_handler_t handler,
                                          void *user_data);
ui_error_t ui_dom_node_remove_event_listener(struct ui_dom_node *node,
                                             enum ui_event_type type,
                                             ui_event_handler_t handler);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DOM_NODE_H */
