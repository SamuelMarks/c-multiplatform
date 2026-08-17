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
  UI_ARIA_ROLE_NONE = 0,    /**< No role specified */
  UI_ARIA_ROLE_BUTTON,      /**< Button role */
  UI_ARIA_ROLE_CHECKBOX,    /**< Checkbox role */
  UI_ARIA_ROLE_DIALOG,      /**< Dialog role */
  UI_ARIA_ROLE_ALERT,       /**< Alert role */
  UI_ARIA_ROLE_STATUS,      /**< Status role */
  UI_ARIA_ROLE_SLIDER,      /**< Slider role */
  UI_ARIA_ROLE_PROGRESSBAR, /**< Progressbar role */
  UI_ARIA_ROLE_HEADING,     /**< Heading role */
  UI_ARIA_ROLE_LINK,        /**< Link role */
  UI_ARIA_ROLE_TEXTBOX,     /**< Textbox role */
  UI_ARIA_ROLE_SEPARATOR,   /**< Separator role */
  UI_ARIA_ROLE_LIST,        /**< List role */
  UI_ARIA_ROLE_LISTITEM,    /**< Listitem role */
  UI_ARIA_ROLE_RADIOGROUP,  /**< Radiogroup role */
  UI_ARIA_ROLE_TABLIST,     /**< Tablist role */
  UI_ARIA_ROLE_NAVIGATION,  /**< Navigation role */
  UI_ARIA_ROLE_GROUP        /**< Group role */
};

/**
 * @brief Computed semantic state for OS accessibility synchronization.
 */
struct ui_aria_state {
  enum ui_aria_role role; /**< The computed ARIA role */
  int is_hidden;          /**< 1 if aria-hidden="true" */
  int is_disabled;        /**< 1 if disabled or aria-disabled="true" */
  int is_expanded;        /**< -1 = unset, 0 = false, 1 = true */
  int is_checked;         /**< -1 = unset, 0 = false, 1 = true, 2 = mixed */
  char *label; /**< Accessible label (from aria-label), allocated, must be freed
                */
  char *description; /**< Accessible description (from aria-description),
                        allocated, must be freed */
};

/**
 * @brief Maps a string role (e.g., "button") to the ui_aria_role enum.
 *
 * @param role_str The role string.
 * @param out_role Pointer to receive the mapped role enum.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_aria_role_from_string(const char *role_str,
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
ui_error_t ui_aria_state_parse(const struct ui_dom_node *node,
                               struct ui_aria_state *out_state);

/**
 * @brief Cleans up allocated resources within a parsed ARIA state struct.
 *
 * @param state The state struct to clean up.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_aria_state_cleanup(struct ui_aria_state *state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ARIA_H */
