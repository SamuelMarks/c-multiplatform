/* clang-format off */
#include "ui_swipe_action_base.h"
#include "ui_error.h"
#include "ui_component.h"
#include <stdio.h>
/* clang-format on */

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

int main(void) {
  struct ui_swipe_action_base swipe;
  struct ui_component *comp = NULL;
  int failed = 0;

  ACCUM_ERR(failed, ui_component_create(&comp));

  /* NULL checks */
  failed |=
      (ui_swipe_action_base_init(NULL, comp) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_swipe_action_base_init(&swipe, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_swipe_action_base_update(NULL, 10.0f) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_swipe_action_base_commit(NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_swipe_action_base_reset(NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_swipe_action_base_bind_disabled(NULL, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_swipe_action_base_bind_text(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);

  /* Init */
  ACCUM_ERR(failed, ui_swipe_action_base_init(&swipe, comp));

  /* Update */
  ACCUM_ERR(failed, ui_swipe_action_base_update(&swipe, 20.0f));
  failed |= (swipe.state != UI_SWIPE_ACTION_SWIPING);

  /* Commit threshold left */
  ACCUM_ERR(failed,
            ui_swipe_action_base_update(&swipe, 50.0f)); /* Total 70 > 60 */
  ACCUM_ERR(failed, ui_swipe_action_base_commit(&swipe));
  failed |= (swipe.state != UI_SWIPE_ACTION_REVEALED_LEFT);

  /* Transition from REVEALED_LEFT to SWIPING */
  ACCUM_ERR(failed, ui_swipe_action_base_update(&swipe, 10.0f));
  failed |= (swipe.state != UI_SWIPE_ACTION_SWIPING);

  /* Reset */
  ACCUM_ERR(failed, ui_swipe_action_base_reset(&swipe));
  failed |= (swipe.state != UI_SWIPE_ACTION_IDLE || swipe.offset_x != 0.0f);

  /* Update from idle to swiping negative */
  ACCUM_ERR(failed, ui_swipe_action_base_update(&swipe, -10.0f));

  /* Commit threshold right */
  ACCUM_ERR(failed,
            ui_swipe_action_base_update(&swipe, -60.0f)); /* Total -70 < -60 */
  ACCUM_ERR(failed, ui_swipe_action_base_commit(&swipe));
  failed |= (swipe.state != UI_SWIPE_ACTION_REVEALED_RIGHT);

  /* Update from revealed state back to swiping */
  ACCUM_ERR(failed, ui_swipe_action_base_update(&swipe, 20.0f));
  failed |= (swipe.state != UI_SWIPE_ACTION_SWIPING);

  /* Commit under threshold (back to idle) */
  ACCUM_ERR(failed, ui_swipe_action_base_reset(&swipe));
  ACCUM_ERR(failed, ui_swipe_action_base_update(&swipe, 30.0f)); /* 30 < 60 */
  ACCUM_ERR(failed, ui_swipe_action_base_commit(&swipe));
  failed |= (swipe.state != UI_SWIPE_ACTION_IDLE || swipe.offset_x != 0.0f);

  /* Bindings */
  ACCUM_ERR(failed, ui_swipe_action_base_bind_disabled(&swipe, NULL));
  ACCUM_ERR(failed, ui_swipe_action_base_bind_text(&swipe, NULL));

  (void)ui_component_destroy(comp);

  if (!failed) {
    printf("All ui_swipe_action_base tests passed.\n");
  }
  return failed;
}
