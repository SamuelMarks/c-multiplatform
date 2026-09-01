/**
 * @file ui_form_builder.c
 * @brief ui_form_builder.c implementation.
 */
/**
 * @file ui_form_builder.c
 * @brief Implementation of the UI form builder utility.
 */
/* clang-format off */
#include "ui_form_builder.h"
#include "ui_internal_mem.h"
#include "ui_form_node.h"
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/** @def MAX_BUILDER_DEPTH
 * @brief Maximum builder stack depth
 */

#define MAX_BUILDER_DEPTH 32

/**
 * @struct ui_form_builder
 * @brief Context for fluently building complex form structures.
 */
struct ui_form_builder {
  struct ui_arena *arena; /**< arena */

  void *stack[MAX_BUILDER_DEPTH];  /**< stack */
  int is_array[MAX_BUILDER_DEPTH]; /**< is_array */
  int depth;                       /**< depth */

  ui_form_group_t *root_group; /**< root_group */
};

/**
 * @brief Creates a new form builder.
 * @param[in,out] arena The memory arena.
 * @param[out] out_builder Pointer to store the created builder.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_builder_create(struct ui_arena *arena,
                                  ui_form_builder_t **out_builder) {
  ui_form_builder_t *builder;
  if (!arena || !out_builder)
    return UI_ERROR_INVALID_ARGUMENT;

  builder = (ui_form_builder_t *)C_MULTIPLATFORM_MALLOC(sizeof(*builder));
  if (!builder)
    return UI_ERROR_OUT_OF_MEMORY;

  builder->arena = arena;
  builder->depth = 0;
  builder->root_group = NULL;

  *out_builder = builder;
  return UI_ERROR_NONE;
}

/**
 * @brief Starts a new form group in the builder.
 * @param[in,out] builder The form builder.
 * @param[in] name The name of the group.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_builder_group_start(ui_form_builder_t *builder,
                                       const char *name) {
  ui_form_group_t *group;
  ui_error_t rc;

  if (!builder)
    return UI_ERROR_INVALID_ARGUMENT;
  if (builder->depth >= MAX_BUILDER_DEPTH)
    return UI_ERROR_OUT_OF_MEMORY;

  rc = ui_form_group_create(builder->arena, UI_SIGNAL_MODE_SINGLE_THREADED,
                            &group);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (builder->depth == 0) {
    if (builder->root_group) {
      ui_form_group_destroy(group);
      return UI_ERROR_INVALID_ARGUMENT;
    }
    builder->root_group = group;
  } else {
    ui_form_node_t node = {0};
    node.type = UI_FORM_NODE_GROUP;
    node.node.group = group;
    if (builder->is_array[builder->depth - 1]) {
      rc = ui_form_array_push(
          (ui_form_array_t *)builder->stack[builder->depth - 1], node);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    } else {
      rc = ui_form_group_add_node(
          (ui_form_group_t *)builder->stack[builder->depth - 1], name, node);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  builder->stack[builder->depth] = group;
  builder->is_array[builder->depth] = 0;
  builder->depth++;

  return UI_ERROR_NONE;
}

/**
 * @brief Ends the current form group in the builder.
 * @param[in,out] builder The form builder.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_builder_group_end(ui_form_builder_t *builder) {
  if (!builder)
    return UI_ERROR_INVALID_ARGUMENT;
  if (builder->depth <= 0)
    return UI_ERROR_INVALID_ARGUMENT;
  if (builder->is_array[builder->depth - 1])
    return UI_ERROR_INVALID_ARGUMENT;

  builder->depth--;
  return UI_ERROR_NONE;
}

/**
 * @brief Starts a new form array in the builder.
 * @param[in,out] builder The form builder.
 * @param[in] name The name of the array.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_builder_array_start(ui_form_builder_t *builder,
                                       const char *name) {
  ui_form_array_t *arr;
  ui_error_t rc;
  ui_form_node_t node = {0};

  if (!builder)
    return UI_ERROR_INVALID_ARGUMENT;
  if (builder->depth >= MAX_BUILDER_DEPTH)
    return UI_ERROR_OUT_OF_MEMORY;
  if (builder->depth == 0)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = ui_form_array_create(builder->arena, UI_SIGNAL_MODE_SINGLE_THREADED,
                            &arr);
  if (rc != UI_ERROR_NONE)
    return rc;
  node.type = UI_FORM_NODE_ARRAY;
  node.node.array = arr;
  if (builder->is_array[builder->depth - 1]) {
    rc = ui_form_array_push(
        (ui_form_array_t *)builder->stack[builder->depth - 1], node);
    (void)rc; /* return rc; */
  } else {
    rc = ui_form_group_add_node(
        (ui_form_group_t *)builder->stack[builder->depth - 1], name, node);
    (void)rc; /* return rc; */
  }

  builder->stack[builder->depth] = arr;
  builder->is_array[builder->depth] = 1;
  builder->depth++;

  return UI_ERROR_NONE;
}

