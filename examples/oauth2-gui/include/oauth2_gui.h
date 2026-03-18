#ifndef OAUTH2_GUI_H
#define OAUTH2_GUI_H

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_event.h"
#include "cmpc/cmp_router.h"
#include "cmpc/cmp_http_integration.h"
#include "cmpc/cmp_ui_builder.h"

#ifndef C_ABSTRACT_HTTP_NO_MULTIPLATFORM_INTEGRATION
#define C_ABSTRACT_HTTP_NO_MULTIPLATFORM_INTEGRATION
#endif
#include <c_abstract_http/c_abstract_http.h>

#include <c_orm_oauth2.h>
/* clang-format on */

/**
 * @brief OAuth2 GUI application state structure.
 */
typedef struct OAuth2Gui {
  CMPRouter router;              /**< Router instance for navigation */
  CMPHttpIntegration http;       /**< HTTP integration state */
  CMPEventDispatcher dispatcher; /**< Event dispatcher */

  char *server_url;   /**< OAuth2 server URL */
  char *access_token; /**< Currently valid access token */
  char *error_msg;    /**< Last error message */

  CMPWidget *username_field;   /**< Field for entering username */
  CMPWidget *password_field;   /**< Field for entering password */
  CMPWidget *server_url_field; /**< Field for entering server URL */
  CMPWidget *login_button;     /**< Button to trigger login */

  CMPWidget *logout_button; /**< Button to trigger logout */
  CMPWidget *token_label;   /**< Label displaying the token */

  struct HttpClient *client;   /**< HTTP client instance */
  struct HttpFuture *future;   /**< HTTP future for pending requests */
  struct HttpRequest *request; /**< Current HTTP request */
} OAuth2Gui;

/**
 * @brief Initialize the OAuth2 GUI application state.
 * @param gui Pointer to the GUI state.
 * @return 0 on success, or an error code.
 */
extern int oauth2_gui_init(OAuth2Gui *gui);

/**
 * @brief Free resources associated with the OAuth2 GUI.
 * @param gui Pointer to the GUI state.
 * @return 0 on success, or an error code.
 */
extern int oauth2_gui_shutdown(OAuth2Gui *gui);

/**
 * @brief Process one tick of the GUI (event loop, http async polling).
 * @param gui Pointer to the GUI state.
 * @return 0 on success, or an error code.
 */
extern int oauth2_gui_tick(OAuth2Gui *gui);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OAUTH2_GUI_H */