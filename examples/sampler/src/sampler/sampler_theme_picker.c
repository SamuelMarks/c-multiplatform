/**
 * @file sampler_theme_picker.c
 * @brief Implementation of Theme Picker Modal Bottom Sheet.
 */

/* clang-format off */
#include "sampler/sampler_theme_picker.h"
#include "material3/md3_overlay.h"
#include "material3/md3_selection_controls.h"
#include "material3/md3_action_widgets.h"
#include "ui_bottom_sheet_base.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct sampler_theme_picker {
  struct md3_bottom_sheet *sheet;
  struct ui_dom_node *root_node;
};

sampler_error_t
sampler_theme_picker_create(struct ui_engine *engine,
                            struct sampler_theme_picker **out_picker) {
  struct sampler_theme_picker *picker = NULL;
  ui_error_t u_rc;

  if (engine == NULL || out_picker == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  picker = (struct sampler_theme_picker *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct sampler_theme_picker));
  if (picker == NULL) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }
  memset(picker, 0, sizeof(*picker));

  u_rc =
      md3_bottom_sheet_create(engine, MD3_BOTTOM_SHEET_MODAL, &picker->sheet);
  if (u_rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(picker);
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  if (picker->sheet != NULL && picker->sheet->base != NULL) {
    struct ui_component *comp = NULL;
    u_rc = ui_bottom_sheet_base_get_component(picker->sheet->base, &comp);
    if (u_rc == UI_ERROR_NONE && comp != NULL) {
      picker->root_node = comp->shadow_root;
    }
  }

  /* The actual populating of radio buttons and switches could be done here. */
  /* For now, we leave the bottom sheet empty or append dummy elements. */
  /* If we need to instantiate all MD3 radio buttons, we'd need
   * md3_radio_button_create etc. */

  *out_picker = picker;
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_theme_picker_get_root(const struct sampler_theme_picker *picker,
                              struct ui_dom_node **out_root) {
  if (picker == NULL || out_root == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  *out_root = picker->root_node;
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_theme_picker_destroy(struct sampler_theme_picker **picker) {
  if (picker == NULL || *picker == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  if ((*picker)->sheet != NULL) {
    md3_bottom_sheet_destroy((*picker)->sheet);
  }

  C_MULTIPLATFORM_FREE(*picker);
  *picker = NULL;

  return SAMPLER_SUCCESS;
}
