/**
 * @file views.c
 * @brief Implementation of the UI component factories for auth flow routing.
 */

/* clang-format off */
#include "views.h"
#include "i18n_dict.h"
#include "mock_backend.h"
#include "ui_button_base.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "ui_i18n.h"
#include "ui_input_base.h"
#include "ui_router.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Internal duplicate of the application state structure for view factory
 * logic.
 */
struct app_state {
  void *arena;
  void *theme_manager;
  void *i18n;
  struct ui_router *router;
  void *root;
  void *toolbar;
  void *toolbar_title;
  void *router_outlet;
  void *btn_theme;
  void *btn_lang;
  void *host_theme;
  void *host_lang;
  int current_lang_idx;
  int is_authenticated;
  char current_user[64];
  char auth_error_message[128];

  struct ui_input_base *input_username;
  struct ui_input_base *input_password;
  struct ui_button_base *btn_submit;
  struct ui_button_base *btn_logout;
};

/**
 * @brief Callback for the login form submission.
 * @param button The submit button instance.
 * @param user_data Application state context.
 */
static ui_error_t on_login_submit(struct ui_button_base *button,
                                  void *user_data) {
  const char *locale = NULL;
  struct app_state *state = (struct app_state *)user_data;
  const char *user = "";
  const char *pass = "";
  ui_error_t err;

  err = ui_input_base_get_text(state->input_username, &user);
  if (err != UI_ERROR_NONE)
    return err;

  err = ui_i18n_get_locale(state->i18n, &locale);
  if (err != UI_ERROR_NONE)
    return err;

  err = ui_input_base_get_text(state->input_password, &pass);
  if (err != UI_ERROR_NONE)
    return err;

  (void)button;

  err = mock_login((void *)state, user ? user : "", pass ? pass : "");
  if (err == UI_ERROR_NONE) {
    ui_error_t nav_err = ui_router_navigate(state->router, "/secrets");
    if (nav_err != UI_ERROR_NONE)
      return nav_err;
    return err;
  } else {
    ui_error_t nav_err =
        ui_router_navigate(state->router, "/login"); /* Force refresh for now */
    if (nav_err != UI_ERROR_NONE)
      return nav_err;
    return err;
  }
}

/**
 * @brief Callback for the signup form submission.
 * @param button The submit button instance.
 * @param user_data Application state context.
 */
static ui_error_t on_signup_submit(struct ui_button_base *button,
                                   void *user_data) {
  const char *locale = NULL;
  struct app_state *state = (struct app_state *)user_data;
  const char *user = "";
  const char *pass = "";
  ui_error_t err;

  err = ui_input_base_get_text(state->input_username, &user);
  if (err != UI_ERROR_NONE)
    return err;

  err = ui_i18n_get_locale(state->i18n, &locale);
  if (err != UI_ERROR_NONE)
    return err;

  err = ui_input_base_get_text(state->input_password, &pass);
  if (err != UI_ERROR_NONE)
    return err;

  (void)button;

  err = mock_signup((void *)state, user ? user : "", pass ? pass : "");
  if (err == UI_ERROR_NONE) {
    ui_error_t nav_err = ui_router_navigate(state->router, "/secrets");
    if (nav_err != UI_ERROR_NONE)
      return nav_err;
    return err;
  } else {
    ui_error_t nav_err = ui_router_navigate(
        state->router, "/signup"); /* Force refresh for now */
    if (nav_err != UI_ERROR_NONE)
      return nav_err;
    return err;
  }
}

/**
 * @brief Callback for the logout button.
 * @param button The logout button instance.
 * @param user_data Application state context.
 */
static ui_error_t on_logout(struct ui_button_base *button, void *user_data) {
  const char *locale = NULL;
  struct app_state *state = (struct app_state *)user_data;
  ui_error_t err;
  (void)button;

  err = ui_i18n_get_locale(state->i18n, &locale);
  if (err != UI_ERROR_NONE)
    return err;

  err = mock_logout((void *)state);
  if (err != UI_ERROR_NONE)
    return err;

  err = ui_router_navigate(state->router, "/login");
  if (err != UI_ERROR_NONE)
    return err;
  return UI_ERROR_NONE;
}

