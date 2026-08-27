/* clang-format off */
#include "../include/ui_e2e_headful.h"
#include <stdlib.h>
#include "ui_internal_mem.h"

#if defined(_WIN32) || defined(WIN32)
#ifndef _WIN32_WINNT
/** @brief internal */
#define _WIN32_WINNT 0x0500
#endif
/** @brief internal */
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <winuser.h>
/* clang-format on */
#endif

/**
 * @struct ui_e2e_headful_ctx
 * @struct ui_e2e_headful_ctx
 * @brief Internal representation of an end-to-end headful test context.
 */
struct ui_e2e_headful_ctx {
  /* @brief OS window backend to interact with. */
  struct ui_window_backend *backend; /**< backend */
  /* @brief Window handle to interact with. */
  struct ui_window *window; /**< window */
};

/*
 * @brief ui_e2e_headful_create.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_ctx Parameter out_ctx.
 * @return Return value.
 */
ui_error_t ui_e2e_headful_create(struct ui_window_backend *backend,
                                 struct ui_window *window,
                                 struct ui_e2e_headful_ctx **out_ctx) {
  struct ui_e2e_headful_ctx *ctx;
  if (!backend || !window || !out_ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ctx = (struct ui_e2e_headful_ctx *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_e2e_headful_ctx));
  if (!ctx) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  ctx->backend = backend;
  ctx->window = window;
  *out_ctx = ctx;

  return UI_ERROR_NONE;
}

/*
 * @brief ui_e2e_headful_destroy.
 * @param ctx Parameter ctx.
 * @return Return value.
 */
ui_error_t ui_e2e_headful_destroy(struct ui_e2e_headful_ctx *ctx) {
  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(ctx);
  return UI_ERROR_NONE;
}

#if defined(_WIN32) || defined(WIN32)

/*
 * @brief map_ui_key_to_vk.
 * @param key_code Parameter key_code.
 * @return Return value.
 */
static WORD map_ui_key_to_vk(int key_code) {
  switch (key_code) {
  case UI_KEY_SPACE:
    return VK_SPACE;
  case UI_KEY_ENTER:
    return VK_RETURN;
  case UI_KEY_ESCAPE:
    return VK_ESCAPE;
  case UI_KEY_BACKSPACE:
    return VK_BACK;
  case UI_KEY_TAB:
    return VK_TAB;
  case UI_KEY_UP:
    return VK_UP;
  case UI_KEY_DOWN:
    return VK_DOWN;
  case UI_KEY_LEFT:
    return VK_LEFT;
  case UI_KEY_RIGHT:
    return VK_RIGHT;
  case UI_KEY_HOME:
    return VK_HOME;
  case UI_KEY_END:
    return VK_END;
  default:
    return 0;
  }
}

/*
 * @brief ui_e2e_headful_click.
 * @param ctx Parameter ctx.
 * @param x Parameter x.
 * @param y Parameter y.
 * @return Return value.
 */
ui_error_t ui_e2e_headful_click(struct ui_e2e_headful_ctx *ctx, int x, int y) {
  HWND hwnd;
  POINT pt;
  INPUT inputs[2];
  int screen_w, screen_h;

  ui_error_t handle_rc;
  void *os_handle = NULL;

  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!ctx->backend || !ctx->window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!ctx->backend->get_os_handle) {
    return UI_ERROR_UNSUPPORTED;
  }

  handle_rc =
      ctx->backend->get_os_handle(ctx->backend, ctx->window, &os_handle);
  if (handle_rc != UI_ERROR_NONE) {
    return handle_rc;
  }
  hwnd = (HWND)os_handle;
  if (!hwnd) {
    return UI_ERROR_UNSUPPORTED;
  }

  pt.x = x;
  pt.y = y;
  ClientToScreen(hwnd, &pt);

  screen_w = GetSystemMetrics(SM_CXSCREEN);
  screen_h = GetSystemMetrics(SM_CYSCREEN);

  /* Move and Mouse Down */
  inputs[0].type = INPUT_MOUSE;
  inputs[0].mi.dx = (pt.x * 65536) / screen_w;
  inputs[0].mi.dy = (pt.y * 65536) / screen_h;
  inputs[0].mi.mouseData = 0;
  inputs[0].mi.dwFlags =
      MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN;
  inputs[0].mi.time = 0;
  inputs[0].mi.dwExtraInfo = 0;

  /* Mouse Up */
  inputs[1].type = INPUT_MOUSE;
  inputs[1].mi.dx = inputs[0].mi.dx;
  inputs[1].mi.dy = inputs[0].mi.dy;
  inputs[1].mi.mouseData = 0;
  inputs[1].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
  inputs[1].mi.time = 0;
  inputs[1].mi.dwExtraInfo = 0;

  SendInput(2, inputs, sizeof(INPUT));

  return UI_ERROR_NONE;
}

