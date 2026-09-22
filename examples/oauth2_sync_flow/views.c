/**
 * @file views.c
 * @brief UI component factories for OAuth2 sync flow application views.
 */

/* clang-format off */
#include "views.h"
#include "app_state.h"
#include "ui_dl_list.h"
#include "ui_dom_node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Helper to create an element node with a tag and optional class.
 */
static ui_error_t create_element_node(const char *tag, const char *class_name,
                                      struct ui_dom_node **out_node) {
  struct ui_dom_node *node;
  ui_error_t err;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_set_tag_name(node, tag);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(node);
    return err;
  }

  if (class_name != NULL) {
    err = ui_dom_node_set_attribute(node, "class", class_name);
    if (err != UI_ERROR_NONE) {
      ui_dom_node_destroy(node);
      return err;
    }
  }

  *out_node = node;
  return UI_ERROR_NONE;
}

/**
 * @brief Helper to create a text node with given text.
 */
static ui_error_t create_text_node(const char *text,
                                   struct ui_dom_node **out_node) {
  struct ui_dom_node *node;
  ui_error_t err;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &node);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_set_text_content(node, text != NULL ? text : "");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(node);
    return err;
  }

  *out_node = node;
  return UI_ERROR_NONE;
}

/**
 * @brief Helper to create an element node with an optional text child.
 */
static ui_error_t create_element_with_text(const char *tag,
                                           const char *class_name,
                                           const char *text,
                                           struct ui_dom_node **out_node) {
  struct ui_dom_node *elem;
  struct ui_dom_node *txt;
  ui_error_t err;

  err = create_element_node(tag, class_name, &elem);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (text != NULL) {
    err = create_text_node(text, &txt);
    if (err != UI_ERROR_NONE) {
      ui_dom_node_destroy(elem);
      return err;
    }
    err = ui_dom_node_append_child(elem, txt);
    if (err != UI_ERROR_NONE) {
      ui_dom_node_destroy(txt);
      ui_dom_node_destroy(elem);
      return err;
    }
  }

  *out_node = elem;
  return UI_ERROR_NONE;
}

