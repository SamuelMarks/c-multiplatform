/**
 * @file views.h
 * @brief UI Component factories for the routing endpoints.
 */
#ifndef VIEWS_H
#define VIEWS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Forward declaration of the component structure.
 */
struct ui_component;

/**
 * @brief Forward declaration of the route request structure.
 */
struct ui_route_request;

/**
 * @brief View factory for the Auth Container (Login/Signup tabs).
 *
 * @param req The router request.
 * @param user_data Pointer to the application state context.
 * @param out_screen Pointer to receive the constructed auth component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t auth_container_factory(const struct ui_route_request *req,
                                  void *user_data,
                                  struct ui_component **out_screen);

/**
 * @brief View factory for the Secrets dashboard.
 *
 * @param req The router request.
 * @param user_data Pointer to the application state context.
 * @param out_screen Pointer to receive the constructed secrets component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t secrets_view_factory(const struct ui_route_request *req,
                                void *user_data,
                                struct ui_component **out_screen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* VIEWS_H */