ui_error_t auth_container_factory(const struct ui_route_request *req,
                                  void *user_data,
                                  struct ui_component **out_screen) {
  struct ui_component *comp;
  struct ui_dom_node *root, *tablist, *tab_login, *tab_signup, *content, *form,
      *lbl_user, *host_user, *lbl_pass, *host_pass, *host_submit, *err_msg;
  const char *path = NULL;
  ui_error_t err;

  err = ui_route_request_get_path(req, &path);
  if (err != UI_ERROR_NONE)
    return err;

  const char *locale = NULL;
  struct app_state *state = (struct app_state *)user_data;
  int is_signup = 0;

  err = ui_i18n_get_locale(state->i18n, &locale);
  if (err != UI_ERROR_NONE)
    return err;

  if (state->is_authenticated) {
    /* Need to defer navigation, returning empty node for now */
  }

  if (strstr(path, "/signup")) {
    is_signup = 1;
  }

  err = ui_component_create(&comp);
  if (err != UI_ERROR_NONE)
    return err;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(root, "div");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(root, "class", "auth-container");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tablist);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(tablist, "div");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(tablist, "role", "tablist");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(root, tablist);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tab_login);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(tab_login, "a");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(tab_login, "role", "tab");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(tab_login, "href", "/login");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  const char *tab_login_str = NULL;
  err = get_translated_string(locale, "tab_login", &tab_login_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_text_content(tab_login, tab_login_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(tablist, tab_login);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tab_signup);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(tab_signup, "a");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(tab_signup, "role", "tab");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(tab_signup, "href", "/signup");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  const char *tab_signup_str = NULL;
  err = get_translated_string(locale, "tab_signup", &tab_signup_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_text_content(tab_signup, tab_signup_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(tablist, tab_signup);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(content, "div");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(content, "class", "auth-content");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &form);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(form, "div");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(form, "class", "auth-form");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(content, form);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  /* Username Field */
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &lbl_user);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(lbl_user, "label");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(lbl_user, "for", "input-user");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  const char *lbl_username_str = NULL;
  err = get_translated_string(locale, "lbl_username", &lbl_username_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_text_content(lbl_user, lbl_username_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(form, lbl_user);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host_user);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(host_user, "div");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(host_user, "id", "input-user");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(form, host_user);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  if (!state->input_username) {
    err = ui_input_base_create(&state->input_username);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  }
  {
    struct ui_component *tmp_comp;
    err = ui_input_base_get_component(state->input_username, &tmp_comp);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_component_mount(tmp_comp, host_user);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  }

  /* Password Field */
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &lbl_pass);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(lbl_pass, "label");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(lbl_pass, "for", "input-pass");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  const char *lbl_password_str = NULL;
  err = get_translated_string(locale, "lbl_password", &lbl_password_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_text_content(lbl_pass, lbl_password_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(form, lbl_pass);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host_pass);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(host_pass, "div");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(host_pass, "id", "input-pass");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(form, host_pass);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  if (!state->input_password) {
    err = ui_input_base_create(&state->input_password);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  }
  err = ui_input_base_set_type(state->input_password, "password");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  {
    struct ui_component *tmp_comp;
    err = ui_input_base_get_component(state->input_password, &tmp_comp);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_component_mount(tmp_comp, host_pass);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  }

  /* Submit Button */
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host_submit);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(host_submit, "div");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(form, host_submit);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  if (!state->btn_submit) {
    err = ui_button_base_create(&state->btn_submit);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  }

  if (is_signup) {
    err = ui_dom_node_set_attribute(tab_signup, "aria-selected", "true");
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_dom_node_set_attribute(tab_login, "aria-selected", "false");
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err =
        ui_button_base_set_on_click(state->btn_submit, on_signup_submit, state);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    const char *btn_submit_signup_str = NULL;
    err = get_translated_string(locale, "btn_submit_signup",
                                &btn_submit_signup_str);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_dom_node_set_text_content(host_submit, btn_submit_signup_str);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  } else {
    err = ui_dom_node_set_attribute(tab_login, "aria-selected", "true");
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_dom_node_set_attribute(tab_signup, "aria-selected", "false");
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err =
        ui_button_base_set_on_click(state->btn_submit, on_login_submit, state);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    const char *btn_submit_login_str = NULL;
    err = get_translated_string(locale, "btn_submit_login",
                                &btn_submit_login_str);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_dom_node_set_text_content(host_submit, btn_submit_login_str);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  }
  {
    struct ui_component *tmp_comp;
    err = ui_button_base_get_component(state->btn_submit, &tmp_comp);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_component_mount(tmp_comp, host_submit);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  }

  /* Error Message */
  if (state->auth_error_message[0] != '\0') {
    err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &err_msg);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_dom_node_set_tag_name(err_msg, "div");
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_dom_node_set_attribute(err_msg, "class", "error-msg");
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_dom_node_set_attribute(err_msg, "aria-live", "assertive");
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_dom_node_set_text_content(err_msg, state->auth_error_message);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_dom_node_append_child(form, err_msg);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  }

  err = ui_dom_node_append_child(root, content);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  comp->shadow_root = root;
  *out_screen = comp;
  return UI_ERROR_NONE;