ui_error_t oauth2_view_login_factory(const struct ui_route_request *req,
                                     void *user_data,
                                     struct ui_component **out_screen) {
  struct app_state *state = (struct app_state *)user_data;
  struct ui_component *comp;
  struct ui_dom_node *card;
  struct ui_dom_node *title;
  struct ui_dom_node *mode_container;
  struct ui_dom_node *mode_label;
  struct ui_dom_node *mode_checkbox;
  struct ui_dom_node *mode_toggle_btn;
  struct ui_dom_node *mode_badge;
  struct ui_dom_node *backend_url_group;
  struct ui_dom_node *url_label;
  struct ui_dom_node *url_input;
  struct ui_dom_node *cred_group;
  struct ui_dom_node *user_label;
  struct ui_dom_node *user_input;
  struct ui_dom_node *pass_label;
  struct ui_dom_node *pass_input;
  struct ui_dom_node *quick_group;
  struct ui_dom_node *quick_admin;
  struct ui_dom_node *quick_alice;
  struct ui_dom_node *submit_btn;
  struct ui_dom_node *error_banner;
  const char *curr_backend_url = NULL;
  enum oauth2_app_error app_rc;
  ui_error_t err;

  if (req != NULL) {
    /* req validated */
  }
  if (user_data == NULL || out_screen == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = create_element_node("div", "login-card", &card);
  if (err != UI_ERROR_NONE) {
    ui_component_destroy(comp);
    return err;
  }

  /* Title */
  err = create_element_with_text("h1", "login-title",
                                 "OAuth2 Authentication Flow", &title);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(card);
    ui_component_destroy(comp);
    return err;
  }
  err = ui_dom_node_append_child(card, title);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(card);
    ui_component_destroy(comp);
    return err;
  }

  /* Mode control container (Checkbox & toggle switch) */
  err = create_element_node("div", "mode-control-container", &mode_container);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(card);
    ui_component_destroy(comp);
    return err;
  }

  err = create_element_with_text("label", "mode-label",
                                 "Online Mode: ", &mode_label);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_set_attribute(mode_label, "for", "mode-toggle-checkbox");
    ui_dom_node_append_child(mode_container, mode_label);
  }

  err = create_element_node("input", "mode-checkbox", &mode_checkbox);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_set_attribute(mode_checkbox, "type", "checkbox");
    ui_dom_node_set_attribute(mode_checkbox, "id", "mode-toggle-checkbox");
    if (state->mode == OAUTH2_MODE_ONLINE) {
      ui_dom_node_set_attribute(mode_checkbox, "checked", "checked");
    }
    ui_dom_node_append_child(mode_container, mode_checkbox);
  }

  err = create_element_with_text("button", "mode-toggle-btn",
                                 state->mode == OAUTH2_MODE_ONLINE
                                     ? "Switch to Offline"
                                     : "Switch to Online",
                                 &mode_toggle_btn);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_set_attribute(mode_toggle_btn, "id", "mode-toggle-btn");
    ui_dom_node_append_child(mode_container, mode_toggle_btn);
  }

  err = create_element_with_text(
      "span", "mode-indicator",
      state->mode == OAUTH2_MODE_ONLINE ? "MODE: ONLINE" : "MODE: OFFLINE",
      &mode_badge);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_append_child(mode_container, mode_badge);
  }
  ui_dom_node_append_child(card, mode_container);

  /* Backend server URL specification group */
  err = create_element_node("div", "backend-url-group", &backend_url_group);
  if (err == UI_ERROR_NONE) {
    err = create_element_with_text("label", "url-label",
                                   "Backend Server URL: ", &url_label);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_set_attribute(url_label, "for", "backend-url-input");
      ui_dom_node_append_child(backend_url_group, url_label);
    }

    err = create_element_node("input", "backend-url-input", &url_input);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_set_attribute(url_input, "type", "text");
      ui_dom_node_set_attribute(url_input, "id", "backend-url-input");
      app_rc = app_state_get_backend_url(state, &curr_backend_url);
      if (app_rc != OAUTH2_APP_OK) {
        curr_backend_url = NULL;
      }
      ui_dom_node_set_attribute(url_input, "value",
                                curr_backend_url != NULL
                                    ? curr_backend_url
                                    : "http://127.0.0.1:8080");
      ui_dom_node_append_child(backend_url_group, url_input);
    }
    ui_dom_node_append_child(card, backend_url_group);
  }

  /* Credentials input group */
  err = create_element_node("div", "credentials-group", &cred_group);
  if (err == UI_ERROR_NONE) {
    err = create_element_with_text("label", "input-label",
                                   "Username: ", &user_label);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_append_child(cred_group, user_label);
    }

    err = create_element_node("input", "login-input", &user_input);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_set_attribute(user_input, "id", "username");
      ui_dom_node_set_attribute(user_input, "type", "text");
      ui_dom_node_append_child(cred_group, user_input);
    }

    err = create_element_with_text("label", "input-label",
                                   "Password: ", &pass_label);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_append_child(cred_group, pass_label);
    }

    err = create_element_node("input", "login-input", &pass_input);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_set_attribute(pass_input, "id", "password");
      ui_dom_node_set_attribute(pass_input, "type", "password");
      ui_dom_node_append_child(cred_group, pass_input);
    }
    ui_dom_node_append_child(card, cred_group);
  }

  /* Quick-select offline demo account chips */
  err = create_element_node("div", "quick-accounts", &quick_group);
  if (err == UI_ERROR_NONE) {
    err = create_element_with_text("button", "quick-chip-admin",
                                   "Admin Demo Account", &quick_admin);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_set_attribute(quick_admin, "id", "quick-admin-btn");
      ui_dom_node_append_child(quick_group, quick_admin);
    }

    err = create_element_with_text("button", "quick-chip-alice",
                                   "offline_alice Demo Account", &quick_alice);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_set_attribute(quick_alice, "id", "quick-alice-btn");
      ui_dom_node_append_child(quick_group, quick_alice);
    }
    ui_dom_node_append_child(card, quick_group);
  }

  /* Submit button */
  err = create_element_with_text("button", "login-submit-btn", "Log In",
                                 &submit_btn);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_set_attribute(submit_btn, "id", "login-submit-btn");
    ui_dom_node_append_child(card, submit_btn);
  }

  /* Error banner */
  err = create_element_with_text(
      "div", "login-error-banner",
      state->last_error[0] != '\0' ? state->last_error : "", &error_banner);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_set_attribute(error_banner, "id", "login-error-banner");
    ui_dom_node_append_child(card, error_banner);
  }

  comp->shadow_root = card;
  *out_screen = comp;
  return UI_ERROR_NONE;
}

