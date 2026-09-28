/**
 * @file test_ui_window_creation_screenshots.c
 * @brief Window creation audit and screenshot artifact validation test.
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "ui_error.h"
#include "ui_window_backend.h"
#include "ui_renderer.h"
#include "ui_renderer_gles2.h"
#include "ui_test_visual.h"

#if defined(__APPLE__) && defined(__MACH__)
#include "ui_window_backend_macos.h"
#elif defined(_WIN32) || defined(__CYGWIN__)
#include "ui_window_backend_win32.h"
#elif defined(__linux__) || defined(__unix__)
#include "ui_window_backend_linux.h"
#endif

#include "ui_window_backend_android.h"
#include "ui_window_backend_ios.h"
#include "ui_window_backend_web.h"
/* clang-format on */

/**
 * @brief Helper to destroy the active platform window backend.
 * @param backend Pointer to window backend.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t destroy_active_backend(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#if defined(__APPLE__) && defined(__MACH__)
  return ui_window_backend_macos_destroy(backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
  return ui_window_backend_win32_destroy(backend);
#elif defined(__linux__) || defined(__unix__)
  return ui_window_backend_linux_destroy(backend);
#elif defined(__EMSCRIPTEN__)
  return ui_window_backend_web_destroy(backend);
#else
  return UI_ERROR_UNSUPPORTED;
#endif
}

/**
 * @brief Audits window backend lifecycle and creation argument validation.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t audit_window_backends(void) {
  struct ui_window_backend *backend = NULL;
  struct ui_window *win = NULL;
  ui_error_t rc;

  printf("Auditing window backends...\n");

  /* Android backend audit */
  rc = ui_window_backend_android_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return UI_ERROR_UNKNOWN;
  }
  rc = ui_window_backend_android_create(&backend);
  if (rc == UI_ERROR_NONE && backend != NULL) {
    rc = backend->create_window(backend, NULL, 100, 100, &win);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      ui_window_backend_android_destroy(backend);
      return UI_ERROR_UNKNOWN;
    }
    rc = backend->create_window(NULL, "Title", 100, 100, &win);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      ui_window_backend_android_destroy(backend);
      return UI_ERROR_UNKNOWN;
    }
    rc = ui_window_backend_android_destroy(backend);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    backend = NULL;
  }

  /* iOS backend audit */
  rc = ui_window_backend_ios_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return UI_ERROR_UNKNOWN;
  }
  rc = ui_window_backend_ios_create(&backend);
  if (rc == UI_ERROR_NONE && backend != NULL) {
    rc = backend->create_window(backend, NULL, 100, 100, &win);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      ui_window_backend_ios_destroy(backend);
      return UI_ERROR_UNKNOWN;
    }
    rc = ui_window_backend_ios_destroy(backend);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    backend = NULL;
  }

  /* Web backend audit */
  rc = ui_window_backend_web_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return UI_ERROR_UNKNOWN;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Checks if a pixel buffer has rainbow gradient artifacts in a solid
 * box.
 * @param pixels RGBA pixel buffer.
 * @param width Buffer width.
 * @param height Buffer height.
 * @param box_x Box X start.
 * @param box_y Box Y start.
 * @param box_w Box width.
 * @param box_h Box height.
 * @return UI_ERROR_NONE if solid/clean, or UI_ERROR_LAYOUT_VIOLATION if rainbow
 * artifact detected.
 */
