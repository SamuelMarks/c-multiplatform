/**
 * @file ui_app_load.h
 * @brief Unified isomorphic application loader supporting both Runtime JSON
 * schemas and AoT compiled functions.
 */

#ifndef UI_APP_LOAD_H
#define UI_APP_LOAD_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_component_registry.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

/**
 * @enum ui_app_load_mode
 * @brief Execution mode for loading and mounting the application UI tree.
 */
enum ui_app_load_mode {
  UI_APP_LOAD_MODE_RUNTIME = 0, /**< Load from serialized JSON schema payload */
  UI_APP_LOAD_MODE_AOT =
      1 /**< Load from precompiled native C function pointer */
};

/**
 * @brief Function pointer signature for Ahead-of-Time generated mount
 * functions.
 */
typedef ui_error_t (*ui_aot_mount_fn)(struct ui_dom_node *parent_node,
                                      struct ui_dynamic_context *ctx,
                                      struct ui_app_state_registry *app_state,
                                      struct ui_dom_node **out_root);

/**
 * @struct ui_app_load_config
 * @brief Configuration parameters for unified application mounting.
 */
struct ui_app_load_config {
  enum ui_app_load_mode mode; /**< Runtime or AoT mode */
  const char *schema_payload; /**< Schema JSON string (Runtime mode) */
  ui_aot_mount_fn aot_mount;  /**< Compiled function pointer (AoT mode) */
  struct ui_component_registry *registry;  /**< Optional component registry */
  struct ui_dynamic_context *ctx;          /**< Optional dynamic context */
  struct ui_app_state_registry *app_state; /**< Optional app state registry */
};

/**
 * @brief Mounts an entire application UI tree using the isomorphic execution
 * model.
 *
 * @param config Loader configuration specifying mode and source
 * payload/function.
 * @param mount_host Host DOM node to mount the screen or application into.
 * @param out_root Pointer to receive the constructed root DOM element.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_app_load(const struct ui_app_load_config *config,
            struct ui_dom_node *mount_host, struct ui_dom_node **out_root);

/**
 * @brief Mounts a dynamic runtime schema snippet into an existing DOM tree.
 * Enables Over-The-Air (OTA) dynamic widget injection into statically compiled
 * AoT apps.
 *
 * @param snippet_json Serialized JSON widget node snippet.
 * @param parent_node DOM container node to append the dynamic snippet into.
 * @param ctx Dynamic context for data binding.
 * @param app_state Global application state registry.
 * @param out_snippet_root Pointer to receive the root DOM node of the mounted
 * snippet.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_app_load_snippet(
    const char *snippet_json, struct ui_dom_node *parent_node,
    struct ui_dynamic_context *ctx, struct ui_app_state_registry *app_state,
    struct ui_dom_node **out_snippet_root);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_APP_LOAD_H */
