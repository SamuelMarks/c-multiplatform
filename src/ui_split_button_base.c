/**
 * @file ui_split_button_base.c
 * @brief ui_split_button_base.c implementation.
 */
/* clang-format off */
#include "ui_split_button_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_split_button_mock_destroy_fail = 0;
int g_split_button_mock_btn_destroy_fail = 0;
int g_split_button_mock_get_comp_fail = 0;
int g_split_button_mock_append_fail = 0;
int g_split_button_mock_node_destroy_fail = 0;
int g_split_button_mock_parse_css_fail = 0;
int g_split_button_mock_set_style_fail = 0;

/**
 * @brief mock_split_button_parse_css.
 * @param css Parameter css.
 * @param out_sheet Parameter out_sheet.
 * @return Return value.
 */
static ui_error_t
mock_split_button_parse_css(const char *css,
                            struct ui_css_stylesheet **out_sheet) {
  if (g_split_button_mock_parse_css_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_css_parse_stylesheet)(css, out_sheet);
}
#undef ui_css_parse_stylesheet
/** @cond */
#define ui_css_parse_stylesheet mock_split_button_parse_css
/** @endcond */

/**
 * @brief mock_split_button_set_default_style.
 * @param comp Parameter comp.
 * @param sheet Parameter sheet.
 * @return Return value.
 */
static ui_error_t
mock_split_button_set_default_style(struct ui_component *comp,
                                    struct ui_css_stylesheet *sheet) {
  if (g_split_button_mock_set_style_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_set_default_style)(comp, sheet);
}
#undef ui_component_set_default_style
/** @cond */
#define ui_component_set_default_style mock_split_button_set_default_style
/** @endcond */

/**
 * @brief mock_split_button_component_destroy.
 * @param comp Parameter comp.
 * @return Return value.
 */
static ui_error_t
mock_split_button_component_destroy(struct ui_component *comp) {
  if (g_split_button_mock_destroy_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_split_button_component_destroy
/** @endcond */

/**
 * @brief mock_split_button_btn_destroy.
 * @param btn Parameter btn.
 * @return Return value.
 */
static ui_error_t mock_split_button_btn_destroy(struct ui_button_base *btn) {
  if (g_split_button_mock_btn_destroy_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_button_base_destroy)(btn);
}
#undef ui_button_base_destroy
/** @cond */
#define ui_button_base_destroy mock_split_button_btn_destroy
/** @endcond */

/**
 * @brief mock_split_button_get_component.
 * @param btn Parameter btn.
 * @param comp Parameter comp.
 * @return Return value.
 */
static ui_error_t mock_split_button_get_component(struct ui_button_base *btn,
                                                  struct ui_component **comp) {
  if (g_split_button_mock_get_comp_fail > 0) {
    g_split_button_mock_get_comp_fail--;
    if (g_split_button_mock_get_comp_fail == 0) {
      return UI_ERROR_UNKNOWN;
    }
  }
  return (ui_button_base_get_component)(btn, comp);
}
#undef ui_button_base_get_component
/** @cond */
#define ui_button_base_get_component mock_split_button_get_component
/** @endcond */

/**
 * @brief mock_split_button_append_child.
 * @param parent Parameter parent.
 * @param child Parameter child.
 * @return Return value.
 */
static ui_error_t mock_split_button_append_child(struct ui_dom_node *parent,
                                                 struct ui_dom_node *child) {
  if (g_split_button_mock_append_fail > 0) {
    g_split_button_mock_append_fail--;
    if (g_split_button_mock_append_fail == 0) {
      return UI_ERROR_UNKNOWN;
    }
  }
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_split_button_append_child
/** @endcond */

/**
 * @brief mock_split_button_node_destroy.
 * @param node Parameter node.
 * @return Return value.
 */
static ui_error_t mock_split_button_node_destroy(struct ui_dom_node *node) {
  if (g_split_button_mock_node_destroy_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_destroy)(node);
}
#undef ui_dom_node_destroy
/** @cond */
#define ui_dom_node_destroy mock_split_button_node_destroy
/** @endcond */
#endif

/*
 * \file ui_split_button_base.c
 * \brief Split button base component implementation.
 */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/** @brief Default CSS stylesheet */
static const char *ui_split_button_base_default_css =
    "div.split-button { "
    "display: flex; "
    "flex-direction: row; "
    "align-items: stretch; "
    "gap: var(--split-btn-gap, 1px); "
    "background: var(--split-btn-bg, transparent); "
    "}";

/**
 * @struct ui_split_button_base
 * \brief ui_split_button_base structure.
 * \details Internal state for the split button base component.
 */
struct ui_split_button_base {
  struct ui_component *component;        /**< component */
  struct ui_button_base *main_button;    /**< main_button */
  struct ui_button_base *trigger_button; /**< trigger_button */
  struct ui_signal *disabled_signal;     /**< disabled_signal */
  struct ui_signal *text_signal;         /**< text_signal */
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
  struct ui_component *tmp_comp = NULL;
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
  rc = ui_button_base_get_component(split_btn->main_button, &tmp_comp);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_append_child(root_node, tmp_comp->shadow_root);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_button_base_get_component(split_btn->trigger_button, &tmp_comp);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_append_child(root_node, tmp_comp->shadow_root);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc =
      ui_css_parse_stylesheet(ui_split_button_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_set_default_style(split_btn->component, default_style);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(default_style);
    goto cleanup;
  }

  split_btn->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  *out_split_button = split_btn;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    tmp_comp = NULL;
    ui_button_base_get_component(split_btn->trigger_button, &tmp_comp);
    if (tmp_comp->shadow_root->parent == root_node) {
      ui_dom_node_remove_child(root_node, tmp_comp->shadow_root);
    }
    tmp_comp = NULL;
    ui_button_base_get_component(split_btn->main_button, &tmp_comp);
    if (tmp_comp->shadow_root->parent == root_node) {
      ui_dom_node_remove_child(root_node, tmp_comp->shadow_root);
    }
    ui_dom_node_destroy(root_node);
  }
  if (split_btn->trigger_button) {
    ui_button_base_destroy(split_btn->trigger_button);
  }
  if (split_btn->main_button) {
    ui_button_base_destroy(split_btn->main_button);
  }
  if (split_btn->component) {
    ui_component_destroy(split_btn->component);
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
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t rc_cleanup;

  if (!split_button) {
    return UI_ERROR_NONE;
  }

  if (split_button->trigger_button) {
    tmp_comp = NULL;
    ui_button_base_get_component(split_button->trigger_button, &tmp_comp);
    if (split_button->component && tmp_comp) {
      ui_dom_node_remove_child(split_button->component->shadow_root,
                               tmp_comp->shadow_root);
    }
    rc_cleanup = ui_button_base_destroy(split_button->trigger_button);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }

  if (split_button->main_button) {
    tmp_comp = NULL;
    ui_button_base_get_component(split_button->main_button, &tmp_comp);
    if (split_button->component && tmp_comp) {
      ui_dom_node_remove_child(split_button->component->shadow_root,
                               tmp_comp->shadow_root);
    }
    rc_cleanup = ui_button_base_destroy(split_button->main_button);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }

  if (split_button->component) {
    split_button->component->shadow_root = NULL;
    rc_cleanup = ui_component_destroy(split_button->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }

  C_MULTIPLATFORM_FREE(split_button);
  return rc;
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
  ui_error_t rc;

  if (!split_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_button_base_set_disabled(split_button->main_button, disabled);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return ui_button_base_set_disabled(split_button->trigger_button, disabled);
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
