/* clang-format off */
#include "app.h"
#include <cmp.h>
#include <stdio.h>
/* clang-format on */

/**
 * @brief Global pointer to the application window.
 */
static cmp_window_t *g_window = NULL;

/**
 * @brief Global pointer to the application UI tree.
 */
static cmp_ui_node_t *g_ui_tree = NULL;

/**
 * @brief Global state indicating if content is shown.
 */
static int g_show_content = 0;

/**
 * @brief Retrieves the platform string for display.
 * @param out_str Pointer to receive the platform string.
 * @return 0 on success, error code otherwise.
 */
static int get_platform_string(const char **out_str) {
  if (!out_str) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(_WIN32)
#if defined(_M_X64) || defined(__x86_64__)
#define CMP_ARCH_STR "x64"
#elif defined(_M_ARM64) || defined(__aarch64__)
#define CMP_ARCH_STR "ARM64"
#elif defined(_M_IX86) || defined(__i386__)
#define CMP_ARCH_STR "x86"
#else
#define CMP_ARCH_STR "Unknown Arch"
#endif

#if defined(NTDDI_VERSION) &&                                                  \
    NTDDI_VERSION >= 0x0A00000B /* NTDDI_WIN10_CO, Windows 11 */
  *out_str = "Windows 11 " CMP_ARCH_STR;
#elif defined(NTDDI_VERSION) &&                                                \
    NTDDI_VERSION >= 0x0A000000 /* NTDDI_WIN10, Windows 10 */
  *out_str = "Windows 10 " CMP_ARCH_STR;
#elif defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0603     /* Windows 8.1 */
  *out_str = "Windows 8.1 " CMP_ARCH_STR;
#elif defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0602     /* Windows 8 */
  *out_str = "Windows 8 " CMP_ARCH_STR;
#elif defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0601     /* Windows 7 */
  *out_str = "Windows 7 " CMP_ARCH_STR;
#elif defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600     /* Windows Vista */
  *out_str = "Windows Vista " CMP_ARCH_STR;
#elif defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0501     /* Windows XP */
  *out_str = "Windows XP " CMP_ARCH_STR;
#elif defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0500     /* Windows 2000 */
  *out_str = "Windows 2000 " CMP_ARCH_STR;
#elif defined(_WIN32_WINDOWS) && _WIN32_WINDOWS == 0x0490 /* Windows ME */
  *out_str = "Windows ME " CMP_ARCH_STR;
#elif defined(_WIN32_WINDOWS) && _WIN32_WINDOWS >= 0x0410 /* Windows 98 */
  *out_str = "Windows 98 " CMP_ARCH_STR;
#elif defined(_WIN32_WINDOWS) && _WIN32_WINDOWS >= 0x0400 /* Windows 95 */
  *out_str = "Windows 95 " CMP_ARCH_STR;
#else
  *out_str = "Windows " CMP_ARCH_STR;
#endif
#elif defined(__APPLE__)
  *out_str = "Apple";
#elif defined(__linux__) && !defined(__ANDROID__)
  *out_str = "Linux";
#elif defined(__ANDROID__)
#define STR(x) #x
#define XSTR(x) STR(x)
  *out_str = "Android " XSTR(__ANDROID_API__);
#elif defined(__EMSCRIPTEN__)
  *out_str = "WebAssembly";
#else
  *out_str = "Unknown";
#endif
  return CMP_SUCCESS;
}

static int manual_hit_test(cmp_ui_node_t *node, float x, float y) {
  size_t i;
  if (!node || !node->layout) {
    return -1;
  }
  if (x >= node->layout->computed_rect.x &&
      x <= node->layout->computed_rect.x + node->layout->computed_rect.width &&
      y >= node->layout->computed_rect.y &&
      y <= node->layout->computed_rect.y + node->layout->computed_rect.height) {
    int hit = -1;
    for (i = 0; i < node->child_count; ++i) {
      int child_hit = manual_hit_test(node->children[i], x, y);
      if (child_hit != -1) {
        hit = child_hit;
      }
    }
    if (hit != -1) {
      return hit;
    }
    if (node->layout->id != 0) {
      return node->layout->id;
    }
  }
  return -1;
}

/**
 * @brief Builds the UI tree for the application.
 * @return 0 on success, error code otherwise.
 */
