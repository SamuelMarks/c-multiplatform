/* clang-format off */
#include <stddef.h>
#include "../include/ui_handle_manager.h"
#include "../include/ui_atomic.h"
#include "ui_internal_mem.h"
/* clang-format on */

#define HANDLE_INDEX(h) ((ui_uint32)((h) & 0xFFFFFFFF))
#define HANDLE_GEN(h) ((ui_uint32)((h) >> 32))
#define MAKE_HANDLE(i, g) ((((ui_uint64)(g)) << 32) | (ui_uint32)(i))

/** \brief ui_handle_entry */
struct ui_handle_entry {
  void *data;
  ui_uint32 next_free;
  ui_uint32 generation;
};

/** \brief ui_handle_manager */
struct ui_handle_manager {
  struct ui_handle_entry *entries;
  ui_uint32 capacity;
  ui_uint32 first_free;
  ui_uint32 active_count;
  ui_atomic_t lock;
};

static enum ui_error spin_lock(ui_atomic_t *lock) {
#ifndef UI_SINGLE_THREADED
  int is_swapped = 0;
  while (ui_atomic_cas(lock, 0, 1, &is_swapped) == UI_ERROR_NONE &&
         is_swapped == 0) {
    /* busy wait */
  }
#else
  (void)lock;
#endif
  return UI_ERROR_NONE;
}

static enum ui_error spin_unlock(ui_atomic_t *lock) {
#ifndef UI_SINGLE_THREADED
  ui_atomic_store(lock, 0);
#else
  (void)lock;
#endif
  return UI_ERROR_NONE;
}

enum ui_error ui_handle_manager_create(ui_uint32 capacity,
                                       struct ui_handle_manager **out_manager) {
  enum ui_error rc = UI_ERROR_NONE;
  struct ui_handle_manager *manager = NULL;
  ui_uint32 i;

  if (capacity == 0 || !out_manager) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  manager =
      (struct ui_handle_manager *)UI_MALLOC(sizeof(struct ui_handle_manager));
  if (!manager) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  manager->entries = (struct ui_handle_entry *)UI_MALLOC(
      capacity * sizeof(struct ui_handle_entry));
  if (!manager->entries) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  manager->capacity = capacity;
  manager->active_count = 0;
  manager->first_free = 0;
  ui_atomic_store(&manager->lock, 0);

  for (i = 0; i < capacity; ++i) {
    manager->entries[i].data = NULL;
    manager->entries[i].generation = 1;
    manager->entries[i].next_free = i + 1;
  }

  *out_manager = manager;
  manager = NULL;

cleanup:
  if (manager) {
    ui_handle_manager_destroy(manager);
  }
  return rc;
}

enum ui_error ui_handle_manager_destroy(struct ui_handle_manager *manager) {
  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (manager->entries) {
    UI_FREE(manager->entries);
  }
  UI_FREE(manager);
  return UI_ERROR_NONE;
}

enum ui_error ui_handle_manager_alloc(struct ui_handle_manager *manager,
                                      void *data, ui_uint64 *out_handle) {
  ui_uint32 index;
  ui_uint32 generation;

  if (!manager || !out_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)spin_lock(&manager->lock);

  if (manager->active_count >= manager->capacity) {
    (void)spin_unlock(&manager->lock);
    return UI_ERROR_QUEUE_FULL;
  }

  index = manager->first_free;
  manager->first_free = manager->entries[index].next_free;
  manager->active_count++;

  manager->entries[index].data = data;
  generation = manager->entries[index].generation;

  (void)spin_unlock(&manager->lock);

  *out_handle = MAKE_HANDLE(index, generation);
  return UI_ERROR_NONE;
}

enum ui_error ui_handle_manager_get(struct ui_handle_manager *manager,
                                    ui_uint64 handle, void **out_data) {
  ui_uint32 index;
  ui_uint32 generation;

  if (!manager || !out_data || handle == UI_INVALID_HANDLE) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  index = HANDLE_INDEX(handle);
  generation = HANDLE_GEN(handle);

  if (index >= manager->capacity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)spin_lock(&manager->lock);

  if (manager->entries[index].generation != generation ||
      manager->entries[index].data == NULL) {
    (void)spin_unlock(&manager->lock);
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_data = manager->entries[index].data;

  (void)spin_unlock(&manager->lock);
  return UI_ERROR_NONE;
}

enum ui_error ui_handle_manager_free(struct ui_handle_manager *manager,
                                     ui_uint64 handle) {
  ui_uint32 index;
  ui_uint32 generation;

  if (!manager || handle == UI_INVALID_HANDLE) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  index = HANDLE_INDEX(handle);
  generation = HANDLE_GEN(handle);

  if (index >= manager->capacity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)spin_lock(&manager->lock);

  if (manager->entries[index].generation != generation ||
      manager->entries[index].data == NULL) {
    (void)spin_unlock(&manager->lock);
    return UI_ERROR_INVALID_ARGUMENT;
  }

  manager->entries[index].data = NULL;
  manager->entries[index].generation++;
  manager->entries[index].next_free = manager->first_free;
  manager->first_free = index;
  manager->active_count--;

  (void)spin_unlock(&manager->lock);
  return UI_ERROR_NONE;
}
