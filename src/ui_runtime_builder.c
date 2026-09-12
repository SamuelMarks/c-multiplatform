/**
 * @file ui_runtime_builder.c
 * @brief Runtime interpreter implementation constructing live DOM trees from UI
 * Schema ASTs.
 */

/* clang-format off */
#include "ui_runtime_builder.h"
#include "ui_workflow_engine.h"
#include "ui_runtime_router.h"
#include "ui_form_validators.h"
#include "ui_internal_mem.h"
#include "ui_coercion_utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

/**
 * @struct ui_runtime_workflow_bridge
 * @brief Context package linking a DOM event callback to the workflow engine.
 */
struct ui_runtime_workflow_bridge {
  const struct ui_runtime_workflow *workflow; /**< The workflow to execute */
  struct ui_workflow_context wf_ctx;          /**< Execution context */
};

/**
 * @brief Generic event dispatcher invoked by widget event callbacks.
 * @param user_data Pointer to the workflow bridge context.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t on_bridge_event(void *user_data) {
  struct ui_runtime_workflow_bridge *bridge =
      (struct ui_runtime_workflow_bridge *)user_data;

  if (!bridge || !bridge->workflow) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_workflow_engine_execute(bridge->workflow, &bridge->wf_ctx);
}

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
ui_error_t ui_runtime_build_tree(const struct ui_runtime_node *node,
                                 struct ui_component_registry *registry,
                                 struct ui_dynamic_context *ctx,
                                 struct ui_app_state_registry *app_state,
                                 struct ui_runtime_router *router,
                                 struct ui_dom_node *mount_host,
                                 struct ui_dom_node **out_root) {
  const struct ui_component_vtable *vtable = NULL;
  void *instance = NULL;
  struct ui_dom_node *dom_root = NULL;
  const struct ui_runtime_prop *p;
  const struct ui_runtime_binding *b;
  const struct ui_runtime_workflow *wf;
  const struct ui_runtime_node *child;
  ui_error_t rc;

  if (!node || !out_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!registry) {
    (void)ui_component_registry_get_default(&registry);
  }

  rc = ui_component_registry_lookup(registry, node->type, &vtable);
  if (rc != UI_ERROR_NONE || !vtable->factory) {
    return UI_ERROR_NOT_FOUND;
  }

  rc = vtable->factory(&instance, &dom_root);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (node->id && dom_root) {
    rc = ui_dom_node_set_attribute(dom_root, "id", node->id);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  /* Apply static properties */
  p = node->props;
  while (p) {
    if (vtable->set_prop) {
      rc = vtable->set_prop(instance, p->key, p->val);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
    p = p->next;
  }

  /* Apply dynamic bindings */
  b = node->bindings;
  while (b) {
    if (b->type == UI_RUNTIME_BINDING_CVA && vtable->get_cva && ctx) {
      struct ui_control_value_accessor cva;
      ui_form_control_t *ctrl = NULL;

      memset(&cva, 0, sizeof(cva));
      rc = vtable->get_cva(instance, &cva);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      rc = ui_dynamic_context_resolve_form_control(ctx, b->source_path, &ctrl);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      {
        const struct ui_runtime_validator_def *val_def;

        rc = ui_form_control_bind_cva(ctrl, &cva);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }

        /* Attach validators to form control */
        val_def = node->validators;
        while (val_def) {
          ui_validator_fn vfn = NULL;
          if (val_def->type == UI_RUNTIME_VALIDATOR_REQUIRED) {
            (void)ui_validators_required(&vfn);
            rc = ui_form_control_add_validator(ctrl, vfn, NULL);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
          } else if (val_def->type == UI_RUNTIME_VALIDATOR_PATTERN &&
                     val_def->param) {
            (void)ui_validators_pattern(&vfn);
            rc = ui_form_control_add_validator(ctrl, vfn,
                                               (void *)val_def->param);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
          } else if (val_def->type == UI_RUNTIME_VALIDATOR_MIN_LENGTH) {
            (void)ui_validators_min_length(&vfn);
            rc = ui_form_control_add_validator(ctrl, vfn,
                                               (void *)&val_def->int_param);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
          } else if (val_def->type == UI_RUNTIME_VALIDATOR_MAX_LENGTH) {
            (void)ui_validators_max_length(&vfn);
            rc = ui_form_control_add_validator(ctrl, vfn,
                                               (void *)&val_def->int_param);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
          }
          val_def = val_def->next;
        }
      }
    } else if (b->type == UI_RUNTIME_BINDING_TEXT && ctx) {
      struct ui_signal *sig = NULL;
      rc = ui_dynamic_context_resolve_signal(ctx, app_state, b->source_path,
                                             &sig);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      if (vtable->set_prop) {
        union ui_signal_payload cur_payload;
        (void)ui_signal_get(sig, &cur_payload);
        if (cur_payload.ptr_val) {
          rc = vtable->set_prop(instance, "text",
                                (const char *)cur_payload.ptr_val);
          if (rc != UI_ERROR_NONE) {
            return rc;
          }
        } else {
          char num_buf[32];
          (void)ui_safe_string_format(num_buf, sizeof(num_buf), "%d",
                                      (int)cur_payload.int_val);
          rc = vtable->set_prop(instance, "text", num_buf);
          if (rc != UI_ERROR_NONE) {
            return rc;
          }
        }
      }
    } else if (b->type == UI_RUNTIME_BINDING_DISABLED && ctx) {
      struct ui_signal *sig = NULL;
      rc = ui_dynamic_context_resolve_signal(ctx, app_state, b->source_path,
                                             &sig);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      if (vtable->set_prop) {
        union ui_signal_payload cur_payload;
        (void)ui_signal_get(sig, &cur_payload);
        rc = vtable->set_prop(instance, "disabled",
                              cur_payload.bool_val ? "true" : "false");
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      }
    } else if (b->type == UI_RUNTIME_BINDING_VISIBILITY && ctx && dom_root) {
      struct ui_signal *sig = NULL;
      rc = ui_dynamic_context_resolve_signal(ctx, app_state, b->source_path,
                                             &sig);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      {
        union ui_signal_payload cur_payload;
        (void)ui_signal_get(sig, &cur_payload);
        if (!cur_payload.bool_val) {
          rc = ui_dom_node_set_attribute(dom_root, "style", "display: none;");
          if (rc != UI_ERROR_NONE) {
            return rc;
          }
        }
      }
    }

    b = b->next;
  }

  /* Wire event workflows */
  wf = node->workflows;
  while (wf) {
    if (vtable->attach_event) {
      struct ui_runtime_workflow_bridge *bridge =
          (struct ui_runtime_workflow_bridge *)C_MULTIPLATFORM_MALLOC(
              sizeof(struct ui_runtime_workflow_bridge));
      if (!bridge) {
        return UI_ERROR_OUT_OF_MEMORY;
      }

      bridge->workflow = wf;
      bridge->wf_ctx.ctx = ctx;
      bridge->wf_ctx.app_state = app_state;
      bridge->wf_ctx.router = router;
      bridge->wf_ctx.mount_host = mount_host;
      bridge->wf_ctx.registry = registry;

      rc = vtable->attach_event(instance, wf->event_name, on_bridge_event,
                                bridge);
      if (rc != UI_ERROR_NONE) {
        C_MULTIPLATFORM_FREE(bridge);
        return rc;
      }
    }
    wf = wf->next;
  }

  /* Recursively construct child nodes */
  child = node->first_child;
  while (child) {
    struct ui_dom_node *child_dom = NULL;
    rc = ui_runtime_build_tree(child, registry, ctx, app_state, router,
                               mount_host, &child_dom);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    if (child_dom) {
      if (vtable->append_child) {
        rc = vtable->append_child(instance, child_dom);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      } else if (dom_root) {
        rc = ui_dom_node_append_child(dom_root, child_dom);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      }
    }

    child = child->next_sibling;
  }

  *out_root = dom_root;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets simulated canvas sandbox dimensions on a preview viewport DOM
 * node.
 *
 * @param root The root DOM node of the preview canvas.
 * @param width Viewport width in pixels.
 * @param height Viewport height in pixels.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t ui_runtime_preview_viewport_set_dimensions(struct ui_dom_node *root,
                                                      int width, int height) {
  char style_buf[256];
  char w_buf[32];
  char h_buf[32];
  ui_error_t rc;

  if (!root || width <= 0 || height <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_safe_string_format(
      style_buf, sizeof(style_buf),
      "width: %dpx; height: %dpx; max-width: %dpx; max-height: %dpx; "
      "overflow: auto;",
      width, height, width, height);

  rc = ui_dom_node_set_attribute(root, "style", style_buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  (void)ui_safe_string_format(w_buf, sizeof(w_buf), "%d", width);
  rc = ui_dom_node_set_attribute(root, "data-viewport-width", w_buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  (void)ui_safe_string_format(h_buf, sizeof(h_buf), "%d", height);
  return ui_dom_node_set_attribute(root, "data-viewport-height", h_buf);
}

/**
 * @brief Retrieves simulated canvas sandbox dimensions from a preview viewport
 * DOM node.
 *
 * @param root The root DOM node of the preview canvas.
 * @param out_width Pointer to receive width in pixels.
 * @param out_height Pointer to receive height in pixels.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
ui_error_t
ui_runtime_preview_viewport_get_dimensions(const struct ui_dom_node *root,
                                           int *out_width, int *out_height) {
  const char *w_str = NULL;
  const char *h_str = NULL;
  ui_error_t rc;

  if (!root || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_dom_node_get_attribute(root, "data-viewport-width", &w_str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_dom_node_get_attribute(root, "data-viewport-height", &h_str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_coerce_string_to_int(w_str, out_width);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_coerce_string_to_int(h_str, out_height);
}
