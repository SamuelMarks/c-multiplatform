/* clang-format off */
#include "ui_file_uploader_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_mock_io_fail;
/** @cond */
#define UI_FSEEK(f, o, w) (g_mock_io_fail == 1 ? -1 : fseek(f, o, w))
/** @endcond */
/** @cond */
#define UI_FTELL(f) (g_mock_io_fail == 2 ? -1 : ftell(f))
/** @endcond */
/** @cond */
#define UI_FREAD(p, s, n, f) (g_mock_io_fail == 3 ? 0 : fread(p, s, n, f))
/** @endcond */
/** @cond */
#define UI_FSEEK_SET_FAIL(f, o, w) (g_mock_io_fail == 4 ? -1 : fseek(f, o, w))
/** @endcond */
#else
/** @cond */
#define UI_FSEEK(f, o, w) fseek(f, o, w)
/** @endcond */
/** @cond */
#define UI_FTELL(f) ftell(f)
/** @endcond */
/** @cond */
#define UI_FREAD(p, s, n, f) fread(p, s, n, f)
/** @endcond */
/** @cond */
#define UI_FSEEK_SET_FAIL(f, o, w) fseek(f, o, w)
/** @endcond */
#endif

/*
 * @brief ui_file_uploader_cva_write_value.
 * @param component Parameter component.
 * @param value Parameter value.
 * @return Return value.
 */
static ui_error_t
ui_file_uploader_cva_write_value(void *component,
                                 union ui_signal_payload value) {
  struct ui_file_uploader_base *uploader =
      (struct ui_file_uploader_base *)component;
  int i;
  /* When CVA writes a value, it may pass an array of files or clear them.
     For a file uploader, tearing down pointers on clear is important.
     If payload is NULL, clear the queue.
  */
  if (value.ptr_val == NULL) {
    for (i = 0; i < uploader->file_count; i++) {
      if (uploader->files[i].data != NULL) {
        C_MULTIPLATFORM_FREE(uploader->files[i].data);
        uploader->files[i].data = NULL;
      }
      memset(&uploader->files[i], 0, sizeof(struct ui_file_uploader_file));
    }
    uploader->file_count = 0;
    uploader->state = UI_FILE_UPLOADER_STATE_IDLE;
  }
  /* Advanced setting of file references from external source is not typically
     required for dropzones, mainly just clearing. */
  return UI_ERROR_NONE;
}

/* \brief ui_file_uploader_cva_register_on_change
 */
static ui_error_t ui_file_uploader_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_file_uploader_base *uploader =
      (struct ui_file_uploader_base *)component;
  uploader->on_change_cb = callback;
  uploader->on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/* \brief ui_file_uploader_cva_register_on_touched
 */
static ui_error_t ui_file_uploader_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_file_uploader_base *uploader =
      (struct ui_file_uploader_base *)component;
  uploader->on_touched_cb = callback;
  uploader->on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_file_uploader_cva_set_disabled_state.
 * @param component Parameter component.
 * @param is_disabled Parameter is_disabled.
 * @return Return value.
 */
