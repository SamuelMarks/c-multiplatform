/* clang-format off */
#include "../include/ui_execution_context.h"
#include "../include/ui_types.h"
#include "ui_internal_mem.h"
/* clang-format on */

static UI_THREAD_LOCAL struct ui_execution_context *g_current_context = NULL;

/** \brief ui_execution_context */
struct ui_execution_context {
  ui_error_t (*task_callback)(void *);
  void *task_user_data;
};

/** \brief ui_error */
ui_error_t ui_execution_context_create(struct ui_execution_context **out_ctx) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_execution_context *ctx = NULL;

  if (!out_ctx) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  ctx = (struct ui_execution_context *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_execution_context));
  if (!ctx) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  ctx->task_callback = NULL;
  ctx->task_user_data = NULL;

  *out_ctx = ctx;

cleanup:
  return rc;
}

ui_error_t ui_execution_context_destroy(struct ui_execution_context *ctx) {
  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(ctx);
  return UI_ERROR_NONE;
}

ui_error_t ui_execution_context_schedule(struct ui_execution_context *ctx,
                                         ui_error_t (*callback)(void *),
                                         void *user_data) {
  if (!ctx || !callback) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ctx->task_callback = callback;
  ctx->task_user_data = user_data;
  return UI_ERROR_NONE;
}

ui_error_t ui_execution_context_tick(struct ui_execution_context *ctx) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (ctx->task_callback) {
    rc = ctx->task_callback(ctx->task_user_data);
    ctx->task_callback = NULL;
    ctx->task_user_data = NULL;
    if (rc != UI_ERROR_NONE)
      return rc;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_execution_context_cancel(struct ui_execution_context *ctx) {
  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ctx->task_callback = NULL;
  ctx->task_user_data = NULL;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_execution_context_set_current(struct ui_execution_context *ctx) {
  g_current_context = ctx;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_execution_context_get_current(struct ui_execution_context **out_ctx) {
  if (!out_ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_ctx = g_current_context;
  return UI_ERROR_NONE;
}
