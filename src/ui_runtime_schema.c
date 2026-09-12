/**
 * @file ui_runtime_schema.c
 * @brief Schema validation and AST parser implementation for runtime-defined UI
 * widgets.
 */

/* clang-format off */
#include "ui_runtime_schema.h"
#include "ui_internal_mem.h"
#include "ui_coercion_utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "parson.h"
/* clang-format on */

/**
 * @brief Duplicates a C string into memory allocated from a UI arena.
 *
 * @param arena The memory arena.
 * @param str The string to duplicate.
 * @param out_str Pointer to receive the duplicated string.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t arena_strdup(struct ui_arena *arena, const char *str,
                               char **out_str) {
  size_t len;
  char *buf;
  ui_error_t rc;

  len = strlen(str);
  rc = ui_arena_alloc(arena, len + 1, 1, (void **)&buf);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (UI_STRCPY(buf, len + 1, str) != 0) {
    return UI_ERROR_UNKNOWN;
  }

  *out_str = buf;
  return UI_ERROR_NONE;
}

/* Forward declarations for validation */
static ui_error_t validate_node_object(const JSON_Object *node_obj);

/**
 * @brief Validates a JSON validators array.
 * @param val_arr The JSON array to validate.
 * @return UI_ERROR_NONE if valid, UI_ERROR_PARSE_FAILED if invalid.
 */
