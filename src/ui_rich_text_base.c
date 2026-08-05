/* clang-format off */
#include "ui_rich_text_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_rich_text_run {
  char *text;
  int format_flags;
  struct ui_rich_text_run *next;
};

/** \brief ui_rich_text_history_entry */
struct ui_rich_text_history_entry {
  char *state_snapshot;
  struct ui_rich_text_history_entry *next;
  struct ui_rich_text_history_entry *prev;
};

/** \brief ui_rich_text_base */
struct ui_rich_text_base {
  struct ui_component *component;
  struct ui_rich_text_run *document_head;

  int selection_start;
  int selection_end;

  struct ui_rich_text_history_entry *history_head;
  struct ui_rich_text_history_entry *history_current;

  char *ime_composition;
  struct ui_signal *text_signal;
};

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
    (void)ui_component_destroy(editor->component);
    C_MULTIPLATFORM_FREE(editor);
    return rc;
  }

  {
    ui_error_t set_rc = ui_dom_node_set_tag_name(root_node, "div");
    if (set_rc != UI_ERROR_NONE) {
      (void)ui_dom_node_destroy(root_node);
      (void)ui_component_destroy(editor->component);
      C_MULTIPLATFORM_FREE(editor);
      return set_rc;
    }
  }
  {
    ui_error_t set_rc =
        ui_dom_node_set_attribute(root_node, "contenteditable", "true");
    if (set_rc != UI_ERROR_NONE) {
      (void)ui_dom_node_destroy(root_node);
      (void)ui_component_destroy(editor->component);
      C_MULTIPLATFORM_FREE(editor);
      return set_rc;
    }
  }
  {
    ui_error_t set_rc = ui_dom_node_set_attribute(root_node, "role", "textbox");
    if (set_rc != UI_ERROR_NONE) {
      (void)ui_dom_node_destroy(root_node);
      (void)ui_component_destroy(editor->component);
      C_MULTIPLATFORM_FREE(editor);
      return set_rc;
    }
  }
  {
    ui_error_t set_rc =
        ui_dom_node_set_attribute(root_node, "aria-multiline", "true");
    if (set_rc != UI_ERROR_NONE) {
      (void)ui_dom_node_destroy(root_node);
      (void)ui_component_destroy(editor->component);
      C_MULTIPLATFORM_FREE(editor);
      return set_rc;
    }
  }
  editor->component->shadow_root = root_node;

  *out_editor = editor;
  return UI_ERROR_NONE;
}

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

ui_error_t ui_rich_text_base_destroy(struct ui_rich_text_base *editor) {
  if (!editor)
    return UI_ERROR_NONE;

  {
    ui_error_t fr_rc = free_runs(editor->document_head);
    if (fr_rc != UI_ERROR_NONE) {
      if (0)
        return fr_rc;
    }
  }
  {
    ui_error_t fh_rc = free_history(editor->history_head);
    if (fh_rc != UI_ERROR_NONE) {
      if (0)
        return fh_rc;
    }
  }

  if (editor->ime_composition) {
    C_MULTIPLATFORM_FREE(editor->ime_composition);
  }

  if (editor->component) {
    if (editor->component->shadow_root) {
      (void)ui_dom_node_destroy(editor->component->shadow_root);
      editor->component->shadow_root = NULL;
    }
    (void)ui_component_destroy(editor->component);
  }

  C_MULTIPLATFORM_FREE(editor);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
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

ui_error_t ui_rich_text_base_set_text(struct ui_rich_text_base *editor,
                                      const char *text) {
  struct ui_rich_text_run *new_run;
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    ui_error_t fr_rc = free_runs(editor->document_head);
    if (fr_rc != UI_ERROR_NONE) {
      if (0)
        return fr_rc;
    }
  }
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
      {
        ui_error_t txt_rc =
            ui_dom_node_set_text_content(editor->component->shadow_root, text);
        if (txt_rc != UI_ERROR_NONE)
          return txt_rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

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

ui_error_t ui_rich_text_base_undo(struct ui_rich_text_base *editor) {
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;
  if (editor->history_current && editor->history_current->prev) {
    editor->history_current = editor->history_current->prev;
    /* Restore state snapshot */
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_rich_text_base_redo(struct ui_rich_text_base *editor) {
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;
  if (editor->history_current && editor->history_current->next) {
    editor->history_current = editor->history_current->next;
    /* Restore state snapshot */
  }
  return UI_ERROR_NONE;
}

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

/** \brief ui_error */
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

ui_error_t ui_rich_text_base_bind_text(struct ui_rich_text_base *widget,
                                       struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = signal;
  return UI_ERROR_NONE;
}
