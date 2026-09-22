/**
 * @file test_oauth2_e2e.c
 * @brief End-to-end (E2E) integration test exercising authentication, mode
 * toggling, profile editing, description list navigation, and synchronization.
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../oauth2_types.h"
#include "../offline_store.h"
#include "../oauth2_client.h"
#include "../app_state.h"
#include "../ui_dl_list.h"
#include "../views.h"
#include "ui_engine.h"
#include "ui_router.h"
#include "ui_arena.h"
#include "ui_dom_node.h"
/* clang-format on */

/**
 * @brief Registers application routes onto the router instance.
 *
 * @param router Pointer to router.
 * @param state Pointer to application state.
 * @return UI_ERROR_NONE on success, or an error code.
 */
static ui_error_t register_e2e_routes(struct ui_router *router,
                                      struct app_state *state) {
  ui_error_t err;

  err = ui_router_add_route(router, "/login", oauth2_view_login_factory, state);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_router_add_route(router, "/profile", oauth2_view_profile_factory,
                            state);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_router_add_route(router, "/admin",
                            oauth2_view_admin_directory_factory, state);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_router_add_route(router, "/users/:id",
                            oauth2_view_user_detail_factory, state);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Runs the complete end-to-end user journey test suite.
 *
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t run_e2e_user_journey(void) {
  struct app_state *state = NULL;
  struct ui_router *router = NULL;
  struct ui_arena *arena = NULL;
  struct ui_component *login_comp = NULL;
  struct ui_component *profile_comp = NULL;
  struct ui_component *admin_comp = NULL;
  struct ui_component *detail_comp = NULL;
  const char *backend_url = NULL;
  size_t synced_count = 0;
  enum oauth2_app_error app_rc;
  ui_error_t err;

  /* Step 1: Initialize App State & Memory Arena */
  app_rc = app_state_init(&state);
  if (app_rc != OAUTH2_APP_OK || state == NULL) {
    fprintf(stderr, "E2E: app_state_init failed: %d\n", (int)app_rc);
    return UI_ERROR_UNKNOWN;
  }

  err = ui_arena_create(1024 * 1024, &arena);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "E2E: ui_arena_create failed: %d\n", (int)err);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  state->arena = arena;

  /* Step 2: Initialize Router and Register Endpoints */
  err = ui_router_create(&router);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "E2E: ui_router_create failed: %d\n", (int)err);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  state->router = router;

  err = register_e2e_routes(router, state);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "E2E: register_e2e_routes failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  /* Step 3: Verify and configure backend URL */
  err =
      oauth2_view_set_backend_url(state, "http://live-api.internal.local:9000");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "E2E: oauth2_view_set_backend_url failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  app_rc = app_state_get_backend_url(state, &backend_url);
  if (app_rc != OAUTH2_APP_OK || backend_url == NULL ||
      strcmp(backend_url, "http://live-api.internal.local:9000") != 0) {
    fprintf(stderr, "E2E: backend_url verification failed\n");
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  /* Step 4: Navigate to Login View and verify DOM construction */
  err = ui_router_navigate(router, "/login");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "E2E: Navigate /login failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  err = oauth2_view_login_factory(NULL, state, &login_comp);
  if (err != UI_ERROR_NONE || login_comp == NULL ||
      login_comp->shadow_root == NULL) {
    fprintf(stderr, "E2E: login_factory failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  ui_component_destroy(login_comp);

  /* Step 5: Test mode toggling between Offline and Online */
  if (state->mode != OAUTH2_MODE_OFFLINE) {
    fprintf(stderr, "E2E: Default mode is not offline\n");
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  err = oauth2_view_toggle_mode(state);
  if (err != UI_ERROR_NONE || state->mode != OAUTH2_MODE_ONLINE) {
    fprintf(stderr, "E2E: Failed to toggle mode to ONLINE\n");
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  err = oauth2_view_toggle_mode(state);
  if (err != UI_ERROR_NONE || state->mode != OAUTH2_MODE_OFFLINE) {
    fprintf(stderr, "E2E: Failed to toggle mode back to OFFLINE\n");
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  /* Step 6: Test authentication failure handling */
  err = oauth2_view_submit_login(state, "nonexistent_user", "bad_password");
  if (err == UI_ERROR_NONE || state->is_authenticated) {
    fprintf(stderr, "E2E: Expected login failure on invalid credentials\n");
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  /* Step 7: Perform offline login with demo account 'offline_alice' */
  err = oauth2_view_submit_login(state, "offline_alice", "password");
  if (err != UI_ERROR_NONE || !state->is_authenticated) {
    fprintf(stderr, "E2E: Offline login for offline_alice failed: %d\n",
            (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  /* Step 8: Navigate to /profile view and verify Profile DOM */
  err = ui_router_navigate(router, "/profile");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "E2E: Navigate /profile failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  err = oauth2_view_profile_factory(NULL, state, &profile_comp);
  if (err != UI_ERROR_NONE || profile_comp == NULL ||
      profile_comp->shadow_root == NULL) {
    fprintf(stderr, "E2E: profile_factory failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  ui_component_destroy(profile_comp);

  /* Step 9: Edit profile 'About Me' offline and verify status transitions to
   * MODIFIED_LOCALLY */
  err = oauth2_view_save_profile(state, "Updated bio from E2E automated test.");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "E2E: save_profile failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  if (state->current_user.sync_status != OAUTH2_SYNC_STATUS_MODIFIED_LOCALLY) {
    fprintf(stderr,
            "E2E: Expected user sync_status MODIFIED_LOCALLY after edit\n");
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  /* Step 10: Log out and log in as 'admin' */
  app_rc = app_state_logout(state);
  if (app_rc != OAUTH2_APP_OK || state->is_authenticated) {
    fprintf(stderr, "E2E: app_state_logout failed: %d\n", (int)app_rc);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  err = oauth2_view_submit_login(state, "admin", "password");
  if (err != UI_ERROR_NONE || !state->is_authenticated) {
    fprintf(stderr, "E2E: Admin login failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  /* Step 11: Navigate to /admin view and verify <dl> Description List DOM */
  err = ui_router_navigate(router, "/admin");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "E2E: Navigate /admin failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  err = oauth2_view_admin_directory_factory(NULL, state, &admin_comp);
  if (err != UI_ERROR_NONE || admin_comp == NULL ||
      admin_comp->shadow_root == NULL) {
    fprintf(stderr, "E2E: admin_directory_factory failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  ui_component_destroy(admin_comp);

  /* Step 12: Navigate to User Detail view /users/user_alice_02 */
  err = ui_router_navigate(router, "/users/user_alice_02");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "E2E: Navigate /users/user_alice_02 failed: %d\n",
            (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  err = oauth2_view_user_detail_factory(NULL, state, &detail_comp);
  if (err != UI_ERROR_NONE || detail_comp == NULL ||
      detail_comp->shadow_root == NULL) {
    fprintf(stderr, "E2E: user_detail_factory failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }
  ui_component_destroy(detail_comp);

  /* Step 13: Switch mode to ONLINE and synchronize dirty offline edits */
  err = oauth2_view_toggle_mode(state);
  if (err != UI_ERROR_NONE || state->mode != OAUTH2_MODE_ONLINE) {
    fprintf(stderr, "E2E: Toggle to online mode failed: %d\n", (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return err;
  }

  err = oauth2_view_sync_changes(state, &synced_count);
  if (err != UI_ERROR_NONE || synced_count == 0) {
    fprintf(stderr, "E2E: oauth2_view_sync_changes failed: %d, count: %lu\n",
            (int)err, (unsigned long)synced_count);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  /* Step 14: Simulate network failure during online operations */
  app_rc = oauth2_client_set_network_failure(state->online_client, 1);
  if (app_rc != OAUTH2_APP_OK) {
    fprintf(stderr, "E2E: oauth2_client_set_network_failure failed: %d\n",
            (int)app_rc);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  /* Attempt online update with network failure active */
  err = oauth2_view_save_profile(state, "Should fail due to network outage");
  if (err != UI_ERROR_IO_FAILED) {
    fprintf(stderr,
            "E2E: Expected UI_ERROR_IO_FAILED on network outage, got %d\n",
            (int)err);
    ui_router_destroy(router);
    ui_arena_destroy(arena);
    app_rc = app_state_destroy(state);
    if (app_rc != OAUTH2_APP_OK) {
      return UI_ERROR_UNKNOWN;
    }
    return UI_ERROR_UNKNOWN;
  }

  app_rc = oauth2_client_set_network_failure(state->online_client, 0);
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_UNKNOWN;
  }

  /* Teardown */
  ui_router_destroy(router);
  ui_arena_destroy(arena);
  app_rc = app_state_destroy(state);
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_UNKNOWN;
  }

  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t err;

  printf("Running test_oauth2_e2e...\n");
  err = run_e2e_user_journey();
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "test_oauth2_e2e failed with code %d\n", (int)err);
    return 1;
  }

  printf("test_oauth2_e2e passed successfully!\n");
  return 0;
}
