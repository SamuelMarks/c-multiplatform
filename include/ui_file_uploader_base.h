#ifndef UI_FILE_UPLOADER_BASE_H
#define UI_FILE_UPLOADER_BASE_H

/* clang-format off */
#include "ui_error.h"
#include "ui_drag_drop.h"
#include "ui_control_value_accessor.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief States of the file uploader dropzone.
 */
enum ui_file_uploader_state {
  UI_FILE_UPLOADER_STATE_IDLE = 0, /**< Waiting for files */
  UI_FILE_UPLOADER_STATE_DRAG_OVER =
      1, /**< A file is being dragged over the dropzone */
  UI_FILE_UPLOADER_STATE_READING =
      2, /**< Files are currently being read from the filesystem */
  UI_FILE_UPLOADER_STATE_COMPLETE = 3, /**< Files have been read successfully */
  UI_FILE_UPLOADER_STATE_ERROR = 4     /**< An error occurred while reading */
};

/**
 * @brief Represents a single file within the uploader.
 */
struct ui_file_uploader_file {
  char file_name[256];  /**< The name of the file */
  char file_path[1024]; /**< The full path to the file */
  size_t file_size;     /**< The size of the file in bytes */
  unsigned char
      *data; /**< The binary data of the file (dynamically allocated) */
};

/**
 * @brief The base structure for a file uploader / dropzone component.
 */
struct ui_file_uploader_base {
  enum ui_file_uploader_state state;   /**< Current state of the uploader */
  struct ui_file_uploader_file *files; /**< Array of uploaded files */
  int file_count;                      /**< Number of files currently held */
  int max_files;                       /**< Maximum number of files allowed */

  int x;      /**< X coordinate of the dropzone bounding box */
  int y;      /**< Y coordinate of the dropzone bounding box */
  int width;  /**< Width of the dropzone bounding box */
  int height; /**< Height of the dropzone bounding box */

  struct ui_drag_list
      dropzone_list; /**< Internal drag list for hooking into ui_drag_drop */

  /* CVA Integration */
  enum ui_error (*on_change_cb)(union ui_signal_payload new_value,
                                void *user_data);
  void *on_change_user_data;

  enum ui_error (*on_touched_cb)(void *user_data);
  void *on_touched_user_data;

  ui_bool_t is_disabled;
};

/**
 * @brief Initializes a file uploader component.
 * @param uploader Pointer to the uploader structure to initialize.
 * @param max_files Maximum number of files to allow.
 * @param x X coordinate of the dropzone.
 * @param y Y coordinate of the dropzone.
 * @param width Width of the dropzone.
 * @param height Height of the dropzone.
 * @param out_cva Optional CVA output pointer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_file_uploader_init(struct ui_file_uploader_base *uploader,
                                    int max_files, int x, int y, int width,
                                    int height,
                                    struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a file uploader component, freeing all resources.
 * @param uploader Pointer to the uploader structure.
 */
void ui_file_uploader_destroy(struct ui_file_uploader_base *uploader);

/**
 * @brief Simulates a drag enter event (e.g., from an OS file drag).
 * @param uploader Pointer to the uploader structure.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_file_uploader_on_drag_enter(struct ui_file_uploader_base *uploader);

/**
 * @brief Simulates a drag leave event.
 * @param uploader Pointer to the uploader structure.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_file_uploader_on_drag_leave(struct ui_file_uploader_base *uploader);

/**
 * @brief Adds a file to the uploader's list by its path. Does not read it yet.
 * @param uploader Pointer to the uploader structure.
 * @param file_path The path to the dropped file.
 * @return UI_ERROR_NONE on success, UI_ERROR_QUEUE_FULL if max_files is
 * reached.
 */
enum ui_error ui_file_uploader_drop_file(struct ui_file_uploader_base *uploader,
                                         const char *file_path);

/**
 * @brief Reads all newly dropped files from the filesystem securely.
 * @param uploader Pointer to the uploader structure.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_file_uploader_read_files(struct ui_file_uploader_base *uploader);

/**
 * @brief Registers the uploader's dropzone with a drag and drop context.
 * @param uploader Pointer to the uploader structure.
 * @param drag_ctx Pointer to the drag and drop context.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_file_uploader_register_dropzone(struct ui_file_uploader_base *uploader,
                                   struct ui_drag_drop_context *drag_ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FILE_UPLOADER_BASE_H */
