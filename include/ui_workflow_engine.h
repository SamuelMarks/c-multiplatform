/**
 * @file ui_workflow_engine.h
 * @brief Sequential workflow execution engine implementing Arazzo Specification
 * semantics.
 */

#ifndef UI_WORKFLOW_ENGINE_H
#define UI_WORKFLOW_ENGINE_H

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
 * @struct ui_workflow_context
 * @brief Context passed into workflow execution containing runtime environment.
 */
struct ui_workflow_context {
  struct ui_dynamic_context *ctx;          /**< Local dynamic context */
  struct ui_app_state_registry *app_state; /**< App state registry */
  struct ui_runtime_router *router;        /**< Runtime router for navigation */
  struct ui_dom_node *mount_host;          /**< Current mount container */
  struct ui_component_registry *registry;  /**< Component registry */
};

/**
 * @brief Executes a sequence of workflow steps according to Arazzo semantics.
 *
 * @param workflow The workflow action sequence to execute.
 * @param wf_ctx Execution context containing state, router, and DOM mount host.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_workflow_engine_execute(const struct ui_runtime_workflow *workflow,
                           struct ui_workflow_context *wf_ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WORKFLOW_ENGINE_H */
