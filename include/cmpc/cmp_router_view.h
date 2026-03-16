#ifndef CMP_ROUTER_VIEW_H
#define CMP_ROUTER_VIEW_H

/**
 * @file cmp_router_view.h
 * @brief Router view component for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_router.h"
/* clang-format on */

/**
 * @brief Router view component.
 *
 * Automatically displays the current component of a given router.
 */
typedef struct CMPRouterView {
  CMPWidget widget;        /**< Widget interface. */
  CMPRect bounds;          /**< Layout bounds. */
  const CMPRouter *router; /**< Router to monitor. */
} CMPRouterView;

/**
 * @brief Initialize a router view.
 * @param view View instance.
 * @param router Router to monitor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_router_view_init(CMPRouterView *view,
                                          const CMPRouter *router);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_ROUTER_VIEW_H */
