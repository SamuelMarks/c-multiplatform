/**
 * @file ui_runtime_schema.h
 * @brief Formal schema definition, validator, and AST parser for
 * runtime-defined UI widgets.
 */

#ifndef UI_RUNTIME_SCHEMA_H
#define UI_RUNTIME_SCHEMA_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_arena.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

/**
 * @enum ui_runtime_binding_type
 * @brief Types of dynamic reactive bindings supported on widget nodes.
 */
enum ui_runtime_binding_type {
  UI_RUNTIME_BINDING_NONE = 0, /**< No binding */
  UI_RUNTIME_BINDING_CVA =
      1, /**< Control Value Accessor binding (e.g., user.email) */
  UI_RUNTIME_BINDING_TEXT = 2, /**< Dynamic text signal binding */
  UI_RUNTIME_BINDING_DISABLED =
      3, /**< Dynamic disabled boolean signal binding */
  UI_RUNTIME_BINDING_VISIBILITY =
      4,                        /**< Dynamic visibility/skip logic binding */
  UI_RUNTIME_BINDING_SIGNAL = 5 /**< Generic signal binding */
};

/**
 * @enum ui_runtime_validator_type
 * @brief Standard validation rule types for runtime form controls.
 */
enum ui_runtime_validator_type {
  UI_RUNTIME_VALIDATOR_NONE = 0,       /**< No validation */
  UI_RUNTIME_VALIDATOR_REQUIRED = 1,   /**< Field is required */
  UI_RUNTIME_VALIDATOR_PATTERN = 2,    /**< Regex pattern matching */
  UI_RUNTIME_VALIDATOR_MIN_LENGTH = 3, /**< Minimum string length */
  UI_RUNTIME_VALIDATOR_MAX_LENGTH = 4  /**< Maximum string length */
};

/**
 * @enum ui_runtime_workflow_action
 * @brief Action types for workflow execution steps aligned with Arazzo
 * semantics.
 */
enum ui_runtime_workflow_action {
  UI_RUNTIME_ACTION_NONE = 0, /**< No action */
  UI_RUNTIME_ACTION_MUTATE_STATE =
      1, /**< Mutate a state signal in global/local context */
  UI_RUNTIME_ACTION_NAVIGATE = 2, /**< Navigate to another route */
  UI_RUNTIME_ACTION_HTTP = 3 /**< Trigger an HTTP operation or workflow step */
};

/**
 * @struct ui_runtime_prop
 * @brief A key-value static property on a widget node.
 */
struct ui_runtime_prop {
  char *key;                    /**< Property name */
  char *val;                    /**< String-serialized value */
  struct ui_runtime_prop *next; /**< Next property in linked list */
};

/**
 * @struct ui_runtime_binding
 * @brief A reactive binding mapping a widget property to a state path.
 */
struct ui_runtime_binding {
  enum ui_runtime_binding_type type; /**< Type of binding */
  char *target_prop; /**< Target property on widget (e.g. text, disabled) */
  char *source_path; /**< Source identifier/path (e.g. app.cart_count,
                        user.email) */
  struct ui_runtime_binding *next; /**< Next binding in linked list */
};

/**
 * @struct ui_runtime_validator_def
 * @brief Definition of a validation rule attached to a form control widget.
 */
struct ui_runtime_validator_def {
  enum ui_runtime_validator_type type; /**< Validator type */
  char *param;   /**< String parameter (e.g. regex pattern) */
  int int_param; /**< Numeric parameter (e.g. min/max length) */
  struct ui_runtime_validator_def *next; /**< Next validator in linked list */
};

/**
 * @struct ui_runtime_workflow_step
 * @brief Single step in an action workflow sequence.
 */
struct ui_runtime_workflow_step {
  char *step_id; /**< Unique identifier for this step */
  enum ui_runtime_workflow_action action; /**< Action to perform */
  char *target; /**< Target of action (e.g. signal path or route path) */
  char *value;  /**< Value parameter for mutation or payload */
  char *success_criteria; /**< Optional Arazzo success criteria expression */
  struct ui_runtime_workflow_step *next; /**< Next step in sequence */
};

/**
 * @struct ui_runtime_workflow
 * @brief An action chain bound to a widget event hook.
 */
struct ui_runtime_workflow {
  char *event_name; /**< Event hook name (e.g. on_click, on_change) */
  struct ui_runtime_workflow_step *first_step; /**< Head of steps sequence */
  struct ui_runtime_workflow *next; /**< Next workflow in linked list */
};

/**
 * @struct ui_runtime_node
 * @brief Abstract Syntax Tree node representing a runtime-defined widget.
 */
struct ui_runtime_node {
  char *id;                            /**< Optional unique DOM ID */
  char *type;                          /**< Registered component type name */
  struct ui_runtime_prop *props;       /**< Static properties */
  struct ui_runtime_binding *bindings; /**< Reactive bindings */
  struct ui_runtime_validator_def *validators; /**< Validation rules */
  struct ui_runtime_workflow *workflows;       /**< Event workflows */
  struct ui_runtime_node *first_child;         /**< First child node */
  struct ui_runtime_node *next_sibling;        /**< Next sibling node */
};

/**
 * @struct ui_runtime_route_def
 * @brief Screen/page route definition in the application manifest.
 */
struct ui_runtime_route_def {
  char *path;  /**< URL/Route path (e.g. "/" or "/survey") */
  char *title; /**< Screen title */
  struct ui_runtime_node *root_node; /**< Root widget tree for this screen */
  struct ui_runtime_route_def *next; /**< Next route in linked list */
};

/**
 * @struct ui_runtime_global_state_def
 * @brief Initial global state key-value entry.
 */
struct ui_runtime_global_state_def {
  char *key;                                /**< Global state key */
  char *initial_val;                        /**< Initial value string */
  struct ui_runtime_global_state_def *next; /**< Next entry in linked list */
};

/**
 * @struct ui_runtime_app_manifest
 * @brief Top-level application manifest parsed from schema.
 */
struct ui_runtime_app_manifest {
  char *app_id;        /**< Application identifier */
  char *initial_route; /**< Initial route to load */
  struct ui_runtime_global_state_def *global_state; /**< Global state entries */
  struct ui_runtime_route_def *routes;              /**< Routes list */
};

/**
 * @brief Validates a JSON schema string against strict schema rules.
 * Enforces additionalProperties: false on all objects, ensures valid component
 * types, and rejects unknown fields or malformed structures.
 *
 * @param json_str The JSON string to validate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_PARSE_FAILED /
 * UI_ERROR_INVALID_ARGUMENT on validation error.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_runtime_schema_validate(const char *json_str);

/**
 * @brief Parses a validated JSON schema payload into an AST allocated on the
 * provided arena.
 *
 * @param arena The transient or persistent arena to allocate AST nodes on.
 * @param json_str The JSON string to parse.
 * @param out_manifest Pointer to receive the parsed application manifest.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_runtime_schema_parse(struct ui_arena *arena, const char *json_str,
                        struct ui_runtime_app_manifest **out_manifest);

/**
 * @brief Parses a single widget node JSON snippet into an AST node allocated on
 * the arena.
 *
 * @param arena The arena to allocate on.
 * @param json_str The JSON node snippet.
 * @param out_node Pointer to receive the root AST node.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_runtime_schema_parse_node(struct ui_arena *arena, const char *json_str,
                             struct ui_runtime_node **out_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RUNTIME_SCHEMA_H */
