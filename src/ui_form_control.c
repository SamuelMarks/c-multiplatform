/**
 * \file ui_form_control.c
 * \brief Implementation of individual form controls.
 */
/* clang-format off */
#include "ui_form_control.h"
#include "ui_internal_mem.h"
#include <string.h>
#include "ui_form_validators.h"
#include <stdlib.h>
/* clang-format on */

/**
 * \struct ui_form_control_async_task
 * \brief Context for an asynchronous validation task.
 */
struct ui_form_control_async_task {
  ui_form_control_t *control;
  ui_async_validator_fn validator;
  void *user_data;
  ui_int32 generation;
  struct ui_reactor *reactor;
  union ui_signal_payload value;
  ui_bool_t is_valid;
};

/**
 * \struct ui_form_control
 * \brief Represents an individual UI form control and its state.
 */
struct ui_form_control {
  struct ui_arena *arena;
  ui_signal_t *value_signal;
  ui_signal_t *status_signal;
  ui_signal_t *touched_signal;
  ui_signal_t *dirty_signal;
  ui_signal_t *errors_signal;
  char *error_str;

  ui_validator_t *sync_validators;
  size_t sync_validators_count;
  size_t sync_validators_capacity;

  ui_async_validator_t *async_validators;
  size_t async_validators_count;
  size_t async_validators_capacity;

  struct ui_thread_pool *thread_pool;
  struct ui_reactor *reactor;

  ui_int32 validation_generation;
  size_t pending_async_count;
};

/* Forward declarations */
/**
 * \brief Runs all validators (sync and async) for a form control.
 * \param[in,out] control The form control.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t ui_form_control_run_validation(ui_form_control_t *control);

/**
 * \brief Creates a new form control.
 * \param[in,out] arena The memory arena.
 * \param[in] initial_value The initial value payload.
 * \param[in] type The signal type for the control.
 * \param[in] equality_fn Function to test payload equality.
 * \param[in] destructor_fn Function to destruct payloads.
 * \param[in] mode The signaling mode.
 * \param[out] out_control Pointer to store the created control.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_control_create(struct ui_arena *arena,
                                  union ui_signal_payload initial_value,
                                  enum ui_signal_type type,
                                  ui_equality_fn equality_fn,
                                  ui_destructor_fn destructor_fn,
                                  enum ui_signal_mode mode,
                                  ui_form_control_t **out_control) {
  struct ui_form_control *ctrl;
  ui_error_t rc;
  union ui_signal_payload status_payload = {0};
  union ui_signal_payload bool_payload = {0};

  if (!arena || !out_control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_arena_alloc(arena, sizeof(struct ui_form_control), 8, (void **)&ctrl);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  ctrl->arena = arena;
  ctrl->sync_validators = NULL;
  ctrl->sync_validators_count = 0;
  ctrl->sync_validators_capacity = 0;
  ctrl->async_validators = NULL;
  ctrl->async_validators_count = 0;
  ctrl->async_validators_capacity = 0;
  ctrl->thread_pool = NULL;
  ctrl->reactor = NULL;
  ctrl->validation_generation = 0;
  ctrl->pending_async_count = 0;

  rc = ui_signal_create(arena, initial_value, type, equality_fn, destructor_fn,
                        mode, &ctrl->value_signal);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  status_payload.int_val = (ui_int32)UI_FORM_STATUS_VALID;
  rc = ui_signal_create(arena, status_payload, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        mode, &ctrl->status_signal);
  if (rc != UI_ERROR_NONE) {
    (void)ui_signal_destroy(ctrl->value_signal);
    return rc;
  }

  bool_payload.bool_val = UI_FALSE;
  rc = ui_signal_create(arena, bool_payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                        mode, &ctrl->touched_signal);
  if (rc != UI_ERROR_NONE) {
    (void)ui_signal_destroy(ctrl->status_signal);
    (void)ui_signal_destroy(ctrl->value_signal);
    return rc;
  }

  rc = ui_signal_create(arena, bool_payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                        mode, &ctrl->dirty_signal);
  if (rc != UI_ERROR_NONE) {
    (void)ui_signal_destroy(ctrl->touched_signal);
    (void)ui_signal_destroy(ctrl->status_signal);
    (void)ui_signal_destroy(ctrl->value_signal);
    return rc;
  }

  rc = ui_signal_create(arena, bool_payload, UI_SIGNAL_TYPE_POINTER, NULL, NULL,
                        mode, &ctrl->errors_signal);
  if (rc != UI_ERROR_NONE) {
    (void)ui_signal_destroy(ctrl->dirty_signal);
    (void)ui_signal_destroy(ctrl->touched_signal);
    (void)ui_signal_destroy(ctrl->status_signal);
    (void)ui_signal_destroy(ctrl->value_signal);
    return rc;
  }
  ctrl->error_str = NULL;

  *out_control = ctrl;

  /* Initial validation */
  (void)ui_form_control_run_validation(ctrl);

  return UI_ERROR_NONE;
}

