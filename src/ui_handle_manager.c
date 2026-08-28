/**
 * @file ui_handle_manager.c
 * @brief ui_handle_manager.c implementation.
 */
/*
 * @file ui_handle_manager.c
 * @brief Implementation of the UI handle manager.
 */
/* clang-format off */
#include <stddef.h>
#include "../include/ui_handle_manager.h"
#include "../include/ui_atomic.h"
#include "ui_internal_mem.h"
/* clang-format on */

/** @def HANDLE_INDEX
 * @brief Handle index extraction
 */
#define HANDLE_INDEX(h) ((ui_uint32)((h) & 0xFFFFFFFF))
/** @def HANDLE_GEN
 * @brief Handle generation extraction
 */
#define HANDLE_GEN(h) ((ui_uint32)((h) >> 32))
/** @def MAKE_HANDLE
 * @brief Handle generation
 */
#define MAKE_HANDLE(i, g) ((((ui_uint64)(g)) << 32) | (ui_uint32)(i))

/**
 * @struct ui_handle_entry
 * @brief ui_handle_entry
 */
struct ui_handle_entry {
  void *data;           /**< data */
  ui_uint32 next_free;  /**< next_free */
  ui_uint32 generation; /**< generation */
};

/**
 * @struct ui_handle_manager
 * @brief ui_handle_manager
 */
struct ui_handle_manager {
  struct ui_handle_entry *entries; /**< entries */
  ui_uint32 capacity;              /**< capacity */
  ui_uint32 first_free;            /**< first_free */
  ui_uint32 active_count;          /**< active_count */
  ui_atomic_t lock;                /**< lock */
};

/**
 * @brief Acquires a spin lock.
 * @param[in,out] lock The lock to acquire.
 * @return UI_ERROR_NONE on success.
 */
/**
 * @brief spin_lock.
 * @param lock Parameter lock.
 * @return Return value.
 */
static ui_error_t spin_lock(ui_atomic_t *lock) {
#ifndef UI_SINGLE_THREADED
  int is_swapped = 0;
  do {
    {
      ui_error_t cas_rc = ui_atomic_cas(lock, 0, 1, &is_swapped);
      (void)cas_rc;
    }
  } while (is_swapped != 0);
#else
  (void)lock;
#endif
  return UI_ERROR_NONE;
}

/**
 * @brief Releases a spin lock.
 * @param[in,out] lock The lock to release.
 * @return UI_ERROR_NONE on success.
 */
/**
 * @brief spin_unlock.
 * @param lock Parameter lock.
 * @return Return value.
 */
static ui_error_t spin_unlock(ui_atomic_t *lock) {
#ifndef UI_SINGLE_THREADED
  {
    ui_error_t store_rc = ui_atomic_store(lock, 0);
    (void)store_rc;
  }
#else
  (void)lock;
#endif
  return UI_ERROR_NONE;
}

/**
 * @brief Creates a handle manager with a specified capacity.
 * @param[in] capacity The maximum number of handles.
 * @param[out] out_manager Pointer to store the created manager.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_handle_manager_create(ui_uint32 capacity,
                                    struct ui_handle_manager **out_manager) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_handle_manager *manager = NULL;
  ui_uint32 i;

  if (capacity == 0 || !out_manager) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  manager = (struct ui_handle_manager *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_handle_manager));
  if (!manager) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  manager->entries = (struct ui_handle_entry *)C_MULTIPLATFORM_MALLOC(
      (size_t)capacity * sizeof(struct ui_handle_entry));
  if (!manager->entries) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  manager->capacity = capacity;
  manager->active_count = 0;
  manager->first_free = 0;
  {
    ui_error_t st_rc = ui_atomic_store(&manager->lock, 0);
    (void)st_rc;
  }

  for (i = 0; i < capacity; ++i) {
    manager->entries[i].data = NULL;
    manager->entries[i].generation = 1;
    manager->entries[i].next_free = i + 1;
  }

  *out_manager = manager;
  manager = NULL;

cleanup:
  if (manager) {
    C_MULTIPLATFORM_FREE(manager->entries);
    C_MULTIPLATFORM_FREE(manager);
  }
  return rc;
}

/**
 * @brief Destroys a handle manager.
 * @param[in,out] manager The manager to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_handle_manager_destroy(struct ui_handle_manager *manager) {
  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(manager->entries);

  C_MULTIPLATFORM_FREE(manager);
  return UI_ERROR_NONE;
}

/**
 * @brief Allocates a new handle and associates it with data.
 * @param[in,out] manager The handle manager.
 * @param[in] data The data pointer to associate with the handle.
 * @param[out] out_handle Pointer to store the allocated handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_QUEUE_FULL if at capacity.
 */
ui_error_t ui_handle_manager_alloc(struct ui_handle_manager *manager,
                                   void *data, ui_uint64 *out_handle) {
  ui_uint32 index;
  ui_uint32 generation;

  if (!manager || !out_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t sl_rc = spin_lock(&manager->lock);
    (void)sl_rc;
  }

  if (manager->active_count >= manager->capacity) {
    {
      ui_error_t sul_rc = spin_unlock(&manager->lock);
      (void)sul_rc;
    }
    return UI_ERROR_QUEUE_FULL;
  }

  index = manager->first_free;
  manager->first_free = manager->entries[index].next_free;
  manager->active_count++;

  manager->entries[index].data = data;
  generation = manager->entries[index].generation;

  {
    ui_error_t sul_rc = spin_unlock(&manager->lock);
    (void)sul_rc;
  }

  *out_handle = MAKE_HANDLE(index, generation);
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the data associated with a valid handle.
 * @param[in,out] manager The handle manager.
 * @param[in] handle The handle to look up.
 * @param[out] out_data Pointer to store the associated data.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT for
 * invalid/stale handle.
 */
ui_error_t ui_handle_manager_get(struct ui_handle_manager *manager,
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

  {
    ui_error_t sl_rc = spin_lock(&manager->lock);
    (void)sl_rc;
  }

  if (manager->entries[index].generation != generation ||
      manager->entries[index].data == NULL) {
    {
      ui_error_t sul_rc = spin_unlock(&manager->lock);
      (void)sul_rc;
    }
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_data = manager->entries[index].data;

  {
    ui_error_t sul_rc = spin_unlock(&manager->lock);
    (void)sul_rc;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Frees a handle, making its slot available for reuse and incrementing
 * its generation.
 * @param[in,out] manager The handle manager.
 * @param[in] handle The handle to free.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_handle_manager_free(struct ui_handle_manager *manager,
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

  {
    ui_error_t sl_rc = spin_lock(&manager->lock);
    (void)sl_rc;
  }

  if (manager->entries[index].generation != generation ||
      manager->entries[index].data == NULL) {
    {
      ui_error_t sul_rc = spin_unlock(&manager->lock);
      (void)sul_rc;
    }
    return UI_ERROR_INVALID_ARGUMENT;
  }

  manager->entries[index].data = NULL;
  manager->entries[index].generation++;
  manager->entries[index].next_free = manager->first_free;
  manager->first_free = index;
  manager->active_count--;

  {
    ui_error_t sul_rc = spin_unlock(&manager->lock);
    (void)sul_rc;
  }
  return UI_ERROR_NONE;
}
