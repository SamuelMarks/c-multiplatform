#ifndef UI_PORTAL_H
#define UI_PORTAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
/* clang-format on */

struct ui_portal;

/**
 * @brief Creates a new portal to manage the rendering of content in a separate
 * physical tree location.
 *
 * @param out_portal Pointer to receive the allocated portal.
 * @param physical_target The node in the physical DOM where the content will be
 * rendered.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_portal_create(struct ui_portal **out_portal,
                               struct ui_dom_node *physical_target);

/**
 * @brief Destroys a portal, safely unmounting and destroying its content to
 * prevent memory leaks.
 *
 * @param portal The portal to destroy.
 */
void ui_portal_destroy(struct ui_portal *portal);

/**
 * @brief Sets the content of the portal. This appends the content to the
 * physical target. If the portal already has content, the old content is
 * unmounted and destroyed.
 *
 * @param portal The portal.
 * @param content_node The node to render within the portal.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_portal_set_content(struct ui_portal *portal,
                                    struct ui_dom_node *content_node);

/**
 * @brief Retrieves the content node of the portal.
 *
 * @param portal The portal.
 * @param out_content Pointer to receive the content node.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_portal_get_content(const struct ui_portal *portal,
                                    struct ui_dom_node **out_content);

#ifdef __cplusplus
}
#endif

#endif /* UI_PORTAL_H */