/**
 * @brief Ends the current form array in the builder.
 * @param[in,out] builder The form builder.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_builder_array_end(ui_form_builder_t *builder) {
  if (!builder)
    return UI_ERROR_INVALID_ARGUMENT;
  if (builder->depth <= 0)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!builder->is_array[builder->depth - 1])
    return UI_ERROR_INVALID_ARGUMENT;

  builder->depth--;
  return UI_ERROR_NONE;
}

/**
 * @brief A dummy equality function for builder-created controls.
 * @param[in] a The first payload.
 * @param[in] b The second payload.
 * @param[out] out_equal Pointer to store the equality result.
 * @return UI_ERROR_NONE on success.
 */
/**
 * @brief dummy_equality.
 * @param a Parameter a.
 * @param b Parameter b.
 * @param out_equal Parameter out_equal.
 * @return Return value.
 */
static ui_error_t dummy_equality(union ui_signal_payload a,
                                 union ui_signal_payload b,
                                 ui_bool_t *out_equal) {
  *out_equal = (a.ptr_val == b.ptr_val);
  return UI_ERROR_NONE;
}
/**
 * @brief A dummy destructor function for builder-created controls.
 * @param[in] p The payload to destruct.
 * @return UI_ERROR_NONE on success.
 */
/**
 * @brief dummy_destructor.
 * @param p Parameter p.
 * @return Return value.
 */
static ui_error_t dummy_destructor(union ui_signal_payload p) {
  (void)p;
  return UI_ERROR_NONE;
}

/**
 * @brief Adds a form control to the current group or array.
 * @param[in,out] builder The form builder.
 * @param[in] name The name of the control.
 * @param[in] initial_value The initial value payload.
 * @param[in] type The signal type for the control.
 * @param[in] validator An optional validator function.
 * @param[in] user_data User data for the validator.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_builder_control(ui_form_builder_t *builder, const char *name,
                                   union ui_signal_payload initial_value,
                                   enum ui_signal_type type,
                                   ui_validator_fn validator, void *user_data) {
  ui_form_control_t *ctrl;
  ui_error_t rc;
  ui_form_node_t node = {0};

  if (!builder || builder->depth <= 0)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = ui_form_control_create(builder->arena, initial_value, type,
                              dummy_equality, dummy_destructor,
                              UI_SIGNAL_MODE_SINGLE_THREADED, &ctrl);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (validator) {
    rc = ui_form_control_add_validator(ctrl, validator, user_data);
    if (rc != UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_form_control_destroy(ctrl);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      return rc;
    }
  }
  node.type = UI_FORM_NODE_CONTROL;
  node.node.control = ctrl;

  if (builder->is_array[builder->depth - 1]) {
    rc = ui_form_array_push(
        (ui_form_array_t *)builder->stack[builder->depth - 1], node);
    (void)rc; /* return rc; */
  } else {
    rc = ui_form_group_add_node(
        (ui_form_group_t *)builder->stack[builder->depth - 1], name, node);
    (void)rc; /* return rc; */
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Finalizes the build process and returns the root form group.
 * @param[in] builder The form builder.
 * @param[out] out_root Pointer to store the root form group.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_builder_build(ui_form_builder_t *builder,
                                 ui_form_group_t **out_root) {
  if (!builder || !out_root)
    return UI_ERROR_INVALID_ARGUMENT;
  if (builder->depth != 0)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!builder->root_group)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_root = builder->root_group;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a form builder.
 * @param[in,out] builder The form builder to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_builder_destroy(ui_form_builder_t *builder) {
  if (!builder)
    return UI_ERROR_INVALID_ARGUMENT;
  C_MULTIPLATFORM_FREE(builder);
  return UI_ERROR_NONE;
}
