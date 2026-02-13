#ifndef CMP_RENDER_H
#define CMP_RENDER_H

/**
 * @file cmp_render.h
 * @brief Render command list and widget tree traversal for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"

/** @brief Render command: begin frame. */
#define CMP_RENDER_CMD_BEGIN_FRAME 1
/** @brief Render command: end frame. */
#define CMP_RENDER_CMD_END_FRAME 2
/** @brief Render command: clear render target. */
#define CMP_RENDER_CMD_CLEAR 3
/** @brief Render command: draw rectangle. */
#define CMP_RENDER_CMD_DRAW_RECT 4
/** @brief Render command: draw line segment. */
#define CMP_RENDER_CMD_DRAW_LINE 5
/** @brief Render command: push clip rectangle. */
#define CMP_RENDER_CMD_PUSH_CLIP 6
/** @brief Render command: pop clip rectangle. */
#define CMP_RENDER_CMD_POP_CLIP 7
/** @brief Render command: set transform. */
#define CMP_RENDER_CMD_SET_TRANSFORM 8
/** @brief Render command: draw textured quad. */
#define CMP_RENDER_CMD_DRAW_TEXTURE 9
/** @brief Render command: draw text. */
#define CMP_RENDER_CMD_DRAW_TEXT 10
/** @brief Render command: draw path. */
#define CMP_RENDER_CMD_DRAW_PATH 11

/**
 * @brief Begin frame render command payload.
 */
typedef struct CMPRenderCmdBeginFrame {
  CMPHandle window;    /**< Target window handle. */
  cmp_i32 width;       /**< Backbuffer width in pixels. */
  cmp_i32 height;      /**< Backbuffer height in pixels. */
  CMPScalar dpi_scale; /**< DPI scale factor. */
} CMPRenderCmdBeginFrame;

/**
 * @brief End frame render command payload.
 */
typedef struct CMPRenderCmdEndFrame {
  CMPHandle window; /**< Target window handle. */
} CMPRenderCmdEndFrame;

/**
 * @brief Clear render command payload.
 */
typedef struct CMPRenderCmdClear {
  CMPColor color; /**< Clear color. */
} CMPRenderCmdClear;

/**
 * @brief Draw rectangle render command payload.
 */
typedef struct CMPRenderCmdDrawRect {
  CMPRect rect;            /**< Rectangle to draw. */
  CMPColor color;          /**< Fill color. */
  CMPScalar corner_radius; /**< Corner radius in pixels. */
} CMPRenderCmdDrawRect;

/**
 * @brief Draw line render command payload.
 */
typedef struct CMPRenderCmdDrawLine {
  CMPScalar x0;        /**< Start X coordinate. */
  CMPScalar y0;        /**< Start Y coordinate. */
  CMPScalar x1;        /**< End X coordinate. */
  CMPScalar y1;        /**< End Y coordinate. */
  CMPColor color;      /**< Line color. */
  CMPScalar thickness; /**< Line thickness in pixels. */
} CMPRenderCmdDrawLine;

/**
 * @brief Push clip render command payload.
 */
typedef struct CMPRenderCmdPushClip {
  CMPRect rect; /**< Clip rectangle. */
} CMPRenderCmdPushClip;

/**
 * @brief Set transform render command payload.
 */
typedef struct CMPRenderCmdSetTransform {
  CMPMat3 transform; /**< Transform matrix. */
} CMPRenderCmdSetTransform;

/**
 * @brief Draw texture render command payload.
 */
typedef struct CMPRenderCmdDrawTexture {
  CMPHandle texture; /**< Texture handle. */
  CMPRect src;       /**< Source rectangle in texture space. */
  CMPRect dst;       /**< Destination rectangle in window space. */
  CMPScalar opacity; /**< Opacity multiplier (0..1). */
} CMPRenderCmdDrawTexture;

/**
 * @brief Draw text render command payload.
 */
typedef struct CMPRenderCmdDrawText {
  CMPHandle font;     /**< Font handle. */
  const char *utf8;  /**< UTF-8 text pointer (must remain valid for the list
                        lifetime). */
  cmp_usize utf8_len; /**< UTF-8 length in bytes. */
  CMPScalar x;        /**< X origin in pixels. */
  CMPScalar y;        /**< Y origin in pixels. */
  CMPColor color;     /**< Text color. */
} CMPRenderCmdDrawText;

/**
 * @brief Draw path render command payload.
 */
typedef struct CMPRenderCmdDrawPath {
  const CMPPath *path; /**< Path pointer (must remain valid for the list
                         lifetime). */
  CMPColor color;      /**< Fill color. */
} CMPRenderCmdDrawPath;

/**
 * @brief Render command payload union.
 */
