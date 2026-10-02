/**
 * @file ui_font_provider_win32.c
 * @brief Native system font discovery for Windows fonts directory.
 */

/* clang-format off */
#include "../include/ui_font_provider.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if defined(_WIN32) || defined(_WIN64)

ui_error_t ui_font_provider_load_system_font(struct ui_font_manager *manager,
                                             const char *family_name,
                                             int weight, int is_italic,
                                             struct ui_font **out_font) {
  char full_path[512];
  const char *windir = NULL;
  const char *candidate_file;
  ui_error_t rc;
#if defined(_MSC_VER)
  char *env_val = NULL;
  size_t env_len = 0;
#endif

  if (!manager || !family_name || !out_font) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_font = NULL;

#if defined(_MSC_VER)
  if (_dupenv_s(&env_val, &env_len, "WINDIR") == 0 && env_val) {
    windir = env_val;
  } else {
    windir = "C:\\Windows";
  }
#else
  windir = getenv("WINDIR");
  if (!windir) {
    windir = "C:\\Windows";
  }
#endif

  if (strcmp(family_name, "Segoe UI") == 0 ||
      strcmp(family_name, "sans-serif") == 0 ||
      strcmp(family_name, "system-ui") == 0) {
    candidate_file = (weight >= 700) ? "segoeuib.ttf" : "segoeui.ttf";
  } else if (strcmp(family_name, "Arial") == 0) {
    candidate_file = (weight >= 700) ? "arialbd.ttf" : "arial.ttf";
  } else if (strcmp(family_name, "Times New Roman") == 0 ||
             strcmp(family_name, "serif") == 0) {
    candidate_file = (weight >= 700) ? "timesbd.ttf" : "times.ttf";
  } else if (strcmp(family_name, "Courier New") == 0 ||
             strcmp(family_name, "monospace") == 0) {
    candidate_file = (weight >= 700) ? "courbd.ttf" : "cour.ttf";
  } else {
    candidate_file = "arial.ttf";
  }

#if defined(_MSC_VER)
  sprintf_s(full_path, sizeof(full_path), "%s\\Fonts\\%s", windir,
            candidate_file);
  if (env_val) {
    free(env_val);
  }
#else
  sprintf(full_path, "%s\\Fonts\\%s", windir, candidate_file);
#endif

  rc = ui_font_manager_load_font_file(manager, full_path, out_font);
  if (rc == UI_ERROR_NONE && *out_font) {
    rc = ui_font_set_metadata(*out_font, family_name, weight, is_italic);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return rc;
}

#else

/* Non-Windows fallback dummy */
ui_error_t ui_font_provider_win32_dummy(void) { return UI_ERROR_NONE; }

#endif
