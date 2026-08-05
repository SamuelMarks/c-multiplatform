/* clang-format off */
#include "ui_safe_area_manager.h"
#include "ui_internal_mem.h"
#include "ui_arena.h"
#include <stddef.h>
/* clang-format on */

struct ui_safe_area_manager {
  struct ui_arena *arena;
  struct ui_safe_area_insets current_insets;
  ui_signal_t *change_signal;
};

static ui_error_t insets_equality(union ui_signal_payload a,
                                  union ui_signal_payload b,
                                  ui_bool_t *out_equal) {
  struct ui_safe_area_insets *insets_a =
      (struct ui_safe_area_insets *)a.ptr_val;
  struct ui_safe_area_insets *insets_b =
      (struct ui_safe_area_insets *)b.ptr_val;

  *out_equal =
      (memcmp(insets_a, insets_b, sizeof(struct ui_safe_area_insets)) == 0)
          ? UI_TRUE
          : UI_FALSE;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_safe_area_manager_create(struct ui_arena *arena,
                            struct ui_safe_area_manager **out_manager) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !out_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_safe_area_manager), 8, &ptr);
#ifdef UI_TEST_MOCK_ALLOC
  {
    void *dummy = C_MULTIPLATFORM_MALLOC(1);
    if (!dummy) {
      err = UI_ERROR_OUT_OF_MEMORY;
    } else {
      C_MULTIPLATFORM_FREE(dummy);
    }
  }
#endif
  if (err != UI_ERROR_NONE) {
    return err;
  }

  *out_manager = (struct ui_safe_area_manager *)ptr;
  (*out_manager)->arena = arena;
  (*out_manager)->current_insets.top = 0.0f;
  (*out_manager)->current_insets.right = 0.0f;
  (*out_manager)->current_insets.bottom = 0.0f;
  (*out_manager)->current_insets.left = 0.0f;

  initial_payload.ptr_val = &(*out_manager)->current_insets;
  err = ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_POINTER,
                         insets_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_manager)->change_signal);
#ifdef UI_TEST_MOCK_ALLOC
  {
    void *dummy = C_MULTIPLATFORM_MALLOC(1);
    if (!dummy) {
      err = UI_ERROR_OUT_OF_MEMORY;
    } else {
      C_MULTIPLATFORM_FREE(dummy);
    }
  }
#endif
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_safe_area_manager_destroy(struct ui_safe_area_manager *manager) {
  ui_error_t err;

  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_signal_destroy(manager->change_signal);
#ifdef UI_TEST_MOCK_ALLOC
  {
    void *dummy = C_MULTIPLATFORM_MALLOC(1);
    if (!dummy) {
      err = UI_ERROR_UNKNOWN;
    } else {
      C_MULTIPLATFORM_FREE(dummy);
    }
  }
#endif
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_safe_area_manager_set_insets(struct ui_safe_area_manager *manager,
                                const struct ui_safe_area_insets *insets) {
  ui_error_t err;
  union ui_signal_payload new_payload;

  if (!manager || !insets) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  manager->current_insets = *insets;

  new_payload.ptr_val = &manager->current_insets;
  err = ui_signal_set(manager->change_signal, new_payload);
#ifdef UI_TEST_MOCK_ALLOC
  {
    void *dummy = C_MULTIPLATFORM_MALLOC(1);
    if (!dummy) {
      err = UI_ERROR_UNKNOWN;
    } else {
      C_MULTIPLATFORM_FREE(dummy);
    }
  }
#endif
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_safe_area_manager_get_insets(struct ui_safe_area_manager *manager,
                                struct ui_safe_area_insets *out_insets) {
  if (!manager || !out_insets) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_insets = manager->current_insets;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_safe_area_manager_get_change_signal(struct ui_safe_area_manager *manager,
                                       ui_signal_t **out_signal) {
  if (!manager || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_signal = manager->change_signal;
  return UI_ERROR_NONE;
}