static int build_ui(void) {
  cmp_ui_node_t *btn;
  int res;

  if (g_ui_tree != NULL) {
    cmp_ui_node_destroy(g_ui_tree);
    g_ui_tree = NULL;
  }

  res = cmp_ui_box_create(&g_ui_tree);
  if (res != CMP_SUCCESS) {
    return res;
  }

  g_ui_tree->layout->direction = CMP_FLEX_COLUMN;
  g_ui_tree->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
  g_ui_tree->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  g_ui_tree->layout->width = 1024.0f;
  g_ui_tree->layout->height = 1024.0f;

  g_ui_tree->layout->padding[0] = 0.0f;

  {
    cmp_ui_node_t *spacer_top;
    if (cmp_ui_box_create(&spacer_top) == CMP_SUCCESS) {
      spacer_top->layout->flex_grow = 1.0f;
      cmp_ui_node_add_child(g_ui_tree, spacer_top);
    }
  }

  res = cmp_ui_button_create(&btn, "Click me!", -1);
  if (res != CMP_SUCCESS) {
    return res;
  }

  btn->layout->id = 100;
  btn->layout->width = 320.0f;
  btn->layout->height = 80.0f;
  cmp_ui_node_add_child(g_ui_tree, btn);

  if (g_show_content) {
    cmp_ui_node_t *img;
    cmp_ui_node_t *txt;
    char text_buf[256];
    const char *platform_str = NULL;

    res = cmp_ui_image_view_create(&img, "virt:/assets/compose.svg");
    if (res == CMP_SUCCESS) {
      img->layout->margin[0] = 60.0f; /* top margin */
      img->layout->width = 240.0f;
      img->layout->height = 240.0f;
      cmp_ui_node_add_child(g_ui_tree, img);
    }

    res = get_platform_string(&platform_str);
    if (res != CMP_SUCCESS) {
      platform_str = "Unknown";
    }

#if defined(_MSC_VER)
    sprintf_s(text_buf, sizeof(text_buf), "Compose: %s!", platform_str);
#else
    sprintf(text_buf, "Compose: %s!", platform_str);
#endif

    res = cmp_ui_text_create(&txt, text_buf, -1);
    if (res == CMP_SUCCESS) {
      txt->layout->margin[0] = 60.0f; /* top margin */
      txt->layout->width = 600.0f;
      txt->layout->height = 80.0f;
      cmp_ui_node_add_child(g_ui_tree, txt);
    }
  }

  return CMP_SUCCESS;
}

int app_init(void) {
  cmp_window_config_t config;

  cmp_event_system_init();
  cmp_vfs_init();
  cmp_window_system_init();

  config.title = "Compose Multiplatform Starter";
  config.width = 1024;
  config.height = 1024;
  config.x = -1;
  config.y = -1;
  config.hidden = 0;
  config.frameless = 0;
  config.use_legacy_backend = 0;

  if (cmp_window_create(&config, &g_window) != CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (build_ui() != CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (g_ui_tree) {
    cmp_layout_calculate(g_ui_tree->layout, 1024.0f, 1024.0f);
  }
  cmp_window_set_ui_tree(g_window, g_ui_tree);

  cmp_window_show(g_window);
  return CMP_SUCCESS;
}

int app_run(void) {
  cmp_event_t evt;
  int running = 1;

  while (running) {
    cmp_window_poll_events(g_window);

    while (cmp_event_pop(&evt) == CMP_SUCCESS) {
      if (evt.action == CMP_ACTION_DOWN && (evt.type == 1 || evt.type == 2)) {
        int hit_node = manual_hit_test(g_ui_tree, (float)evt.x, (float)evt.y);
        if (hit_node == 100) {
          g_show_content = !g_show_content;
          build_ui();
          if (g_ui_tree) {
            cmp_layout_calculate(g_ui_tree->layout, 1024.0f, 1024.0f);
          }
          cmp_window_set_ui_tree(g_window, g_ui_tree);
        }
      }
    }

    if (cmp_window_should_close(g_window)) {
      running = 0;
    }

    /* Wait briefly to reduce CPU usage since we are event driven */
    /* Sleep equivalent or yield */
  }

  return CMP_SUCCESS;
}

int app_shutdown(void) {
  if (g_ui_tree) {
    cmp_ui_node_destroy(g_ui_tree);
    g_ui_tree = NULL;
  }

  if (g_window) {
    cmp_window_destroy(g_window);
    g_window = NULL;
  }

  cmp_window_system_shutdown();
  cmp_vfs_shutdown();
  cmp_event_system_shutdown();

  return CMP_SUCCESS;
}
