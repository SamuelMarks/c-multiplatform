/**
 * @file cupertino_alert.c
 * @brief Cupertino Alert Dialog component implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_alert.h"
#include "cupertino/cupertino_spring.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_alert_mock_base_create_fail = 0;
int g_cupertino_alert_mock_base_set_role_fail = 0;
int g_cupertino_alert_mock_base_set_dismissible_fail = 0;
int g_cupertino_alert_mock_base_set_open_fail = 0;
int g_cupertino_alert_mock_base_destroy_fail = 0;
int g_cupertino_alert_mock_spring_get_preset_fail = 0;
int g_cupertino_alert_mock_spring_evaluate_fail = 0;
struct ui_alert_base *g_cupertino_alert_last_created_base = NULL;

static ui_error_t mock_alert_base_create(struct ui_alert_base **out_alert) {
  ui_error_t rc;
  if (g_cupertino_alert_mock_base_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  rc = ui_alert_base_create(out_alert);
  if (rc == UI_ERROR_NONE) {
    g_cupertino_alert_last_created_base = *out_alert;
  }
  return rc;
}
#undef ui_alert_base_create
/** @cond */
#define ui_alert_base_create mock_alert_base_create
/** @endcond */

static ui_error_t mock_alert_base_set_role(struct ui_alert_base *alert,
                                           enum ui_alert_role role) {
  if (g_cupertino_alert_mock_base_set_role_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_alert_base_set_role(alert, role);
}
#undef ui_alert_base_set_role
/** @cond */
#define ui_alert_base_set_role mock_alert_base_set_role
/** @endcond */

static ui_error_t mock_alert_base_set_dismissible(struct ui_alert_base *alert,
                                                  int dismissible) {
  if (g_cupertino_alert_mock_base_set_dismissible_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_alert_base_set_dismissible(alert, dismissible);
}
#undef ui_alert_base_set_dismissible
/** @cond */
#define ui_alert_base_set_dismissible mock_alert_base_set_dismissible
/** @endcond */

static ui_error_t mock_alert_base_set_open(struct ui_alert_base *alert,
                                           int is_open) {
  if (g_cupertino_alert_mock_base_set_open_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_alert_base_set_open(alert, is_open);
}
#undef ui_alert_base_set_open
/** @cond */
#define ui_alert_base_set_open mock_alert_base_set_open
/** @endcond */

static ui_error_t mock_alert_base_destroy(struct ui_alert_base *alert) {
  if (g_cupertino_alert_mock_base_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_alert_base_destroy)(alert);
}
#undef ui_alert_base_destroy
/** @cond */
#define ui_alert_base_destroy mock_alert_base_destroy
/** @endcond */

static ui_error_t
mock_cupertino_spring_get_preset(enum cupertino_spring_preset preset,
                                 struct cupertino_spring_config *out_config) {
  if (g_cupertino_alert_mock_spring_get_preset_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return cupertino_spring_get_preset(preset, out_config);
}
#undef cupertino_spring_get_preset
/** @cond */
#define cupertino_spring_get_preset mock_cupertino_spring_get_preset
/** @endcond */

static ui_error_t
mock_cupertino_spring_evaluate(const struct cupertino_spring_config *config,
                               float t, float *out_pos, float *out_vel) {
  if (g_cupertino_alert_mock_spring_evaluate_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return cupertino_spring_evaluate(config, t, out_pos, out_vel);
}
#undef cupertino_spring_evaluate
/** @cond */
#define cupertino_spring_evaluate mock_cupertino_spring_evaluate
/** @endcond */
#endif

ui_error_t cupertino_alert_create(struct ui_engine *engine,
                                  const struct cupertino_alert_descriptor *desc,
                                  struct cupertino_alert **out_alert) {
  struct cupertino_alert *alert;
  ui_error_t rc;

  if (!engine || !desc || !out_alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  alert = (struct cupertino_alert *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_alert));
  if (!alert) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(alert, 0, sizeof(*alert));

  if (desc->title) {
#if defined(_MSC_VER)
    strncpy_s(alert->title, sizeof(alert->title), desc->title,
              sizeof(alert->title) - 1);
#else
    strncpy(alert->title, desc->title, sizeof(alert->title) - 1);
    alert->title[sizeof(alert->title) - 1] = '\0';
#endif
  }

  if (desc->message) {
#if defined(_MSC_VER)
    strncpy_s(alert->message, sizeof(alert->message), desc->message,
              sizeof(alert->message) - 1);
#else
    strncpy(alert->message, desc->message, sizeof(alert->message) - 1);
    alert->message[sizeof(alert->message) - 1] = '\0';
#endif
  }

  rc = ui_alert_base_create(&alert->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(alert);
    return rc;
  }

  rc = ui_alert_base_set_role(alert->base, desc->is_status_role
                                               ? UI_ALERT_ROLE_STATUS
                                               : UI_ALERT_ROLE_ALERT);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_alert_base_destroy(alert->base);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(alert);
    return rc;
  }

  rc = ui_alert_base_set_dismissible(alert->base, 1);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_alert_base_destroy(alert->base);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(alert);
    return rc;
  }

  *out_alert = alert;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_destroy(struct cupertino_alert *alert) {
  ui_error_t rc;

  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (alert->base) {
    rc = ui_alert_base_destroy(alert->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    alert->base = NULL;
  }

  C_MULTIPLATFORM_FREE(alert);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_set_title(struct cupertino_alert *alert,
                                     const char *title) {
  if (!alert || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(alert->title, sizeof(alert->title), title,
            sizeof(alert->title) - 1);
#else
  strncpy(alert->title, title, sizeof(alert->title) - 1);
  alert->title[sizeof(alert->title) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_get_title(const struct cupertino_alert *alert,
                                     const char **out_title) {
  if (!alert || !out_title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_title = alert->title;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_set_message(struct cupertino_alert *alert,
                                       const char *message) {
  if (!alert || !message) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(alert->message, sizeof(alert->message), message,
            sizeof(alert->message) - 1);
#else
  strncpy(alert->message, message, sizeof(alert->message) - 1);
  alert->message[sizeof(alert->message) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_get_message(const struct cupertino_alert *alert,
                                       const char **out_message) {
  if (!alert || !out_message) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_message = alert->message;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_add_action(struct cupertino_alert *alert,
                                      const char *title,
                                      enum cupertino_alert_action_style style,
                                      size_t *out_index) {
  struct cupertino_alert_action *action;
  size_t idx;

  if (!alert || !title) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (alert->action_count >= CUPERTINO_ALERT_MAX_ACTIONS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = alert->action_count;
  action = &alert->actions[idx];
  memset(action, 0, sizeof(*action));
  action->style = style;
  action->is_disabled = 0;

#if defined(_MSC_VER)
  strncpy_s(action->title, sizeof(action->title), title,
            sizeof(action->title) - 1);
#else
  strncpy(action->title, title, sizeof(action->title) - 1);
  action->title[sizeof(action->title) - 1] = '\0';
#endif

  alert->action_count++;
  if (out_index) {
    *out_index = idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_add_text_field(struct cupertino_alert *alert,
                                          const char *placeholder,
                                          int is_secure, size_t *out_index) {
  struct cupertino_alert_text_field *field;
  size_t idx;

  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (alert->text_field_count >= CUPERTINO_ALERT_MAX_TEXT_FIELDS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = alert->text_field_count;
  field = &alert->text_fields[idx];
  memset(field, 0, sizeof(*field));
  field->is_secure = is_secure ? 1 : 0;

  if (placeholder) {
#if defined(_MSC_VER)
    strncpy_s(field->placeholder, sizeof(field->placeholder), placeholder,
              sizeof(field->placeholder) - 1);
#else
    strncpy(field->placeholder, placeholder, sizeof(field->placeholder) - 1);
    field->placeholder[sizeof(field->placeholder) - 1] = '\0';
#endif
  }

  alert->text_field_count++;
  if (out_index) {
    *out_index = idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_set_text_field_text(struct cupertino_alert *alert,
                                               size_t index, const char *text) {
  if (!alert || index >= alert->text_field_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (text) {
#if defined(_MSC_VER)
    strncpy_s(alert->text_fields[index].text,
              sizeof(alert->text_fields[index].text), text,
              sizeof(alert->text_fields[index].text) - 1);
#else
    strncpy(alert->text_fields[index].text, text,
            sizeof(alert->text_fields[index].text) - 1);
    alert->text_fields[index].text[sizeof(alert->text_fields[index].text) - 1] =
        '\0';
#endif
  } else {
    alert->text_fields[index].text[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_alert_get_text_field_text(const struct cupertino_alert *alert,
                                    size_t index, const char **out_text) {
  if (!alert || index >= alert->text_field_count || !out_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_text = alert->text_fields[index].text;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_set_open(struct cupertino_alert *alert,
                                    int is_open) {
  ui_error_t rc;

  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  alert->is_open = is_open ? 1 : 0;

  if (alert->base) {
    rc = ui_alert_base_set_open(alert->base, alert->is_open);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_is_open(const struct cupertino_alert *alert,
                                   int *out_is_open) {
  if (!alert || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_open = alert->is_open;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_get_action_count(const struct cupertino_alert *alert,
                                            size_t *out_count) {
  if (!alert || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = alert->action_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_get_spring_scale(const struct cupertino_alert *alert,
                                            float progress, float *out_scale) {
  struct cupertino_spring_config cfg;
  float vel = 0.0f;
  ui_error_t rc;

  if (!alert || !out_scale || progress < 0.0f || progress > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_BOUNCY, &cfg);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  cfg.initial_position = 0.85f;
  cfg.target_position = 1.0f;

  rc = cupertino_spring_evaluate(&cfg, progress * 0.5f, out_scale, &vel);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_alert_get_base(struct cupertino_alert *alert,
                                    struct ui_alert_base **out_base) {
  if (!alert || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = alert->base;
  return UI_ERROR_NONE;
}
