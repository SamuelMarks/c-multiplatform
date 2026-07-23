/* clang-format off */
#include "../include/ui_tick_engine.h"
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_task_node {
  enum ui_error (*callback)(void *);
  void *user_data;
  struct ui_task_node *next;
};

/** \brief ui_tick_engine */
struct ui_tick_engine {
  struct ui_task_node *head;
  struct ui_task_node *tail;
};

enum ui_error ui_tick_engine_create(struct ui_tick_engine **out_engine) {
  enum ui_error rc = UI_ERROR_NONE;
  struct ui_tick_engine *engine = NULL;

  if (!out_engine) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  engine = (struct ui_tick_engine *)UI_MALLOC(sizeof(struct ui_tick_engine));
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

enum ui_error ui_tick_engine_destroy(struct ui_tick_engine *engine) {
  struct ui_task_node *current = NULL;
  struct ui_task_node *next = NULL;

  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = engine->head;
  while (current) {
    next = current->next;
    UI_FREE(current);
    current = next;
  }

  UI_FREE(engine);
  return UI_ERROR_NONE;
}

enum ui_error ui_tick_engine_schedule(struct ui_tick_engine *engine,
                                      enum ui_error (*callback)(void *),
                                      void *user_data) {
  enum ui_error rc = UI_ERROR_NONE;
  struct ui_task_node *node = NULL;

  if (!engine || !callback) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  node = (struct ui_task_node *)UI_MALLOC(sizeof(struct ui_task_node));
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

enum ui_error ui_tick_engine_tick(struct ui_tick_engine *engine) {
  struct ui_task_node *current = NULL;
  struct ui_task_node *next = NULL;

  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = engine->head;
  engine->head = NULL;
  engine->tail = NULL;

  while (current) {
    next = current->next;
    if (current->callback) {
      current->callback(current->user_data);
    }
    UI_FREE(current);
    current = next;
  }

  return UI_ERROR_NONE;
}
