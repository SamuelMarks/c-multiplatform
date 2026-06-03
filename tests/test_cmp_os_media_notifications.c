/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

static void dummy_cb(int cmd, void *u) {
  (void)cmd;
  (void)u;
}

TEST test_os_media_notifications_lifecycle(void) {
  cmp_os_media_controls_t *mc = NULL;
  int rc;

  rc = cmp_os_notify("title", "body");
  ASSERT(rc == CMP_SUCCESS || rc == CMP_ERROR_NOT_FOUND);

  ASSERT_EQ(CMP_SUCCESS, cmp_os_media_controls_create(&mc));
  ASSERT_NEQ(NULL, mc);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_os_media_controls_update(mc, "Title", "Artist", 120.0f, 10.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_os_media_controls_set_handler(mc, dummy_cb, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_os_media_controls_destroy(mc));
  PASS();
}

TEST test_os_media_notifications_null(void) {
  cmp_os_media_controls_t *mc = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_notify(NULL, "body"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_notify("title", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_media_controls_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_media_controls_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_os_media_controls_create(&mc));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_os_media_controls_update(
                                       NULL, "Title", "Artist", 120.0f, 10.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_media_controls_update(mc, NULL, "Artist", 120.0f, 10.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_media_controls_update(mc, "Title", NULL, 120.0f, 10.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_media_controls_set_handler(NULL, dummy_cb, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_os_media_controls_set_handler(mc, NULL, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_os_media_controls_destroy(mc));
  PASS();
}

SUITE(cmp_os_media_notifications_suite) {
  RUN_TEST(test_os_media_notifications_lifecycle);
  RUN_TEST(test_os_media_notifications_null);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_os_media_notifications_suite);
  GREATEST_MAIN_END();
}
