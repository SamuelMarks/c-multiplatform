/**
 * @file test_views.c
 * @brief Unit tests for OAuth2 sync flow UI view factories.
 */

/* clang-format off */
#include "../views.h"
#include "../app_state.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

static ui_error_t test_view_factories(void) {
  struct app_state *state;
  struct ui_component *comp_login = NULL;
  struct ui_component *comp_profile = NULL;
  struct ui_component *comp_admin = NULL;
  struct ui_component *comp_detail = NULL;
  enum oauth2_app_error app_rc;
  ui_error_t err;

  /* Null checks */
  err = oauth2_view_login_factory(NULL, NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT on NULL login params\n");
    return err;
  }

  err = oauth2_view_profile_factory(NULL, NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT on NULL profile params\n");
    return err;
  }

  err = oauth2_view_admin_directory_factory(NULL, NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT on NULL admin params\n");
    return err;
  }

  err = oauth2_view_user_detail_factory(NULL, NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT on NULL detail params\n");
    return err;
  }

  /* Initialize app state */
  app_rc = app_state_init(&state);
  if (app_rc != OAUTH2_APP_OK) {
    fprintf(stderr, "App state init failed\n");
    return UI_ERROR_UNKNOWN;
  }

  /* Test login view creation */
  err = oauth2_view_login_factory(NULL, state, &comp_login);
  if (err != UI_ERROR_NONE || comp_login == NULL) {
    fprintf(stderr, "Login view factory failed: %d\n", (int)err);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  ui_component_destroy(comp_login);

  /* Test profile view creation */
  app_rc = app_state_login(state, "admin", "password");
  if (app_rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Login failed\n");
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  err = oauth2_view_profile_factory(NULL, state, &comp_profile);
  if (err != UI_ERROR_NONE || comp_profile == NULL) {
    fprintf(stderr, "Profile view factory failed: %d\n", (int)err);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  ui_component_destroy(comp_profile);

  /* Test admin directory view creation */
  err = oauth2_view_admin_directory_factory(NULL, state, &comp_admin);
  if (err != UI_ERROR_NONE || comp_admin == NULL) {
    fprintf(stderr, "Admin directory view factory failed: %d\n", (int)err);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  ui_component_destroy(comp_admin);

  /* Test user detail view creation */
  err = oauth2_view_user_detail_factory(NULL, state, &comp_detail);
  if (err != UI_ERROR_NONE || comp_detail == NULL) {
    fprintf(stderr, "User detail view factory failed: %d\n", (int)err);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  ui_component_destroy(comp_detail);

  /* Test action handlers */
  err = oauth2_view_toggle_mode(NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT on NULL toggle_mode\n");
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  err = oauth2_view_set_backend_url(NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT on NULL set_backend_url\n");
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  err = oauth2_view_submit_login(NULL, NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT on NULL submit_login\n");
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  err = oauth2_view_save_profile(NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT on NULL save_profile\n");
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  err = oauth2_view_sync_changes(NULL, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT on NULL sync_changes\n");
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  /* Test valid toggle_mode */
  err = oauth2_view_toggle_mode(state);
  if (err != UI_ERROR_NONE || state->mode != OAUTH2_MODE_ONLINE) {
    fprintf(stderr, "toggle_mode to ONLINE failed\n");
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  err = oauth2_view_toggle_mode(state);
  if (err != UI_ERROR_NONE || state->mode != OAUTH2_MODE_OFFLINE) {
    fprintf(stderr, "toggle_mode back to OFFLINE failed\n");
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  /* Test valid set_backend_url */
  err = oauth2_view_set_backend_url(state, "http://api.backend.local:8080");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "set_backend_url failed: %d\n", (int)err);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  /* Test valid save_profile */
  err = oauth2_view_save_profile(state, "Updated bio from view action");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "save_profile failed: %d\n", (int)err);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  /* Test valid sync_changes in online mode */
  err = oauth2_view_toggle_mode(state);
  if (err != UI_ERROR_NONE) {
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  {
    size_t synced_num = 0;
    err = oauth2_view_sync_changes(state, &synced_num);
    if (err != UI_ERROR_NONE) {
      fprintf(stderr, "sync_changes failed: %d\n", (int)err);
      app_rc = app_state_destroy(state);
      if (app_rc != OAUTH2_APP_OK) {
        return UI_ERROR_UNKNOWN;
      }
      return err;
    }
  }

  app_rc = app_state_destroy(state);
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_UNKNOWN;
  }
  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t err;

  printf("Running test_views...\n");
  err = test_view_factories();
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "test_views failed with code %d\n", (int)err);
    return 1;
  }

  printf("test_views passed successfully!\n");
  return 0;
}
