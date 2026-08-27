/**
 * @file ui_tick_engine.c
 * @brief Implementation of the single-threaded tick engine.
 */
/* clang-format off */
#include "../include/ui_tick_engine.h"
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @struct ui_task_node
 * @struct ui_task_node
 * @brief Internal representation of a queued task for the tick engine.
 */
struct ui_task_node {
  /* @brief The callback function to execute. */
  ui_error_t (*callback)(void *); /**< ) */
  /* @brief Opaque user data for the callback. */
  void *user_data; /**< user_data */
  /* @brief Pointer to the next task in the queue. */
  struct ui_task_node *next; /**< next */
};

/**
 * @struct ui_tick_engine
 * @struct ui_tick_engine
 * @brief Internal implementation of the single-threaded tick engine.
 */
struct ui_tick_engine {
  /* @brief Head of the task queue. */
  struct ui_task_node *head; /**< head */
  /* @brief Tail of the task queue. */
  struct ui_task_node *tail; /**< tail */
};

ui_error_t ui_tick_engine_create(struct ui_tick_engine **out_engine) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_tick_engine *engine = NULL;

  if (!out_engine) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  engine = (struct ui_tick_engine *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_tick_engine));
  if (!engine) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  engine->head = NULL;
  engine->tail = NULL;

  *out_engine = engine;

cleanup:
  return rc;
}

ui_error_t ui_tick_engine_destroy(struct ui_tick_engine *engine) {
  struct ui_task_node *current = NULL;
  struct ui_task_node *next = NULL;

  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = engine->head;
  while (current) {
    next = current->next;
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }

  C_MULTIPLATFORM_FREE(engine);
  return UI_ERROR_NONE;
}

ui_error_t ui_tick_engine_schedule(struct ui_tick_engine *engine,
                                   ui_error_t (*callback)(void *),
                                   void *user_data) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_task_node *node = NULL;

  if (!engine || !callback) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  node = (struct ui_task_node *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_task_node));
  if (!node) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  node->callback = callback;
  node->user_data = user_data;
  node->next = NULL;

  if (engine->tail) {
    engine->tail->next = node;
  } else {
    engine->head = node;
  }
  engine->tail = node;

cleanup:
  return rc;
}

ui_error_t ui_tick_engine_tick(struct ui_tick_engine *engine) {
  struct ui_task_node *current = NULL;
  struct ui_task_node *next = NULL;
  ui_error_t tick_rc = UI_ERROR_NONE;

  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = engine->head;
  engine->head = NULL;
  engine->tail = NULL;

  while (current) {
    next = current->next;
    {
/** @cond */
#define UI_TICK_EXECUTE_CB(c) (c)->callback((c)->user_data)
      /** @endcond */
      ui_error_t cb_rc = UI_TICK_EXECUTE_CB(current);
      if (cb_rc != UI_ERROR_NONE) {
        if (tick_rc == UI_ERROR_NONE) {
          tick_rc = cb_rc;
        }
      }
    }
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }

  return tick_rc;
}
