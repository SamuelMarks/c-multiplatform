/**
 * @file test_ui_window_handle_hook.c
 * @brief Test suite verifying window handle hooks, live drag resizing, and
 * responsive layout transitions.
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui_window_backend.h"
#include "ui_event.h"
#include "ui_dom_node.h"
#include "ui_css_parser.h"
#include "ui_layout.h"
#include "ui_error.h"

#if defined(__APPLE__) && defined(__MACH__)
#include "ui_window_backend_macos.h"
#include <CoreGraphics/CoreGraphics.h>
#include <objc/message.h>
#include <objc/runtime.h>
#elif defined(_WIN32) || defined(__CYGWIN__)
#include "ui_window_backend_win32.h"
#elif defined(__linux__) || defined(__unix__)
#include "ui_window_backend_linux.h"
#endif
/* clang-format on */

/**
 * @brief Default stylesheet for wide viewports (row flex layout).
 */
static const char HOOK_ROW_CSS[] = "body {"
                                   "  display: flex;"
                                   "  flex-direction: row;"
                                   "  flex-wrap: wrap;"
                                   "  justify-content: center;"
                                   "  align-items: center;"
                                   "  align-content: center;"
                                   "  width: 100%;"
                                   "  height: 100%;"
                                   "}"
                                   ".box {"
                                   "  width: 100px;"
                                   "  height: 100px;"
                                   "  margin: 15px;"
                                   "}";

/**
 * @brief Stylesheet for narrow or portrait viewports (column flex layout).
 */
static const char HOOK_COL_CSS[] = "body {"
                                   "  display: flex;"
                                   "  flex-direction: column;"
                                   "  justify-content: center;"
                                   "  align-items: center;"
                                   "  align-content: center;"
                                   "  width: 100%;"
                                   "  height: 100%;"
                                   "}"
                                   ".box {"
                                   "  width: 100px;"
                                   "  height: 100px;"
                                   "  margin: 15px;"
                                   "}";

/**
 * @struct hook_test_context
 * @brief Context passed to window resize hook callbacks.
 */
struct hook_test_context {
  struct ui_dom_node *root;             /**< Root DOM body node */
  struct ui_css_stylesheet *stylesheet; /**< Active CSS stylesheet */
  struct ui_layout_node *layout_tree;   /**< Current layout tree */
  float width;                          /**< Current window width */
  float height;                         /**< Current window height */
  int call_count;                       /**< Number of times hook was called */
  int is_column;                        /**< 1 if column layout, 0 if row */
};

/**
 * @brief Recomputes the layout tree for the hook test context.
 * @param ctx Pointer to the hook test context.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t hook_test_recompute_layout(struct hook_test_context *ctx) {
  const char *css;
  ui_error_t rc;

  if (!ctx || !ctx->root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ctx->is_column = (ctx->width < 600.0f || ctx->width < ctx->height) ? 1 : 0;
  css = ctx->is_column ? HOOK_COL_CSS : HOOK_ROW_CSS;

  if (ctx->stylesheet) {
    rc = ui_css_stylesheet_destroy(ctx->stylesheet);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    ctx->stylesheet = NULL;
  }

  rc = ui_css_parse_stylesheet(css, &ctx->stylesheet);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (ctx->layout_tree) {
    rc = ui_layout_tree_destroy(ctx->layout_tree);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    ctx->layout_tree = NULL;
  }

  rc = ui_layout_tree_generate(ctx->root, ctx->stylesheet, &ctx->layout_tree);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_layout_solve_viewport(ctx->layout_tree, ctx->width, ctx->height);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Callback invoked immediately upon live window resize during dragging.
 * @param user_data Pointer to the hook test context.
 * @param width New window width.
 * @param height New window height.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t hook_resize_callback(void *user_data, int width, int height) {
  struct hook_test_context *ctx;
  ui_error_t rc;

  if (!user_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ctx = (struct hook_test_context *)user_data;
  ctx->width = (float)width;
  ctx->height = (float)height;
  ctx->call_count++;

  rc = hook_test_recompute_layout(ctx);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Builds a 3-box DOM tree for testing layout row/column arrangements.
 * @param out_root Pointer to receive the allocated root DOM node.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
static ui_error_t hook_test_build_dom(struct ui_dom_node **out_root) {
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *b1 = NULL;
  struct ui_dom_node *b2 = NULL;
  struct ui_dom_node *b3 = NULL;
  ui_error_t rc;

  if (!out_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  rc = ui_dom_node_set_tag_name(root, "body");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &b1);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return rc;
  }
  rc = ui_dom_node_set_attribute(b1, "class", "box");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(b1);
    ui_dom_node_destroy(root);
    return rc;
  }
  rc = ui_dom_node_append_child(root, b1);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(b1);
    ui_dom_node_destroy(root);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &b2);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return rc;
  }
  rc = ui_dom_node_set_attribute(b2, "class", "box");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(b2);
    ui_dom_node_destroy(root);
    return rc;
  }
  rc = ui_dom_node_append_child(root, b2);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(b2);
    ui_dom_node_destroy(root);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &b3);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return rc;
  }
  rc = ui_dom_node_set_attribute(b3, "class", "box");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(b3);
    ui_dom_node_destroy(root);
    return rc;
  }
  rc = ui_dom_node_append_child(root, b3);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(b3);
    ui_dom_node_destroy(root);
    return rc;
  }

  *out_root = root;
  return UI_ERROR_NONE;
}

/**
 * @brief Asserts whether the 3 child boxes form a row or a column.
 * @param ctx Pointer to the hook test context.
 * @param expected_column 1 if expected to be a column, 0 if row.
 * @return UI_ERROR_NONE on match, UI_ERROR_UNKNOWN on failure.
 */