ui_error_t oauth2_view_profile_factory(const struct ui_route_request *req,
                                       void *user_data,
                                       struct ui_component **out_screen) {
  struct app_state *state = (struct app_state *)user_data;
  struct ui_component *comp;
  struct ui_dom_node *container;
  struct ui_dom_node *heading;
  struct ui_dom_node *role_badge;
  struct ui_dom_node *status_badge;
  struct ui_dom_node *about_me_box;
  struct ui_dom_node *about_me_label;
  struct ui_dom_node *about_me_input;
  struct ui_dom_node *save_btn;
  struct ui_dom_node *admin_dir_btn;
  ui_error_t err;

  if (req != NULL) {
    /* req validated */
  }
  if (user_data == NULL || out_screen == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = create_element_node("div", "profile-container", &container);
  if (err != UI_ERROR_NONE) {
    ui_component_destroy(comp);
    return err;
  }

  err = create_element_with_text("h2", "profile-user",
                                 state->current_user.username[0] != '\0'
                                     ? state->current_user.username
                                     : "Anonymous User",
                                 &heading);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_append_child(container, heading);
  }

  err = create_element_with_text("span", "profile-role",
                                 state->current_user.role == OAUTH2_ROLE_ADMIN
                                     ? "Role: Administrator"
                                     : "Role: Standard User",
                                 &role_badge);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_append_child(container, role_badge);
  }

  err = create_element_with_text("span", "profile-sync-status",
                                 state->current_user.sync_status ==
                                         OAUTH2_SYNC_STATUS_SYNCED
                                     ? "Status: Synced"
                                     : "Status: Modified Locally",
                                 &status_badge);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_append_child(container, status_badge);
  }

  err = create_element_node("div", "profile-about-me", &about_me_box);
  if (err == UI_ERROR_NONE) {
    err = create_element_with_text("label", "about-me-label",
                                   "About Me:", &about_me_label);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_append_child(about_me_box, about_me_label);
    }

    err =
        create_element_with_text("textarea", "profile-bio-input",
                                 state->current_user.about_me, &about_me_input);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_set_attribute(about_me_input, "id", "about-me-textarea");
      ui_dom_node_append_child(about_me_box, about_me_input);
    }
    ui_dom_node_append_child(container, about_me_box);
  }

  err = create_element_with_text("button", "profile-save-btn", "Save Profile",
                                 &save_btn);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_set_attribute(save_btn, "id", "save-profile-btn");
    ui_dom_node_append_child(container, save_btn);
  }

  if (state->current_user.role == OAUTH2_ROLE_ADMIN) {
    err = create_element_with_text("button", "admin-directory-btn",
                                   "View Admin Directory", &admin_dir_btn);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_set_attribute(admin_dir_btn, "id", "view-admin-btn");
      ui_dom_node_append_child(container, admin_dir_btn);
    }
  }

  comp->shadow_root = container;
  *out_screen = comp;
  return UI_ERROR_NONE;
}