cleanup_comp:
  ui_component_destroy(comp);
  return err;
}

ui_error_t secrets_view_factory(const struct ui_route_request *req,
                                void *user_data,
                                struct ui_component **out_screen) {
  struct ui_component *comp;
  struct ui_dom_node *root, *msg, *host_logout;

  const char *locale = NULL;
  struct app_state *state = (struct app_state *)user_data;
  char welcome_msg[256];
  ui_error_t err;

  err = ui_i18n_get_locale(state->i18n, &locale);
  if (err != UI_ERROR_NONE)
    return err;

  if (!state->is_authenticated) {
    /* Route guard failed */
  }

  err = ui_component_create(&comp);
  if (err != UI_ERROR_NONE)
    return err;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(root, "div");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_attribute(root, "class", "secrets-container");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &msg);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(msg, "h1");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  const char *msg_welcome_secret_str = NULL;
  err = get_translated_string(locale, "msg_welcome_secret",
                              &msg_welcome_secret_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
#if defined(_MSC_VER)
  sprintf_s(welcome_msg, sizeof(welcome_msg), msg_welcome_secret_str,
            state->current_user);
#else
  sprintf(welcome_msg, msg_welcome_secret_str, state->current_user);
#endif
  err = ui_dom_node_set_text_content(msg, welcome_msg);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(root, msg);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host_logout);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_tag_name(host_logout, "div");
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  const char *btn_logout_str = NULL;
  err = get_translated_string(locale, "btn_logout", &btn_logout_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_set_text_content(host_logout, btn_logout_str);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  err = ui_dom_node_append_child(root, host_logout);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;

  if (!state->btn_logout) {
    err = ui_button_base_create(&state->btn_logout);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  }
  err = ui_button_base_set_on_click(state->btn_logout, on_logout, state);
  if (err != UI_ERROR_NONE)
    goto cleanup_comp;
  {
    struct ui_component *tmp_comp;
    err = ui_button_base_get_component(state->btn_logout, &tmp_comp);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
    err = ui_component_mount(tmp_comp, host_logout);
    if (err != UI_ERROR_NONE)
      goto cleanup_comp;
  }

  comp->shadow_root = root;
  *out_screen = comp;
  return UI_ERROR_NONE;

cleanup_comp:
  ui_component_destroy(comp);
  return err;
}