static ui_error_t hook_test_verify_arrangement(struct hook_test_context *ctx,
                                               int expected_column) {
  struct ui_layout_node *n1;
  struct ui_layout_node *n2;
  struct ui_layout_node *n3;

  if (!ctx || !ctx->layout_tree) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  n1 = ctx->layout_tree->first_child;
  n2 = n1 ? n1->next_sibling : NULL;
  n3 = n2 ? n2->next_sibling : NULL;

  if (!n1 || !n2 || !n3) {
    return UI_ERROR_UNKNOWN;
  }

  if (expected_column) {
    /* Vertical column: Y positions increase, X positions are identical */
    if (!(n1->y < n2->y && n2->y < n3->y) ||
        (n1->x != n2->x || n2->x != n3->x)) {
      printf("Failed column check: x=(%f,%f,%f) y=(%f,%f,%f)\n", n1->x, n2->x,
             n3->x, n1->y, n2->y, n3->y);
      return UI_ERROR_UNKNOWN;
    }
  } else {
    /* Horizontal row: X positions increase, Y positions are identical */
    if (!(n1->x < n2->x && n2->x < n3->x) ||
        (n1->y != n2->y || n2->y != n3->y)) {
      printf("Failed row check: x=(%f,%f,%f) y=(%f,%f,%f)\n", n1->x, n2->x,
             n3->x, n1->y, n2->y, n3->y);
      return UI_ERROR_UNKNOWN;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Runs through multi-step live drag resize workflow.
 * @return 0 on success, non-zero on failure.
 */
static int test_live_drag_workflow(void) {
  struct hook_test_context ctx;
  ui_error_t rc;

  memset(&ctx, 0, sizeof(ctx));
  rc = hook_test_build_dom(&ctx.root);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  /* Step 1: Initial 800x600 landscape window (ROW) */
  rc = hook_resize_callback(&ctx, 800, 600);
  if (rc != UI_ERROR_NONE || ctx.call_count != 1) {
    ui_dom_node_destroy(ctx.root);
    return 2;
  }
  rc = hook_test_verify_arrangement(&ctx, 0);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 3;
  }

  /* Step 2: Live dragging incrementally narrower to 700x600 (still ROW) */
  rc = hook_resize_callback(&ctx, 700, 600);
  if (rc != UI_ERROR_NONE || ctx.call_count != 2) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 4;
  }
  rc = hook_test_verify_arrangement(&ctx, 0);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 5;
  }

  /* Step 3: Dragging across the 600px breakpoint to 550x600 (COLUMN) */
  rc = hook_resize_callback(&ctx, 550, 600);
  if (rc != UI_ERROR_NONE || ctx.call_count != 3) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 6;
  }
  rc = hook_test_verify_arrangement(&ctx, 1);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 7;
  }

  /* Step 4: Dragging narrower to 400x600 (COLUMN) */
  rc = hook_resize_callback(&ctx, 400, 600);
  if (rc != UI_ERROR_NONE || ctx.call_count != 4) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 8;
  }
  rc = hook_test_verify_arrangement(&ctx, 1);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 9;
  }

  /* Step 5: Dragging to narrow portrait 300x500 (COLUMN) */
  rc = hook_resize_callback(&ctx, 300, 500);
  if (rc != UI_ERROR_NONE || ctx.call_count != 5) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 10;
  }
  rc = hook_test_verify_arrangement(&ctx, 1);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 11;
  }

  /* Step 6: Dragging back to wider landscape 650x600 (ROW) */
  rc = hook_resize_callback(&ctx, 650, 600);
  if (rc != UI_ERROR_NONE || ctx.call_count != 6) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 12;
  }
  rc = hook_test_verify_arrangement(&ctx, 0);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 13;
  }

  /* Step 7: Dragging all the way back to 800x600 (ROW) */
  rc = hook_resize_callback(&ctx, 800, 600);
  if (rc != UI_ERROR_NONE || ctx.call_count != 7) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 14;
  }
  rc = hook_test_verify_arrangement(&ctx, 0);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(ctx.layout_tree);
    ui_css_stylesheet_destroy(ctx.stylesheet);
    ui_dom_node_destroy(ctx.root);
    return 15;
  }

  ui_layout_tree_destroy(ctx.layout_tree);
  ui_css_stylesheet_destroy(ctx.stylesheet);
  ui_dom_node_destroy(ctx.root);
  return 0;
}

