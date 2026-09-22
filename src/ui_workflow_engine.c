/**
 * @file ui_workflow_engine.c
 * @brief Implementation of the sequential workflow engine aligned with Arazzo
 * semantics.
 */

/* clang-format off */
#include "ui_workflow_engine.h"
#include "ui_runtime_router.h"
#include "ui_internal_mem.h"
#include "ui_coercion_utils.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_workflow_mock_signal_fail = 0;
int g_workflow_mock_criteria_fail = 0;

/**
 * @brief mock_workflow_signal_get.
 * @param signal Parameter signal.
 * @param out_payload Parameter out_payload.
 * @return Return value.
 */
static ui_error_t
mock_workflow_signal_get(struct ui_signal *signal,
                         union ui_signal_payload *out_payload) {
  if (g_workflow_mock_signal_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_signal_get)(signal, out_payload);
}
#undef ui_signal_get
/** @cond */
#define ui_signal_get mock_workflow_signal_get
/** @endcond */

/**
 * @brief mock_workflow_signal_set.
 * @param signal Parameter signal.
 * @param payload Parameter payload.
 * @return Return value.
 */
static ui_error_t mock_workflow_signal_set(struct ui_signal *signal,
                                           union ui_signal_payload payload) {
  if (g_workflow_mock_signal_fail == 2 || g_workflow_mock_signal_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_signal_set)(signal, payload);
}
#undef ui_signal_set
/** @cond */
#define ui_signal_set mock_workflow_signal_set
/** @endcond */
#endif

/**
 * @brief Evaluates an Arazzo successCriteria expression.
 *
 * @param criteria The expression string (e.g. "$code == 200" or "$statusCode ==
 * 200").
 * @param status_code Simulated or actual HTTP status code.
 * @param out_matches Pointer to receive boolean match result.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
static ui_error_t evaluate_success_criteria(const char *criteria,
                                            int status_code,
                                            ui_bool_t *out_matches);

#ifdef UI_TEST_MOCK_ALLOC
/**
 * @brief test_ui_workflow_evaluate_success_criteria_null.
 * @return Return value.
 */
ui_error_t test_ui_workflow_evaluate_success_criteria_null(void);
ui_error_t test_ui_workflow_evaluate_success_criteria_null(void) {
  return evaluate_success_criteria(NULL, 200, NULL);
}
#endif

static ui_error_t evaluate_success_criteria(const char *criteria,
                                            int status_code,
                                            ui_bool_t *out_matches) {
  const char *p;

#ifdef UI_TEST_MOCK_ALLOC
  if (g_workflow_mock_criteria_fail) {
    return UI_ERROR_UNKNOWN;
  }
#endif

  if (!out_matches) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!criteria || strlen(criteria) == 0) {
    *out_matches = UI_TRUE;
    return UI_ERROR_NONE;
  }

  for (p = criteria; *p != '\0'; p++) {
    if (*p >= '0' && *p <= '9') {
      int parsed = atoi(p);
      *out_matches = (status_code == parsed) ? UI_TRUE : UI_FALSE;
      return UI_ERROR_NONE;
    }
  }

  *out_matches = UI_TRUE;
  return UI_ERROR_NONE;
}

/**
 * @brief Executes a sequence of workflow steps according to Arazzo semantics.
 *
 * @param workflow The workflow action sequence to execute.
 * @param wf_ctx Execution context containing state, router, and DOM mount host.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_workflow_engine_execute(const struct ui_runtime_workflow *workflow,
                           struct ui_workflow_context *wf_ctx) {
  const struct ui_runtime_workflow_step *step;
  ui_error_t rc = UI_ERROR_NONE;

  if (!workflow || !wf_ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  step = workflow->first_step;
  while (step) {
    if (step->action == UI_RUNTIME_ACTION_MUTATE_STATE) {
      if (step->target && step->value) {
        union ui_signal_payload payload;
        int int_val = 0;
        ui_error_t coerce_rc;

        if (strcmp(step->value, "increment") == 0) {
          struct ui_signal *target_sig = NULL;
          rc = ui_dynamic_context_resolve_signal(wf_ctx->ctx, wf_ctx->app_state,
                                                 step->target, &target_sig);
          if (rc != UI_ERROR_NONE) {
            return rc;
          }
          rc = ui_signal_get(target_sig, &payload);
          if (rc != UI_ERROR_NONE) {
            return rc;
          }
          payload.int_val += 1;
          rc = ui_signal_set(target_sig, payload);
          if (rc != UI_ERROR_NONE) {
            return rc;
          }
        } else {
          coerce_rc = ui_coerce_string_to_int(step->value, &int_val);
          if (coerce_rc == UI_ERROR_NONE) {
            payload.int_val = int_val;
          } else {
            payload.ptr_val = (void *)step->value;
          }

          if (strncmp(step->target, "app.", 4) == 0 && wf_ctx->app_state) {
            rc = ui_app_state_registry_set_value(wf_ctx->app_state,
                                                 step->target + 4, payload);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
          } else {
            struct ui_signal *target_sig = NULL;
            rc = ui_dynamic_context_resolve_signal(
                wf_ctx->ctx, wf_ctx->app_state, step->target, &target_sig);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
            rc = ui_signal_set(target_sig, payload);
            if (rc != UI_ERROR_NONE) {
              return rc;
            }
          }
        }
      }
    } else if (step->action == UI_RUNTIME_ACTION_NAVIGATE) {
      if (step->target && wf_ctx->router && wf_ctx->mount_host) {
        rc = ui_runtime_router_navigate(wf_ctx->router, step->target,
                                        wf_ctx->mount_host, wf_ctx->registry,
                                        wf_ctx->ctx, wf_ctx->app_state);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      }
    } else if (step->action == UI_RUNTIME_ACTION_HTTP) {
      /* Simulated HTTP operation evaluated against Arazzo successCriteria */
      int simulated_status = 200;
      ui_bool_t matches = UI_FALSE;
      rc = evaluate_success_criteria(step->success_criteria, simulated_status,
                                     &matches);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      if (!matches) {
        return UI_ERROR_UNKNOWN;
      }
    }

    step = step->next;
  }

  return UI_ERROR_NONE;
}
