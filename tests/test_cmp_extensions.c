/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_widget_lifecycle(void) {
  cmp_widget_ctx_t *ctx = NULL;
  cmp_ui_node_t dummy_node; /* opaque stub */

  ASSERT_EQ(CMP_SUCCESS,
            cmp_widget_ctx_create(&ctx, CMP_WIDGET_FAMILY_SYSTEM_MEDIUM));

  /* Mount static snapshot */
  ASSERT_EQ(CMP_SUCCESS, cmp_widget_mount_snapshot(ctx, &dummy_node));

  /* Bind App Intent (Interactive Widget) */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_widget_bind_intent(ctx, &dummy_node, "UpdateTaskIntent"));

  ASSERT_EQ(CMP_SUCCESS, cmp_widget_ctx_destroy(ctx));
  PASS();
}

TEST test_live_activity_island(void) {
  cmp_live_activity_ctx_t *ctx = NULL;
  cmp_ui_node_t dummy_node;

  ASSERT_EQ(CMP_SUCCESS, cmp_live_activity_ctx_create(&ctx));

  /* Map Dynamic Island configurations */
  ASSERT_EQ(
      CMP_SUCCESS,
      cmp_live_activity_mount_presentation(
          ctx, CMP_LIVE_ACTIVITY_PRESENTATION_COMPACT_LEADING, &dummy_node));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_live_activity_mount_presentation(
                ctx, CMP_LIVE_ACTIVITY_PRESENTATION_EXPANDED, &dummy_node));

  ASSERT_EQ(CMP_SUCCESS, cmp_live_activity_ctx_destroy(ctx));
  PASS();
}

TEST test_footprint_compliance(void) {
  int is_compliant;

  /* App Clips / Widgets have strict memory/disk size limits */
  ASSERT_EQ(CMP_SUCCESS, cmp_extension_verify_footprint(&is_compliant));
  ASSERT_EQ(1, is_compliant);

  PASS();
}

TEST test_null_args(void) {
  cmp_widget_ctx_t *w = NULL;
  cmp_live_activity_ctx_t *la = NULL;
  cmp_ui_node_t node;
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_widget_ctx_create(NULL, CMP_WIDGET_FAMILY_SYSTEM_SMALL));
  cmp_widget_ctx_create(&w, CMP_WIDGET_FAMILY_SYSTEM_SMALL);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_widget_mount_snapshot(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_widget_mount_snapshot(w, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_widget_bind_intent(NULL, &node, "intent"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_widget_bind_intent(w, NULL, "intent"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_widget_bind_intent(w, &node, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_live_activity_ctx_create(NULL));
  cmp_live_activity_ctx_create(&la);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_live_activity_mount_presentation(
                NULL, CMP_LIVE_ACTIVITY_PRESENTATION_MINIMAL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_live_activity_mount_presentation(
                la, CMP_LIVE_ACTIVITY_PRESENTATION_MINIMAL, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_extension_verify_footprint(NULL));

  cmp_widget_ctx_destroy(w);
  cmp_live_activity_ctx_destroy(la);
  PASS();
}

SUITE(extensions_suite) {
  RUN_TEST(test_widget_lifecycle);
  RUN_TEST(test_live_activity_island);
  RUN_TEST(test_footprint_compliance);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(extensions_suite);
  GREATEST_MAIN_END();
}
