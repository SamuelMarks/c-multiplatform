#ifndef UI_WEB_BRIDGE_H
#define UI_WEB_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

/**
 * \file ui_web_bridge.h
 * \brief Command buffer format for the Wasm-JS bridge.
 */

/** \brief Opcodes for the Wasm to JS Command Buffer */
enum ui_web_cmd {
  CMD_CREATE_NODE = 1,
  CMD_DESTROY_NODE,
  CMD_SET_TEXT,
  CMD_APPEND_CHILD,
  CMD_INSERT_BEFORE,
  CMD_REMOVE_CHILD,
  CMD_SET_BOUNDS,
  CMD_SET_STYLE,
  CMD_SET_ARIA,
  CMD_PUSH_STATE,
  CMD_REPLACE_STATE,
  CMD_SET_ATTRIBUTE,
  CMD_SET_PROPERTY
};

/**
 * \brief Flushes the command buffer to JavaScript.
 * \return UI_ERROR_NONE on success, or a relevant error code.
 */
ui_error_t ui_web_bridge_flush(void);

/**
 * \brief Shuts down the web bridge, freeing any allocated resources.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_shutdown(void);

/**
 * \brief Enqueues a CMD_CREATE_NODE command.
 * \param id The ID of the node.
 * \param tag_name The HTML tag name (e.g., "button", "div").
 * \return UI_ERROR_NONE on success, or a relevant error code.
 */
ui_error_t ui_web_bridge_create_node(ui_uint32 id, const char *tag_name);

/**
 * \brief Enqueues a CMD_DESTROY_NODE command.
 * \param id The ID of the node.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_destroy_node(ui_uint32 id);

/**
 * \brief Enqueues a CMD_SET_TEXT command.
 * \param id The ID of the node.
 * \param text The text content.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_set_text(ui_uint32 id, const char *text);

/**
 * \brief Enqueues a CMD_APPEND_CHILD command.
 * \param parent_id The ID of the parent node.
 * \param child_id The ID of the child node.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_append_child(ui_uint32 parent_id, ui_uint32 child_id);

/**
 * \brief Enqueues a CMD_REMOVE_CHILD command.
 * \param parent_id The ID of the parent node.
 * \param child_id The ID of the child node.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_remove_child(ui_uint32 parent_id, ui_uint32 child_id);

/**
 * \brief Enqueues a CMD_SET_BOUNDS command.
 * \param id The ID of the node.
 * \param x The X coordinate.
 * \param y The Y coordinate.
 * \param w The width.
 * \param h The height.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_set_bounds(ui_uint32 id, float x, float y, float w,
                                    float h);

/**
 * \brief Enqueues a CMD_INSERT_BEFORE command.
 * \param parent_id The ID of the parent node.
 * \param child_id The ID of the child node to insert.
 * \param reference_id The ID of the reference node before which child is
 * inserted.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_insert_before(ui_uint32 parent_id, ui_uint32 child_id,
                                       ui_uint32 reference_id);

/**
 * \brief Enqueues a CMD_SET_STYLE command.
 * \param id The ID of the node.
 * \param property The CSS property name (e.g. "src", "type").
 * \param value The string value to assign.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_set_style(ui_uint32 id, const char *property,
                                   const char *value);

/**
 * \brief Dispatches a pointer or wheel event from JS to C.
 * \param type 1=down, 2=up, 3=move, 4=cancel, 5=wheel, 6=contextmenu
 * \param x The X coordinate (or deltaX for wheel).
 * \param y The Y coordinate (or deltaY for wheel).
 * \param buttons The pressed buttons bitmask.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_dispatch_event(int type, float x, float y,
                                        int buttons);

/**
 * \brief Dispatches a resize event from JS to C.
 * \param w The new width.
 * \param h The new height.
 * \param dpr The device pixel ratio.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_dispatch_resize(float w, float h, float dpr);

/**
 * \brief Dispatches a keyboard event from JS to C.
 * \param type 10=keydown, 11=keyup
 * \param key The key string (e.g. "Escape", "a").
 * \param modifiers Bitmask of modifiers (1=ctrl, 2=shift, 4=alt, 8=meta).
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_dispatch_key(int type, const char *key, int modifiers);

/**
 * \brief Enqueues a CMD_SET_ARIA command.
 * \param id The ID of the node.
 * \param role The ARIA role string.
 * \param label The aria-label string.
 * \param hidden Integer flag (1 = true, 0 = false).
 * \param disabled Integer flag (1 = true, 0 = false).
 * \param expanded Integer flag (-1 = unset, 0 = false, 1 = true).
 * \param checked Integer flag (-1 = unset, 0 = false, 1 = true, 2 = mixed).
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_set_aria(ui_uint32 id, const char *role,
                                  const char *label, int hidden, int disabled,
                                  int expanded, int checked);

/**
 * \brief Enqueues a CMD_PUSH_STATE command.
 * \param path The URL path string.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_push_state(const char *path);

/**
 * \brief Enqueues a CMD_REPLACE_STATE command.
 * \param path The URL path string.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_replace_state(const char *path);

/**
 * \brief Enqueues a CMD_SET_ATTRIBUTE command.
 * \param id The ID of the node.
 * \param name The attribute name.
 * \param value The attribute value.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_set_attribute(ui_uint32 id, const char *name,
                                       const char *value);

/**
 * \brief Enqueues a CMD_SET_PROPERTY command.
 * \param id The ID of the node.
 * \param name The property name.
 * \param value The property value.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_web_bridge_set_property(ui_uint32 id, const char *name,
                                      const char *value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WEB_BRIDGE_H */
