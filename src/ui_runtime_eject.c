/**
 * @file ui_runtime_eject.c
 * @brief Ahead-of-Time (AoT) code generator ejecting UI Schema ASTs to native
 * C89 code.
 */

/* clang-format off */
#include "ui_runtime_eject.h"
#include "ui_internal_mem.h"
#include "ui_coercion_utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
/* clang-format on */

/**
 * @struct emit_buffer
 * @brief Helper buffer accumulator for generating C code strings.
 */
struct emit_buffer {
  char *buf;       /**< Output character buffer */
  size_t capacity; /**< Total capacity */
  size_t length;   /**< Current length */
};

/**
 * @brief Appends formatted string to emit_buffer.
 */
static ui_error_t emit_printf(struct emit_buffer *eb, const char *format, ...) {
  va_list args;
  int written;
  size_t available;

  available = eb->capacity - eb->length;
  va_start(args, format);
#if defined(_MSC_VER)
  written =
      vsnprintf_s(eb->buf + eb->length, available, _TRUNCATE, format, args);
#else
  written = vsnprintf(eb->buf + eb->length, available, format, args);
#endif
  va_end(args);

  if ((size_t)written >= available) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  eb->length += (size_t)written;
  return UI_ERROR_NONE;
}

/**
 * @struct node_meta
 * @brief Generated variable names and indexing for a single AST node.
 */
struct node_meta {
  int id_num;
  char inst_var[64];
  char comp_var[64];
  char dom_var[64];
  char cva_var[64];
  char ctrl_var[64];
  char sig_var[64];
};

/**
 * @brief Assigns variable names to AST nodes.
 * @param meta Pointer to node metadata.
 * @param id_num Unique identifier number.
 * @param node Pointer to AST node.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
static ui_error_t init_node_meta(struct node_meta *meta, int id_num,
                                 const struct ui_runtime_node *node) {
  meta->id_num = id_num;
  if (strcmp(node->type, "ui_button_base") == 0) {
    (void)ui_safe_string_format(meta->inst_var, sizeof(meta->inst_var),
                                "btn_%d", id_num);
  } else if (strcmp(node->type, "ui_input_base") == 0) {
    (void)ui_safe_string_format(meta->inst_var, sizeof(meta->inst_var),
                                "input_%d", id_num);
  } else if (strcmp(node->type, "ui_card_base") == 0) {
    (void)ui_safe_string_format(meta->inst_var, sizeof(meta->inst_var),
                                "card_%d", id_num);
  } else if (strcmp(node->type, "ui_checkbox_base") == 0) {
    (void)ui_safe_string_format(meta->inst_var, sizeof(meta->inst_var),
                                "chk_%d", id_num);
  } else if (strcmp(node->type, "ui_label_base") == 0) {
    (void)ui_safe_string_format(meta->inst_var, sizeof(meta->inst_var),
                                "lbl_%d", id_num);
  } else if (strcmp(node->type, "ui_slider_base") == 0) {
    (void)ui_safe_string_format(meta->inst_var, sizeof(meta->inst_var),
                                "sld_%d", id_num);
  } else {
    (void)ui_safe_string_format(meta->inst_var, sizeof(meta->inst_var),
                                "layout_%d", id_num);
  }

  (void)ui_safe_string_format(meta->comp_var, sizeof(meta->comp_var), "comp_%d",
                              id_num);
  (void)ui_safe_string_format(meta->dom_var, sizeof(meta->dom_var), "dom_%d",
                              id_num);
  (void)ui_safe_string_format(meta->cva_var, sizeof(meta->cva_var), "cva_%d",
                              id_num);
  (void)ui_safe_string_format(meta->ctrl_var, sizeof(meta->ctrl_var), "ctrl_%d",
                              id_num);
  (void)ui_safe_string_format(meta->sig_var, sizeof(meta->sig_var), "sig_%d",
                              id_num);

  return UI_ERROR_NONE;
}

/**
 * @brief Recursively emits variable declarations at top of function (C89
 * compliance).
 */
