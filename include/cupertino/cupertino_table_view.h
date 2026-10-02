/**
 * @file cupertino_table_view.h
 * @brief Cupertino Table View, Swipe Actions, Edit/Reorder & Section Index
 * Scrub Bar.
 */

#ifndef CUPERTINO_CUPERTINO_TABLE_VIEW_H
#define CUPERTINO_CUPERTINO_TABLE_VIEW_H

/* clang-format off */
#include "cupertino/cupertino_list_section.h"
#include "cupertino/cupertino_list_tile.h"
#include "ui_list_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_TABLE_VIEW_MAX_SECTIONS 16
#define CUPERTINO_TABLE_VIEW_MAX_SWIPE_ACTIONS 4
#define CUPERTINO_TABLE_VIEW_MAX_INDEX_TITLES 32

/**
 * @enum cupertino_table_view_style
 * @brief Table view visual presentation styles.
 */
enum cupertino_table_view_style {
  CUPERTINO_TABLE_VIEW_PLAIN =
      0, /**< Plain edge-to-edge style with sticky headers. */
  CUPERTINO_TABLE_VIEW_GROUPED, /**< Grouped non-inset style with gray section
                                   backgrounds. */
  CUPERTINO_TABLE_VIEW_INSET_GROUPED /**< Modern rounded squircle cards with
                                        16pt margins. */
};

/**
 * @enum cupertino_swipe_action_style
 * @brief Swipe action button semantic style.
 */
enum cupertino_swipe_action_style {
  CUPERTINO_SWIPE_ACTION_NORMAL = 0,  /**< Standard action (Blue/Gray). */
  CUPERTINO_SWIPE_ACTION_DESTRUCTIVE, /**< Destructive action (Red). */
  CUPERTINO_SWIPE_ACTION_FLAG,        /**< Flag action (Orange). */
  CUPERTINO_SWIPE_ACTION_MORE         /**< More / options action (Gray). */
};

/**
 * @struct cupertino_swipe_action
 * @brief Configuration for a single swipe action button.
 */
struct cupertino_swipe_action {
  char title[32];                          /**< Button title text. */
  char icon[32];                           /**< SF symbol icon name. */
  enum cupertino_swipe_action_style style; /**< Semantic action style. */
  int (*handler)(void *user_data);         /**< Callback handler on trigger. */
  void *user_data;                         /**< User context pointer. */
};

/**
 * @struct cupertino_table_view_descriptor
 * @brief Configuration descriptor for creating a Cupertino Table View.
 */
struct cupertino_table_view_descriptor {
  enum cupertino_table_view_style style; /**< Presentation style. */
  int edit_mode; /**< Initial edit mode state (0 or 1). */
  int is_rtl;    /**< Layout direction (1 for RTL). */
};

/**
 * @struct cupertino_section_index_scrub
 * @brief Trailing alphabet index scrub bar (A-Z, #).
 */
struct cupertino_section_index_scrub {
  char titles[CUPERTINO_TABLE_VIEW_MAX_INDEX_TITLES]
             [8];             /**< Index title strings. */
  size_t count;               /**< Number of index titles. */
  int is_active;              /**< Touch active on scrub bar. */
  int selected_index;         /**< Currently scrubbed index (-1 if none). */
  float touch_y;              /**< Current touch Y position. */
  int show_magnifying_bubble; /**< Display magnifying letter bubble. */
};

/**
 * @struct cupertino_table_view
 * @brief Cupertino Table View instance wrapping ui_list_base.
 */
struct cupertino_table_view {
  struct ui_list_base *base;             /**< Underlying CDK list primitive. */
  enum cupertino_table_view_style style; /**< Presentation style. */
  int edit_mode; /**< Edit mode active (reorder/delete). */
  int is_rtl;    /**< RTL layout flag. */

  size_t section_count; /**< Number of sections. */
  struct cupertino_list_section
      *sections[CUPERTINO_TABLE_VIEW_MAX_SECTIONS]; /**< Sections. */

  /* Swipe state */
  int swiped_section;          /**< Currently swiped section (-1 if none). */
  int swiped_row;              /**< Currently swiped row (-1 if none). */
  float swipe_offset;          /**< Horizontal swipe distance in pt. */
  size_t leading_action_count; /**< Number of leading actions. */
  struct cupertino_swipe_action
      leading_actions[CUPERTINO_TABLE_VIEW_MAX_SWIPE_ACTIONS]; /**< Leading
                                                                  actions. */
  size_t trailing_action_count; /**< Number of trailing actions. */
  struct cupertino_swipe_action
      trailing_actions[CUPERTINO_TABLE_VIEW_MAX_SWIPE_ACTIONS]; /**< Trailing
                                                                   actions. */
  int full_swipe_triggered; /**< Flag if full-swipe (>60% width) triggered. */

  /* Section Index Scrub Bar */
  struct cupertino_section_index_scrub index_scrub; /**< Alphabet scrub bar. */

  /* Reorder State */
  int reorder_active;       /**< Currently dragging a row. */
  int reorder_from_section; /**< Drag source section. */
  int reorder_from_row;     /**< Drag source row. */
  int reorder_to_section;   /**< Current target section. */
  int reorder_to_row;       /**< Current target row. */
};

