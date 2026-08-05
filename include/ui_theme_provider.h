#ifndef UI_THEME_PROVIDER_H
#define UI_THEME_PROVIDER_H

/* clang-format off */
#include "ui_design_tokens.h"
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a theme context in the UI tree.
 */
struct ui_theme_provider {
  struct ui_component base;
  struct ui_design_token_dict *tokens;
};

/**
 * @brief Creates a new theme provider component.
 *
 * @param arena The memory arena to allocate from.
 * @param tokens The design token dictionary to provide to children.
 * @param out_provider Pointer to output the initialized provider.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_theme_provider_create(struct ui_arena *arena,
                                    struct ui_design_token_dict *tokens,
                                    struct ui_theme_provider **out_provider);

/**
 * @brief Mounts the theme provider to a DOM node, making its tokens available
 * to that subtree.
 *
 * @param provider The theme provider.
 * @param node The DOM node to mount to.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_theme_provider_mount(struct ui_theme_provider *provider,
                                   struct ui_dom_node *node);

/**
 * @brief Retrieves the closest design token dictionary from the component's
 * ancestry.
 *
 * @param node The DOM node of the component.
 * @param out_tokens Pointer to store the resolved token dictionary.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if no theme provider is
 * found.
 */
ui_error_t ui_theme_provider_get(struct ui_dom_node *node,
                                 struct ui_design_token_dict **out_tokens);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_THEME_PROVIDER_H */