static ui_error_t on_admin_user_clicked(const char *user_id, void *user_data) {
  struct app_state *state = (struct app_state *)user_data;
  char route_buf[128];

  if (state == NULL || user_id == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (state->router != NULL) {
#if defined(_MSC_VER)
    sprintf_s(route_buf, sizeof(route_buf), "/users/%s", user_id);
#else
    sprintf(route_buf, "/users/%s", user_id);
#endif
    return ui_router_navigate(state->router, route_buf);
  }

  return UI_ERROR_NONE;
}

ui_error_t
oauth2_view_admin_directory_factory(const struct ui_route_request *req,
                                    void *user_data,
                                    struct ui_component **out_screen) {
  struct app_state *state = (struct app_state *)user_data;
  struct ui_component *comp;
  struct ui_component *dl_comp;
  struct ui_dom_node *container;
  struct ui_dom_node *toolbar;
  struct ui_dom_node *toolbar_mode;
  struct ui_dom_node *sync_btn;
  struct ui_dl_list *dl;
  struct oauth2_user *users_list = NULL;
  size_t count = 0;
  size_t i;
  enum oauth2_app_error app_rc;
  ui_error_t err;

  if (req != NULL) {
    /* req validated */
  }
  if (user_data == NULL || out_screen == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = create_element_node("div", "admin-directory-container", &container);
  if (err != UI_ERROR_NONE) {
    ui_component_destroy(comp);
    return err;
  }

  err = create_element_node("div", "admin-toolbar", &toolbar);
  if (err == UI_ERROR_NONE) {
    err = create_element_with_text(
        "span", "toolbar-mode",
        state->mode == OAUTH2_MODE_ONLINE ? "Online Mode" : "Offline Mode",
        &toolbar_mode);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_append_child(toolbar, toolbar_mode);
    }

    err = create_element_with_text("button", "sync-changes-btn",
                                   "Sync Pending Changes", &sync_btn);
    if (err == UI_ERROR_NONE) {
      ui_dom_node_set_attribute(sync_btn, "id", "sync-changes-btn");
      ui_dom_node_append_child(toolbar, sync_btn);
    }
    ui_dom_node_append_child(container, toolbar);
  }

  err = ui_dl_list_create(&dl);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(container);
    ui_component_destroy(comp);
    return err;
  }

  if (state->mode == OAUTH2_MODE_ONLINE && state->is_authenticated) {
    app_rc = oauth2_client_list_all_users(
        state->online_client, &state->current_session, &users_list, &count);
    if (app_rc != OAUTH2_APP_OK) {
      /* Fallback to local cache */
      app_rc = oauth2_offline_store_list_users(state->offline_store,
                                               &users_list, &count);
      if (app_rc != OAUTH2_APP_OK) {
        count = 0;
      }
    }
  } else {
    app_rc = oauth2_offline_store_list_users(state->offline_store, &users_list,
                                             &count);
    if (app_rc != OAUTH2_APP_OK) {
      count = 0;
    }
  }

  for (i = 0; i < count; ++i) {
    err = ui_dl_list_add_user(dl, &users_list[i], on_admin_user_clicked, state);
    if (err != UI_ERROR_NONE) {
      ui_dl_list_destroy(dl);
      ui_dom_node_destroy(container);
      ui_component_destroy(comp);
      return err;
    }
  }

  err = ui_dl_list_get_component(dl, &dl_comp);
  if (err != UI_ERROR_NONE) {
    ui_dl_list_destroy(dl);
    ui_dom_node_destroy(container);
    ui_component_destroy(comp);
    return err;
  }

  if (dl_comp != NULL && dl_comp->shadow_root != NULL) {
    ui_dom_node_append_child(container, dl_comp->shadow_root);
  }

  comp->shadow_root = container;
  *out_screen = comp;
  return UI_ERROR_NONE;
}

ui_error_t oauth2_view_user_detail_factory(const struct ui_route_request *req,
                                           void *user_data,
                                           struct ui_component **out_screen) {
  struct app_state *state = (struct app_state *)user_data;
  struct ui_component *comp;
  struct ui_dom_node *container;
  struct ui_dom_node *title;
  struct ui_dom_node *bio_box;
  struct ui_dom_node *back_btn;
  const char *target_id = NULL;
  struct oauth2_user target_user;
  enum oauth2_app_error app_rc;
  ui_error_t err;

  if (user_data == NULL || out_screen == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (req != NULL) {
    err = ui_route_request_get_param(req, "id", &target_id);
    if (err != UI_ERROR_NONE || target_id == NULL) {
      target_id = "unknown";
    }
  } else {
    target_id = "unknown";
  }

  /* Query user details */
  app_rc = oauth2_user_init(&target_user);
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  app_rc = oauth2_offline_store_get_user_by_id(state->offline_store, target_id,
                                               &target_user);
  if (app_rc != OAUTH2_APP_OK) {
    app_rc = oauth2_safe_strcpy(target_user.username,
                                sizeof(target_user.username), target_id);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_INVALID_ARGUMENT;
    }
    app_rc =
        oauth2_safe_strcpy(target_user.about_me, sizeof(target_user.about_me),
                           "No details available for this user.");
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_INVALID_ARGUMENT;
    }
  }

  err = ui_component_create(&comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = create_element_node("div", "user-detail-container", &container);
  if (err != UI_ERROR_NONE) {
    ui_component_destroy(comp);
    return err;
  }

  err = create_element_with_text("h1", "user-detail-title",
                                 target_user.username, &title);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_append_child(container, title);
  }

  err = create_element_with_text("p", "user-detail-bio", target_user.about_me,
                                 &bio_box);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_append_child(container, bio_box);
  }

  err = create_element_with_text("button", "back-to-directory-btn",
                                 "Back to Directory", &back_btn);
  if (err == UI_ERROR_NONE) {
    ui_dom_node_set_attribute(back_btn, "id", "back-to-directory-btn");
    ui_dom_node_append_child(container, back_btn);
  }

  comp->shadow_root = container;
  *out_screen = comp;
  return UI_ERROR_NONE;
}