/**
 * \brief Adds a synchronous validator to a form control.
 * \param[in,out] control The form control.
 * \param[in] validator The validator function.
 * \param[in] user_data User data for the validator.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_control_add_validator(ui_form_control_t *control,
                                         ui_validator_fn validator,
                                         void *user_data) {
  ui_validator_t *new_validators = NULL;
  size_t new_cap;

  if (!control || !validator)
    return UI_ERROR_INVALID_ARGUMENT;

  if (control->sync_validators_count >= control->sync_validators_capacity) {
    ui_error_t alloc_rc;
    new_cap = control->sync_validators_capacity == 0
                  ? 4
                  : control->sync_validators_capacity * 2;
    /* Use arena or malloc? Arena is bounded per-form but no realloc.
       For long-lived, we can just alloc from arena. */
    alloc_rc = ui_arena_alloc(control->arena, new_cap * sizeof(ui_validator_t),
                              8, (void **)&new_validators);
    if (alloc_rc != UI_ERROR_NONE)
      return UI_ERROR_OUT_OF_MEMORY;

    if (control->sync_validators_count > 0) {
      size_t i;
      for (i = 0; i < control->sync_validators_count; i++) {
        new_validators[i] = control->sync_validators[i];
      }
    }
    control->sync_validators = new_validators;
    control->sync_validators_capacity = new_cap;
  }

  control->sync_validators[control->sync_validators_count].fn = validator;
  control->sync_validators[control->sync_validators_count].user_data =
      user_data;
  control->sync_validators_count++;

  /* Re-run validation */
  { (void)ui_form_control_run_validation(control); }
  return UI_ERROR_NONE;
}

/**
 * \brief Adds an asynchronous validator to a form control.
 * \param[in,out] control The form control.
 * \param[in] validator The async validator function.
 * \param[in] user_data User data for the validator.
 * \param[in,out] thread_pool The thread pool for async execution.
 * \param[in,out] reactor The reactor to handle the result.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_control_add_async_validator(
    ui_form_control_t *control, ui_async_validator_fn validator,
    void *user_data, struct ui_thread_pool *thread_pool,
    struct ui_reactor *reactor) {
  ui_async_validator_t *new_validators = NULL;
  size_t new_cap;

  if (!control || !validator || !thread_pool)
    return UI_ERROR_INVALID_ARGUMENT;

  /* Save thread pool and reactor to control so we can dispatch */
  control->thread_pool = thread_pool;
  control->reactor = reactor;

  if (control->async_validators_count >= control->async_validators_capacity) {
    ui_error_t alloc_rc;
    new_cap = control->async_validators_capacity == 0
                  ? 4
                  : control->async_validators_capacity * 2;
    alloc_rc =
        ui_arena_alloc(control->arena, new_cap * sizeof(ui_async_validator_t),
                       8, (void **)&new_validators);
    if (alloc_rc != UI_ERROR_NONE)
      return UI_ERROR_OUT_OF_MEMORY;

    if (control->async_validators_count > 0) {
      size_t i;
      for (i = 0; i < control->async_validators_count; i++) {
        new_validators[i] = control->async_validators[i];
      }
    }
    control->async_validators = new_validators;
    control->async_validators_capacity = new_cap;
  }

  control->async_validators[control->async_validators_count].fn = validator;
  control->async_validators[control->async_validators_count].user_data =
      user_data;
  control->async_validators_count++;

  /* Re-run validation */
  { (void)ui_form_control_run_validation(control); }
  return UI_ERROR_NONE;
}

/**
 * \brief Callback triggered when an async validation task completes.
 * \param[in,out] user_data Pointer to the task context.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t ui_form_control_async_result_cb(void *user_data) {
  struct ui_form_control_async_task *task =
      (struct ui_form_control_async_task *)user_data;
  ui_form_control_t *control = task->control;
  union ui_signal_payload status_payload = {0};
  ui_error_t rc = UI_ERROR_NONE;

  if (control->validation_generation == task->generation) {
    control->pending_async_count--;

    if (!task->is_valid) {
      /* Failed async validation */
      status_payload.int_val = (ui_int32)UI_FORM_STATUS_INVALID;
      rc = ui_signal_set(control->status_signal, status_payload);
      (void)rc;
      control->pending_async_count = 0; /* Stop pending */
    } else {
      /* All passed */
      status_payload.int_val = (ui_int32)UI_FORM_STATUS_VALID;
      {
        ui_error_t set_rc =
            ui_signal_set(control->status_signal, status_payload);
        (void)set_rc;
      }
    }
  }

  C_MULTIPLATFORM_FREE(task);
  return rc;
}

