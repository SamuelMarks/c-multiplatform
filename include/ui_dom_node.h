/**
 * @file ui_dom_node.h
 * @brief DOM node structures, state management, and tree manipulation.
 *
 * This header defines the data structures for managing the UI DOM tree,
 * including DOM nodes, attributes, event listeners, and node state.
 */

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
  UI_DOM_NODE_TYPE_ELEMENT = 1, /**< Element node. */
  UI_DOM_NODE_TYPE_TEXT = 3,    /**< Text node. */
  UI_DOM_NODE_TYPE_COMMENT = 8, /**< Comment node. */
  UI_DOM_NODE_TYPE_DOCUMENT = 9 /**< Document node. */
};

/**
 * @brief Represents a single key-value attribute pair for an element node.
 */
struct ui_dom_attribute {
  char *name;                    /**< The attribute name. */
  char *value;                   /**< The attribute value. */
  struct ui_dom_attribute *next; /**< Pointer to the next attribute. */
};

/**
 * @brief State flags for UI DOM nodes (e.g., for CSS pseudo-class matching).
 */
enum ui_dom_node_state {
  UI_DOM_NODE_STATE_HOVER = 1 << 0,         /**< Hover pseudo-class. */
  UI_DOM_NODE_STATE_ACTIVE = 1 << 1,        /**< Active pseudo-class. */
  UI_DOM_NODE_STATE_FOCUS = 1 << 2,         /**< Focus pseudo-class. */
  UI_DOM_NODE_STATE_FOCUS_VISIBLE = 1 << 3, /**< Focus-visible pseudo-class. */
  UI_DOM_NODE_STATE_FOCUS_WITHIN = 1 << 4,  /**< Focus-within pseudo-class. */
  UI_DOM_NODE_STATE_TARGET = 1 << 5,        /**< Target pseudo-class. */
  UI_DOM_NODE_STATE_TARGET_WITHIN = 1 << 6  /**< Target-within pseudo-class. */
};

struct ui_dom_node;

/**
 * @brief Type definition for a DOM event handler callback.
 *
 * @param node Pointer to the DOM node on which the event occurred.
 * @param event Pointer to the event data.
 * @param user_data Opaque user data provided during registration.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_event_handler_t)(struct ui_dom_node *node,
                                         const struct ui_event *event,
                                         void *user_data);

/**
 * @brief Represents an event listener attached to a DOM node.
 */
struct ui_dom_event_listener {
  enum ui_event_type type;            /**< The type of event to listen for. */
  ui_event_handler_t handler;         /**< The callback function to invoke. */
  void *user_data;                    /**< Opaque user data. */
  struct ui_dom_event_listener *next; /**< Pointer to the next listener. */
};

/**
 * @brief Represents a node in the UI engine's DOM tree.
 */
struct ui_dom_node {
  struct ui_dom_event_listener
      *listeners;             /**< Linked list of event listeners. */
  enum ui_dom_node_type type; /**< The node type. */
  char *tag_name;             /**< Applicable for ELEMENT nodes. */
  char *text_content;         /**< Applicable for TEXT and COMMENT nodes. */

  struct ui_dom_attribute *attributes; /**< Linked list of node attributes. */
  unsigned int state_flags; /**< Bitmask of `ui_dom_node_state` flags. */

  struct ui_dom_node *parent;      /**< Pointer to the parent node. */
  struct ui_dom_node *first_child; /**< Pointer to the first child node. */
  struct ui_dom_node *last_child;  /**< Pointer to the last child node. */
  struct ui_dom_node
      *previous_sibling; /**< Pointer to the previous sibling node. */
  struct ui_dom_node *next_sibling; /**< Pointer to the next sibling node. */
};

/**
 * @brief Creates a new DOM node of the specified type.
 *
 * @param type The type of node to create.
 * @param out_node Pointer to receive the allocated node structure.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_create(enum ui_dom_node_type type,
                              struct ui_dom_node **out_node);

/**
 * @brief Recursively destroys a DOM node and all its descendants.
 *
 * @param node Pointer to the node to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_destroy(struct ui_dom_node *node);

/**
 * @brief Appends a child node to the parent node.
 *
 * @param parent Pointer to the parent node.
 * @param child Pointer to the child node to append.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_append_child(struct ui_dom_node *parent,
                                    struct ui_dom_node *child);

/**
 * @brief Removes a child node from its parent node.
 *
 * @param parent Pointer to the parent node.
 * @param child Pointer to the child node to remove.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_remove_child(struct ui_dom_node *parent,
                                    struct ui_dom_node *child);

/**
 * @brief Sets an attribute on an element node.
 *
 * @param node Pointer to the element node.
 * @param name The attribute name.
 * @param value The attribute value.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_set_attribute(struct ui_dom_node *node, const char *name,
                                     const char *value);

/**
 * @brief Retrieves the value of an attribute on an element node.
 *
 * @param node Pointer to the element node.
 * @param name The attribute name.
 * @param out_value Pointer to receive the attribute value (valid until the
 * attribute is changed or removed).
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_NOT_FOUND` if the attribute
 * doesn't exist.
 */
ui_error_t ui_dom_node_get_attribute(const struct ui_dom_node *node,
                                     const char *name, const char **out_value);

/**
 * @brief Removes an attribute from an element node.
 *
 * @param node Pointer to the element node.
 * @param name The attribute name to remove.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_remove_attribute(struct ui_dom_node *node,
                                        const char *name);

/**
 * @brief Sets the tag name of an element node.
 *
 * @param node Pointer to the element node.
 * @param tag_name The new tag name string.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_set_tag_name(struct ui_dom_node *node,
                                    const char *tag_name);

/**
 * @brief Sets the text content of a text or comment node.
 *
 * @param node Pointer to the node.
 * @param text The new text content.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_set_text_content(struct ui_dom_node *node,
                                        const char *text);

/**
 * @brief Adds an event listener to the specified node.
 *
 * @param node Pointer to the node.
 * @param type The type of event to listen to.
 * @param handler The callback function.
 * @param user_data Opaque user data provided to the callback.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_add_event_listener(struct ui_dom_node *node,
                                          enum ui_event_type type,
                                          ui_event_handler_t handler,
                                          void *user_data);

/**
 * @brief Removes an event listener from the specified node.
 *
 * @param node Pointer to the node.
 * @param type The type of event.
 * @param handler The specific callback function to remove.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_dom_node_remove_event_listener(struct ui_dom_node *node,
                                             enum ui_event_type type,
                                             ui_event_handler_t handler);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DOM_NODE_H */