/**
 * @brief Creates a new Cupertino table view instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_table Pointer to receive newly created table view.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_create(struct ui_engine *engine,
                            const struct cupertino_table_view_descriptor *desc,
                            struct cupertino_table_view **out_table);

/**
 * @brief Destroys a Cupertino table view instance.
 *
 * @param table Table view instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_destroy(struct cupertino_table_view *table);

/**
 * @brief Adds a section to the table view.
 *
 * @param table Table view instance.
 * @param section Section instance to add.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if full, or
 * UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_table_view_add_section(
    struct cupertino_table_view *table, struct cupertino_list_section *section);

/**
 * @brief Retrieves section count.
 *
 * @param table Table view instance.
 * @param out_count Pointer to receive section count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_table_view_get_section_count(
    const struct cupertino_table_view *table, size_t *out_count);

/**
 * @brief Retrieves a section by index.
 *
 * @param table Table view instance.
 * @param index 0-based section index.
 * @param out_section Pointer to receive section pointer.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if index invalid, or
 * UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_table_view_get_section(
    const struct cupertino_table_view *table, size_t index,
    struct cupertino_list_section **out_section);

/**
 * @brief Toggles or sets edit mode.
 *
 * @param table Table view instance.
 * @param edit_mode 1 to enable edit mode, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_table_view_set_edit_mode(
    struct cupertino_table_view *table, int edit_mode);

/**
 * @brief Retrieves edit mode status.
 *
 * @param table Table view instance.
 * @param out_edit_mode Pointer to receive edit mode flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_table_view_get_edit_mode(
    const struct cupertino_table_view *table, int *out_edit_mode);

/**
 * @brief Adds a leading swipe action button.
 *
 * @param table Table view instance.
 * @param action Swipe action descriptor.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if full, or
 * UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_add_leading_swipe_action(
    struct cupertino_table_view *table,
    const struct cupertino_swipe_action *action);

/**
 * @brief Adds a trailing swipe action button.
 *
 * @param table Table view instance.
 * @param action Swipe action descriptor.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if full, or
 * UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_add_trailing_swipe_action(
    struct cupertino_table_view *table,
    const struct cupertino_swipe_action *action);

/**
 * @brief Clears all leading and trailing swipe actions.
 *
 * @param table Table view instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_clear_swipe_actions(struct cupertino_table_view *table);

/**
 * @brief Simulates pan swipe gesture on a row.
 *
 * @param table Table view instance.
 * @param section Section index.
 * @param row Row index within section.
 * @param offset Horizontal delta in pt (positive for leading, negative for
 * trailing).
 * @param row_width Width of row in pt for full-swipe threshold calculation.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_swipe_pan(struct cupertino_table_view *table, int section,
                               int row, float offset, float row_width);

/**
 * @brief Concludes swipe gesture with release or cancel.
 *
 * @param table Table view instance.
 * @param row_width Width of row in pt.
 * @param out_action_triggered Pointer to receive 1 if full swipe action was
 * triggered, 0 otherwise.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_swipe_release(struct cupertino_table_view *table,
                                   float row_width, int *out_action_triggered);

/**
 * @brief Sets index scrub titles (e.g. A-Z, #).
 *
 * @param table Table view instance.
 * @param titles Array of title strings.
 * @param count Number of titles (up to CUPERTINO_TABLE_VIEW_MAX_INDEX_TITLES).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_set_index_titles(struct cupertino_table_view *table,
                                      const char *const *titles, size_t count);

/**
 * @brief Handles touch scrubbing along the index bar.
 *
 * @param table Table view instance.
 * @param touch_y Y coordinate within scrub bar height.
 * @param bar_height Total height of scrub bar in pt.
 * @param out_selected_section Pointer to receive matched section/title index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_table_view_scrub_index(
    struct cupertino_table_view *table, float touch_y, float bar_height,
    int *out_selected_section);

/**
 * @brief Releases index scrub bar touch.
 *
 * @param table Table view instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_scrub_release(struct cupertino_table_view *table);

/**
 * @brief Begins interactive row drag reordering in edit mode.
 *
 * @param table Table view instance.
 * @param section Source section index.
 * @param row Source row index within section.
 * @return UI_ERROR_NONE on success, UI_ERROR_UNSUPPORTED if not in edit mode,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_table_view_reorder_begin(
    struct cupertino_table_view *table, int section, int row);

/**
 * @brief Updates target position during row drag reordering.
 *
 * @param table Table view instance.
 * @param to_section Target section index.
 * @param to_row Target row index.
 * @return UI_ERROR_NONE on success, UI_ERROR_UNSUPPORTED if reorder not active,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_table_view_reorder_update(
    struct cupertino_table_view *table, int to_section, int to_row);

/**
 * @brief Commits row reorder, moving the tile in the data structure.
 *
 * @param table Table view instance.
 * @return UI_ERROR_NONE on success, UI_ERROR_UNSUPPORTED if reorder not active,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_reorder_commit(struct cupertino_table_view *table);

/**
 * @brief Cancels active row reorder.
 *
 * @param table Table view instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_reorder_cancel(struct cupertino_table_view *table);

/**
 * @brief Calculates separator leading inset according to Apple HIG.
 *
 * For Plain style: separator insets to match cell content leading (default
 * 16pt, or 56pt with icon). For Inset Grouped style: last cell of card has NO
 * separator (insets to infinity / hidden).
 *
 * @param table Table view instance.
 * @param section Section index.
 * @param row Row index.
 * @param out_inset Pointer to receive leading inset in pt.
 * @param out_hidden Pointer to receive 1 if separator should be hidden, 0 if
 * visible.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_table_view_get_separator_layout(
    const struct cupertino_table_view *table, size_t section, size_t row,
    float *out_inset, int *out_hidden);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_CUPERTINO_TABLE_VIEW_H */
