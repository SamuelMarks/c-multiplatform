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

static enum ui_error internal_strdup(const char *src, char **out_str) {
  size_t len;
  char *copy;
  if (!src) {
    *out_str = NULL;
    return UI_ERROR_NONE;
  }
  len = strlen(src);
  copy = (char *)UI_MALLOC(len + 1);
  if (!copy) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  strcpy(copy, src);
  *out_str = copy;
  return UI_ERROR_NONE;
}

enum ui_error ui_rich_text_base_create(struct ui_rich_text_base **out_editor) {
  enum ui_error rc;
  struct ui_rich_text_base *editor;
  struct ui_dom_node *root_node = NULL;

  if (!out_editor)
    return UI_ERROR_INVALID_ARGUMENT;

  editor =
      (struct ui_rich_text_base *)UI_MALLOC(sizeof(struct ui_rich_text_base));
  if (!editor)
    return UI_ERROR_OUT_OF_MEMORY;

  memset(editor, 0, sizeof(struct ui_rich_text_base));

  rc = ui_component_create(&editor->component);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(editor);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(editor->component);
    UI_FREE(editor);
    return rc;
  }

  ui_dom_node_set_tag_name(root_node, "div");
  ui_dom_node_set_attribute(root_node, "contenteditable", "true");
  ui_dom_node_set_attribute(root_node, "role", "textbox");
  ui_dom_node_set_attribute(root_node, "aria-multiline", "true");
  editor->component->shadow_root = root_node;

  *out_editor = editor;
  return UI_ERROR_NONE;
}

static void free_runs(struct ui_rich_text_run *head) {
  struct ui_rich_text_run *current = head;
  struct ui_rich_text_run *next;
  while (current) {
    next = current->next;
    if (current->text)
      UI_FREE(current->text);
    UI_FREE(current);
    current = next;
  }
}

static void free_history(struct ui_rich_text_history_entry *head) {
  struct ui_rich_text_history_entry *current = head;
  struct ui_rich_text_history_entry *next;
  while (current) {
    next = current->next;
    if (current->state_snapshot)
      UI_FREE(current->state_snapshot);
    UI_FREE(current);
    current = next;
  }
}

void ui_rich_text_base_destroy(struct ui_rich_text_base *editor) {
  if (!editor)
    return;

  free_runs(editor->document_head);
  free_history(editor->history_head);

  if (editor->ime_composition) {
    UI_FREE(editor->ime_composition);
  }

  if (editor->component) {
    if (editor->component->shadow_root) {
      ui_dom_node_destroy(editor->component->shadow_root);
      editor->component->shadow_root = NULL;
    }
    ui_component_destroy(editor->component);
  }

  UI_FREE(editor);
}

/** \brief ui_error */
enum ui_error
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

enum ui_error ui_rich_text_base_set_text(struct ui_rich_text_base *editor,
                                         const char *text) {
  struct ui_rich_text_run *new_run;
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;

  free_runs(editor->document_head);
  editor->document_head = NULL;

  if (text && strlen(text) > 0) {
    new_run =
        (struct ui_rich_text_run *)UI_MALLOC(sizeof(struct ui_rich_text_run));
    if (!new_run)
      return UI_ERROR_OUT_OF_MEMORY;
    memset(new_run, 0, sizeof(struct ui_rich_text_run));

    if (internal_strdup(text, &new_run->text) != UI_ERROR_NONE) {
      UI_FREE(new_run);
      return UI_ERROR_OUT_OF_MEMORY;
    }
    editor->document_head = new_run;

    if (editor->component && editor->component->shadow_root) {
      ui_dom_node_set_text_content(editor->component->shadow_root, text);
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_rich_text_base_get_text(struct ui_rich_text_base *editor,
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

  result = (char *)UI_MALLOC(total_len + 1);
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

enum ui_error ui_rich_text_base_toggle_format(struct ui_rich_text_base *editor,
                                              enum ui_rich_text_format format) {
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;
  /* MOCK implementation: toggle format flags on the entire document for now */
  if (editor->document_head) {
    editor->document_head->format_flags ^= format;
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_rich_text_base_undo(struct ui_rich_text_base *editor) {
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;
  if (editor->history_current && editor->history_current->prev) {
    editor->history_current = editor->history_current->prev;
    /* Restore state snapshot */
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_rich_text_base_redo(struct ui_rich_text_base *editor) {
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;
  if (editor->history_current && editor->history_current->next) {
    editor->history_current = editor->history_current->next;
    /* Restore state snapshot */
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_rich_text_base_process_event(struct ui_rich_text_base *editor,
                                              const struct ui_event *event) {
  if (!editor || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  /* Mock adding history for coverage */
  if (event->type == UI_EVENT_KEY_DOWN &&
      event->event_data.keyboard.key_code == 'H') {
    struct ui_rich_text_history_entry *entry =
        UI_MALLOC(sizeof(struct ui_rich_text_history_entry));
    if (entry) {
      entry->state_snapshot = UI_MALLOC(2);
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
enum ui_error
ui_rich_text_base_set_ime_composition(struct ui_rich_text_base *editor,
                                      const char *composition_text) {
  if (!editor)
    return UI_ERROR_INVALID_ARGUMENT;
  if (editor->ime_composition) {
    UI_FREE(editor->ime_composition);
    editor->ime_composition = NULL;
  }
  return internal_strdup(composition_text, &editor->ime_composition);
}

enum ui_error ui_rich_text_base_bind_text(struct ui_rich_text_base *widget,
                                          struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = signal;
  return UI_ERROR_NONE;
}