typedef union CMPRenderCmdData {
  CMPRenderCmdBeginFrame begin_frame;     /**< Begin frame payload. */
  CMPRenderCmdEndFrame end_frame;         /**< End frame payload. */
  CMPRenderCmdClear clear;                /**< Clear payload. */
  CMPRenderCmdDrawRect draw_rect;         /**< Draw rect payload. */
  CMPRenderCmdDrawLine draw_line;         /**< Draw line payload. */
  CMPRenderCmdPushClip push_clip;         /**< Push clip payload. */
  CMPRenderCmdSetTransform set_transform; /**< Set transform payload. */
  CMPRenderCmdDrawTexture draw_texture;   /**< Draw texture payload. */
  CMPRenderCmdDrawText draw_text;         /**< Draw text payload. */
  CMPRenderCmdDrawPath draw_path;         /**< Draw path payload. */
} CMPRenderCmdData;

/**
 * @brief Render command entry.
 */
typedef struct CMPRenderCmd {
  cmp_u32 type;          /**< Command type (CMP_RENDER_CMD_*). */
  CMPRenderCmdData data; /**< Command payload. */
} CMPRenderCmd;

/**
 * @brief Render command list.
 */
typedef struct CMPRenderList {
  CMPRenderCmd *commands; /**< Command array (owned by the list). */
  cmp_usize count;        /**< Number of commands in the list. */
  cmp_usize capacity;     /**< Command array capacity. */
  CMPAllocator allocator; /**< Allocator used for command storage. */
} CMPRenderList;

/**
 * @brief Render tree node for widget traversal.
 */
typedef struct CMPRenderNode {
  CMPWidget *widget;               /**< Widget instance. */
  struct CMPRenderNode **children; /**< Child node array. */
  cmp_usize child_count;           /**< Number of children. */
  CMPRect bounds;                  /**< Layout bounds in pixels. */
} CMPRenderNode;

/**
 * @brief Initialize a render list.
 * @param list Render list to initialize.
 * @param allocator Allocator to use; NULL uses the default allocator.
 * @param initial_capacity Initial command capacity (0 selects a default).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_list_init(CMPRenderList *list,
                                       const CMPAllocator *allocator,
                                       cmp_usize initial_capacity);

/**
 * @brief Reset a render list without freeing its storage.
 * @param list Render list to reset.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_list_reset(CMPRenderList *list);

/**
 * @brief Shut down a render list and release its storage.
 * @param list Render list to shut down.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_list_shutdown(CMPRenderList *list);

/**
 * @brief Append a render command to the list.
 * @param list Render list instance.
 * @param cmd Command to append.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_list_append(CMPRenderList *list,
                                         const CMPRenderCmd *cmd);

/**
 * @brief Execute a render list on a graphics backend.
 * @param list Render list to execute.
 * @param gfx Graphics backend interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_list_execute(const CMPRenderList *list, CMPGfx *gfx);

/**
 * @brief Initialize a render tree node.
 * @param node Render node to initialize.
 * @param widget Widget instance for the node.
 * @param bounds Layout bounds for the widget.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_node_init(CMPRenderNode *node, CMPWidget *widget,
                                       const CMPRect *bounds);

/**
 * @brief Update the bounds for a render node.
 * @param node Render node to update.
 * @param bounds New bounds in pixels.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_node_set_bounds(CMPRenderNode *node,
                                             const CMPRect *bounds);

/**
 * @brief Assign children to a render node.
 * @param node Render node to update.
 * @param children Child node array (may be NULL when count is 0).
 * @param count Number of children.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_node_set_children(CMPRenderNode *node,
                                               CMPRenderNode **children,
                                               cmp_usize count);

/**
 * @brief Build a render command list from a widget tree.
 * @param root Root render node.
 * @param list Render list to append to.
 * @param dpi_scale DPI scale factor for paint contexts.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_build(const CMPRenderNode *root, CMPRenderList *list,
                                   CMPScalar dpi_scale);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for render add-overflow helper.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives sum.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_test_add_overflow(cmp_usize a, cmp_usize b,
                                               cmp_usize *out_value);

/**
 * @brief Test wrapper for render multiply-overflow helper.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives product.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_test_mul_overflow(cmp_usize a, cmp_usize b,
                                               cmp_usize *out_value);

/**
 * @brief Test wrapper for render list reserve helper.
 * @param list Render list to grow.
 * @param additional Additional slots needed.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_test_list_reserve(CMPRenderList *list,
                                               cmp_usize additional);

/**
 * @brief Test wrapper for render node validation.
 * @param node Render node to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_test_validate_node(const CMPRenderNode *node);

/**
 * @brief Test hook to force render list reserve path.
 * @param enable Whether to force the reserve path.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_test_set_force_reserve(CMPBool enable);

/**
 * @brief Test hook to force clip intersection failure.
 * @param enable Whether to force failure.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_render_test_set_force_intersect_fail(CMPBool enable);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_RENDER_H */
