/**
 * @file views.h
 * @brief UI component factories for OAuth2 sync flow routing endpoints.
 */

#ifndef VIEWS_H
#define VIEWS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_router.h"
#include <stddef.h>
/* clang-format on */

struct app_state;

/**
 * @brief View factory for the Login screen (Online/Offline mode toggle).
 *
 * @param req The router request.
 * @param user_data Pointer to the app_state context.
 * @param out_screen Pointer to receive constructed component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t oauth2_view_login_factory(const struct ui_route_request *req,
                                     void *user_data,
                                     struct ui_component **out_screen);

/**
 * @brief View factory for the Profile screen (editable 'About Me' section).
 *
 * @param req The router request.
 * @param user_data Pointer to the app_state context.
 * @param out_screen Pointer to receive constructed component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t oauth2_view_profile_factory(const struct ui_route_request *req,
                                       void *user_data,
                                       struct ui_component **out_screen);

/**
 * @brief View factory for the Admin Directory (<dl> description list view).
 *
 * @param req The router request.
 * @param user_data Pointer to the app_state context.
 * @param out_screen Pointer to receive constructed component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t
oauth2_view_admin_directory_factory(const struct ui_route_request *req,
                                    void *user_data,
                                    struct ui_component **out_screen);

/**
 * @brief View factory for the Single User Detail page.
 *
 * @param req The router request containing the :id path param.
 * @param user_data Pointer to the app_state context.
 * @param out_screen Pointer to receive constructed component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t oauth2_view_user_detail_factory(const struct ui_route_request *req,
                                           void *user_data,
                                           struct ui_component **out_screen);

/**
 * @brief Toggles operating mode between Online and Offline.
 *
 * @param state Pointer to app_state context.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t oauth2_view_toggle_mode(struct app_state *state);

/**
 * @brief Sets the live backend server URL endpoint.
 *
 * @param state Pointer to app_state context.
 * @param url New backend URL string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t oauth2_view_set_backend_url(struct app_state *state,
                                       const char *url);

/**
 * @brief Performs authentication from the login screen.
 *
 * @param state Pointer to app_state context.
 * @param username Credential username.
 * @param password Credential password.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t oauth2_view_submit_login(struct app_state *state,
                                    const char *username, const char *password);

/**
 * @brief Updates user bio description from the profile screen.
 *
 * @param state Pointer to app_state context.
 * @param new_about_me New description text.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t oauth2_view_save_profile(struct app_state *state,
                                    const char *new_about_me);

/**
 * @brief Triggers synchronization of offline dirty records to live server.
 *
 * @param state Pointer to app_state context.
 * @param out_synced_count Pointer to receive count of synced records.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t oauth2_view_sync_changes(struct app_state *state,
                                    size_t *out_synced_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* VIEWS_H */
