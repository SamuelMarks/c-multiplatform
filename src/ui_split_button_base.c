/* clang-format off */
#include "ui_split_button_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

/**
 * \file ui_split_button_base.c
 * \brief Split button base component implementation.
 */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_split_button_base_default_css =
    "div.split-button { "
    "display: flex; "
    "flex-direction: row; "
    "align-items: stretch; "
    "gap: var(--split-btn-gap, 1px); "
    "background: var(--split-btn-bg, transparent); "
    "}";

/**
 * \brief ui_split_button_base structure.
 * \details Internal state for the split button base component.
 */
struct ui_split_button_base {
  struct ui_component *component;
  struct ui_button_base *main_button;
  struct ui_button_base *trigger_button;
  struct ui_signal *disabled_signal;
  struct ui_signal *text_signal;
};

/**
 * \brief Creates a new split button base component.
 * \param out_split_button Pointer to store the component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_button_base_create(struct ui_split_button_base **out_split_button) {
  struct ui_split_button_base *split_btn;
  ui_error_t rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_split_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  split_btn = (struct ui_split_button_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_split_button_base));
  if (!split_btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  split_btn->component = NULL;
  split_btn->main_button = NULL;
  split_btn->trigger_button = NULL;

  rc = ui_component_create(&split_btn->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_button_base_create(&split_btn->main_button);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_button_base_create(&split_btn->trigger_button);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "class", "split-button");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Mount child components to the root node */
  {
    struct ui_component *tmp_comp;
    {
      ui_error_t _ign_rc =
          ui_button_base_get_component(split_btn->main_button, &tmp_comp);
      (void)_ign_rc;
    }
    {
      ui_error_t _ign_rc =
          ui_dom_node_append_child(root_node, tmp_comp->shadow_root);
      (void)_ign_rc;
    }

    {

      ui_error_t _ign_rc =
          ui_button_base_get_component(split_btn->trigger_button, &tmp_comp);

      (void)_ign_rc;
    }
    {
      ui_error_t _ign_rc =
          ui_dom_node_append_child(root_node, tmp_comp->shadow_root);
      (void)_ign_rc;
    }
  }

  rc =
      ui_css_parse_stylesheet(ui_split_button_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  {

    ui_error_t _ign_rc =
        ui_component_set_default_style(split_btn->component, default_style);

    (void)_ign_rc;
  }

  split_btn->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  *out_split_button = split_btn;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    (void)ui_dom_node_destroy(root_node);
  }
  if (split_btn->trigger_button) {
    struct ui_component *tmp_comp;
    {
      ui_error_t _ign_rc =
          ui_button_base_get_component(split_btn->trigger_button, &tmp_comp);
      (void)_ign_rc;
    }
    tmp_comp->shadow_root = NULL;
    (void)ui_button_base_destroy(split_btn->trigger_button);
  }
  if (split_btn->main_button) {
    struct ui_component *tmp_comp;
    {
      ui_error_t _ign_rc =
          ui_button_base_get_component(split_btn->main_button, &tmp_comp);
      (void)_ign_rc;
    }
    tmp_comp->shadow_root = NULL;
    (void)ui_button_base_destroy(split_btn->main_button);
  }
  if (split_btn->component) {
    (void)ui_component_destroy(split_btn->component);
  }
  C_MULTIPLATFORM_FREE(split_btn);
  return rc;
}

/**
 * \brief Destroys a split button base component.
 * \param split_button The component to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_button_base_destroy(struct ui_split_button_base *split_button) {
  struct ui_component *tmp_comp;

  if (!split_button) {
    return UI_ERROR_NONE;
  }

  /* Unlink from parent to prevent double free */
  {
    ui_error_t _ign_rc =
        ui_button_base_get_component(split_button->trigger_button, &tmp_comp);
    (void)_ign_rc;
  }
  tmp_comp->shadow_root = NULL;
  (void)ui_button_base_destroy(split_button->trigger_button);

  /* Unlink from parent to prevent double free */
  {
    ui_error_t _ign_rc =
        ui_button_base_get_component(split_button->main_button, &tmp_comp);
    (void)_ign_rc;
  }
  tmp_comp->shadow_root = NULL;
  (void)ui_button_base_destroy(split_button->main_button);

  (void)ui_component_destroy(split_button->component);

  C_MULTIPLATFORM_FREE(split_button);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the disabled state.
 * \param split_button The split button component.
 * \param disabled The disabled state.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_button_base_set_disabled(struct ui_split_button_base *split_button,
                                  int disabled) {
  if (!split_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {

    ui_error_t _ign_rc =
        ui_button_base_set_disabled(split_button->main_button, disabled);

    (void)_ign_rc;
  }
  {
    ui_error_t _ign_rc =
        ui_button_base_set_disabled(split_button->trigger_button, disabled);
    (void)_ign_rc;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets the main button.
 * \param split_button The split button component.
 * \param out_main_btn Pointer to store the main button.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_button_base_get_main_button(struct ui_split_button_base *split_button,
                                     struct ui_button_base **out_main_btn) {
  if (!split_button || !out_main_btn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_main_btn = split_button->main_button;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the trigger button.
 * \param split_button The split button component.
 * \param out_trigger_btn Pointer to store the trigger button.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_split_button_base_get_trigger_button(
    struct ui_split_button_base *split_button,
    struct ui_button_base **out_trigger_btn) {
  if (!split_button || !out_trigger_btn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_trigger_btn = split_button->trigger_button;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the base component for the split button.
 * \param split_button The split button component.
 * \param out_comp Pointer to store the component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_button_base_get_component(struct ui_split_button_base *split_button,
                                   struct ui_component **out_comp) {
  if (!split_button || !out_comp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_comp = split_button->component;
  return UI_ERROR_NONE;
}

/**
 * \brief Binds the disabled state to a signal.
 * \param widget The split button component.
 * \param disabled_signal The signal to bind.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_split_button_base_bind_disabled(struct ui_split_button_base *widget,
                                   struct ui_signal *disabled_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->disabled_signal = disabled_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Binds the text state to a signal.
 * \param widget The split button component.
 * \param text_signal The signal to bind.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_split_button_base_bind_text(struct ui_split_button_base *widget,
                                          struct ui_signal *text_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = text_signal;
  return UI_ERROR_NONE;
}
