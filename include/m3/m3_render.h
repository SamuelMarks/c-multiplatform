#ifndef M3_RENDER_H
#define M3_RENDER_H

/**
 * @file m3_render.h
 * @brief Render command list and widget tree traversal for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_ui.h"

/** @brief Render command: begin frame. */
#define M3_RENDER_CMD_BEGIN_FRAME 1
/** @brief Render command: end frame. */
#define M3_RENDER_CMD_END_FRAME 2
/** @brief Render command: clear render target. */
#define M3_RENDER_CMD_CLEAR 3
/** @brief Render command: draw rectangle. */
#define M3_RENDER_CMD_DRAW_RECT 4
/** @brief Render command: draw line segment. */
#define M3_RENDER_CMD_DRAW_LINE 5
/** @brief Render command: push clip rectangle. */
#define M3_RENDER_CMD_PUSH_CLIP 6
/** @brief Render command: pop clip rectangle. */
#define M3_RENDER_CMD_POP_CLIP 7
/** @brief Render command: set transform. */
#define M3_RENDER_CMD_SET_TRANSFORM 8
/** @brief Render command: draw textured quad. */
#define M3_RENDER_CMD_DRAW_TEXTURE 9
/** @brief Render command: draw text. */
#define M3_RENDER_CMD_DRAW_TEXT 10

/**
 * @brief Begin frame render command payload.
 */
typedef struct M3RenderCmdBeginFrame {
    M3Handle window; /**< Target window handle. */
    m3_i32 width; /**< Backbuffer width in pixels. */
    m3_i32 height; /**< Backbuffer height in pixels. */
    M3Scalar dpi_scale; /**< DPI scale factor. */
} M3RenderCmdBeginFrame;

/**
 * @brief End frame render command payload.
 */
typedef struct M3RenderCmdEndFrame {
    M3Handle window; /**< Target window handle. */
} M3RenderCmdEndFrame;

/**
 * @brief Clear render command payload.
 */
typedef struct M3RenderCmdClear {
    M3Color color; /**< Clear color. */
} M3RenderCmdClear;

/**
 * @brief Draw rectangle render command payload.
 */
typedef struct M3RenderCmdDrawRect {
    M3Rect rect; /**< Rectangle to draw. */
    M3Color color; /**< Fill color. */
    M3Scalar corner_radius; /**< Corner radius in pixels. */
} M3RenderCmdDrawRect;

/**
 * @brief Draw line render command payload.
 */
typedef struct M3RenderCmdDrawLine {
    M3Scalar x0; /**< Start X coordinate. */
    M3Scalar y0; /**< Start Y coordinate. */
    M3Scalar x1; /**< End X coordinate. */
    M3Scalar y1; /**< End Y coordinate. */
    M3Color color; /**< Line color. */
    M3Scalar thickness; /**< Line thickness in pixels. */
} M3RenderCmdDrawLine;

/**
 * @brief Push clip render command payload.
 */
typedef struct M3RenderCmdPushClip {
    M3Rect rect; /**< Clip rectangle. */
} M3RenderCmdPushClip;

/**
 * @brief Set transform render command payload.
 */
typedef struct M3RenderCmdSetTransform {
    M3Mat3 transform; /**< Transform matrix. */
} M3RenderCmdSetTransform;

/**
 * @brief Draw texture render command payload.
 */
typedef struct M3RenderCmdDrawTexture {
    M3Handle texture; /**< Texture handle. */
    M3Rect src; /**< Source rectangle in texture space. */
    M3Rect dst; /**< Destination rectangle in window space. */
    M3Scalar opacity; /**< Opacity multiplier (0..1). */
} M3RenderCmdDrawTexture;

/**
 * @brief Draw text render command payload.
 */
typedef struct M3RenderCmdDrawText {
    M3Handle font; /**< Font handle. */
    const char *utf8; /**< UTF-8 text pointer (must remain valid for the list lifetime). */
    m3_usize utf8_len; /**< UTF-8 length in bytes. */
    M3Scalar x; /**< X origin in pixels. */
    M3Scalar y; /**< Y origin in pixels. */
    M3Color color; /**< Text color. */
} M3RenderCmdDrawText;

/**
 * @brief Render command payload union.
 */
