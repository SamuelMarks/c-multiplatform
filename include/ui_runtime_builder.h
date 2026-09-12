/**
 * @file ui_runtime_builder.h
 * @brief Runtime interpreter for constructing live DOM trees from UI Schema
 * ASTs.
 */

#ifndef UI_RUNTIME_BUILDER_H
#define UI_RUNTIME_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_runtime_schema.h"
#include "ui_component_registry.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_dom_node.h"
#include <stddef.h>
/* clang-format on */

struct ui_runtime_router;

/**
 * @brief Interprets an AST node hierarchy and constructs the corresponding live
 * DOM element tree.
 *
 * @param node The root AST node of the subtree to build.
 * @param registry The component registry (uses default registry if NULL).
 * @param ctx The scoped dynamic context for signal and form resolution.
 * @param app_state The application state registry.
 * @param router The optional runtime router for workflow navigation.
 * @param mount_host The DOM node acting as the viewport or screen host.
 * @param out_root Pointer to receive the constructed root DOM element.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_runtime_build_tree(
    const struct ui_runtime_node *node, struct ui_component_registry *registry,
    struct ui_dynamic_context *ctx, struct ui_app_state_registry *app_state,
    struct ui_runtime_router *router, struct ui_dom_node *mount_host,
    struct ui_dom_node **out_root);

/**
 * @brief Sets simulated canvas sandbox dimensions on a preview viewport DOM
 * node. Enables WYSIWYG editors to preview responsive reflows for iOS (e.g.
 * 390x844) or Desktop (1920x1080).
 *
 * @param root The root DOM node of the preview canvas.
 * @param width Viewport width in pixels.
 * @param height Viewport height in pixels.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_runtime_preview_viewport_set_dimensions(struct ui_dom_node *root, int width,
                                           int height);

/**
 * @brief Retrieves simulated canvas sandbox dimensions from a preview viewport
 * DOM node.
 *
 * @param root The root DOM node of the preview canvas.
 * @param out_width Pointer to receive width in pixels.
 * @param out_height Pointer to receive height in pixels.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_runtime_preview_viewport_get_dimensions(const struct ui_dom_node *root,
                                           int *out_width, int *out_height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RUNTIME_BUILDER_H */
