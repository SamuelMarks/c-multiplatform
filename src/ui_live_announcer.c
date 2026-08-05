/* clang-format off */
#include "ui_live_announcer.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/* Windows (MSVC) dependencies for platform-specific bindings */
#if defined(_MSC_VER)
/* We will mock the bindings here to keep it strictly C89 and UI-engine
   standalone. In a real implementation, this would call UIA or IUIAutomation.
 */
#endif

struct ui_live_message {
  char *text;
  enum ui_live_politeness politeness;
  struct ui_live_message *next;
};

/** \brief ui_live_announcer */
struct ui_live_announcer {
  struct ui_live_message *queue_head;
  struct ui_live_message *queue_tail;
};

/** \brief ui_error */
ui_error_t ui_live_announcer_create(struct ui_live_announcer **out_announcer) {
  struct ui_live_announcer *announcer;

  if (!out_announcer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  announcer = (struct ui_live_announcer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_live_announcer));
  if (!announcer) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  announcer->queue_head = NULL;
  announcer->queue_tail = NULL;

  *out_announcer = announcer;
  return UI_ERROR_NONE;
}

ui_error_t ui_live_announcer_destroy(struct ui_live_announcer *announcer) {
  if (announcer) {
    ui_error_t rc = ui_live_announcer_clear(announcer);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    C_MULTIPLATFORM_FREE(announcer);
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_live_announce(struct ui_live_announcer *announcer,
                            const char *message,
                            enum ui_live_politeness politeness) {
  struct ui_live_message *msg;
  size_t len;
  char *text_copy;

  if (!announcer || !message) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  len = strlen(message);
  text_copy = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!text_copy) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  UI_STRCPY(text_copy, len + 1, message);

  msg = (struct ui_live_message *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_live_message));
  if (!msg) {
    C_MULTIPLATFORM_FREE(text_copy);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  msg->text = text_copy;
  msg->politeness = politeness;
  msg->next = NULL;

  if (announcer->queue_tail) {
    announcer->queue_tail->next = msg;
  } else {
    announcer->queue_head = msg;
  }
  announcer->queue_tail = msg;

  /* Here we would hook into OS specific platform bindings.
     e.g. macOS NSAccessibilityPostNotification or Windows UIA
     RaiseNotificationEvent */

  return UI_ERROR_NONE;
}

ui_error_t ui_live_announcer_clear(struct ui_live_announcer *announcer) {
  struct ui_live_message *current;
  struct ui_live_message *next;

  if (!announcer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = announcer->queue_head;
  while (current) {
    next = current->next;
    C_MULTIPLATFORM_FREE(current->text);
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }

  announcer->queue_head = NULL;
  announcer->queue_tail = NULL;

  return UI_ERROR_NONE;
}
