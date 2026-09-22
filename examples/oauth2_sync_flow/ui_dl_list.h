/**
 * @file ui_dl_list.h
 * @brief HTML <dl> description list component for username (<dt>) and bio
 * (<dd>).
 */

#ifndef UI_DL_LIST_H
#define UI_DL_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stddef.h>
#include "ui_error.h"
#include "ui_component.h"
#include "oauth2_types.h"
/* clang-format on */

/**
 * @brief Callback signature invoked when a <dl> row is clicked or activated.
 */
typedef ui_error_t (*ui_dl_list_click_fn)(const char *user_id, void *user_data);

/**
 * @brief Single item in the description list.
 */
struct ui_dl_item {
  char user_id[64];
  char username[64];
  char about_me[512];
  enum oauth2_user_role role;
  enum oauth2_sync_status sync_status;
  ui_dl_list_click_fn on_click;
  void *user_data;
  struct ui_dom_node *row_node;
  struct ui_dom_node *dt_node;
  struct ui_dom_node *dd_node;
};

/**
 * @brief Maximum items rendered concurrently in the description list.
 */
#define UI_DL_LIST_MAX_ITEMS 64

/**
 * @brief Description list component structure.
 */
struct ui_dl_list {
  struct ui_component *component;
  struct ui_dom_node *dl_container;
  struct ui_dl_item items[UI_DL_LIST_MAX_ITEMS];
  size_t count;
  char selected_user_id[64];
};

/**
 * @brief Allocates and initializes the <dl> description list component.
 *
 * @param out_dl Pointer to receive created list instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_dl_list_create(struct ui_dl_list **out_dl);

/**
 * @brief Destroys the <dl> description list component and child DOM nodes.
 *
 * @param dl Pointer to list instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_dl_list_destroy(struct ui_dl_list *dl);

/**
 * @brief Retrieves the root ui_component handle for mounting into the tree.
 *
 * @param dl Pointer to list instance.
 * @param out_comp Pointer to receive ui_component instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_dl_list_get_component(struct ui_dl_list *dl,
                                    struct ui_component **out_comp);

/**
 * @brief Removes all child <dt> and <dd> rows from the description list.
 *
 * @param dl Pointer to list instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_dl_list_clear(struct ui_dl_list *dl);

/**
 * @brief Appends a user row composed of a <dt> and <dd> element.
 *
 * @param dl Pointer to list instance.
 * @param user User data to populate the row.
 * @param on_click Activation callback.
 * @param user_data Context pointer passed to on_click.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_dl_list_add_user(struct ui_dl_list *dl,
                               const struct oauth2_user *user,
                               ui_dl_list_click_fn on_click, void *user_data);

/**
 * @brief Sets the currently highlighted user ID.
 *
 * @param dl Pointer to list instance.
 * @param user_id User identifier to select.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_dl_list_set_selected(struct ui_dl_list *dl, const char *user_id);

/**
 * @brief Simulates clicking/activating a row by user ID.
 *
 * @param dl Pointer to list instance.
 * @param user_id User identifier to activate.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_dl_list_trigger_click(struct ui_dl_list *dl, const char *user_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DL_LIST_H */
