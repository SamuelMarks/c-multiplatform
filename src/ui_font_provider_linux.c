/**
 * @file ui_font_provider_linux.c
 * @brief System font discovery for Linux using standard system font
 * directories.
 */

/* clang-format off */
#include "../include/ui_font_provider.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if !defined(__APPLE__) && !defined(_WIN32) && !defined(_WIN64)

ui_error_t ui_font_provider_load_system_font(struct ui_font_manager *manager,
                                             const char *family_name,
                                             int weight, int is_italic,
                                             struct ui_font **out_font) {
  static const char *const search_paths[] = {
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
      "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
      "/usr/share/fonts/TTF/DejaVuSans.ttf",
      "/usr/share/fonts/dejavu/DejaVuSans.ttf",
      NULL};
  int i;
  ui_error_t rc = UI_ERROR_NOT_FOUND;

  if (!manager || !family_name || !out_font) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_font = NULL;

  for (i = 0; search_paths[i]; ++i) {
    rc = ui_font_manager_load_font_file(manager, search_paths[i], out_font);
    if (rc == UI_ERROR_NONE && *out_font) {
      rc = ui_font_set_metadata(*out_font, family_name, weight, is_italic);
      return rc;
    }
  }

  return UI_ERROR_NOT_FOUND;
}

#else

/* Dummy for Apple and Windows */
ui_error_t ui_font_provider_linux_dummy(void) { return UI_ERROR_NONE; }

#endif
