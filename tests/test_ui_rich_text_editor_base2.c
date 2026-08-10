#include "ui_rich_text_editor_base.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static ui_error_t failing_cva_on_change(union ui_signal_payload payload,
                                        void *user_data) {
  return UI_ERROR_NOT_FOUND;
}
static ui_error_t failing_cva_on_touched(void *user_data) {
  return UI_ERROR_NOT_FOUND;
}

void test_ui_rte_coverage_errs14(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    cva.register_on_change(rte, failing_cva_on_change, NULL);
    ui_rich_text_editor_base_insert_text(rte, "foo"); /* hits 295 */

    cva.register_on_touched(rte, failing_cva_on_touched, NULL);
    ui_rich_text_editor_base_insert_text(rte, "foo"); /* hits 266 */

    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
