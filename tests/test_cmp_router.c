/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

static int g_view_call_count = 0;
static int g_guard_call_count = 0;
static int g_guard_block = 0;

static void *dummy_view_builder(const char *uri) {
  g_view_call_count++;
  (void)uri;
  return (void *)(size_t)0xDEADBEEF;
}

static int dummy_guard(const char *to_route, void *user_data) {
  g_guard_call_count++;
  (void)to_route;
  (void)user_data;
  return !g_guard_block;
}

TEST test_router_lifecycle(void) {
  cmp_router_t *router = NULL;
  int res;

  res = cmp_router_create(&router);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(router != NULL);

  res = cmp_router_destroy(router);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_router_navigation(void) {
  cmp_router_t *router = NULL;
  cmp_string_t current;
  int res;

  g_view_call_count = 0;
  g_guard_call_count = 0;
  g_guard_block = 0;

  cmp_router_create(&router);

  /* Register routes */
  res = cmp_router_register(router, "/home", dummy_view_builder, NULL, NULL);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_router_register(router, "/settings", dummy_view_builder,
                            dummy_guard, NULL);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res =
      cmp_router_register(router, "/new_home", dummy_view_builder, NULL, NULL);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Push home */
  res = cmp_router_push(router, "/home");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT(1, g_view_call_count, "%d");

  /* Verify current */
  res = cmp_router_get_current(router, &current);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_STR_EQ("/home", current.data);
  cmp_string_destroy(&current);

  /* Push settings */
  res = cmp_router_push(router, "/settings");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT(2, g_view_call_count, "%d");
  ASSERT_EQ_FMT(1, g_guard_call_count, "%d");

  /* Verify current is settings */
  res = cmp_router_get_current(router, &current);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_STR_EQ("/settings", current.data);
  cmp_string_destroy(&current);

  /* Pop back to home */
  res = cmp_router_pop(router);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT(3, g_view_call_count, "%d"); /* re-builds home */

  /* Verify current is home again */
  res = cmp_router_get_current(router, &current);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_STR_EQ("/home", current.data);
  cmp_string_destroy(&current);

  /* Replace home with new_home */
  res = cmp_router_replace(router, "/new_home");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT(4, g_view_call_count, "%d");
  res = cmp_router_get_current(router, &current);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_STR_EQ("/new_home", current.data);
  cmp_string_destroy(&current);

  /* Test guard blocking */
  g_guard_block = 1;
  res = cmp_router_push(router, "/settings");
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d"); /* Blocked */
  ASSERT_EQ_FMT(4, g_view_call_count, "%d");     /* didn't build */
  ASSERT_EQ_FMT(2, g_guard_call_count, "%d");    /* guard fired */

  cmp_router_destroy(router);
  PASS();
}

TEST test_router_dynamic_params(void) {
  cmp_router_t *router = NULL;
  cmp_string_t current;
  int res;

  g_view_call_count = 0;

  cmp_router_create(&router);

  res =
      cmp_router_register(router, "/user/:id", dummy_view_builder, NULL, NULL);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_router_push(router, "/user/42");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT(1, g_view_call_count, "%d");

  res = cmp_router_get_current(router, &current);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_STR_EQ("/user/42", current.data);
  cmp_string_destroy(&current);

  res = cmp_router_push(router, "/user");
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");

  res = cmp_router_push(router, "/user/42/extra");
  ASSERT_EQ_FMT(CMP_ERROR_NOT_FOUND, res, "%d");

  cmp_router_destroy(router);
  PASS();
}

void *dummy_builder(const char *route_params) {
  (void)route_params;
  return NULL;
}

TEST test_routing_styles(void) {
  cmp_router_t *router = NULL;
  char title[128];

  ASSERT_EQ(CMP_SUCCESS, cmp_router_create(&router));

  /* Mock register paths */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_router_register(router, "/home", dummy_builder, NULL, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_register(router, "/settings", dummy_builder,
                                             NULL, NULL));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_router_register(router, "/details", dummy_builder, NULL, NULL));

  /* Push with presentation style */
  ASSERT_EQ(CMP_SUCCESS, cmp_router_push_with_style(
                             router, "/home", CMP_PRESENTATION_STYLE_PUSH));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_router_push_with_style(router, "/settings",
                                       CMP_PRESENTATION_STYLE_SHEET));

  /* Get Previous Title (Back button label) */
  ASSERT_EQ(CMP_SUCCESS, cmp_router_get_previous_title(router, title, 128));
  ASSERT_STR_EQ("/home", title);

  /* Pop */
  ASSERT_EQ(CMP_SUCCESS, cmp_router_pop_with_style(router));

  /* Cannot get previous if only 1 item on stack */
  ASSERT_EQ(CMP_ERROR_NOT_FOUND,
            cmp_router_get_previous_title(router, title, 128));

  /* Tab switching flushes stack */
  ASSERT_EQ(CMP_SUCCESS, cmp_router_push_with_style(
                             router, "/details", CMP_PRESENTATION_STYLE_PUSH));
  ASSERT_EQ(CMP_SUCCESS, cmp_router_switch_tab(router, "/settings"));

  ASSERT_EQ(CMP_ERROR_NOT_FOUND,
            cmp_router_get_previous_title(router, title, 128));

  ASSERT_EQ(CMP_SUCCESS, cmp_router_destroy(router));
  PASS();
}

TEST test_split_view(void) {
  cmp_split_view_t *sv = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_split_view_create(&sv));

  ASSERT_EQ(CMP_SUCCESS, cmp_split_view_set_routes(sv, "/master", "/detail"));

  /* Bounds */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_split_view_set_routes(NULL, "/m", "/d"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_split_view_set_routes(sv, NULL, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_split_view_destroy(sv));
  PASS();
}

SUITE(router_suite) {
  RUN_TEST(test_router_lifecycle);
  RUN_TEST(test_router_navigation);
  RUN_TEST(test_routing_styles);
  RUN_TEST(test_split_view);
  RUN_TEST(test_router_dynamic_params);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(router_suite);
  GREATEST_MAIN_END();
}
