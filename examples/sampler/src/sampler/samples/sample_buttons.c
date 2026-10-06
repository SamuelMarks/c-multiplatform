/**
 * @file sample_buttons.c
 * @brief Implementations of interactive button samples.
 */

/* clang-format off */
#include "sampler/sampler_samples.h"
#include "material3/md3_button.h"
#include "ui_dom_node.h"
#include "ui_button_base.h"
#include <stddef.h>
/* clang-format on */

static sampler_error_t create_and_append_button(struct ui_engine *engine,
                                                struct ui_dom_node *container,
                                                enum md3_button_variant variant,
                                                const char *label,
                                                const char *icon) {
  struct md3_button *btn = NULL;
  struct ui_button_base *base = NULL;
  struct ui_component *comp = NULL;
  struct ui_dom_node *root = NULL;
  ui_error_t u_rc;

  u_rc = md3_button_create(engine, variant, &btn);
  if (u_rc != UI_ERROR_NONE) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  u_rc = md3_button_set_text(btn, label);
  if (u_rc != UI_ERROR_NONE) {
    md3_button_destroy(btn);
    return SAMPLER_ERROR_STYLE_PROP_FAILED;
  }

  if (icon != NULL) {
    u_rc = md3_button_set_icon(btn, icon);
    if (u_rc != UI_ERROR_NONE) {
      md3_button_destroy(btn);
      return SAMPLER_ERROR_STYLE_PROP_FAILED;
    }
  }

  u_rc = md3_button_get_base(btn, &base);
  if (u_rc == UI_ERROR_NONE && base != NULL) {
    u_rc = ui_button_base_get_component(base, &comp);
    if (u_rc == UI_ERROR_NONE && comp != NULL) {
      root = comp->shadow_root;
    }
  }

  if (root != NULL) {
    u_rc = ui_dom_node_append_child(container, root);
    if (u_rc != UI_ERROR_NONE) {
      md3_button_destroy(btn);
      return SAMPLER_ERROR_DOM_ATTACH_FAILED;
    }
  } else {
    md3_button_destroy(btn);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  /* Keep a reference if we need to clean it up, but for the sample we assume
   * container cleanup handles children */
  return SAMPLER_SUCCESS;
}

sampler_error_t sample_filled_button(struct ui_engine *engine,
                                     struct ui_dom_node *container) {
  return create_and_append_button(engine, container, MD3_BUTTON_FILLED,
                                  "Filled", NULL);
}

sampler_error_t sample_elevated_button(struct ui_engine *engine,
                                       struct ui_dom_node *container) {
  return create_and_append_button(engine, container, MD3_BUTTON_ELEVATED,
                                  "Elevated", NULL);
}

sampler_error_t sample_filled_tonal_button(struct ui_engine *engine,
                                           struct ui_dom_node *container) {
  return create_and_append_button(engine, container, MD3_BUTTON_FILLED_TONAL,
                                  "Tonal", NULL);
}

sampler_error_t sample_outlined_button(struct ui_engine *engine,
                                       struct ui_dom_node *container) {
  return create_and_append_button(engine, container, MD3_BUTTON_OUTLINED,
                                  "Outlined", NULL);
}

sampler_error_t sample_text_button(struct ui_engine *engine,
                                   struct ui_dom_node *container) {
  return create_and_append_button(engine, container, MD3_BUTTON_TEXT, "Text",
                                  NULL);
}

sampler_error_t sample_button_with_icon(struct ui_engine *engine,
                                        struct ui_dom_node *container) {
  return create_and_append_button(engine, container, MD3_BUTTON_FILLED,
                                  "Icon Button", "add");
}

sampler_error_t sample_button_animated_shape(struct ui_engine *engine,
                                             struct ui_dom_node *container) {
  /* No explicit animated shape variant in md3_button yet, map to filled */
  return create_and_append_button(engine, container, MD3_BUTTON_FILLED,
                                  "Animated Shape", NULL);
}