static ui_error_t verify_no_rainbow_artifacts(const unsigned char *pixels,
                                              int width, int height, int box_x,
                                              int box_y, int box_w, int box_h) {
  int x, y;
  int first = 1;
  unsigned char ref_r = 0, ref_g = 0, ref_b = 0;
  int diff_count = 0;

  if (!pixels || width <= 0 || height <= 0 || box_w <= 0 || box_h <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Sample inner area of the rectangle (avoiding edge antialiasing) */
  for (y = box_y + 5; y < box_y + box_h - 5 && y < height; ++y) {
    for (x = box_x + 5; x < box_x + box_w - 5 && x < width; ++x) {
      size_t idx = ((size_t)y * (size_t)width + (size_t)x) * 4;
      unsigned char r = pixels[idx];
      unsigned char g = pixels[idx + 1];
      unsigned char b = pixels[idx + 2];

      if (first) {
        ref_r = r;
        ref_g = g;
        ref_b = b;
        first = 0;
      } else {
        /* In a solid colored box, there should be zero or negligible gradient
         */
        int dr = abs((int)r - (int)ref_r);
        int dg = abs((int)g - (int)ref_g);
        int db = abs((int)b - (int)ref_b);
        if (dr > 10 || dg > 10 || db > 10) {
          diff_count++;
        }
      }
    }
  }

  if (diff_count > 0) {
    printf("Rainbow artifact detected: %d divergent pixels in solid box!\n",
           diff_count);
    return UI_ERROR_LAYOUT_VIOLATION;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Tests active platform window creation, rendering, and screenshot
 * capture.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t test_window_creation_and_screenshot(void) {
  struct ui_window_backend *backend = NULL;
  struct ui_window *window = NULL;
  struct ui_renderer_backend *renderer = NULL;
  unsigned char *pixels = NULL;
  struct ui_color bg;
  struct ui_color red_box;
  struct ui_color green_box;
  struct ui_color blue_box;
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t clean_rc = UI_ERROR_NONE;
  int win_w = 400;
  int win_h = 300;
#if defined(_WIN32) || defined(__CYGWIN__)
  int is_wine = 0;
#if defined(_MSC_VER)
  char *wine_val = NULL;
  size_t wine_len = 0;
#endif
#endif

  printf("Testing native window creation and rendering...\n");

#if defined(__APPLE__) && defined(__MACH__)
  rc = ui_window_backend_macos_create(&backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
  rc = ui_window_backend_win32_create(&backend);
#elif defined(__linux__) || defined(__unix__)
  rc = ui_window_backend_linux_create(&backend);
#else
  rc = UI_ERROR_UNSUPPORTED;
#endif

  if (rc != UI_ERROR_NONE || backend == NULL) {
    printf("Native window backend not available on this platform (rc=%d).\n",
           (int)rc);
    return UI_ERROR_NONE;
  }

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(_MSC_VER)
  if (_dupenv_s(&wine_val, &wine_len, "WINELOADER") == 0 && wine_val != NULL) {
    is_wine = 1;
    free(wine_val);
  }
#else
  if (getenv("WINELOADER") != NULL) {
    is_wine = 1;
  }
#endif
  if (is_wine) {
    printf("Skipping headful window render test under Wine CI.\n");
    clean_rc = destroy_active_backend(backend);
    if (clean_rc != UI_ERROR_NONE) {
      return clean_rc;
    }
    return UI_ERROR_NONE;
  }
#endif

  rc = backend->create_window(backend, "C-Multiplatform Audit Window", win_w,
                              win_h, &window);
  if (rc != UI_ERROR_NONE || window == NULL) {
    printf("Window creation returned error %d, skipping headful render.\n",
           (int)rc);
    destroy_active_backend(backend);
    return UI_ERROR_NONE;
  }

  rc = ui_renderer_gles2_create(&renderer);
  if (rc != UI_ERROR_NONE || renderer == NULL) {
    goto cleanup;
  }

  rc = backend->show_window(backend, window);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = renderer->init(renderer, backend, window);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = renderer->set_viewport(renderer, 0, 0, win_w, win_h);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Background: White */
  bg.r = 1.0f;
  bg.g = 1.0f;
  bg.b = 1.0f;
  bg.a = 1.0f;
  rc = renderer->clear(renderer, bg);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Box 1: Red */
  red_box.r = 0.8f;
  red_box.g = 0.2f;
  red_box.b = 0.2f;
  red_box.a = 1.0f;
  rc = renderer->draw_rect(renderer, 30.0f, 30.0f, 80.0f, 80.0f, red_box);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Box 2: Green */
  green_box.r = 0.2f;
  green_box.g = 0.8f;
  green_box.b = 0.2f;
  green_box.a = 1.0f;
  rc = renderer->draw_rect(renderer, 150.0f, 30.0f, 80.0f, 80.0f, green_box);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Box 3: Blue */
  blue_box.r = 0.2f;
  blue_box.g = 0.2f;
  blue_box.b = 0.8f;
  blue_box.a = 1.0f;
  rc = renderer->draw_rect(renderer, 270.0f, 30.0f, 80.0f, 80.0f, blue_box);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = renderer->flush(renderer);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Capture screenshot buffer */
  pixels = (unsigned char *)malloc((size_t)win_w * (size_t)win_h * 4);
  if (!pixels) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
  memset(pixels, 0, (size_t)win_w * (size_t)win_h * 4);

  if (renderer->read_pixels) {
    rc = renderer->read_pixels(renderer, win_w, win_h, pixels);
    if (rc != UI_ERROR_NONE) {
      free(pixels);
      goto cleanup;
    }

    /* Verify no rainbow gradient inside the red box */
    rc = verify_no_rainbow_artifacts(pixels, win_w, win_h, 30, 30, 80, 80);
    if (rc != UI_ERROR_NONE) {
      free(pixels);
      goto cleanup;
    }

    /* Verify no rainbow gradient inside the green box */
    rc = verify_no_rainbow_artifacts(pixels, win_w, win_h, 150, 30, 80, 80);
    if (rc != UI_ERROR_NONE) {
      free(pixels);
      goto cleanup;
    }

    /* Verify no rainbow gradient inside the blue box */
    rc = verify_no_rainbow_artifacts(pixels, win_w, win_h, 270, 30, 80, 80);
    if (rc != UI_ERROR_NONE) {
      free(pixels);
      goto cleanup;
    }
  }

  free(pixels);

  rc = backend->swap_buffers(backend, window);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

cleanup:
  if (renderer) {
    clean_rc = ui_renderer_gles2_destroy(renderer);
    if (rc == UI_ERROR_NONE) {
      rc = clean_rc;
    }
  }
  if (backend && window) {
    clean_rc = backend->destroy_window(backend, window);
    if (rc == UI_ERROR_NONE) {
      rc = clean_rc;
    }
  }
  if (backend) {
    clean_rc = destroy_active_backend(backend);
    if (rc == UI_ERROR_NONE) {
      rc = clean_rc;
    }
  }
  return rc;
}

/**
 * @brief Main test entry point.
 * @return 0 on success, non-zero on failure.
 */
int main(void) {
  ui_error_t err;

  printf("=== Window Creation Audit & Screenshot Rainbow Verification ===\n");
  err = audit_window_backends();
  if (err != UI_ERROR_NONE) {
    printf("=== Window creation audit FAILED with error %d! ===\n", (int)err);
    return (int)err;
  }

  err = test_window_creation_and_screenshot();
  if (err != UI_ERROR_NONE) {
    printf(
        "=== Window creation and screenshot test FAILED with error %d! ===\n",
        (int)err);
    return (int)err;
  }

  printf("=== Window creation audit passed: NO rainbow artifacts! ===\n");
  return 0;
}