static ui_error_t
emit_declarations_recursive(struct emit_buffer *eb,
                            const struct ui_runtime_node *node, int *counter) {
  struct node_meta meta;
  const struct ui_runtime_node *child;
  ui_error_t rc;

  (*counter)++;
  (void)init_node_meta(&meta, *counter, node);

  if (strcmp(node->type, "ui_button_base") == 0) {
    rc =
        emit_printf(eb, "  struct ui_button_base *%s = NULL;\n", meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  struct ui_component *%s = NULL;\n", meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (strcmp(node->type, "ui_input_base") == 0) {
    rc = emit_printf(eb, "  struct ui_input_base *%s = NULL;\n", meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  struct ui_component *%s = NULL;\n", meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  struct ui_control_value_accessor %s;\n",
                     meta.cva_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  ui_form_control_t *%s = NULL;\n", meta.ctrl_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (strcmp(node->type, "ui_card_base") == 0) {
    rc = emit_printf(eb, "  struct ui_card_base *%s = NULL;\n", meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  struct ui_component *%s = NULL;\n", meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (strcmp(node->type, "ui_checkbox_base") == 0) {
    rc = emit_printf(eb, "  struct ui_checkbox_base *%s = NULL;\n",
                     meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  struct ui_component *%s = NULL;\n", meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (strcmp(node->type, "ui_label_base") == 0) {
    rc = emit_printf(eb, "  struct ui_label_base *%s = NULL;\n", meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  struct ui_component *%s = NULL;\n", meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (strcmp(node->type, "ui_slider_base") == 0) {
    rc =
        emit_printf(eb, "  struct ui_slider_base *%s = NULL;\n", meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  struct ui_component *%s = NULL;\n", meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  rc = emit_printf(eb, "  struct ui_dom_node *%s = NULL;\n", meta.dom_var);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (node->bindings) {
    rc = emit_printf(eb, "  struct ui_signal *%s = NULL;\n", meta.sig_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (node->validators) {
    rc = emit_printf(eb, "  ui_validator_fn val_fn_%d = NULL;\n", *counter);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  child = node->first_child;
  while (child) {
    rc = emit_declarations_recursive(eb, child, counter);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    child = child->next_sibling;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Recursively emits instantiation and wiring logic for AST nodes.
 */
static ui_error_t emit_body_recursive(struct emit_buffer *eb,
                                      const struct ui_runtime_node *node,
                                      const char *parent_dom_var, int *counter,
                                      char *out_dom_var,
                                      size_t out_dom_var_size) {
  struct node_meta meta;
  const struct ui_runtime_prop *p;
  const struct ui_runtime_binding *b;
  const struct ui_runtime_validator_def *v;
  const struct ui_runtime_node *child;
  ui_error_t rc;

  (*counter)++;
  (void)init_node_meta(&meta, *counter, node);
  (void)ui_safe_string_copy(out_dom_var, out_dom_var_size, meta.dom_var);

  /* Instantiation */
  if (strcmp(node->type, "ui_button_base") == 0) {
    rc = emit_printf(eb, "  rc = ui_button_base_create(&%s);\n", meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  rc = ui_button_base_get_component(%s, &%s);\n",
                     meta.inst_var, meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  %s = %s->shadow_root;\n", meta.dom_var,
                     meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (strcmp(node->type, "ui_input_base") == 0) {
    rc = emit_printf(eb, "  rc = ui_input_base_create(&%s);\n", meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  rc = ui_input_base_get_component(%s, &%s);\n",
                     meta.inst_var, meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  %s = %s->shadow_root;\n", meta.dom_var,
                     meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (strcmp(node->type, "ui_card_base") == 0) {
    rc = emit_printf(eb, "  rc = ui_card_base_create(&%s);\n", meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  rc = ui_card_base_get_component(%s, &%s);\n",
                     meta.inst_var, meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  %s = %s->shadow_root;\n", meta.dom_var,
                     meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (strcmp(node->type, "ui_checkbox_base") == 0) {
    rc = emit_printf(eb, "  rc = ui_checkbox_base_create(&%s);\n",
                     meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  rc = ui_checkbox_base_get_component(%s, &%s);\n",
                     meta.inst_var, meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  %s = %s->shadow_root;\n", meta.dom_var,
                     meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (strcmp(node->type, "ui_label_base") == 0) {
    rc = emit_printf(eb, "  rc = ui_label_base_create(&%s);\n", meta.inst_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  rc = ui_label_base_get_component(%s, &%s);\n",
                     meta.inst_var, meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  %s = %s->shadow_root;\n", meta.dom_var,
                     meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else if (strcmp(node->type, "ui_slider_base") == 0) {
    rc = emit_printf(eb, "  rc = ui_slider_base_create(&%s, &%s);\n",
                     meta.inst_var, meta.cva_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  rc = ui_slider_base_get_component(%s, &%s);\n",
                     meta.inst_var, meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  %s = %s->shadow_root;\n", meta.dom_var,
                     meta.comp_var);
    if (rc != UI_ERROR_NONE)
      return rc;
  } else {
    /* Layout primitives */
    const char *layout_style = "display: block;";
    if (strcmp(node->type, "row") == 0) {
      layout_style = "display: flex; flex-direction: row;";
    } else if (strcmp(node->type, "column") == 0) {
      layout_style = "display: flex; flex-direction: column;";
    } else if (strcmp(node->type, "grid") == 0) {
      layout_style = "display: grid;";
    }

    rc = emit_printf(
        eb, "  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &%s);\n",
        meta.dom_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  rc = ui_dom_node_set_tag_name(%s, \"div\");\n",
                     meta.dom_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(
        eb, "  rc = ui_dom_node_set_attribute(%s, \"style\", \"%s\");\n",
        meta.dom_var, layout_style);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (node->id) {
    rc = emit_printf(eb,
                     "  rc = ui_dom_node_set_attribute(%s, \"id\", \"%s\");\n",
                     meta.dom_var, node->id);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  /* Properties */
  p = node->props;
  while (p) {
    if (strcmp(node->type, "ui_button_base") == 0) {
      if (strcmp(p->key, "text") == 0) {
        rc = emit_printf(eb, "  rc = ui_button_base_set_text(%s, \"%s\");\n",
                         meta.inst_var, p->val);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;
      } else {
        rc = emit_printf(
            eb, "  rc = ui_dom_node_set_attribute(%s, \"%s\", \"%s\");\n",
            meta.dom_var, p->key, p->val);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;
      }
    } else if (strcmp(node->type, "ui_input_base") == 0) {
      if (strcmp(p->key, "placeholder") == 0) {
        rc = emit_printf(eb,
                         "  rc = ui_input_base_set_placeholder(%s, \"%s\");\n",
                         meta.inst_var, p->val);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;
      } else {
        rc = emit_printf(
            eb, "  rc = ui_dom_node_set_attribute(%s, \"%s\", \"%s\");\n",
            meta.dom_var, p->key, p->val);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;
      }
    } else if (strcmp(node->type, "ui_card_base") == 0) {
      if (strcmp(p->key, "title") == 0) {
        rc = emit_printf(eb, "  rc = ui_card_base_set_title(%s, \"%s\");\n",
                         meta.inst_var, p->val);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;
      } else {
        rc = emit_printf(
            eb, "  rc = ui_dom_node_set_attribute(%s, \"%s\", \"%s\");\n",
            meta.dom_var, p->key, p->val);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;
      }
    } else {
      rc = emit_printf(
          eb, "  rc = ui_dom_node_set_attribute(%s, \"%s\", \"%s\");\n",
          meta.dom_var, p->key, p->val);
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    p = p->next;
  }

  /* Bindings */
  b = node->bindings;
  while (b) {
    if (b->type == UI_RUNTIME_BINDING_CVA) {
      if (strcmp(node->type, "ui_input_base") == 0) {
        rc = emit_printf(eb, "  rc = ui_input_base_get_cva(%s, &%s);\n",
                         meta.inst_var, meta.cva_var);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;

        rc = emit_printf(eb,
                         "  rc = ui_dynamic_context_resolve_form_control(ctx, "
                         "\"%s\", &%s);\n",
                         b->source_path, meta.ctrl_var);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;

        rc = emit_printf(eb, "  rc = ui_form_control_bind_cva(%s, &%s);\n",
                         meta.ctrl_var, meta.cva_var);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;

        /* Validators */
        v = node->validators;
        while (v) {
          if (v->type == UI_RUNTIME_VALIDATOR_REQUIRED) {
            rc = emit_printf(eb, "  rc = ui_validators_required(&val_fn_%d);\n",
                             meta.id_num);
            if (rc != UI_ERROR_NONE)
              return rc;
            rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
            if (rc != UI_ERROR_NONE)
              return rc;
            rc = emit_printf(
                eb,
                "  rc = ui_form_control_add_validator(%s, val_fn_%d, NULL);\n",
                meta.ctrl_var, meta.id_num);
            if (rc != UI_ERROR_NONE)
              return rc;
            rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
            if (rc != UI_ERROR_NONE)
              return rc;
          } else if (v->type == UI_RUNTIME_VALIDATOR_PATTERN) {
            if (v->param) {
              rc =
                  emit_printf(eb, "  rc = ui_validators_pattern(&val_fn_%d);\n",
                              meta.id_num);
              if (rc != UI_ERROR_NONE)
                return rc;
              rc =
                  emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
              if (rc != UI_ERROR_NONE)
                return rc;
              rc = emit_printf(eb,
                               "  rc = ui_form_control_add_validator(%s, "
                               "val_fn_%d, (void *)\"%s\");\n",
                               meta.ctrl_var, meta.id_num, v->param);
              if (rc != UI_ERROR_NONE)
                return rc;
              rc =
                  emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
              if (rc != UI_ERROR_NONE)
                return rc;
            }
          }
          v = v->next;
        }
      }
    } else if (b->type == UI_RUNTIME_BINDING_TEXT) {
      rc = emit_printf(eb,
                       "  rc = ui_dynamic_context_resolve_signal(ctx, "
                       "app_state, \"%s\", &%s);\n",
                       b->source_path, meta.sig_var);
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
      if (rc != UI_ERROR_NONE)
        return rc;

      if (strcmp(node->type, "ui_button_base") == 0) {
        rc = emit_printf(eb, "  rc = ui_button_base_bind_text(%s, %s);\n",
                         meta.inst_var, meta.sig_var);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;
      }
    } else if (b->type == UI_RUNTIME_BINDING_DISABLED) {
      rc = emit_printf(eb,
                       "  rc = ui_dynamic_context_resolve_signal(ctx, "
                       "app_state, \"%s\", &%s);\n",
                       b->source_path, meta.sig_var);
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
      if (rc != UI_ERROR_NONE)
        return rc;

      if (strcmp(node->type, "ui_button_base") == 0) {
        rc = emit_printf(eb, "  rc = ui_button_base_bind_disabled(%s, %s);\n",
                         meta.inst_var, meta.sig_var);
        if (rc != UI_ERROR_NONE)
          return rc;
        rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
        if (rc != UI_ERROR_NONE)
          return rc;
      }
    }

    b = b->next;
  }

  /* Append to parent DOM node */
  if (parent_dom_var) {
    rc = emit_printf(eb, "  rc = ui_dom_node_append_child(%s, %s);\n",
                     parent_dom_var, meta.dom_var);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = emit_printf(eb, "  if (rc != UI_ERROR_NONE) goto cleanup;\n");
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  /* Recurse on children */
  child = node->first_child;
  while (child) {
    char child_dom_var[64];
    rc = emit_body_recursive(eb, child, meta.dom_var, counter, child_dom_var,
                             sizeof(child_dom_var));
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    child = child->next_sibling;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Traverses an AST node tree and emits equivalent C89 source and header
 * code into memory buffers.
 *
 * @param node Root AST node.
 * @param func_name Name of the C mount function to generate.
 * @param c_buf Buffer to receive the generated C source code.
 * @param c_buf_size Size of the C source buffer.
 * @param h_buf Buffer to receive the generated C header code.
 * @param h_buf_size Size of the C header buffer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_runtime_eject_tree_to_c_buffer(const struct ui_runtime_node *node,
                                             const char *func_name, char *c_buf,
                                             size_t c_buf_size, char *h_buf,
                                             size_t h_buf_size) {
  struct emit_buffer eb_h;
  struct emit_buffer eb_c;
  char upper_name[128];
  char root_dom_var[64];
  size_t i;
  int decl_counter = 0;
  int body_counter = 0;
  ui_error_t rc;

  if (!node || !func_name || !c_buf || !h_buf || c_buf_size == 0 ||
      h_buf_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Generate UPPER_CASE header guard name */
  rc = ui_safe_string_copy(upper_name, sizeof(upper_name), func_name);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  for (i = 0; upper_name[i] != '\0'; i++) {
    upper_name[i] = (char)toupper((unsigned char)upper_name[i]);
  }

  /* Emit Header (.h) */
  eb_h.buf = h_buf;
  eb_h.capacity = h_buf_size;
  eb_h.length = 0;
  h_buf[0] = '\0';

  rc = emit_printf(&eb_h,
                   "/**\n * @file %s.h\n * @brief Ahead-of-Time generated UI "
                   "definition.\n */\n\n",
                   func_name);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "#ifndef %s_H\n#define %s_H\n\n", upper_name,
                   upper_name);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "/* clang-format off */\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "#include \"ui_error.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "#include \"ui_dom_node.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "#include \"ui_dynamic_context.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "#include \"ui_app_state_registry.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "/* clang-format on */\n\n");
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = emit_printf(&eb_h, "/**\n * @brief Mounts the generated UI tree.\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h,
                   " * @param parent_node The parent DOM node to attach to.\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, " * @param ctx The scoped dynamic context.\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, " * @param app_state The global state registry.\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(
      &eb_h, " * @param out_root Pointer to receive the root DOM node.\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(
      &eb_h, " * @return UI_ERROR_NONE on success, or error enum.\n */\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "extern C_MULTIPLATFORM_EXPORT ui_error_t\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "%s_create(struct ui_dom_node *parent_node,\n",
                   func_name);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "          struct ui_dynamic_context *ctx,\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h,
                   "          struct ui_app_state_registry *app_state,\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_h, "          struct ui_dom_node **out_root);\n\n");
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = emit_printf(
      &eb_h,
      "#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n\n#endif /* %s_H */\n",
      upper_name);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Emit Source (.c) */
  eb_c.buf = c_buf;
  eb_c.capacity = c_buf_size;
  eb_c.length = 0;
  c_buf[0] = '\0';

  rc = emit_printf(&eb_c,
                   "/**\n * @file %s.c\n * @brief Ahead-of-Time generated UI "
                   "implementation.\n */\n\n",
                   func_name);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "/* clang-format off */\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "#include \"%s.h\"\n", func_name);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "#include \"ui_button_base.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "#include \"ui_input_base.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "#include \"ui_card_base.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "#include \"ui_checkbox_base.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "#include \"ui_label_base.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "#include \"ui_slider_base.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "#include \"ui_form_validators.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "#include \"ui_form_control.h\"\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "#include <stddef.h>\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "/* clang-format on */\n\n");
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = emit_printf(&eb_c, "/**\n * @brief Mounts the generated UI tree.\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, " * @param parent_node Parameter parent_node.\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, " * @param ctx Parameter ctx.\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, " * @param app_state Parameter app_state.\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, " * @param out_root Parameter out_root.\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, " * @return UI_ERROR_NONE on success.\n */\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c,
                   "ui_error_t %s_create(struct ui_dom_node *parent_node,\n",
                   func_name);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c,
                   "                     struct ui_dynamic_context *ctx,\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(
      &eb_c, "                     struct ui_app_state_registry *app_state,\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c,
                   "                     struct ui_dom_node **out_root) {\n");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = emit_printf(&eb_c, "  ui_error_t rc = UI_ERROR_NONE;\n");
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Declarations at top of function */
  rc = emit_declarations_recursive(&eb_c, node, &decl_counter);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = emit_printf(
      &eb_c,
      "\n  if (!out_root) {\n    return UI_ERROR_INVALID_ARGUMENT;\n  }\n\n");
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Body execution */
  rc = emit_body_recursive(&eb_c, node, NULL, &body_counter, root_dom_var,
                           sizeof(root_dom_var));
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* Mount to parent if provided */
  rc = emit_printf(&eb_c,
                   "  if (parent_node && %s) {\n"
                   "    rc = ui_dom_node_append_child(parent_node, %s);\n"
                   "    if (rc != UI_ERROR_NONE) goto cleanup;\n"
                   "  }\n\n",
                   root_dom_var, root_dom_var);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = emit_printf(&eb_c, "  *out_root = %s;\n", root_dom_var);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Cleanup block */
  rc = emit_printf(&eb_c, "\ncleanup:\n  return rc;\n}\n");
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/**
 * @brief Ejects an application manifest's routes into generated .c and .h
 * files.
 */
ui_error_t ui_runtime_eject_to_c(const struct ui_runtime_app_manifest *manifest,
                                 const char *func_name,
                                 const char *output_c_path,
                                 const char *output_h_path) {
  if (!manifest || !manifest->routes || !manifest->routes->root_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_runtime_eject_node_to_c(manifest->routes->root_node, func_name,
                                    output_c_path, output_h_path);
}

/**
 * @brief Ejects a single widget tree AST node into generated .c and .h files.
 */
ui_error_t ui_runtime_eject_node_to_c(const struct ui_runtime_node *node,
                                      const char *func_name,
                                      const char *output_c_path,
                                      const char *output_h_path) {
  char *c_buf;
  char *h_buf;
  FILE *fc;
  FILE *fh;
  size_t c_size = 65536;
  size_t h_size = 16384;
  ui_error_t rc;

  if (!node || !func_name || !output_c_path || !output_h_path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  c_buf = (char *)C_MULTIPLATFORM_MALLOC(c_size);
  if (!c_buf) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  h_buf = (char *)C_MULTIPLATFORM_MALLOC(h_size);
  if (!h_buf) {
    C_MULTIPLATFORM_FREE(c_buf);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_runtime_eject_tree_to_c_buffer(node, func_name, c_buf, c_size, h_buf,
                                         h_size);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(c_buf);
    C_MULTIPLATFORM_FREE(h_buf);
    return rc;
  }

#if defined(_MSC_VER)
  if (fopen_s(&fc, output_c_path, "w") != 0 || !fc) {
    fc = NULL;
  }
#else
  fc = fopen(output_c_path, "w");
#endif
  if (!fc) {
    C_MULTIPLATFORM_FREE(c_buf);
    C_MULTIPLATFORM_FREE(h_buf);
    return UI_ERROR_UNKNOWN;
  }
  fputs(c_buf, fc);
  fclose(fc);

#if defined(_MSC_VER)
  if (fopen_s(&fh, output_h_path, "w") != 0 || !fh) {
    fh = NULL;
  }
#else
  fh = fopen(output_h_path, "w");
#endif
  if (!fh) {
    C_MULTIPLATFORM_FREE(c_buf);
    C_MULTIPLATFORM_FREE(h_buf);
    return UI_ERROR_UNKNOWN;
  }
  fputs(h_buf, fh);
  fclose(fh);

  C_MULTIPLATFORM_FREE(c_buf);
  C_MULTIPLATFORM_FREE(h_buf);
  return UI_ERROR_NONE;
}