typedef union M3RenderCmdData {
    M3RenderCmdBeginFrame begin_frame; /**< Begin frame payload. */
    M3RenderCmdEndFrame end_frame; /**< End frame payload. */
    M3RenderCmdClear clear; /**< Clear payload. */
    M3RenderCmdDrawRect draw_rect; /**< Draw rect payload. */
    M3RenderCmdDrawLine draw_line; /**< Draw line payload. */
    M3RenderCmdPushClip push_clip; /**< Push clip payload. */
    M3RenderCmdSetTransform set_transform; /**< Set transform payload. */
    M3RenderCmdDrawTexture draw_texture; /**< Draw texture payload. */
    M3RenderCmdDrawText draw_text; /**< Draw text payload. */
} M3RenderCmdData;

/**
 * @brief Render command entry.
 */
typedef struct M3RenderCmd {
    m3_u32 type; /**< Command type (M3_RENDER_CMD_*). */
    M3RenderCmdData data; /**< Command payload. */
} M3RenderCmd;

/**
 * @brief Render command list.
 */
typedef struct M3RenderList {
    M3RenderCmd *commands; /**< Command array (owned by the list). */
    m3_usize count; /**< Number of commands in the list. */
    m3_usize capacity; /**< Command array capacity. */
    M3Allocator allocator; /**< Allocator used for command storage. */
} M3RenderList;

/**
 * @brief Render tree node for widget traversal.
 */
typedef struct M3RenderNode {
    M3Widget *widget; /**< Widget instance. */
    struct M3RenderNode **children; /**< Child node array. */
    m3_usize child_count; /**< Number of children. */
    M3Rect bounds; /**< Layout bounds in pixels. */
} M3RenderNode;

/**
 * @brief Initialize a render list.
 * @param list Render list to initialize.
 * @param allocator Allocator to use; NULL uses the default allocator.
 * @param initial_capacity Initial command capacity (0 selects a default).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_list_init(M3RenderList *list, const M3Allocator *allocator, m3_usize initial_capacity);

/**
 * @brief Reset a render list without freeing its storage.
 * @param list Render list to reset.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_list_reset(M3RenderList *list);

/**
 * @brief Shut down a render list and release its storage.
 * @param list Render list to shut down.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_list_shutdown(M3RenderList *list);

/**
 * @brief Append a render command to the list.
 * @param list Render list instance.
 * @param cmd Command to append.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_list_append(M3RenderList *list, const M3RenderCmd *cmd);

/**
 * @brief Execute a render list on a graphics backend.
 * @param list Render list to execute.
 * @param gfx Graphics backend interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_list_execute(const M3RenderList *list, M3Gfx *gfx);

/**
 * @brief Initialize a render tree node.
 * @param node Render node to initialize.
 * @param widget Widget instance for the node.
 * @param bounds Layout bounds for the widget.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_node_init(M3RenderNode *node, M3Widget *widget, const M3Rect *bounds);

/**
 * @brief Update the bounds for a render node.
 * @param node Render node to update.
 * @param bounds New bounds in pixels.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_node_set_bounds(M3RenderNode *node, const M3Rect *bounds);

/**
 * @brief Assign children to a render node.
 * @param node Render node to update.
 * @param children Child node array (may be NULL when count is 0).
 * @param count Number of children.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_node_set_children(M3RenderNode *node, M3RenderNode **children, m3_usize count);

/**
 * @brief Build a render command list from a widget tree.
 * @param root Root render node.
 * @param list Render list to append to.
 * @param dpi_scale DPI scale factor for paint contexts.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_build(const M3RenderNode *root, M3RenderList *list, M3Scalar dpi_scale);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for render add-overflow helper.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives sum.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_test_add_overflow(m3_usize a, m3_usize b, m3_usize *out_value);

/**
 * @brief Test wrapper for render multiply-overflow helper.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives product.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_test_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value);

/**
 * @brief Test wrapper for render list reserve helper.
 * @param list Render list to grow.
 * @param additional Additional slots needed.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_test_list_reserve(M3RenderList *list, m3_usize additional);

/**
 * @brief Test wrapper for render node validation.
 * @param node Render node to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_test_validate_node(const M3RenderNode *node);

/**
 * @brief Test hook to force render list reserve path.
 * @param enable Whether to force the reserve path.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_test_set_force_reserve(M3Bool enable);

/**
 * @brief Test hook to force clip intersection failure.
 * @param enable Whether to force failure.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_render_test_set_force_intersect_fail(M3Bool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_RENDER_H */