static ui_error_t ui_form_control_async_worker(void *user_data) {
  struct ui_form_control_async_task *task =
      (struct ui_form_control_async_task *)user_data;
  ui_bool_t is_valid = UI_FALSE;
  ui_error_t rc;

  rc = task->validator(task->control, task->value, task->user_data, &is_valid);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
    is_valid = UI_FALSE;
  }

  task->is_valid = is_valid;

  /* Schedule back to main reactor */
  if (task->reactor) {
    ui_error_t sched_rc = ui_reactor_schedule(
        task->reactor, ui_form_control_async_result_cb, task);
    (void)sched_rc;
    return rc;
  } else {
    /* If single threaded / no reactor, execute inline */
    ui_error_t cb_rc = ui_form_control_async_result_cb(task);
    (void)cb_rc;
    return rc;
  }
}

/**
 * \brief Runs all validators (sync and async) for a form control.
 * \param[in,out] control The form control.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t ui_form_control_run_validation(ui_form_control_t *control) {
  size_t i;
  ui_bool_t is_valid = UI_TRUE;
  union ui_signal_payload current_value;
  union ui_signal_payload status_payload = {0};
  ui_error_t rc;

  /* Check if disabled */
  {
    ui_error_t sig_rc = ui_signal_get(control->status_signal, &status_payload);
    if (sig_rc != UI_ERROR_NONE) {
      if (0)
        return sig_rc;
    }
  }
  if (status_payload.int_val == UI_FORM_STATUS_DISABLED) {
    return UI_ERROR_NONE;
  }

  control->validation_generation++;
  control->pending_async_count = 0;

  {
    rc = ui_signal_get(control->value_signal, &current_value);
    if (rc != UI_ERROR_NONE) {
      return rc; /* Should not be hit in normal control flow since signals are
                 initialized */
    }
  }

  /* Run sync validators */
  for (i = 0; i < control->sync_validators_count; i++) {
    int failed = 0;
    rc = control->sync_validators[i].fn(control, current_value,
                                        control->sync_validators[i].user_data,
                                        &is_valid);
    if (rc != UI_ERROR_NONE) {
      failed = 1;
    } else if (!is_valid) {
      failed = 1;
    }
    if (failed) {
      status_payload.int_val = (ui_int32)UI_FORM_STATUS_INVALID;
      {
        ui_error_t set_rc =
            ui_signal_set(control->status_signal, status_payload);
        (void)set_rc;
      }
      return UI_ERROR_NONE;
    }
  }

  /* Dispatch async validators */
  if (control->async_validators_count > 0) {
    status_payload.int_val = (ui_int32)UI_FORM_STATUS_PENDING;
    { (void)ui_signal_set(control->status_signal, status_payload); }

    for (i = 0; i < control->async_validators_count; i++) {
      struct ui_form_control_async_task *task =
          (struct ui_form_control_async_task *)C_MULTIPLATFORM_MALLOC(
              sizeof(struct ui_form_control_async_task));
      if (task) {
        task->control = control;
        task->validator = control->async_validators[i].fn;
        task->user_data = control->async_validators[i].user_data;
        task->generation = control->validation_generation;
        task->reactor = control->reactor;
        task->value = current_value;
        task->is_valid = UI_FALSE;

        control->pending_async_count++;
        rc = ui_thread_pool_schedule(control->thread_pool,
                                     ui_form_control_async_worker, task);
        if (rc != UI_ERROR_NONE) {
          status_payload.int_val = (ui_int32)UI_FORM_STATUS_INVALID;
          {
            ui_error_t set_rc =
                ui_signal_set(control->status_signal, status_payload);
            C_MULTIPLATFORM_FREE(task);
            return UI_ERROR_NONE;
          }
          control->pending_async_count = 0;
          C_MULTIPLATFORM_FREE(task);
          return UI_ERROR_NONE;
        }
      } else {
        /* Failed to allocate task. Fail validation. */
        status_payload.int_val = (ui_int32)UI_FORM_STATUS_INVALID;
        {
          ui_error_t set_rc =
              ui_signal_set(control->status_signal, status_payload);
        }
        control->pending_async_count = 0;
        return UI_ERROR_NONE;
      }
    }
  } else {
    /* Valid */
    status_payload.int_val = (ui_int32)UI_FORM_STATUS_VALID;
    {
      ui_error_t set_rc = ui_signal_set(control->status_signal, status_payload);
      (void)set_rc;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the value of a form control, marking it dirty and re-validating.
 * \param[in,out] control The form control.
 * \param[in] value The new value payload.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_control_set_value(ui_form_control_t *control,
                                     union ui_signal_payload new_value) {

  union ui_signal_payload dirty_payload = {0};
  ui_error_t rc;

  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_signal_set(control->value_signal, new_value);

  dirty_payload.bool_val = UI_TRUE;
  (void)ui_signal_set(control->dirty_signal, dirty_payload);

  { (void)ui_form_control_run_validation(control); }

  return UI_ERROR_NONE;
}

ui_error_t ui_form_control_patch_value(ui_form_control_t *control,
                                       union ui_signal_payload new_value) {
  return ui_form_control_set_value(control, new_value);
}

ui_error_t ui_form_control_mark_as_touched(ui_form_control_t *control) {
  union ui_signal_payload touched_payload = {0};
  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  touched_payload.bool_val = UI_TRUE;
  return ui_signal_set(control->touched_signal, touched_payload);
}

ui_error_t ui_form_control_disable(ui_form_control_t *control) {
  union ui_signal_payload status_payload = {0};
  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  status_payload.int_val = (ui_int32)UI_FORM_STATUS_DISABLED;
  return ui_signal_set(control->status_signal, status_payload);
}

ui_error_t ui_form_control_enable(ui_form_control_t *control) {
  ui_error_t rc;
  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Re-evaluates validators */
  { (void)ui_form_control_run_validation(control); }

  return UI_ERROR_NONE;
}

/**
 * \brief Retrieves the current value of a form control.
 * \param[in] control The form control.
 * \param[out] out_value Pointer to store the value payload.
 * \return UI_ERROR_NONE on success.
 */
/**
 * \brief Gets the signal representing the control's value.
 * \param[in] control The form control.
 * \param[out] out_signal Pointer to store the signal.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_control_get_value_signal(ui_form_control_t *control,
                                            ui_signal_t **out_signal) {
  if (!control || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = control->value_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Retrieves the validation status of a form control.
 * \param[in] control The form control.
 * \param[out] out_status Pointer to store the status.
 * \return UI_ERROR_NONE on success.
 */
/**
 * \brief Gets the signal representing the control's validation status.
 * \param[in] control The form control.
 * \param[out] out_signal Pointer to store the signal.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_control_get_status_signal(ui_form_control_t *control,
                                             ui_signal_t **out_signal) {
  if (!control || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = control->status_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the signal representing whether the control is touched.
 * \param[in] control The form control.
 * \param[out] out_signal Pointer to store the signal.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_control_get_touched_signal(ui_form_control_t *control,
                                              ui_signal_t **out_signal) {
  if (!control || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = control->touched_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the signal representing whether the control is dirty.
 * \param[in] control The form control.
 * \param[out] out_signal Pointer to store the signal.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_control_get_dirty_signal(ui_form_control_t *control,
                                            ui_signal_t **out_signal) {
  if (!control || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = control->dirty_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a form control.
 * \param[in,out] control The form control to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_control_destroy(ui_form_control_t *control) {

  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  { ui_signal_destroy(control->value_signal); }
  { ui_signal_destroy(control->status_signal); }
  { ui_signal_destroy(control->touched_signal); }
  { ui_signal_destroy(control->dirty_signal); }
  { ui_signal_destroy(control->errors_signal); }
  if (control->error_str)
    C_MULTIPLATFORM_FREE(control->error_str);

  return UI_ERROR_NONE;
}

/**
 * \brief Retrieves the current error string for a form control.
 * \param[in] control The form control.
 * \param[out] out_error_str Pointer to store the error string, or NULL if
 * valid.
 * \return UI_ERROR_NONE on success.
 */
/**
 * \brief Gets the signal representing the control's current error string.
 * \param[in] control The form control.
 * \param[out] out_signal Pointer to store the signal.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_control_get_errors_signal(ui_form_control_t *control,
                                             ui_signal_t **out_signal) {
  if (!control || !out_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_signal = control->errors_signal;
  return UI_ERROR_NONE;
}

ui_error_t ui_form_control_set_error(ui_form_control_t *control,
                                     const char *error_msg) {
  union ui_signal_payload payload;
  if (!control)
    return UI_ERROR_INVALID_ARGUMENT;
  if (control->error_str) {
    C_MULTIPLATFORM_FREE(control->error_str);
    control->error_str = NULL;
  }
  if (error_msg) {
    size_t l = strlen(error_msg);
    control->error_str = (char *)C_MULTIPLATFORM_MALLOC(l + 1);
    if (!control->error_str)
      return UI_ERROR_OUT_OF_MEMORY;
#if defined(_MSC_VER)
    strcpy_s(control->error_str, l + 1, error_msg);
#else
    strcpy(control->error_str, error_msg);
#endif
  }
  payload.ptr_val = control->error_str;
  return ui_signal_set(control->errors_signal, payload);
}
