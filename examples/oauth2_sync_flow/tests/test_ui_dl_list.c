/**
 * @file test_ui_dl_list.c
 * @brief Unit tests for the <dl> description list component.
 */

/* clang-format off */
#include "../ui_dl_list.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

static int g_clicked = 0;
static char g_last_clicked_user[64] = {0};

static ui_error_t mock_on_user_click(const char *user_id, void *user_data) {
  enum oauth2_app_error app_rc;
  if (user_data != NULL) {
    /* user_data optional */
  }
  g_clicked++;
  app_rc = oauth2_safe_strcpy(g_last_clicked_user, sizeof(g_last_clicked_user),
                              user_id);
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

static ui_error_t test_dl_list_flow(void) {
  struct ui_dl_list *dl;
  struct ui_component *comp;
  struct oauth2_user u1;
  struct oauth2_user u2;
  ui_error_t err;
  enum oauth2_app_error app_rc;

  /* Test NULL checks */
  err = ui_dl_list_create(NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT for NULL out_dl\n");
    return err;
  }

  err = ui_dl_list_destroy(NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT) {
    fprintf(stderr, "Expected INVALID_ARGUMENT for NULL destroy\n");
    return err;
  }

  err = ui_dl_list_create(&dl);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "Create dl list failed: %d\n", (int)err);
    return err;
  }

  err = ui_dl_list_get_component(dl, &comp);
  if (err != UI_ERROR_NONE || comp == NULL) {
    fprintf(stderr, "Get component failed: %d\n", (int)err);
    ui_dl_list_destroy(dl);
    return err;
  }

  /* Populate users */
  app_rc = oauth2_user_init(&u1);
  if (app_rc != OAUTH2_APP_OK) {
    ui_dl_list_destroy(dl);
    return UI_ERROR_UNKNOWN;
  }
  app_rc = oauth2_safe_strcpy(u1.id, sizeof(u1.id), "uid_001");
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  app_rc = oauth2_safe_strcpy(u1.username, sizeof(u1.username), "admin");
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  app_rc = oauth2_safe_strcpy(u1.about_me, sizeof(u1.about_me),
                              "Administrator bio here");
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  u1.role = OAUTH2_ROLE_ADMIN;

  app_rc = oauth2_user_init(&u2);
  if (app_rc != OAUTH2_APP_OK) {
    ui_dl_list_destroy(dl);
    return UI_ERROR_UNKNOWN;
  }
  app_rc = oauth2_safe_strcpy(u2.id, sizeof(u2.id), "uid_002");
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  app_rc = oauth2_safe_strcpy(u2.username, sizeof(u2.username), "alice");
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  app_rc =
      oauth2_safe_strcpy(u2.about_me, sizeof(u2.about_me), "Alice regular bio");
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  u2.role = OAUTH2_ROLE_USER;

  err = ui_dl_list_add_user(dl, &u1, mock_on_user_click, NULL);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "Add user 1 failed: %d\n", (int)err);
    ui_dl_list_destroy(dl);
    return err;
  }

  err = ui_dl_list_add_user(dl, &u2, mock_on_user_click, NULL);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "Add user 2 failed: %d\n", (int)err);
    ui_dl_list_destroy(dl);
    return err;
  }

  /* Trigger click on user 2 */
  g_clicked = 0;
  g_last_clicked_user[0] = '\0';
  err = ui_dl_list_trigger_click(dl, "uid_002");
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "Trigger click failed: %d\n", (int)err);
    ui_dl_list_destroy(dl);
    return err;
  }

  if (g_clicked != 1 || strcmp(g_last_clicked_user, "uid_002") != 0) {
    fprintf(stderr, "Click event mismatch for uid_002\n");
    ui_dl_list_destroy(dl);
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Clear items */
  err = ui_dl_list_clear(dl);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "Clear items failed: %d\n", (int)err);
    ui_dl_list_destroy(dl);
    return err;
  }

  err = ui_dl_list_destroy(dl);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "Destroy dl list failed: %d\n", (int)err);
    return err;
  }

  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t err;

  printf("Running test_ui_dl_list...\n");
  err = test_dl_list_flow();
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "test_ui_dl_list failed with code %d\n", (int)err);
    return 1;
  }

  printf("test_ui_dl_list passed successfully!\n");
  return 0;
}
