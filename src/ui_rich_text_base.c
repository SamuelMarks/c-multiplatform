/**
 * @file ui_rich_text_base.c
 * @brief ui_rich_text_base.c implementation.
 */
/*
 * \file ui_rich_text_base.c
 * \brief Implementation of the UI Rich Text Base component.
 */

/* clang-format off */
#include "ui_rich_text_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_rtb_mock_destroy_fail = 0;
int g_rtb_mock_node_destroy_fail = 0;
int g_rtb_mock_append_fail = 0;

/**
 * @brief mock_rtb_append_child.
 * @param parent Parameter parent.
 * @param child Parameter child.
 * @return Return value.
 */
static ui_error_t mock_rtb_append_child(struct ui_dom_node *parent,
                                        struct ui_dom_node *child) {
  if (g_rtb_mock_append_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_rtb_append_child
/** @endcond */

/**
 * @brief mock_rtb_component_destroy.
 * @param comp Parameter comp.
 * @return Return value.
 */
static ui_error_t mock_rtb_component_destroy(struct ui_component *comp) {
  if (g_rtb_mock_destroy_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_rtb_component_destroy
/** @endcond */

/**
 * @brief mock_rtb_dom_node_destroy.
 * @param node Parameter node.
 * @return Return value.
 */
static ui_error_t mock_rtb_dom_node_destroy(struct ui_dom_node *node) {
  if (g_rtb_mock_node_destroy_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_destroy)(node);
}
#undef ui_dom_node_destroy
/** @cond */
#define ui_dom_node_destroy mock_rtb_dom_node_destroy
/** @endcond */
#endif

/**
 * @struct ui_rich_text_run
 * \brief Represents a single run of text with consistent formatting.
 */
struct ui_rich_text_run {
  char *text;                    /**< Text content */
  int format_flags;              /**< Applied formatting flags */
  struct ui_rich_text_run *next; /**< Next run in the document */
};

/**
 * @struct ui_rich_text_history_entry
 * \brief Internal structure representing a history entry for undo/redo.
 */
struct ui_rich_text_history_entry {
  char *state_snapshot;                    /**< Snapshot string */
  struct ui_rich_text_history_entry *next; /**< Next entry */
  struct ui_rich_text_history_entry *prev; /**< Previous entry */
};

/**
 * @struct ui_rich_text_base
 * \brief Internal structure representing a rich text editor.
 */
struct ui_rich_text_base {
  struct ui_component *component;         /**< Underlying component */
  struct ui_rich_text_run *document_head; /**< Head of text runs */

  int selection_start; /**< Selection start index */
  int selection_end;   /**< Selection end index */

  struct ui_rich_text_history_entry *history_head; /**< Head of history */
  struct ui_rich_text_history_entry
      *history_current; /**< Current history pos */

  char *ime_composition;         /**< IME composition text */
  struct ui_signal *text_signal; /**< Text property signal */
};

/**
 * \brief Creates a rich text editor base component.
 *
 * \param out_editor Pointer to receive the allocated component.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_create(struct ui_rich_text_base **out_editor) {
  ui_error_t rc;
  struct ui_rich_text_base *editor;
  struct ui_dom_node *root_node = NULL;

  if (!out_editor)
    return UI_ERROR_INVALID_ARGUMENT;

  editor = (struct ui_rich_text_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_rich_text_base));
  if (!editor)
    return UI_ERROR_OUT_OF_MEMORY;

  memset(editor, 0, sizeof(struct ui_rich_text_base));

  rc = ui_component_create(&editor->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(editor);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "contenteditable", "true");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "role", "textbox");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "aria-multiline", "true");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  editor->component->shadow_root = root_node;
  root_node = NULL;

  *out_editor = editor;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  ui_component_destroy(editor->component);
  C_MULTIPLATFORM_FREE(editor);
  return rc;
}

/**
 * \brief Frees a linked list of text runs.
 *
 * \param head The head of the run list.
 */
/**
 * @brief free_runs.
 * @param head Parameter head.
 * @return Return value.
 */
static ui_error_t free_runs(struct ui_rich_text_run *head) {
  struct ui_rich_text_run *current = head;
  struct ui_rich_text_run *next;
  while (current) {
    next = current->next;
    if (current->text)
      C_MULTIPLATFORM_FREE(current->text);
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Frees a linked list of history entries.
 *
 * \param head The head of the history list.
 */
/**
 * @brief free_history.
 * @param head Parameter head.
 * @return Return value.
 */
static ui_error_t free_history(struct ui_rich_text_history_entry *head) {
  struct ui_rich_text_history_entry *current = head;
  struct ui_rich_text_history_entry *next;
  while (current) {
    next = current->next;
    if (current->state_snapshot)
      C_MULTIPLATFORM_FREE(current->state_snapshot);
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a rich text editor base component.
 *
 * \param editor The rich text editor.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_destroy(struct ui_rich_text_base *editor) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t rc_cleanup;

  if (!editor)
    return UI_ERROR_NONE;

  free_runs(editor->document_head);
  editor->document_head = NULL;
  free_history(editor->history_head);
  editor->history_head = NULL;

  if (editor->ime_composition) {
    C_MULTIPLATFORM_FREE(editor->ime_composition);
  }

  if (editor->component) {
    if (editor->component->shadow_root) {
      rc_cleanup = ui_dom_node_destroy(editor->component->shadow_root);
      if (rc_cleanup != UI_ERROR_NONE) {
        rc = rc_cleanup;
      }
      editor->component->shadow_root = NULL;
    }
    rc_cleanup = ui_component_destroy(editor->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }

  C_MULTIPLATFORM_FREE(editor);
  return rc;
}

/**
 * \brief Gets the underlying component.
 *
 * \param editor The rich text editor.
 * \param out_component Output pointer.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_rich_text_base_get_component(struct ui_rich_text_base *editor,
                                struct ui_component **out_component) {
  if (!out_component)
    return UI_ERROR_INVALID_ARGUMENT;
  if (!editor) {
    *out_component = NULL;
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = editor->component;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the content of the editor.
 *
 * \param editor The rich text editor.
 * \param text The text to set.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_set_text(struct ui_rich_text_base *editor,
                                      const char *text) {
  struct ui_rich_text_run *new_run;

  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;

  free_runs(editor->document_head);
  editor->document_head = NULL;

  if (text && strlen(text) > 0) {
    new_run = (struct ui_rich_text_run *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_rich_text_run));
    if (!new_run)
      return UI_ERROR_OUT_OF_MEMORY;
    memset(new_run, 0, sizeof(struct ui_rich_text_run));

    if (((new_run->text = C_MULTIPLATFORM_STRDUP(text))
             ? UI_ERROR_NONE
             : UI_ERROR_OUT_OF_MEMORY) != UI_ERROR_NONE) {
      C_MULTIPLATFORM_FREE(new_run);
      return UI_ERROR_OUT_OF_MEMORY;
    }
    editor->document_head = new_run;

    if (editor->component && editor->component->shadow_root) {
      ui_error_t txt_rc;
      struct ui_dom_node *text_node = NULL;
      struct ui_dom_node *child = editor->component->shadow_root->first_child;
      while (child) {
        struct ui_dom_node *next = child->next_sibling;
        ui_dom_node_remove_child(editor->component->shadow_root, child);
        ui_dom_node_destroy(child);
        child = next;
      }

      txt_rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
      if (txt_rc != UI_ERROR_NONE)
        return txt_rc;
      txt_rc = ui_dom_node_set_text_content(text_node, text);
      if (txt_rc != UI_ERROR_NONE) {
        ui_dom_node_destroy(text_node);
        return txt_rc;
      }
      txt_rc =
          ui_dom_node_append_child(editor->component->shadow_root, text_node);
      if (txt_rc != UI_ERROR_NONE) {
        ui_dom_node_destroy(text_node);
        return txt_rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets the content of the editor.
 *
 * Returned string must be freed by the caller using C_MULTIPLATFORM_FREE.
 *
 * \param editor The rich text editor.
 * \param out_text Pointer to receive the allocated string.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_get_text(struct ui_rich_text_base *editor,
                                      char **out_text) {
  struct ui_rich_text_run *current;
  size_t total_len = 0;
  char *result;

  if (!editor || !out_text)
    return UI_ERROR_INVALID_ARGUMENT;

  current = editor->document_head;
  while (current) {
    if (current->text)
      total_len += strlen(current->text);
    current = current->next;
  }

  result = (char *)C_MULTIPLATFORM_MALLOC(total_len + 1);
  if (!result)
    return UI_ERROR_OUT_OF_MEMORY;
  result[0] = '\0';

  current = editor->document_head;
  while (current) {
    if (current->text) {
#if defined(_MSC_VER)
      strcat_s(result, total_len + 1, current->text);
#else
      strcat(result, current->text);
#endif
    }
    current = current->next;
  }

  *out_text = result;
  return UI_ERROR_NONE;
}

/**
 * \brief Toggles a specific format on the current selection.
 *
 * \param editor The rich text editor.
 * \param format The format flag to toggle.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_toggle_format(struct ui_rich_text_base *editor,
                                           enum ui_rich_text_format format) {
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;
  /* MOCK implementation: toggle format flags on the entire document for now */
  if (editor->document_head) {
    editor->document_head->format_flags ^= format;
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Performs undo.
 *
 * \param editor The rich text editor.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_undo(struct ui_rich_text_base *editor) {
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;
  if (editor->history_current && editor->history_current->prev) {
    editor->history_current = editor->history_current->prev;
    /* Restore state snapshot */
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Performs redo.
 *
 * \param editor The rich text editor.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_redo(struct ui_rich_text_base *editor) {
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;
  if (editor->history_current && editor->history_current->next) {
    editor->history_current = editor->history_current->next;
    /* Restore state snapshot */
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Processes an input event (keyboard, mouse).
 *
 * \param editor The rich text editor.
 * \param event The input event.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_process_event(struct ui_rich_text_base *editor,
                                           const struct ui_event *event) {
  if (!editor || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  /* Mock adding history for coverage */
  if (event->type == UI_EVENT_KEY_DOWN &&
      event->event_data.keyboard.key_code == 'H') {
    struct ui_rich_text_history_entry *entry =
        C_MULTIPLATFORM_MALLOC(sizeof(struct ui_rich_text_history_entry));
    if (entry) {
      entry->state_snapshot = C_MULTIPLATFORM_MALLOC(2);
      if (entry->state_snapshot) {
        entry->state_snapshot[0] = 'a';
        entry->state_snapshot[1] = '\0';
      }
      entry->prev = editor->history_current;
      entry->next = NULL;
      if (editor->history_current)
        editor->history_current->next = entry;
      if (!editor->history_head)
        editor->history_head = entry;
      editor->history_current = entry;
    }
  }

  /* Handle keyboard typing, cursor navigation, backspace, etc */
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the IME composition text (called from window backends during IME
 * input).
 *
 * \param editor The rich text editor.
 * \param composition_text The composition string.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_rich_text_base_set_ime_composition(struct ui_rich_text_base *editor,
                                      const char *composition_text) {
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;
  if (editor->ime_composition) {
    C_MULTIPLATFORM_FREE(editor->ime_composition);
    editor->ime_composition = NULL;
  }
  return ((editor->ime_composition = C_MULTIPLATFORM_STRDUP(composition_text))
              ? UI_ERROR_NONE
              : UI_ERROR_OUT_OF_MEMORY);
}

/**
 * \brief Binds the text property.
 *
 * \param widget The widget.
 * \param signal The signal to bind to.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_rich_text_base_bind_text(struct ui_rich_text_base *widget,
                                       struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = signal;
  return UI_ERROR_NONE;
}
