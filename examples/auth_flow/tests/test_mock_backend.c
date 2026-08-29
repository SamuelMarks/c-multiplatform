/* clang-format off */
#include "../mock_backend.h"
#include "../../../src/ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/* We need to define a dummy app_state to test the backend */
struct app_state {
  void *arena;
  void *theme_manager;
  void *i18n;
  void *router;
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
};

static int test_mock_login(void) {
  struct app_state state;
  ui_error_t err;
  int fails = 0;

  memset(&state, 0, sizeof(state));

  err = mock_login(&state, "admin", "password");
  fails |= (err != UI_ERROR_NONE | state.is_authenticated != 1 |
            strcmp(state.current_user, "admin") != 0);

  err = mock_login(&state, "admin", "wrong");
  fails |= (err != UI_ERROR_INVALID_ARGUMENT | state.is_authenticated != 0 |
            strcmp(state.auth_error_message, "err_invalid_credentials") != 0);

  err = mock_login(&state, "wrong", "password");
  fails |= (err != UI_ERROR_INVALID_ARGUMENT | state.is_authenticated != 0);

  return fails;
}

static int test_mock_signup(void) {
  struct app_state state;
  ui_error_t err;
  int fails = 0;

  memset(&state, 0, sizeof(state));

  err = mock_signup(&state, "admin", "newpass");
  fails |= (err != UI_ERROR_INVALID_ARGUMENT | state.is_authenticated != 0 |
            strcmp(state.auth_error_message, "err_user_exists") != 0);

  err = mock_signup(&state, "newuser", "12");
  fails |= (err != UI_ERROR_INVALID_ARGUMENT | state.is_authenticated != 0 |
            strcmp(state.auth_error_message, "err_invalid_credentials") != 0);

  err = mock_signup(&state, "newuser", "goodpass");
  fails |= (err != UI_ERROR_NONE | state.is_authenticated != 1 |
            strcmp(state.current_user, "newuser") != 0);

  return fails;
}

static int test_mock_logout(void) {
  struct app_state state;
  ui_error_t err;
  int fails = 0;

  memset(&state, 0, sizeof(state));
  state.is_authenticated = 1;
  UI_STRCPY(state.current_user, sizeof(state.current_user), "test");
  UI_STRCPY(state.auth_error_message, sizeof(state.auth_error_message),
            "some error");

  err = mock_logout(&state);
  fails |=
      (err != UI_ERROR_NONE | state.is_authenticated != 0 |
       state.current_user[0] != '\0' | state.auth_error_message[0] != '\0');

  return fails;
}

int main(void) {
  int total_fails = 0;

  printf("Running Auth Flow mock backend tests...\n");

  total_fails += test_mock_login();
  total_fails += test_mock_signup();
  total_fails += test_mock_logout();

  return total_fails;
}
