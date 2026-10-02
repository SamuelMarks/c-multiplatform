/**
 * @file ui_font_provider_macos.c
 * @brief Native system font discovery for macOS using CoreText.
 */

/* clang-format off */
#include "../include/ui_font_provider.h"
#include "ui_internal_mem.h"

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#endif
#include <string.h>
/* clang-format on */

#if defined(__APPLE__)

#ifdef UI_TEST_MOCK_ALLOC
int g_mock_macos_font_cfname_fail = 0;
int g_mock_macos_font_desc_fail = 0;
int g_mock_macos_font_font_fail = 0;
int g_mock_macos_font_url_fail = 0;
int g_mock_macos_font_url_not_ok = 0;
int g_mock_macos_font_load_fail = 0;
int g_mock_macos_font_set_metadata_fail = 0;
#endif

ui_error_t ui_font_provider_load_system_font(struct ui_font_manager *manager,
                                             const char *family_name,
                                             int weight, int is_italic,
                                             struct ui_font **out_font) {
  CFStringRef cfName = NULL;
  CTFontDescriptorRef desc = NULL;
  CTFontRef font = NULL;
  CFURLRef url = NULL;
  char path[1024];
  Boolean ok;
  ui_error_t rc;
  const char *actual_family;

  if (!manager || !family_name || !out_font) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_font = NULL;

  /* Resolve CSS generic names to macOS system fonts */
  if (strcmp(family_name, "sans-serif") == 0 ||
      strcmp(family_name, "system-ui") == 0 ||
      strcmp(family_name, "-apple-system") == 0) {
    actual_family = "Helvetica";
  } else if (strcmp(family_name, "serif") == 0) {
    actual_family = "Times";
  } else if (strcmp(family_name, "monospace") == 0) {
    actual_family = "Courier";
  } else {
    actual_family = family_name;
  }

  cfName = CFStringCreateWithCString(kCFAllocatorDefault, actual_family,
                                     kCFStringEncodingUTF8);
#ifdef UI_TEST_MOCK_ALLOC
  if (g_mock_macos_font_cfname_fail) {
    CFRelease(cfName);
    cfName = NULL;
  }
#endif
  if (!cfName) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  desc = CTFontDescriptorCreateWithNameAndSize(cfName, 0.0);
  CFRelease(cfName);
#ifdef UI_TEST_MOCK_ALLOC
  if (g_mock_macos_font_desc_fail) {
    CFRelease(desc);
    desc = NULL;
  }
#endif
  if (!desc) {
    return UI_ERROR_NOT_FOUND;
  }

  font = CTFontCreateWithFontDescriptor(desc, 16.0, NULL);
  CFRelease(desc);
#ifdef UI_TEST_MOCK_ALLOC
  if (g_mock_macos_font_font_fail) {
    CFRelease(font);
    font = NULL;
  }
#endif
  if (!font) {
    return UI_ERROR_NOT_FOUND;
  }

  url = (CFURLRef)CTFontCopyAttribute(font, kCTFontURLAttribute);
#ifdef UI_TEST_MOCK_ALLOC
  if (g_mock_macos_font_url_fail) {
    CFRelease(url);
    url = NULL;
  }
#endif
  if (url) {
    ok = CFURLGetFileSystemRepresentation(url, true, (UInt8 *)path,
                                          sizeof(path));
#ifdef UI_TEST_MOCK_ALLOC
    if (g_mock_macos_font_url_not_ok) {
      ok = false;
    }
#endif
    CFRelease(url);
    CFRelease(font);
    if (ok) {
      rc = ui_font_manager_load_font_file(manager, path, out_font);
#ifdef UI_TEST_MOCK_ALLOC
      if (g_mock_macos_font_load_fail) {
        rc = UI_ERROR_UNKNOWN;
      }
#endif
      if (rc == UI_ERROR_NONE) {
        rc = ui_font_set_metadata(*out_font, actual_family, weight, is_italic);
#ifdef UI_TEST_MOCK_ALLOC
        if (g_mock_macos_font_set_metadata_fail) {
          rc = UI_ERROR_UNKNOWN;
        }
#endif
        return rc;
      }
    }
    return UI_ERROR_NOT_FOUND;
  }

  CFRelease(font);
  return UI_ERROR_NOT_FOUND;
}

#else

/* Non-Apple dummy */
ui_error_t ui_font_provider_macos_dummy(void) { return UI_ERROR_NONE; }

#endif