/*
 * @brief ui_e2e_headful_type_key.
 * @param ctx Parameter ctx.
 * @param key_code Parameter key_code.
 * @param modifiers Parameter modifiers.
 * @return Return value.
 */
ui_error_t ui_e2e_headful_type_key(struct ui_e2e_headful_ctx *ctx, int key_code,
                                   unsigned int modifiers) {
  HWND hwnd;
  WORD vk;
  INPUT inputs[4];
  int num_inputs = 0;

  ui_error_t handle_rc;
  void *os_handle = NULL;

  if (!ctx) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!ctx->backend || !ctx->window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!ctx->backend->get_os_handle) {
    return UI_ERROR_UNSUPPORTED;
  }

  handle_rc =
      ctx->backend->get_os_handle(ctx->backend, ctx->window, &os_handle);
  if (handle_rc != UI_ERROR_NONE) {
    return handle_rc;
  }
  hwnd = (HWND)os_handle;
  if (!hwnd) {
    return UI_ERROR_UNSUPPORTED;
  }

  /* Bring window to foreground before typing */
  SetForegroundWindow(hwnd);

  vk = map_ui_key_to_vk(key_code);
  if (vk == 0) {
    /* Assume ascii character for simplicity in testing */
    if (key_code >= 'a' && key_code <= 'z') {
      vk = (WORD)(key_code - 'a' + 'A');
    } else {
      vk = (WORD)key_code;
    }
  }

  if (modifiers & UI_MODIFIER_SHIFT) {
    inputs[num_inputs].type = INPUT_KEYBOARD;
    inputs[num_inputs].ki.wVk = VK_SHIFT;
    inputs[num_inputs].ki.wScan = 0;
    inputs[num_inputs].ki.dwFlags = 0;
    inputs[num_inputs].ki.time = 0;
    inputs[num_inputs].ki.dwExtraInfo = 0;
    num_inputs++;
  }

  inputs[num_inputs].type = INPUT_KEYBOARD;
  inputs[num_inputs].ki.wVk = vk;
  inputs[num_inputs].ki.wScan = 0;
  inputs[num_inputs].ki.dwFlags = 0;
  inputs[num_inputs].ki.time = 0;
  inputs[num_inputs].ki.dwExtraInfo = 0;
  num_inputs++;

  inputs[num_inputs].type = INPUT_KEYBOARD;
  inputs[num_inputs].ki.wVk = vk;
  inputs[num_inputs].ki.wScan = 0;
  inputs[num_inputs].ki.dwFlags = KEYEVENTF_KEYUP;
  inputs[num_inputs].ki.time = 0;
  inputs[num_inputs].ki.dwExtraInfo = 0;
  num_inputs++;

  if (modifiers & UI_MODIFIER_SHIFT) {
    inputs[num_inputs].type = INPUT_KEYBOARD;
    inputs[num_inputs].ki.wVk = VK_SHIFT;
    inputs[num_inputs].ki.wScan = 0;
    inputs[num_inputs].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[num_inputs].ki.time = 0;
    inputs[num_inputs].ki.dwExtraInfo = 0;
    num_inputs++;
  }

  SendInput((UINT)num_inputs, inputs, sizeof(INPUT));

  return UI_ERROR_NONE;
}

#else

/*
 * @brief ui_e2e_headful_click.
 * @param ctx Parameter ctx.
 * @param x Parameter x.
 * @param y Parameter y.
 * @return Return value.
 */
ui_error_t ui_e2e_headful_click(struct ui_e2e_headful_ctx *ctx, int x, int y) {
  (void)ctx;
  (void)x;
  (void)y;
  return UI_ERROR_UNSUPPORTED;
}

/*
 * @brief ui_e2e_headful_type_key.
 * @param ctx Parameter ctx.
 * @param key_code Parameter key_code.
 * @param modifiers Parameter modifiers.
 * @return Return value.
 */
ui_error_t ui_e2e_headful_type_key(struct ui_e2e_headful_ctx *ctx, int key_code,
                                   unsigned int modifiers) {
  (void)ctx;
  (void)key_code;
  (void)modifiers;
  return UI_ERROR_UNSUPPORTED;
}

#endif
