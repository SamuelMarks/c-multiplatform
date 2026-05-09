/* clang-format off */
#include "greatest.h"
#include "cmp_ui_snackbar.h"
#include "cmp_ui_progress_indicator.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_snackbar(void) {
  cmp_ui_snackbar_t *snackbar;
  cmp_ui_node_t *node;

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_snackbar_create(&snackbar, "Message saved", "UNDO"),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_snackbar_get_node(snackbar, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_snackbar_set_message(snackbar, "New message"), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_snackbar_set_action(snackbar, "RETRY"),
                "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_snackbar_set_message(snackbar, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_snackbar_set_action(snackbar, NULL), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_snackbar_destroy(snackbar), "%d");

  PASS();
}

TEST test_progress_indicator(void) {
  cmp_ui_progress_indicator_t *indicator;
  cmp_ui_node_t *node;

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_progress_indicator_create(
                    &indicator, CMP_UI_PROGRESS_INDICATOR_LINEAR),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_progress_indicator_get_node(indicator, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_progress_indicator_set_progress(indicator, 0.5f), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_progress_indicator_set_progress(indicator, 1.5f),
                "%d"); /* Should clamp */
  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_progress_indicator_set_progress(indicator, -0.5f),
                "%d"); /* Should clamp */

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_progress_indicator_set_type(
                    indicator, CMP_UI_PROGRESS_INDICATOR_CIRCULAR),
                "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_progress_indicator_destroy(indicator),
                "%d");

  PASS();
}

SUITE(m3_communication_suite) {
  RUN_TEST(test_snackbar);
  RUN_TEST(test_progress_indicator);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(m3_communication_suite);
  GREATEST_MAIN_END();
}
