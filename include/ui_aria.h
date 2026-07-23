#ifndef UI_ARIA_H
#define UI_ARIA_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
/* clang-format on */

/**
 * @brief Standard ARIA roles for FFI synchronization to OS screen readers.
 */
enum ui_aria_role {
  UI_ARIA_ROLE_NONE = 0,
  UI_ARIA_ROLE_BUTTON,
  UI_ARIA_ROLE_CHECKBOX,
  UI_ARIA_ROLE_DIALOG,
  UI_ARIA_ROLE_ALERT,
  UI_ARIA_ROLE_STATUS,
  UI_ARIA_ROLE_SLIDER,
  UI_ARIA_ROLE_PROGRESSBAR,
  UI_ARIA_ROLE_HEADING,
  UI_ARIA_ROLE_LINK,
  UI_ARIA_ROLE_TEXTBOX,
  UI_ARIA_ROLE_SEPARATOR,
  UI_ARIA_ROLE_LIST,
  UI_ARIA_ROLE_LISTITEM,
  UI_ARIA_ROLE_RADIOGROUP,
  UI_ARIA_ROLE_TABLIST,
  UI_ARIA_ROLE_NAVIGATION,
  UI_ARIA_ROLE_GROUP
};

/**
 * @brief Computed semantic state for OS accessibility synchronization.
 */
struct ui_aria_state {
  enum ui_aria_role role;
  int is_hidden;   /**< 1 if aria-hidden="true" */
  int is_disabled; /**< 1 if disabled or aria-disabled="true" */
  int is_expanded; /**< -1 = unset, 0 = false, 1 = true */
  int is_checked;  /**< -1 = unset, 0 = false, 1 = true, 2 = mixed */
  char *label; /**< Accessible label (from aria-label), allocated, must be freed
                */
  char *description; /**< Accessible description (from aria-description),
                        allocated, must be freed */
};

/**
 * @brief Maps a string role (e.g., "button") to the ui_aria_role enum.
 *
 * @param role_str The role string.
 * @return The corresponding enum ui_aria_role, or UI_ARIA_ROLE_NONE.
 */
enum ui_error ui_aria_role_from_string(const char *role_str,
                                       enum ui_aria_role *out_role);

/**
 * @brief Parses the ARIA and semantic attributes of a DOM node into a state
 * struct. The caller is responsible for calling ui_aria_state_cleanup when
 * done.
 *
 * @param node The DOM node to evaluate.
 * @param out_state Pointer to the state struct to populate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_aria_state_parse(const struct ui_dom_node *node,
                                  struct ui_aria_state *out_state);

/**
 * @brief Cleans up allocated resources within a parsed ARIA state struct.
 *
 * @param state The state struct to clean up.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_aria_state_cleanup(struct ui_aria_state *state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ARIA_H */
