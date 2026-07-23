/* clang-format off */
#include "../mock_backend.h"
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
  enum ui_error err;
  int fails = 0;

  memset(&state, 0, sizeof(state));

  err = mock_login(&state, "admin", "password");
  if (err != UI_ERROR_NONE || state.is_authenticated != 1 ||
      strcmp(state.current_user, "admin") != 0) {
    printf("FAIL: Valid login should succeed.\n");
    fails++;
  }

  err = mock_login(&state, "admin", "wrong");
  if (err != UI_ERROR_INVALID_ARGUMENT || state.is_authenticated != 0 ||
      strcmp(state.auth_error_message, "err_invalid_credentials") != 0) {
    printf("FAIL: Invalid login should fail.\n");
    fails++;
  }

  return fails;
}

static int test_mock_signup(void) {
  struct app_state state;
  enum ui_error err;
  int fails = 0;

  memset(&state, 0, sizeof(state));

  err = mock_signup(&state, "admin", "newpass");
  if (err != UI_ERROR_INVALID_ARGUMENT || state.is_authenticated != 0 ||
      strcmp(state.auth_error_message, "err_user_exists") != 0) {
    printf("FAIL: Signup with existing user should fail.\n");
    fails++;
  }

  err = mock_signup(&state, "newuser", "12");
  if (err != UI_ERROR_INVALID_ARGUMENT || state.is_authenticated != 0 ||
      strcmp(state.auth_error_message, "err_invalid_credentials") != 0) {
    printf("FAIL: Signup with short password should fail.\n");
    fails++;
  }

  err = mock_signup(&state, "newuser", "goodpass");
  if (err != UI_ERROR_NONE || state.is_authenticated != 1 ||
      strcmp(state.current_user, "newuser") != 0) {
    printf("FAIL: Valid signup should succeed.\n");
    fails++;
  }

  return fails;
}

static int test_mock_logout(void) {
  struct app_state state;
  enum ui_error err;
  int fails = 0;

  memset(&state, 0, sizeof(state));
  state.is_authenticated = 1;
  strcpy(state.current_user, "test");
  strcpy(state.auth_error_message, "some error");

  err = mock_logout(&state);
  if (err != UI_ERROR_NONE || state.is_authenticated != 0 ||
      state.current_user[0] != '\0' || state.auth_error_message[0] != '\0') {
    printf("FAIL: Logout should clear state.\n");
    fails++;
  }

  return fails;
}

int main(void) {
  int total_fails = 0;

  printf("Running Auth Flow mock backend tests...\n");

  total_fails += test_mock_login();
  total_fails += test_mock_signup();
  total_fails += test_mock_logout();

  if (total_fails == 0) {
    printf("All mock backend tests PASSED\n");
  } else {
    printf("Some mock backend tests FAILED\n");
  }

  return total_fails;
}