static ui_error_t validate_validators_array(const JSON_Array *val_arr) {
  size_t i;
  size_t count;

  count = json_array_get_count(val_arr);
  for (i = 0; i < count; i++) {
    const JSON_Object *v_obj = json_array_get_object(val_arr, i);
    size_t k;
    size_t key_count;
    const char *type_str;

    if (!v_obj) {
      return UI_ERROR_PARSE_FAILED;
    }

    key_count = json_object_get_count(v_obj);
    for (k = 0; k < key_count; k++) {
      const char *kname = json_object_get_name(v_obj, k);
      if (strcmp(kname, "type") != 0 && strcmp(kname, "pattern") != 0 &&
          strcmp(kname, "min_length") != 0 &&
          strcmp(kname, "max_length") != 0 && strcmp(kname, "value") != 0) {
        return UI_ERROR_PARSE_FAILED; /* additionalProperties: false */
      }
    }

    type_str = json_object_get_string(v_obj, "type");
    if (!type_str) {
      return UI_ERROR_PARSE_FAILED;
    }
    if (strcmp(type_str, "required") != 0 && strcmp(type_str, "pattern") != 0 &&
        strcmp(type_str, "min_length") != 0 &&
        strcmp(type_str, "max_length") != 0) {
      return UI_ERROR_PARSE_FAILED;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Validates a JSON events object.
 * @param events_obj The events object to validate.
 * @return UI_ERROR_NONE if valid, UI_ERROR_PARSE_FAILED if invalid.
 */
static ui_error_t validate_events_object(const JSON_Object *events_obj) {
  size_t i;
  size_t count;

  count = json_object_get_count(events_obj);
  for (i = 0; i < count; i++) {
    const char *event_name = json_object_get_name(events_obj, i);
    const JSON_Object *evt_def;
    const JSON_Array *steps_arr;
    size_t step_idx;
    size_t step_count;
    size_t k;
    size_t def_key_count;

    if (strcmp(event_name, "on_click") != 0 &&
        strcmp(event_name, "on_change") != 0 &&
        strcmp(event_name, "on_submit") != 0 &&
        strcmp(event_name, "on_blur") != 0) {
      return UI_ERROR_PARSE_FAILED; /* Unknown event hook */
    }

    evt_def = json_object_get_object(events_obj, event_name);
    if (!evt_def) {
      return UI_ERROR_PARSE_FAILED;
    }

    def_key_count = json_object_get_count(evt_def);
    for (k = 0; k < def_key_count; k++) {
      const char *kname = json_object_get_name(evt_def, k);
      if (strcmp(kname, "steps") != 0) {
        return UI_ERROR_PARSE_FAILED; /* additionalProperties: false */
      }
    }

    steps_arr = json_object_get_array(evt_def, "steps");
    if (!steps_arr) {
      return UI_ERROR_PARSE_FAILED;
    }

    step_count = json_array_get_count(steps_arr);
    for (step_idx = 0; step_idx < step_count; step_idx++) {
      const JSON_Object *st_obj = json_array_get_object(steps_arr, step_idx);
      size_t sk;
      size_t st_key_count;
      const char *action_str;

      if (!st_obj) {
        return UI_ERROR_PARSE_FAILED;
      }

      st_key_count = json_object_get_count(st_obj);
      for (sk = 0; sk < st_key_count; sk++) {
        const char *skname = json_object_get_name(st_obj, sk);
        if (strcmp(skname, "stepId") != 0 && strcmp(skname, "action") != 0 &&
            strcmp(skname, "target") != 0 && strcmp(skname, "value") != 0 &&
            strcmp(skname, "successCriteria") != 0) {
          return UI_ERROR_PARSE_FAILED; /* additionalProperties: false */
        }
      }

      action_str = json_object_get_string(st_obj, "action");
      if (!action_str) {
        return UI_ERROR_PARSE_FAILED;
      }
      if (strcmp(action_str, "mutate_state") != 0 &&
          strcmp(action_str, "navigate") != 0 &&
          strcmp(action_str, "http_request") != 0) {
        return UI_ERROR_PARSE_FAILED;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Validates a single JSON node object recursively.
 * @param node_obj The node object to validate.
 * @return UI_ERROR_NONE if valid, UI_ERROR_PARSE_FAILED if invalid.
 */
static ui_error_t validate_node_object(const JSON_Object *node_obj) {
  size_t i;
  size_t count;
  const char *type_str;
  const JSON_Object *props_obj;
  const JSON_Object *bindings_obj;
  const JSON_Array *val_arr;
  const JSON_Object *events_obj;
  const JSON_Array *children_arr;
  ui_error_t rc;

  if (!node_obj) {
    return UI_ERROR_PARSE_FAILED;
  }

  count = json_object_get_count(node_obj);
  for (i = 0; i < count; i++) {
    const char *kname = json_object_get_name(node_obj, i);
    if (strcmp(kname, "id") != 0 && strcmp(kname, "type") != 0 &&
        strcmp(kname, "props") != 0 && strcmp(kname, "bindings") != 0 &&
        strcmp(kname, "validators") != 0 && strcmp(kname, "events") != 0 &&
        strcmp(kname, "children") != 0) {
      return UI_ERROR_PARSE_FAILED; /* additionalProperties: false */
    }
  }

  type_str = json_object_get_string(node_obj, "type");
  if (!type_str) {
    return UI_ERROR_PARSE_FAILED; /* type is mandatory */
  }
  if (type_str[0] == '\0') {
    return UI_ERROR_PARSE_FAILED;
  }

  if (json_object_has_value(node_obj, "id")) {
    const JSON_Value *val = json_object_get_value(node_obj, "id");
    if (json_value_get_type(val) != JSONString) {
      return UI_ERROR_PARSE_FAILED;
    }
  }

  if (json_object_has_value(node_obj, "props")) {
    const JSON_Value *val = json_object_get_value(node_obj, "props");
    if (json_value_get_type(val) != JSONObject) {
      return UI_ERROR_PARSE_FAILED;
    }
  }

  if (json_object_has_value(node_obj, "bindings")) {
    const JSON_Value *val = json_object_get_value(node_obj, "bindings");
    if (json_value_get_type(val) != JSONObject) {
      return UI_ERROR_PARSE_FAILED;
    }
  }

  if (json_object_has_value(node_obj, "validators")) {
    const JSON_Value *val = json_object_get_value(node_obj, "validators");
    if (json_value_get_type(val) != JSONArray) {
      return UI_ERROR_PARSE_FAILED;
    }
  }

  if (json_object_has_value(node_obj, "events")) {
    const JSON_Value *val = json_object_get_value(node_obj, "events");
    if (json_value_get_type(val) != JSONObject) {
      return UI_ERROR_PARSE_FAILED;
    }
  }

  if (json_object_has_value(node_obj, "children")) {
    const JSON_Value *val = json_object_get_value(node_obj, "children");
    if (json_value_get_type(val) != JSONArray) {
      return UI_ERROR_PARSE_FAILED;
    }
  }

  /* Validate props */
  props_obj = json_object_get_object(node_obj, "props");
  if (props_obj) {
    size_t pk;
    size_t pcount = json_object_get_count(props_obj);
    for (pk = 0; pk < pcount; pk++) {
      const JSON_Value *pval = json_object_get_value_at(props_obj, pk);
      JSON_Value_Type pt = json_value_get_type(pval);
      if (pt != JSONString && pt != JSONNumber && pt != JSONBoolean) {
        return UI_ERROR_PARSE_FAILED;
      }
    }
  }

  /* Validate bindings */
  bindings_obj = json_object_get_object(node_obj, "bindings");
  if (bindings_obj) {
    size_t bk;
    size_t bcount = json_object_get_count(bindings_obj);
    for (bk = 0; bk < bcount; bk++) {
      const char *bname = json_object_get_name(bindings_obj, bk);
      const JSON_Value *bval = json_object_get_value_at(bindings_obj, bk);
      if (strcmp(bname, "bind_cva") != 0 && strcmp(bname, "bind_text") != 0 &&
          strcmp(bname, "bind_disabled") != 0 &&
          strcmp(bname, "bind_visibility") != 0 &&
          strcmp(bname, "bind_signal") != 0) {
        return UI_ERROR_PARSE_FAILED; /* Unknown binding */
      }
      if (json_value_get_type(bval) != JSONString) {
        return UI_ERROR_PARSE_FAILED;
      }
    }
  }

  /* Validate validators */
  val_arr = json_object_get_array(node_obj, "validators");
  if (val_arr) {
    rc = validate_validators_array(val_arr);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  /* Validate events */
  events_obj = json_object_get_object(node_obj, "events");
  if (events_obj) {
    rc = validate_events_object(events_obj);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  /* Validate children recursively */
  children_arr = json_object_get_array(node_obj, "children");
  if (children_arr) {
    size_t c;
    size_t child_count = json_array_get_count(children_arr);
    for (c = 0; c < child_count; c++) {
      const JSON_Object *c_obj = json_array_get_object(children_arr, c);
      rc = validate_node_object(c_obj);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Validates a JSON schema string against strict schema rules.
 *
 * @param json_str The JSON string to validate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_PARSE_FAILED /
 * UI_ERROR_INVALID_ARGUMENT on validation error.
 */
ui_error_t ui_runtime_schema_validate(const char *json_str) {
  JSON_Value *root_val;
  const JSON_Object *root_obj;
  size_t i;
  size_t count;
  int is_single_node = 0;
  const JSON_Array *routes_arr;
  size_t r;
  size_t route_count;
  const JSON_Object *global_state_obj;
  ui_error_t rc;

  if (!json_str) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  root_val = json_parse_string(json_str);
  if (!root_val) {
    return UI_ERROR_PARSE_FAILED;
  }

  root_obj = json_value_get_object(root_val);
  if (!root_obj) {
    json_value_free(root_val);
    return UI_ERROR_PARSE_FAILED;
  }

  /* Check if this is a single node snippet or an app manifest */
  if (json_object_has_value(root_obj, "type") &&
      !json_object_has_value(root_obj, "routes")) {
    is_single_node = 1;
  }

  if (is_single_node) {
    rc = validate_node_object(root_obj);
    if (rc != UI_ERROR_NONE) {
      json_value_free(root_val);
      return rc;
    }
    json_value_free(root_val);
    return UI_ERROR_NONE;
  }

  /* App manifest validation */
  count = json_object_get_count(root_obj);
  for (i = 0; i < count; i++) {
    const char *kname = json_object_get_name(root_obj, i);
    if (strcmp(kname, "app_id") != 0 && strcmp(kname, "initial_route") != 0 &&
        strcmp(kname, "global_state") != 0 && strcmp(kname, "routes") != 0) {
      json_value_free(root_val);
      return UI_ERROR_PARSE_FAILED; /* additionalProperties: false */
    }
  }

  if (json_object_has_value(root_obj, "app_id")) {
    const JSON_Value *val = json_object_get_value(root_obj, "app_id");
    if (json_value_get_type(val) != JSONString) {
      json_value_free(root_val);
      return UI_ERROR_PARSE_FAILED;
    }
  }

  if (json_object_has_value(root_obj, "initial_route")) {
    const JSON_Value *val = json_object_get_value(root_obj, "initial_route");
    if (json_value_get_type(val) != JSONString) {
      json_value_free(root_val);
      return UI_ERROR_PARSE_FAILED;
    }
  }

  if (json_object_has_value(root_obj, "global_state")) {
    const JSON_Value *val = json_object_get_value(root_obj, "global_state");
    if (json_value_get_type(val) != JSONObject) {
      json_value_free(root_val);
      return UI_ERROR_PARSE_FAILED;
    }
  }

  if (!json_object_has_value(root_obj, "routes")) {
    json_value_free(root_val);
    return UI_ERROR_PARSE_FAILED;
  }
  if (json_value_get_type(json_object_get_value(root_obj, "routes")) !=
      JSONArray) {
    json_value_free(root_val);
    return UI_ERROR_PARSE_FAILED;
  }

  routes_arr = json_object_get_array(root_obj, "routes");

  global_state_obj = json_object_get_object(root_obj, "global_state");
  if (global_state_obj) {
    size_t gk;
    size_t gcount = json_object_get_count(global_state_obj);
    for (gk = 0; gk < gcount; gk++) {
      const JSON_Value *gval = json_object_get_value_at(global_state_obj, gk);
      JSON_Value_Type gt = json_value_get_type(gval);
      if (gt != JSONString && gt != JSONNumber && gt != JSONBoolean) {
        json_value_free(root_val);
        return UI_ERROR_PARSE_FAILED;
      }
    }
  }

  route_count = json_array_get_count(routes_arr);
  for (r = 0; r < route_count; r++) {
    const JSON_Object *r_obj = json_array_get_object(routes_arr, r);
    size_t rk;
    size_t rk_count;
    const char *path_str;
    const JSON_Object *node_root;

    if (!r_obj) {
      json_value_free(root_val);
      return UI_ERROR_PARSE_FAILED;
    }

    rk_count = json_object_get_count(r_obj);
    for (rk = 0; rk < rk_count; rk++) {
      const char *rkname = json_object_get_name(r_obj, rk);
      if (strcmp(rkname, "path") != 0 && strcmp(rkname, "title") != 0 &&
          strcmp(rkname, "root") != 0) {
        json_value_free(root_val);
        return UI_ERROR_PARSE_FAILED; /* additionalProperties: false */
      }
    }

    path_str = json_object_get_string(r_obj, "path");
    if (!path_str) {
      json_value_free(root_val);
      return UI_ERROR_PARSE_FAILED;
    }

    node_root = json_object_get_object(r_obj, "root");
    if (!node_root) {
      json_value_free(root_val);
      return UI_ERROR_PARSE_FAILED;
    }
    rc = validate_node_object(node_root);
    if (rc != UI_ERROR_NONE) {
      json_value_free(root_val);
      return rc;
    }
  }

  json_value_free(root_val);
  return UI_ERROR_NONE;
}

/* Forward declaration for AST node parser */
static ui_error_t parse_ast_node(struct ui_arena *arena,
                                 const JSON_Object *node_obj,
                                 struct ui_runtime_node **out_node);

/**
 * @brief Parses an AST node and its subtrees from a parson JSON object.
 *
 * @param arena The memory arena.
 * @param node_obj The JSON object for the node.
 * @param out_node Pointer to receive the parsed AST node.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t parse_ast_node(struct ui_arena *arena,
                                 const JSON_Object *node_obj,
                                 struct ui_runtime_node **out_node) {
  struct ui_runtime_node *node;
  ui_error_t rc;
  const char *id_str;
  const char *type_str;
  const JSON_Object *props_obj;
  const JSON_Object *bindings_obj;
  const JSON_Array *val_arr;
  const JSON_Object *events_obj;
  const JSON_Array *children_arr;

  rc = ui_arena_alloc(arena, sizeof(struct ui_runtime_node), 8, (void **)&node);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  memset(node, 0, sizeof(struct ui_runtime_node));

  id_str = json_object_get_string(node_obj, "id");
  if (id_str) {
    rc = arena_strdup(arena, id_str, &node->id);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  type_str = json_object_get_string(node_obj, "type");
  rc = arena_strdup(arena, type_str, &node->type);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* Props */
  props_obj = json_object_get_object(node_obj, "props");
  if (props_obj) {
    size_t i;
    size_t pcount = json_object_get_count(props_obj);
    struct ui_runtime_prop *last_prop = NULL;

    for (i = 0; i < pcount; i++) {
      const char *kname = json_object_get_name(props_obj, i);
      const JSON_Value *pval = json_object_get_value_at(props_obj, i);
      char val_buf[128];
      struct ui_runtime_prop *prop;

      if (json_value_get_type(pval) == JSONString) {
        rc = ui_safe_string_copy(val_buf, sizeof(val_buf),
                                 json_value_get_string(pval));
      } else if (json_value_get_type(pval) == JSONNumber) {
        rc = ui_safe_string_format(val_buf, sizeof(val_buf), "%g",
                                   json_value_get_number(pval));
      } else {
        rc = ui_safe_string_copy(val_buf, sizeof(val_buf),
                                 json_value_get_boolean(pval) ? "true"
                                                              : "false");
      }
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      rc = ui_arena_alloc(arena, sizeof(struct ui_runtime_prop), 8,
                          (void **)&prop);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      prop->next = NULL;
      rc = arena_strdup(arena, kname, &prop->key);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = arena_strdup(arena, val_buf, &prop->val);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      if (!last_prop) {
        node->props = prop;
      } else {
        last_prop->next = prop;
      }
      last_prop = prop;
    }
  }

  /* Bindings */
  bindings_obj = json_object_get_object(node_obj, "bindings");
  if (bindings_obj) {
    size_t i;
    size_t bcount = json_object_get_count(bindings_obj);
    struct ui_runtime_binding *last_b = NULL;

    for (i = 0; i < bcount; i++) {
      const char *bname = json_object_get_name(bindings_obj, i);
      const char *bval = json_object_get_string(bindings_obj, bname);
      struct ui_runtime_binding *b;

      rc = ui_arena_alloc(arena, sizeof(struct ui_runtime_binding), 8,
                          (void **)&b);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      b->next = NULL;
      if (strcmp(bname, "bind_cva") == 0) {
        b->type = UI_RUNTIME_BINDING_CVA;
      } else if (strcmp(bname, "bind_text") == 0) {
        b->type = UI_RUNTIME_BINDING_TEXT;
      } else if (strcmp(bname, "bind_disabled") == 0) {
        b->type = UI_RUNTIME_BINDING_DISABLED;
      } else if (strcmp(bname, "bind_visibility") == 0) {
        b->type = UI_RUNTIME_BINDING_VISIBILITY;
      } else {
        b->type = UI_RUNTIME_BINDING_SIGNAL;
      }

      rc = arena_strdup(arena, bname, &b->target_prop);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      rc = arena_strdup(arena, bval, &b->source_path);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      if (!last_b) {
        node->bindings = b;
      } else {
        last_b->next = b;
      }
      last_b = b;
    }
  }

  /* Validators */
  val_arr = json_object_get_array(node_obj, "validators");
  if (val_arr) {
    size_t i;
    size_t vcount = json_array_get_count(val_arr);
    struct ui_runtime_validator_def *last_val = NULL;

    for (i = 0; i < vcount; i++) {
      const JSON_Object *v_obj = json_array_get_object(val_arr, i);
      const char *vtype_str = json_object_get_string(v_obj, "type");
      struct ui_runtime_validator_def *v;

      rc = ui_arena_alloc(arena, sizeof(struct ui_runtime_validator_def), 8,
                          (void **)&v);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      v->next = NULL;
      v->param = NULL;
      v->int_param = 0;

      if (strcmp(vtype_str, "required") == 0) {
        v->type = UI_RUNTIME_VALIDATOR_REQUIRED;
      } else if (strcmp(vtype_str, "pattern") == 0) {
        const char *pat = json_object_get_string(v_obj, "pattern");
        v->type = UI_RUNTIME_VALIDATOR_PATTERN;
        if (pat) {
          rc = arena_strdup(arena, pat, &v->param);
          if (rc != UI_ERROR_NONE) {
            return rc;
          }
        }
      } else if (strcmp(vtype_str, "min_length") == 0) {
        v->type = UI_RUNTIME_VALIDATOR_MIN_LENGTH;
        if (json_object_has_value(v_obj, "min_length")) {
          v->int_param = (int)json_object_get_number(v_obj, "min_length");
        } else {
          v->int_param = (int)json_object_get_number(v_obj, "value");
        }
      } else {
        v->type = UI_RUNTIME_VALIDATOR_MAX_LENGTH;
        if (json_object_has_value(v_obj, "max_length")) {
          v->int_param = (int)json_object_get_number(v_obj, "max_length");
        } else {
          v->int_param = (int)json_object_get_number(v_obj, "value");
        }
      }

      if (!last_val) {
        node->validators = v;
      } else {
        last_val->next = v;
      }
      last_val = v;
    }
  }

  /* Events & Workflows */
  events_obj = json_object_get_object(node_obj, "events");
  if (events_obj) {
    size_t i;
    size_t ecount = json_object_get_count(events_obj);
    struct ui_runtime_workflow *last_wf = NULL;

    for (i = 0; i < ecount; i++) {
      const char *ename = json_object_get_name(events_obj, i);
      const JSON_Object *evt_def = json_object_get_object(events_obj, ename);
      const JSON_Array *steps_arr;
      struct ui_runtime_workflow *wf;

      rc = ui_arena_alloc(arena, sizeof(struct ui_runtime_workflow), 8,
                          (void **)&wf);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      wf->next = NULL;
      wf->first_step = NULL;

      rc = arena_strdup(arena, ename, &wf->event_name);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      steps_arr = json_object_get_array(evt_def, "steps");
      {
        size_t s;
        size_t scount = json_array_get_count(steps_arr);
        struct ui_runtime_workflow_step *last_step = NULL;

        for (s = 0; s < scount; s++) {
          const JSON_Object *st_obj = json_array_get_object(steps_arr, s);
          const char *step_id = json_object_get_string(st_obj, "stepId");
          const char *act_str = json_object_get_string(st_obj, "action");
          const char *target = json_object_get_string(st_obj, "target");
          const char *val = json_object_get_string(st_obj, "value");
          const char *criteria =
              json_object_get_string(st_obj, "successCriteria");
          struct ui_runtime_workflow_step *st;

          rc = ui_arena_alloc(arena, sizeof(struct ui_runtime_workflow_step), 8,
                              (void **)&st);
          if (rc != UI_ERROR_NONE) {
            return rc;
          }
          st->next = NULL;

          if (step_id) {
            rc = arena_strdup(arena, step_id, &st->step_id);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
          } else {
            st->step_id = NULL;
          }

          if (strcmp(act_str, "mutate_state") == 0) {
            st->action = UI_RUNTIME_ACTION_MUTATE_STATE;
          } else if (strcmp(act_str, "navigate") == 0) {
            st->action = UI_RUNTIME_ACTION_NAVIGATE;
          } else {
            st->action = UI_RUNTIME_ACTION_HTTP;
          }

          if (target) {
            rc = arena_strdup(arena, target, &st->target);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
          } else {
            st->target = NULL;
          }

          if (val) {
            rc = arena_strdup(arena, val, &st->value);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
          } else {
            st->value = NULL;
          }

          if (criteria) {
            rc = arena_strdup(arena, criteria, &st->success_criteria);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
          } else {
            st->success_criteria = NULL;
          }

          if (!last_step) {
            wf->first_step = st;
          } else {
            last_step->next = st;
          }
          last_step = st;
        }
      }

      if (!last_wf) {
        node->workflows = wf;
      } else {
        last_wf->next = wf;
      }
      last_wf = wf;
    }
  }

  /* Children */
  children_arr = json_object_get_array(node_obj, "children");
  if (children_arr) {
    size_t c;
    size_t ccount = json_array_get_count(children_arr);
    struct ui_runtime_node *last_child = NULL;

    for (c = 0; c < ccount; c++) {
      const JSON_Object *c_obj = json_array_get_object(children_arr, c);
      struct ui_runtime_node *child_node = NULL;

      rc = parse_ast_node(arena, c_obj, &child_node);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      if (!last_child) {
        node->first_child = child_node;
      } else {
        last_child->next_sibling = child_node;
      }
      last_child = child_node;
    }
  }

  *out_node = node;
  return UI_ERROR_NONE;
}

/**
 * @brief Parses a validated JSON schema payload into an AST allocated on the
 * provided arena.
 *
 * @param arena The transient or persistent arena to allocate AST nodes on.
 * @param json_str The JSON string to parse.
 * @param out_manifest Pointer to receive the parsed application manifest.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_runtime_schema_parse(struct ui_arena *arena, const char *json_str,
                        struct ui_runtime_app_manifest **out_manifest) {
  ui_error_t rc;
  JSON_Value *root_val;
  const JSON_Object *root_obj;
  struct ui_runtime_app_manifest *manifest;
  const char *app_id;
  const char *init_route;
  const JSON_Object *global_state_obj;
  const JSON_Array *routes_arr;

  if (!arena || !json_str || !out_manifest) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_runtime_schema_validate(json_str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  root_val = json_parse_string(json_str);
  if (!root_val) {
    return UI_ERROR_PARSE_FAILED;
  }

  root_obj = json_value_get_object(root_val);
  rc = ui_arena_alloc(arena, sizeof(struct ui_runtime_app_manifest), 8,
                      (void **)&manifest);
  if (rc != UI_ERROR_NONE) {
    json_value_free(root_val);
    return rc;
  }

  memset(manifest, 0, sizeof(struct ui_runtime_app_manifest));

  app_id = json_object_get_string(root_obj, "app_id");
  if (app_id) {
    rc = arena_strdup(arena, app_id, &manifest->app_id);
    if (rc != UI_ERROR_NONE) {
      json_value_free(root_val);
      return rc;
    }
  }

  init_route = json_object_get_string(root_obj, "initial_route");
  if (init_route) {
    rc = arena_strdup(arena, init_route, &manifest->initial_route);
    if (rc != UI_ERROR_NONE) {
      json_value_free(root_val);
      return rc;
    }
  }

  /* Global state */
  global_state_obj = json_object_get_object(root_obj, "global_state");
  if (global_state_obj) {
    size_t i;
    size_t gcount = json_object_get_count(global_state_obj);
    struct ui_runtime_global_state_def *last_g = NULL;

    for (i = 0; i < gcount; i++) {
      const char *kname = json_object_get_name(global_state_obj, i);
      const JSON_Value *gval = json_object_get_value_at(global_state_obj, i);
      char val_buf[128];
      struct ui_runtime_global_state_def *gdef;

      if (json_value_get_type(gval) == JSONString) {
        rc = ui_safe_string_copy(val_buf, sizeof(val_buf),
                                 json_value_get_string(gval));
      } else if (json_value_get_type(gval) == JSONNumber) {
        rc = ui_safe_string_format(val_buf, sizeof(val_buf), "%g",
                                   json_value_get_number(gval));
      } else {
        rc = ui_safe_string_copy(val_buf, sizeof(val_buf),
                                 json_value_get_boolean(gval) ? "true"
                                                              : "false");
      }
      if (rc != UI_ERROR_NONE) {
        json_value_free(root_val);
        return rc;
      }

      rc = ui_arena_alloc(arena, sizeof(struct ui_runtime_global_state_def), 8,
                          (void **)&gdef);
      if (rc != UI_ERROR_NONE) {
        json_value_free(root_val);
        return rc;
      }

      gdef->next = NULL;
      rc = arena_strdup(arena, kname, &gdef->key);
      if (rc != UI_ERROR_NONE) {
        json_value_free(root_val);
        return rc;
      }
      rc = arena_strdup(arena, val_buf, &gdef->initial_val);
      if (rc != UI_ERROR_NONE) {
        json_value_free(root_val);
        return rc;
      }

      if (!last_g) {
        manifest->global_state = gdef;
      } else {
        last_g->next = gdef;
      }
      last_g = gdef;
    }
  }

  /* Routes */
  routes_arr = json_object_get_array(root_obj, "routes");
  {
    size_t r;
    size_t rcount = json_array_get_count(routes_arr);
    struct ui_runtime_route_def *last_route = NULL;

    for (r = 0; r < rcount; r++) {
      const JSON_Object *r_obj = json_array_get_object(routes_arr, r);
      const char *rpath = json_object_get_string(r_obj, "path");
      const char *rtitle = json_object_get_string(r_obj, "title");
      const JSON_Object *r_root = json_object_get_object(r_obj, "root");
      struct ui_runtime_route_def *rdef;

      rc = ui_arena_alloc(arena, sizeof(struct ui_runtime_route_def), 8,
                          (void **)&rdef);
      if (rc != UI_ERROR_NONE) {
        json_value_free(root_val);
        return rc;
      }

      rdef->next = NULL;
      rdef->root_node = NULL;
      rc = arena_strdup(arena, rpath, &rdef->path);
      if (rc != UI_ERROR_NONE) {
        json_value_free(root_val);
        return rc;
      }

      if (rtitle) {
        rc = arena_strdup(arena, rtitle, &rdef->title);
        if (rc != UI_ERROR_NONE) {
          json_value_free(root_val);
          return rc;
        }
      } else {
        rdef->title = NULL;
      }

      rc = parse_ast_node(arena, r_root, &rdef->root_node);
      if (rc != UI_ERROR_NONE) {
        json_value_free(root_val);
        return rc;
      }

      if (!last_route) {
        manifest->routes = rdef;
      } else {
        last_route->next = rdef;
      }
      last_route = rdef;
    }
  }

  json_value_free(root_val);
  *out_manifest = manifest;
  return UI_ERROR_NONE;
}

/**
 * @brief Parses a single widget node JSON snippet into an AST node allocated on
 * the arena.
 *
 * @param arena The arena to allocate on.
 * @param json_str The JSON node snippet.
 * @param out_node Pointer to receive the root AST node.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_runtime_schema_parse_node(struct ui_arena *arena,
                                        const char *json_str,
                                        struct ui_runtime_node **out_node) {
  ui_error_t rc;
  JSON_Value *root_val;
  const JSON_Object *root_obj;

  if (!arena || !json_str || !out_node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_runtime_schema_validate(json_str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  root_val = json_parse_string(json_str);
  if (!root_val) {
    return UI_ERROR_PARSE_FAILED;
  }

  root_obj = json_value_get_object(root_val);
  rc = parse_ast_node(arena, root_obj, out_node);
  json_value_free(root_val);
  return rc;
}
