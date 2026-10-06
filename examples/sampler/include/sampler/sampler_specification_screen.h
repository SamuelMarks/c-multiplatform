/**
 * @file sampler_specification_screen.h
 * @brief Specification selection screen composing md3_top_app_bar and md3_card
 * widgets.
 */

#ifndef SAMPLER_SPECIFICATION_SCREEN_H
#define SAMPLER_SPECIFICATION_SCREEN_H

/* clang-format off */
#include "sampler/sampler_error.h"
#include "sampler/sampler_nav.h"
#include "material3/md3_card.h"
#include "material3/md3_navigation.h"
#include "material3/md3_top_app_bar.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct ui_engine;
struct ui_dom_node;
struct sampler_specification_screen;

/**
 * @brief Construct the complete Specification selection screen composing M3
 * widgets.
 * @param engine UI engine instance.
 * @param nav Active navigation router handle for routing upon selection.
 * @param out_screen Pointer receiving newly created specification screen
 * instance.
 * @return SAMPLER_SUCCESS on success, or appropriate error code.
 */
sampler_error_t sampler_specification_screen_create(
    struct ui_engine *engine, struct sampler_nav *nav,
    struct sampler_specification_screen **out_screen);

/**
 * @brief Retrieve the root DOM node of the specification screen hierarchy.
 * @param screen Specification screen handle.
 * @param out_root Pointer receiving the root DOM node.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_specification_screen_get_root(
    const struct sampler_specification_screen *screen,
    struct ui_dom_node **out_root);

/**
 * @brief Destroy the specification screen and all child widgets.
 * @param screen Double pointer to specification screen instance to destroy and
 * nullify.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_specification_screen_destroy(
    struct sampler_specification_screen **screen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SAMPLER_SPECIFICATION_SCREEN_H */