/**
 * @brief Tests backend get_os_handle and set_on_resize_callback interaction.
 * @return 0 on success, non-zero on failure.
 */
static int test_backend_handle_hook(void) {
  struct ui_window_backend *backend = NULL;
  struct ui_window *window = NULL;
  void *os_handle = NULL;
  ui_error_t rc;

#if defined(__APPLE__) && defined(__MACH__)
  rc = ui_window_backend_macos_create(&backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
  rc = ui_window_backend_win32_create(&backend);
#elif defined(__linux__) || defined(__unix__)
  rc = ui_window_backend_linux_create(&backend);
#else
  rc = UI_ERROR_UNSUPPORTED;
#endif

  if (rc != UI_ERROR_NONE || !backend) {
    return 0; /* Platform backend not available in headless environment */
  }

  /* Test parameter validation on get_os_handle */
  if (backend->get_os_handle) {
    rc = backend->get_os_handle(NULL, window, &os_handle);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      backend->destroy_window(backend, window);
      return 1;
    }
    rc = backend->get_os_handle(backend, NULL, &os_handle);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      return 2;
    }
    rc = backend->get_os_handle(backend, window, NULL);
    if (rc != UI_ERROR_INVALID_ARGUMENT) {
      return 3;
    }
  }

  /* Create actual window and retrieve native OS handle */
  rc = backend->create_window(backend, "Hook Test Window", 800, 600, &window);
  if (rc != UI_ERROR_NONE || !window) {
#if defined(__APPLE__) && defined(__MACH__)
    ui_window_backend_macos_destroy(backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
    ui_window_backend_win32_destroy(backend);
#elif defined(__linux__) || defined(__unix__)
    ui_window_backend_linux_destroy(backend);
#endif
    return 0;
  }

  if (backend->get_os_handle) {
    rc = backend->get_os_handle(backend, window, &os_handle);
    if (rc != UI_ERROR_NONE || !os_handle) {
      backend->destroy_window(backend, window);
#if defined(__APPLE__) && defined(__MACH__)
      ui_window_backend_macos_destroy(backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
      ui_window_backend_win32_destroy(backend);
#elif defined(__linux__) || defined(__unix__)
      ui_window_backend_linux_destroy(backend);
#endif
      return 4;
    }
  }

  /* Install resize hook callback */
  if (backend->set_on_resize_callback) {
    rc = backend->set_on_resize_callback(backend, window, hook_resize_callback,
                                         NULL);
    if (rc != UI_ERROR_NONE) {
      backend->destroy_window(backend, window);
#if defined(__APPLE__) && defined(__MACH__)
      ui_window_backend_macos_destroy(backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
      ui_window_backend_win32_destroy(backend);
#elif defined(__linux__) || defined(__unix__)
      ui_window_backend_linux_destroy(backend);
#endif
      return 5;
    }
  }

  /* Pump messages briefly to ensure Wine/Windows doesn't hang on destruction */
  if (backend->poll_events) {
    struct ui_event ev;
    int has_event = 0;
    backend->poll_events(backend, window, &ev, &has_event);
  }

  rc = backend->destroy_window(backend, window);
  if (rc != UI_ERROR_NONE) {
    return 6;
  }

#if defined(__APPLE__) && defined(__MACH__)
  rc = ui_window_backend_macos_destroy(backend);
#elif defined(_WIN32) || defined(__CYGWIN__)
  rc = ui_window_backend_win32_destroy(backend);
#elif defined(__linux__) || defined(__unix__)
  rc = ui_window_backend_linux_destroy(backend);
#endif
  if (rc != UI_ERROR_NONE) {
    return 7;
  }

  return 0;
}

/**
 * @brief Application entry point for window handle hook test suite.
 * @return 0 on success, non-zero on failure.
 */
int main(void) {
  int rc;

  printf("Running test_live_drag_workflow...\n");
  rc = test_live_drag_workflow();
  if (rc != 0) {
    printf("test_live_drag_workflow failed with code %d\n", rc);
    return rc;
  }

  printf("Running test_backend_handle_hook...\n");
  rc = test_backend_handle_hook();
  if (rc != 0) {
    printf("test_backend_handle_hook failed with code %d\n", rc);
    return rc;
  }

  printf("All window handle hook tests passed!\n");
  return 0;
}