static ui_error_t
ui_file_uploader_cva_set_disabled_state(void *component,
                                        ui_bool_t is_disabled) {
  struct ui_file_uploader_base *uploader =
      (struct ui_file_uploader_base *)component;
  uploader->is_disabled = is_disabled;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_file_uploader_init.
 * @param uploader Parameter uploader.
 * @param max_files Parameter max_files.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param out_cva Parameter out_cva.
 * @return Return value.
 */
ui_error_t ui_file_uploader_init(struct ui_file_uploader_base *uploader,
                                 int max_files, int x, int y, int width,
                                 int height,
                                 struct ui_control_value_accessor *out_cva) {
  if (uploader == NULL || max_files <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  uploader->state = UI_FILE_UPLOADER_STATE_IDLE;
  uploader->file_count = 0;
  uploader->max_files = max_files;
  uploader->x = x;
  uploader->y = y;
  uploader->width = width;
  uploader->height = height;

  uploader->on_change_cb = NULL;
  uploader->on_change_user_data = NULL;
  uploader->on_touched_cb = NULL;
  uploader->on_touched_user_data = NULL;
  uploader->is_disabled = UI_FALSE;

  uploader->files = (struct ui_file_uploader_file *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_file_uploader_file) * (size_t)max_files);
  if (uploader->files == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(uploader->files, 0,
         sizeof(struct ui_file_uploader_file) * (size_t)max_files);

  /* Initialize internal drag list */
  uploader->dropzone_list.list_id = -1; /* Arbitrary ID for external systems */
  uploader->dropzone_list.x = x;
  uploader->dropzone_list.y = y;
  uploader->dropzone_list.width = width;
  uploader->dropzone_list.height = height;
  uploader->dropzone_list.orientation = UI_DRAG_LIST_ORIENTATION_VERTICAL;
  uploader->dropzone_list.items = NULL;
  uploader->dropzone_list.item_count = 0;

  if (out_cva != NULL) {
    out_cva->write_value = ui_file_uploader_cva_write_value;
    out_cva->register_on_change = ui_file_uploader_cva_register_on_change;
    out_cva->register_on_touched = ui_file_uploader_cva_register_on_touched;
    out_cva->set_disabled_state = ui_file_uploader_cva_set_disabled_state;
  }

  return UI_ERROR_NONE;
}

/*
 * @brief ui_file_uploader_destroy.
 * @param uploader Parameter uploader.
 * @return Return value.
 */
ui_error_t ui_file_uploader_destroy(struct ui_file_uploader_base *uploader) {
  int i;
  if (uploader == NULL) {
    return UI_ERROR_NONE;
  }

  if (uploader->files != NULL) {
    for (i = 0; i < uploader->file_count; i++) {
      if (uploader->files[i].data != NULL) {
        C_MULTIPLATFORM_FREE(uploader->files[i].data);
        uploader->files[i].data = NULL;
      }
    }
    C_MULTIPLATFORM_FREE(uploader->files);
    uploader->files = NULL;
  }
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_file_uploader_on_drag_enter(struct ui_file_uploader_base *uploader) {
  if (uploader == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (uploader->is_disabled) {
    return UI_ERROR_NONE; /* Ignore if disabled */
  }
  uploader->state = UI_FILE_UPLOADER_STATE_DRAG_OVER;

  if (uploader->on_touched_cb != NULL) {
    ui_error_t touch_rc =
        uploader->on_touched_cb(uploader->on_touched_user_data);
    if (touch_rc != UI_ERROR_NONE) {
      return touch_rc;
    }
  }

  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_file_uploader_on_drag_leave(struct ui_file_uploader_base *uploader) {
  if (uploader == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (uploader->is_disabled) {
    return UI_ERROR_NONE;
  }
  uploader->state = UI_FILE_UPLOADER_STATE_IDLE;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_file_uploader_drop_file.
 * @param uploader Parameter uploader.
 * @param file_path Parameter file_path.
 * @return Return value.
 */
ui_error_t ui_file_uploader_drop_file(struct ui_file_uploader_base *uploader,
                                      const char *file_path) {
  const char *last_slash;
  struct ui_file_uploader_file *file_ptr;

  if (uploader == NULL || file_path == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (uploader->is_disabled) {
    return UI_ERROR_NONE;
  }

  if (uploader->file_count >= uploader->max_files) {
    return UI_ERROR_QUEUE_FULL;
  }

  file_ptr = &uploader->files[uploader->file_count];

#if defined(_MSC_VER)
  strncpy_s(file_ptr->file_path, sizeof(file_ptr->file_path), file_path,
            _TRUNCATE);
#else
  strncpy(file_ptr->file_path, file_path, sizeof(file_ptr->file_path) - 1);
  file_ptr->file_path[sizeof(file_ptr->file_path) - 1] = '\0';
#endif

  last_slash = strrchr(file_path, '/');
  if (last_slash == NULL) {
    last_slash = strrchr(file_path, '\\');
  }

  if (last_slash != NULL) {
    last_slash++; /* Skip the slash */
  } else {
    last_slash = file_path; /* No slash found, use whole string */
  }

#if defined(_MSC_VER)
  strncpy_s(file_ptr->file_name, sizeof(file_ptr->file_name), last_slash,
            _TRUNCATE);
#else
  strncpy(file_ptr->file_name, last_slash, sizeof(file_ptr->file_name) - 1);
  file_ptr->file_name[sizeof(file_ptr->file_name) - 1] = '\0';
#endif

  uploader->file_count++;
  uploader->state = UI_FILE_UPLOADER_STATE_IDLE; /* Dropped, waiting to read */

  if (uploader->on_touched_cb != NULL) {
    ui_error_t touch_rc =
        uploader->on_touched_cb(uploader->on_touched_user_data);
    if (touch_rc != UI_ERROR_NONE) {
      return touch_rc;
    }
  }

  if (uploader->on_change_cb != NULL) {
    union ui_signal_payload payload;
    payload.ptr_val = (void *)uploader->files;
    {
      ui_error_t change_rc =
          uploader->on_change_cb(payload, uploader->on_change_user_data);
      if (change_rc != UI_ERROR_NONE) {
        return change_rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t ui_file_uploader_read_files(struct ui_file_uploader_base *uploader) {
  int i;
  FILE *fp = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  long file_size;
  size_t read_bytes;

  if (uploader == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (uploader->is_disabled) {
    return UI_ERROR_NONE;
  }

  uploader->state = UI_FILE_UPLOADER_STATE_READING;

  for (i = 0; i < uploader->file_count; i++) {
    struct ui_file_uploader_file *f = &uploader->files[i];

    if (f->data != NULL) {
      continue; /* Already read */
    }

#if defined(_MSC_VER)
    if (fopen_s(&fp, f->file_path, "rb") != 0) {
      fp = NULL;
    }
#else
    fp = fopen(f->file_path, "rb");
#endif

    if (fp == NULL) {
      rc = UI_ERROR_IO_FAILED;
      goto cleanup;
    }

    if (UI_FSEEK(fp, 0, SEEK_END) != 0) {
      rc = UI_ERROR_IO_FAILED;
      goto cleanup;
    }

    file_size = UI_FTELL(fp);
    if (file_size < 0) {
      rc = UI_ERROR_IO_FAILED;
      goto cleanup;
    }

    if (UI_FSEEK_SET_FAIL(fp, 0, SEEK_SET) != 0) {
      rc = UI_ERROR_IO_FAILED;
      goto cleanup;
    }

    f->file_size = (size_t)file_size;
    f->data = (unsigned char *)C_MULTIPLATFORM_MALLOC(
        f->file_size + 1); /* +1 for safety null terminator */
    if (f->data == NULL) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }

    if (f->file_size > 0) {
      read_bytes = UI_FREAD(f->data, 1, f->file_size, fp);
      if (read_bytes != f->file_size) {
        rc = UI_ERROR_IO_FAILED;
        goto cleanup;
      }
    }
    f->data[f->file_size] =
        '\0'; /* Ensure null terminated just in case it's text */

    fclose(fp);
    fp = NULL;
  }

  uploader->state = UI_FILE_UPLOADER_STATE_COMPLETE;

  if (uploader->on_change_cb != NULL) {
    union ui_signal_payload payload;
    payload.ptr_val = (void *)uploader->files;
    {
      ui_error_t change_rc =
          uploader->on_change_cb(payload, uploader->on_change_user_data);
      if (change_rc != UI_ERROR_NONE) {
        return change_rc;
      }
    }
  }

  return UI_ERROR_NONE;

cleanup:
  printf("read_files failed with rc: %d\n", rc);
  if (fp != NULL) {
    fclose(fp);
  }
  uploader->state = UI_FILE_UPLOADER_STATE_ERROR;
  return rc;
}

/* \brief ui_error
 */
ui_error_t
ui_file_uploader_register_dropzone(struct ui_file_uploader_base *uploader,
                                   struct ui_drag_drop_context *drag_ctx) {
  if (uploader == NULL || drag_ctx == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_drag_drop_add_list(drag_ctx, &uploader->dropzone_list);
}