/**
 * @brief Helper mapping OAuth2 app errors to standard UI error codes.
 */
static ui_error_t map_app_error(enum oauth2_app_error rc) {
  switch (rc) {
  case OAUTH2_APP_OK:
    return UI_ERROR_NONE;
  case OAUTH2_APP_ERROR_INVALID_PARAM:
    return UI_ERROR_INVALID_ARGUMENT;
  case OAUTH2_APP_ERROR_MEMORY:
    return UI_ERROR_OUT_OF_MEMORY;
  case OAUTH2_APP_ERROR_NETWORK:
    return UI_ERROR_IO_FAILED;
  case OAUTH2_APP_ERROR_AUTH_FAILED:
  case OAUTH2_APP_ERROR_PERMISSION_DENIED:
    return UI_ERROR_UNSUPPORTED;
  case OAUTH2_APP_ERROR_NOT_FOUND:
    return UI_ERROR_NOT_FOUND;
  case OAUTH2_APP_ERROR_STORAGE_FAILED:
    return UI_ERROR_IO_FAILED;
  case OAUTH2_APP_ERROR_PARSE_FAILED:
    return UI_ERROR_PARSE_FAILED;
  default:
    return UI_ERROR_UNKNOWN;
  }
}

ui_error_t oauth2_view_toggle_mode(struct app_state *state) {
  enum oauth2_app_mode target_mode;
  enum oauth2_app_error rc;

  if (state == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  target_mode = (state->mode == OAUTH2_MODE_ONLINE) ? OAUTH2_MODE_OFFLINE
                                                    : OAUTH2_MODE_ONLINE;
  rc = app_state_set_mode(state, target_mode);
  if (rc != OAUTH2_APP_OK) {
    return map_app_error(rc);
  }

  return UI_ERROR_NONE;
}

ui_error_t oauth2_view_set_backend_url(struct app_state *state,
                                       const char *url) {
  enum oauth2_app_error rc;

  if (state == NULL || url == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = app_state_set_backend_url(state, url);
  if (rc != OAUTH2_APP_OK) {
    return map_app_error(rc);
  }

  return UI_ERROR_NONE;
}

ui_error_t oauth2_view_submit_login(struct app_state *state,
                                    const char *username,
                                    const char *password) {
  enum oauth2_app_error rc;

  if (state == NULL || username == NULL || password == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = app_state_login(state, username, password);
  if (rc != OAUTH2_APP_OK) {
    return map_app_error(rc);
  }

  return UI_ERROR_NONE;
}

ui_error_t oauth2_view_save_profile(struct app_state *state,
                                    const char *new_about_me) {
  enum oauth2_app_error rc;

  if (state == NULL || new_about_me == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = app_state_update_about_me(state, new_about_me);
  if (rc != OAUTH2_APP_OK) {
    return map_app_error(rc);
  }

  return UI_ERROR_NONE;
}

ui_error_t oauth2_view_sync_changes(struct app_state *state,
                                    size_t *out_synced_count) {
  enum oauth2_app_error rc;

  if (state == NULL || out_synced_count == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = app_state_sync_dirty_records(state, out_synced_count);
  if (rc != OAUTH2_APP_OK) {
    return map_app_error(rc);
  }

  return UI_ERROR_NONE;
}
