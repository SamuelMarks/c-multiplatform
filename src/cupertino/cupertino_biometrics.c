/**
 * @file cupertino_biometrics.c
 * @brief Apple Biometric Prompt Sheet implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_biometrics.h"
#include "ui_internal_mem.h"
#include <math.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_biometrics_mock_dialog_create_fail = 0;
int g_cupertino_biometrics_mock_dialog_destroy_fail = 0;

static ui_error_t mock_dialog_base_create(struct ui_dialog_base **out_dialog) {
  if (g_cupertino_biometrics_mock_dialog_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_dialog_base_create(out_dialog);
}
#undef ui_dialog_base_create
/** @cond */
#define ui_dialog_base_create mock_dialog_base_create
/** @endcond */

static ui_error_t mock_dialog_base_destroy(struct ui_dialog_base *dialog) {
  if (g_cupertino_biometrics_mock_dialog_destroy_fail) {
    (ui_dialog_base_destroy)(dialog);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dialog_base_destroy)(dialog);
}
#undef ui_dialog_base_destroy
/** @cond */
#define ui_dialog_base_destroy mock_dialog_base_destroy
/** @endcond */
#endif

#define CUPERTINO_BIOMETRICS_PI 3.14159265f

ui_error_t
cupertino_biometrics_create(struct ui_engine *engine,
                            const struct cupertino_biometrics_descriptor *desc,
                            struct cupertino_biometrics_prompt **out_prompt) {
  struct cupertino_biometrics_prompt *prompt;
  ui_error_t rc;

  if (!engine || !desc || !out_prompt) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->type < 0 || (int)desc->type > 1) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  prompt = (struct cupertino_biometrics_prompt *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_biometrics_prompt));
  if (!prompt) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(prompt, 0, sizeof(*prompt));
  prompt->type = desc->type;
  prompt->state = CUPERTINO_BIOMETRIC_STATE_READY;
  prompt->is_presented = 0;
  prompt->scan_progress = 0.0f;
  prompt->shake_offset_x = 0.0f;
  prompt->shake_time_ms = 0.0f;
  prompt->width = CUPERTINO_BIOMETRICS_WIDTH;
  prompt->height = CUPERTINO_BIOMETRICS_MIN_HEIGHT;

  if (desc->reason) {
#if defined(_MSC_VER)
    strncpy_s(prompt->reason, sizeof(prompt->reason), desc->reason, _TRUNCATE);
#else
    strncpy(prompt->reason, desc->reason, sizeof(prompt->reason) - 1);
    prompt->reason[sizeof(prompt->reason) - 1] = '\0';
#endif
  }

  if (desc->passcode_button_title) {
#if defined(_MSC_VER)
    strncpy_s(prompt->passcode_button_title,
              sizeof(prompt->passcode_button_title),
              desc->passcode_button_title, _TRUNCATE);
#else
    strncpy(prompt->passcode_button_title, desc->passcode_button_title,
            sizeof(prompt->passcode_button_title) - 1);
    prompt->passcode_button_title[sizeof(prompt->passcode_button_title) - 1] =
        '\0';
#endif
  } else {
#if defined(_MSC_VER)
    strcpy_s(prompt->passcode_button_title,
             sizeof(prompt->passcode_button_title), "Enter Passcode");
#else
    strcpy(prompt->passcode_button_title, "Enter Passcode");
#endif
  }

  if (desc->cancel_button_title) {
#if defined(_MSC_VER)
    strncpy_s(prompt->cancel_button_title, sizeof(prompt->cancel_button_title),
              desc->cancel_button_title, _TRUNCATE);
#else
    strncpy(prompt->cancel_button_title, desc->cancel_button_title,
            sizeof(prompt->cancel_button_title) - 1);
    prompt->cancel_button_title[sizeof(prompt->cancel_button_title) - 1] = '\0';
#endif
  } else {
#if defined(_MSC_VER)
    strcpy_s(prompt->cancel_button_title, sizeof(prompt->cancel_button_title),
             "Cancel");
#else
    strcpy(prompt->cancel_button_title, "Cancel");
#endif
  }

  rc = ui_dialog_base_create(&prompt->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(prompt);
    return rc;
  }

  *out_prompt = prompt;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_biometrics_destroy(struct cupertino_biometrics_prompt *prompt) {
  ui_error_t rc;

  if (!prompt) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (prompt->base) {
    rc = ui_dialog_base_destroy(prompt->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    prompt->base = NULL;
  }

  C_MULTIPLATFORM_FREE(prompt);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_biometrics_present(struct cupertino_biometrics_prompt *prompt) {
  if (!prompt) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  prompt->is_presented = 1;
  prompt->state = CUPERTINO_BIOMETRIC_STATE_SCANNING;
  prompt->scan_progress = 0.0f;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_biometrics_dismiss(struct cupertino_biometrics_prompt *prompt) {
  if (!prompt) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  prompt->is_presented = 0;
  prompt->state = CUPERTINO_BIOMETRIC_STATE_READY;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_biometrics_authenticate(struct cupertino_biometrics_prompt *prompt,
                                  int is_success) {
  if (!prompt) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_success) {
    prompt->state = CUPERTINO_BIOMETRIC_STATE_SUCCESS;
    prompt->shake_time_ms = 0.0f;
    prompt->shake_offset_x = 0.0f;
  } else {
    prompt->state = CUPERTINO_BIOMETRIC_STATE_FAILED;
    prompt->shake_time_ms = 0.0f;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_biometrics_switch_to_passcode(
    struct cupertino_biometrics_prompt *prompt) {
  if (!prompt) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  prompt->state = CUPERTINO_BIOMETRIC_STATE_PASSCODE_FALLBACK;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_biometrics_tick(struct cupertino_biometrics_prompt *prompt,
                                     float delta_ms) {
  float t;
  float decay;

  if (!prompt || delta_ms < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (prompt->state == CUPERTINO_BIOMETRIC_STATE_SCANNING) {
    prompt->scan_progress += delta_ms / 1000.0f;
    if (prompt->scan_progress > 1.0f) {
      prompt->scan_progress = (float)fmod(prompt->scan_progress, 1.0f);
    }
  } else if (prompt->state == CUPERTINO_BIOMETRIC_STATE_FAILED) {
    prompt->shake_time_ms += delta_ms;
    if (prompt->shake_time_ms < 400.0f) {
      t = prompt->shake_time_ms / 400.0f;
      decay = 1.0f - t;
      prompt->shake_offset_x =
          12.0f * decay * (float)sin(t * 8.0f * CUPERTINO_BIOMETRICS_PI);
    } else {
      prompt->shake_offset_x = 0.0f;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_biometrics_get_state(const struct cupertino_biometrics_prompt *prompt,
                               enum cupertino_biometric_state *out_state) {
  if (!prompt || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state = prompt->state;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_biometrics_get_shake_offset(
    const struct cupertino_biometrics_prompt *prompt, float *out_offset_x) {
  if (!prompt || !out_offset_x) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_offset_x = prompt->shake_offset_x;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_biometrics_get_dimensions(
    const struct cupertino_biometrics_prompt *prompt, float *out_width,
    float *out_height) {
  if (!prompt || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = prompt->width;
  *out_height = prompt->height;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_biometrics_get_base(struct cupertino_biometrics_prompt *prompt,
                              struct ui_dialog_base **out_base) {
  if (!prompt || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = prompt->base;
  return UI_ERROR_NONE;
}
