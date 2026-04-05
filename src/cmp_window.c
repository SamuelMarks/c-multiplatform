/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
/* Include Windows headers manually since we don't want them in cmp.h */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windef.h>
#include <winbase.h>
#include <wingdi.h>
#include <winuser.h>
#include <windowsx.h>
#endif
/* clang-format on */

static int g_window_initialized = 0;

struct cmp_window {
  cmp_window_config_t config;
#if defined(_WIN32)
  HWND hwnd;
#endif
  int should_close;
  cmp_renderer_t *renderer;
  cmp_window_drop_cb_t drop_cb;
  void *drop_user_data;
  cmp_ui_node_t *ui_tree;
  float scale_factor;
};

#if defined(_WIN32)
#if (!defined(_MSC_VER) || _MSC_VER >= 1500)
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#define HAS_DWMAPI
#endif
#include <imm.h>
#pragma comment(lib, "imm32.lib")
#include <ole2.h>
#pragma comment(lib, "ole32.lib")
#include <shellapi.h>

/* ShellScalingApi requires Windows 8.1+ so we dynamic load it for compatibility
 */
#ifndef DPI_AWARENESS_CONTEXT_UNAWARE
typedef enum {
  DPI_AWARENESS_CONTEXT_UNAWARE = -1,
  DPI_AWARENESS_CONTEXT_SYSTEM_AWARE = -2,
  DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE = -3,
  DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = -4,
  DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED = -5
} DPI_AWARENESS_CONTEXT;
#endif

typedef BOOL(WINAPI *SetProcessDpiAwarenessContext_fn)(DPI_AWARENESS_CONTEXT);
typedef BOOL(WINAPI *SetProcessDPIAware_fn)(void);

static void enable_high_dpi_awareness(void) {
  HMODULE user32 = LoadLibraryA("user32.dll");
  if (user32) {
    SetProcessDpiAwarenessContext_fn set_dpi_v2 =
        (SetProcessDpiAwarenessContext_fn)GetProcAddress(
            user32, "SetProcessDpiAwarenessContext");
    SetProcessDPIAware_fn set_dpi;

    if (set_dpi_v2) {
      if (set_dpi_v2(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        FreeLibrary(user32);
        return;
      }
    }

    set_dpi =
        (SetProcessDPIAware_fn)GetProcAddress(user32, "SetProcessDPIAware");
    if (set_dpi) {
      set_dpi();
    }
    FreeLibrary(user32);
  }
}

/* Basic IDropTarget implementation for drag and drop */
typedef struct cmp_drop_target {
  IDropTargetVtbl *lpVtbl;
  ULONG ref_count;
  HWND hwnd;
  cmp_window_t *window;
} cmp_drop_target_t;

static HRESULT STDMETHODCALLTYPE drop_target_query_interface(IDropTarget *This,
                                                             REFIID riid,
                                                             void **ppvObject) {
  if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IDropTarget)) {
    *ppvObject = This;
    This->lpVtbl->AddRef(This);
    return S_OK;
  }
  *ppvObject = NULL;
  return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE drop_target_add_ref(IDropTarget *This) {
  cmp_drop_target_t *dt = (cmp_drop_target_t *)This;
  return ++dt->ref_count;
}

static ULONG STDMETHODCALLTYPE drop_target_release(IDropTarget *This) {
  cmp_drop_target_t *dt = (cmp_drop_target_t *)This;
  ULONG count = --dt->ref_count;
  if (count == 0) {
    CMP_FREE(dt->lpVtbl);
    CMP_FREE(dt);
  }
  return count;
}

static HRESULT STDMETHODCALLTYPE drop_target_drag_enter(IDropTarget *This,
                                                        IDataObject *pDataObj,
                                                        DWORD grfKeyState,
                                                        POINTL pt,
                                                        DWORD *pdwEffect) {
  (void)This;
  (void)pDataObj;
  (void)grfKeyState;
  (void)pt;
  *pdwEffect = DROPEFFECT_COPY;
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE drop_target_drag_over(IDropTarget *This,
                                                       DWORD grfKeyState,
                                                       POINTL pt,
                                                       DWORD *pdwEffect) {
  (void)This;
  (void)grfKeyState;
  (void)pt;
  *pdwEffect = DROPEFFECT_COPY;
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE drop_target_drag_leave(IDropTarget *This) {
  (void)This;
  return S_OK;
}

static HRESULT STDMETHODCALLTYPE drop_target_drop(IDropTarget *This,
                                                  IDataObject *pDataObj,
                                                  DWORD grfKeyState, POINTL pt,
                                                  DWORD *pdwEffect) {
  cmp_drop_target_t *dt = (cmp_drop_target_t *)This;
  FORMATETC fmt = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  STGMEDIUM stg;

  (void)grfKeyState;
  (void)pt;
  *pdwEffect = DROPEFFECT_COPY;

  if (dt->window && dt->window->drop_cb &&
      pDataObj->lpVtbl->GetData(pDataObj, &fmt, &stg) == S_OK) {
    HDROP hDrop = (HDROP)stg.hGlobal;
    UINT count = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0);
    UINT i;
    for (i = 0; i < count; i++) {
      char path[MAX_PATH];
      if (DragQueryFileA(hDrop, i, path, MAX_PATH)) {
        dt->window->drop_cb(path, dt->window->drop_user_data);
      }
    }
    ReleaseStgMedium(&stg);
  }

  return S_OK;
}

static cmp_drop_target_t *create_drop_target(HWND hwnd, cmp_window_t *window) {
  cmp_drop_target_t *dt;
  if (CMP_MALLOC(sizeof(cmp_drop_target_t), (void **)&dt) != CMP_SUCCESS) {
    return NULL;
  }
  if (CMP_MALLOC(sizeof(IDropTargetVtbl), (void **)&dt->lpVtbl) !=
      CMP_SUCCESS) {
    CMP_FREE(dt);
    return NULL;
  }

  dt->lpVtbl->QueryInterface = drop_target_query_interface;
  dt->lpVtbl->AddRef = drop_target_add_ref;
  dt->lpVtbl->Release = drop_target_release;
  dt->lpVtbl->DragEnter = drop_target_drag_enter;
  dt->lpVtbl->DragOver = drop_target_drag_over;
  dt->lpVtbl->DragLeave = drop_target_drag_leave;
  dt->lpVtbl->Drop = drop_target_drop;

  dt->ref_count = 1;
  dt->hwnd = hwnd;
  dt->window = window;
  return dt;
}

#ifndef GET_POINTERID_WPARAM
#define GET_POINTERID_WPARAM(wParam) (LOWORD(wParam))
#endif

#if defined(_WIN32)
static void render_node_gdi(HDC hdc, cmp_ui_node_t *node, float scale_factor) {
  size_t i;
  cmp_rect_t rect;

  if (!node || !node->layout)
    return;

  rect = node->layout->computed_rect;
  rect.x *= scale_factor;
  rect.y *= scale_factor;
  rect.width *= scale_factor;
  rect.height *= scale_factor;

  {
    uint32_t box_color = node->bg_color;

    if (node->type == 8) {
      uint32_t bg = node->bg_color;
      uint8_t a = (bg >> 24) & 0xFF;
      if (a > 0) {
        uint8_t r = (bg >> 16) & 0xFF;
        uint8_t g = (bg >> 8) & 0xFF;
        uint8_t b = bg & 0xFF;
        HBRUSH br = CreateSolidBrush(RGB(r, g, b));
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(r, g, b));
        HBRUSH old_br = (HBRUSH)SelectObject(hdc, br);
        HPEN old_pen = (HPEN)SelectObject(hdc, pen);

        /* 12dp corner radius */
        RoundRect(hdc, (int)rect.x, (int)rect.y, (int)(rect.x + rect.width),
                  (int)(rect.y + rect.height), (int)(12.0f * scale_factor), (int)(12.0f * scale_factor));

        SelectObject(hdc, old_br);
        SelectObject(hdc, old_pen);
        DeleteObject(br);
        DeleteObject(pen);
      }
    } else if (box_color != 0) {
      uint8_t a = (box_color >> 24) & 0xFF;
      if (a > 0) {
        uint8_t r = (box_color >> 16) & 0xFF;
        uint8_t g = (box_color >> 8) & 0xFF;
        uint8_t b = box_color & 0xFF;
        HBRUSH br = CreateSolidBrush(RGB(r, g, b));
        RECT rct;
        rct.left = (int)rect.x;
        rct.top = (int)rect.y;
        rct.right = (int)(rect.x + rect.width);
        rct.bottom = (int)(rect.y + rect.height);
        FillRect(hdc, &rct, br);
        DeleteObject(br);
      }
    }

    if (node->type == 2 || node->type == 3 || node->type == 4 || node->type == 11 || node->type == 14) {
      const char *text = (const char *)node->properties;
      if (node->type == 4 && text == NULL) text = "Ask anything";
      if (node->type == 11 && text == NULL) text = "GPT-4o (Default) \xE2\x96\xBE";
      if (node->type == 14 && text == NULL) text = "Let's build";

      if (text) {
        uint32_t tc_uint = node->text_color;
        uint8_t tc_a = (tc_uint >> 24) & 0xFF;
        uint32_t tc = RGB((tc_uint >> 16) & 0xFF, (tc_uint >> 8) & 0xFF, tc_uint & 0xFF);
        HFONT font;
        HFONT old_font;
        int size = (int)(32 * scale_factor);
        if (node->type == 14) size = (int)(48 * scale_factor);
        if (node->type == 4 || node->type == 11) size = (int)(20 * scale_factor);
        if (node->type == 3) size = (int)(24 * scale_factor);

        if (tc_a == 0) {
            tc = RGB(240, 240, 240);
        }

        font = CreateFontA(
            size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        old_font = (HFONT)SelectObject(hdc, font);
        SetTextAlign(hdc, TA_CENTER | TA_TOP);
        SetTextColor(hdc, tc);
        SetBkMode(hdc, TRANSPARENT);

        if (node->type == 14) {
          TextOutA(hdc, (int)(rect.x + rect.width / 2.0f), (int)(rect.y + (rect.height - size) / 2.0f), text, (int)strlen(text));
        } else {
          TextOutA(hdc, (int)(rect.x + rect.width / 2.0f), (int)rect.y, text, (int)strlen(text));
        }

        SelectObject(hdc, old_font);
        DeleteObject(font);
      }
    }
  }
  for (i = 0; i < node->child_count; i++) {
    render_node_gdi(hdc, node->children[i], scale_factor);
  }
}
#endif

static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam) {
  cmp_window_t *window = (cmp_window_t *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

  switch (uMsg) {
  case WM_CREATE: {
    CREATESTRUCTA *cs = (CREATESTRUCTA *)lParam;
    SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
    /* Attempt to get DPI for the created window to set initial scale factor */
    {
      typedef UINT(WINAPI * GetDpiForWindow_fn)(HWND);
      HMODULE user32 = GetModuleHandleA("user32.dll");
      if (user32) {
        GetDpiForWindow_fn get_dpi =
            (GetDpiForWindow_fn)GetProcAddress(user32, "GetDpiForWindow");
        cmp_window_t *w = (cmp_window_t *)cs->lpCreateParams;
        if (get_dpi && w) {
          w->scale_factor = (float)get_dpi(hwnd) / 96.0f;
        } else if (w) {
          w->scale_factor = 1.0f; /* Fallback */
        }
      }
    }
    return 0;
  }
  case WM_CLOSE:
    if (window) {
      window->should_close = 1;
    }
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case 0x02E0: /* WM_DPICHANGED */
  {
    RECT *prcNewWindow = (RECT *)lParam;
    /* Per-Monitor V2 DPI Awareness hooks and dynamic scaling events */
    if (window) {
      UINT new_dpi = HIWORD(wParam);
      window->scale_factor = (float)new_dpi / 96.0f;
    }
    SetWindowPos(hwnd, NULL, prcNewWindow->left, prcNewWindow->top,
                 prcNewWindow->right - prcNewWindow->left,
                 prcNewWindow->bottom - prcNewWindow->top,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }
  case 0x0246: /* WM_POINTERDOWN */
  case 0x0247: /* WM_POINTERUP */
  case 0x0245: /* WM_POINTERUPDATE */
  {
    cmp_event_t evt;
    memset(&evt, 0, sizeof(evt));
    evt.type = 2; /* Touch / Pointer */
    if (uMsg == 0x0246)
      evt.action = CMP_ACTION_DOWN;
    else if (uMsg == 0x0247)
      evt.action = CMP_ACTION_UP;
    else
      evt.action = CMP_ACTION_MOVE;

    evt.source_id = GET_POINTERID_WPARAM(wParam);
    /* Coordinate extraction logic handles mapping screen-to-client correctly */
    {
      POINT pt;
      pt.x = GET_X_LPARAM(lParam);
      pt.y = GET_Y_LPARAM(lParam);
      ScreenToClient(hwnd, &pt);
      evt.x = window ? (int)(pt.x / window->scale_factor) : pt.x;
      evt.y = window ? (int)(pt.y / window->scale_factor) : pt.y;
    }

    cmp_event_push(&evt);
    return 0; /* Handled */
  }
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_MOUSEMOVE: {
    if (window) {
      cmp_event_t evt;
      memset(&evt, 0, sizeof(evt));
      evt.type = 1; /* Mouse */
      if (uMsg == WM_LBUTTONDOWN)
        evt.action = CMP_ACTION_DOWN;
      else if (uMsg == WM_LBUTTONUP)
        evt.action = CMP_ACTION_UP;
      else
        evt.action = CMP_ACTION_MOVE;

      evt.x = (int)(GET_X_LPARAM(lParam) / window->scale_factor);
      evt.y = (int)(GET_Y_LPARAM(lParam) / window->scale_factor);
      evt.source_id = 0; /* Primary mouse */

      cmp_event_push(&evt);
      return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
  }
  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_CHAR: {
    cmp_event_t evt;
    memset(&evt, 0, sizeof(evt));
    evt.type = 3; /* Keyboard */

    if (uMsg == WM_KEYDOWN)
      evt.action = CMP_ACTION_DOWN;
    else if (uMsg == WM_KEYUP)
      evt.action = CMP_ACTION_UP;
    else
      evt.action =
          CMP_ACTION_MOVE; /* Hacky representation for CHAR mapping for now */

    evt.source_id = (int)wParam;
    cmp_event_push(&evt);
    return 0;
  }
  case WM_IME_SETCONTEXT:
  case WM_IME_STARTCOMPOSITION:
  case WM_IME_COMPOSITION:
  case WM_IME_ENDCOMPOSITION:
  case WM_IME_NOTIFY: {
    /* Windows IME (Input Method Editor) for CJK text composition hooks */
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
  }
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    if (window) {
      if (window->config.use_legacy_backend) {
        /* Double-buffered GDI context using DIB sections */
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBM = CreateCompatibleBitmap(hdc, window->config.width,
                                               window->config.height);
        HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = window->config.width;
        rect.bottom = window->config.height;

        FillRect(memDC, &rect, (HBRUSH)(COLOR_WINDOW + 1));

        SetTextAlign(memDC, TA_CENTER);
        SetTextColor(memDC, RGB(0, 0, 0));
        SetBkMode(memDC, TRANSPARENT);
        TextOutA(memDC, window->config.width / 2, window->config.height / 2,
                 "Hello World (Legacy GDI)", 24);

        BitBlt(hdc, 0, 0, window->config.width, window->config.height, memDC, 0,
               0, SRCCOPY);

        SelectObject(memDC, oldBM);
        DeleteObject(memBM);
        DeleteDC(memDC);
      } else {
        /* Normal simple Paint for now, DirectX layers on top later */
        /* FillRect removed to allow transparency */

        if (window->ui_tree) {
          render_node_gdi(hdc, window->ui_tree, 1.0f);
        } else {
          SetTextAlign(hdc, TA_CENTER);
          SetTextColor(hdc, RGB(240, 240, 240));
          SetBkMode(hdc, TRANSPARENT);
          TextOutA(hdc, window->config.width / 2, window->config.height / 2,
                   "Hello World", 11);
        }
      }
    }
    EndPaint(hwnd, &ps);
    return 0;
  }
  }

  return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}
#endif

int cmp_window_system_init(void) {
  if (g_window_initialized) {
    return CMP_SUCCESS;
  }

#if defined(_WIN32)
  enable_high_dpi_awareness();
  {
    WNDCLASSEXA wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "CmpWindowClass";

    if (!RegisterClassExA(&wc)) {
      return CMP_ERROR_NOT_FOUND;
    }

    OleInitialize(NULL);
  }
#endif

  g_window_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_window_system_shutdown(void) {
  if (!g_window_initialized) {
    return CMP_SUCCESS;
  }

#if defined(_WIN32)
  OleUninitialize();
  UnregisterClassA("CmpWindowClass", GetModuleHandleA(NULL));
#endif

  g_window_initialized = 0;
  return CMP_SUCCESS;
}

int cmp_window_create(const cmp_window_config_t *config,
                      cmp_window_t **out_window) {
  cmp_window_t *window;

  if (config == NULL || out_window == NULL || !g_window_initialized) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_window_t), (void **)&window) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(window, 0, sizeof(cmp_window_t));
  window->config = *config;
  window->should_close = 0;

#if defined(_WIN32)
  {
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD ex_style = 0;
    RECT rect;

    if (config->frameless) {
      style = WS_POPUP;
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = config->width;
    rect.bottom = config->height;
    AdjustWindowRectEx(&rect, style, FALSE, ex_style);

    window->hwnd = CreateWindowExA(
        ex_style, "CmpWindowClass", config->title ? config->title : "CMP",
        style, config->x > 0 ? config->x : CW_USEDEFAULT,
        config->y > 0 ? config->y : CW_USEDEFAULT, rect.right - rect.left,
        rect.bottom - rect.top, NULL, NULL, GetModuleHandleA(NULL), window);

    if (!window->hwnd) {
      CMP_FREE(window);
      return CMP_ERROR_NOT_FOUND;
    }

    if (config->frameless) {
#ifdef HAS_DWMAPI
      MARGINS margins = {1, 1, 1, 1};
      DwmExtendFrameIntoClientArea(window->hwnd, &margins);
#endif
    }

    {
      cmp_drop_target_t *dt = create_drop_target(window->hwnd, window);
      if (dt) {
        RegisterDragDrop(window->hwnd, (IDropTarget *)dt);
        /* Hand ownership to COM */
        dt->lpVtbl->Release((IDropTarget *)dt);
      }
    }
  }
#else
  /* POSIX fallback / unsupported stub for Phase 6 */
#endif
#if defined(__APPLE__)
  /* Apple (macOS & iOS) Objective-C/C bridge stubs */
  /* This would allocate an NSWindow/UIView and link it to the pointer */
  (void)config;
#endif
#if defined(__linux__) && !defined(__ANDROID__)
  /* Linux (Wayland / X11) backend stubs */
  /* This would connect to Wayland display/registry and create xdg_surface, or
   * XOpenDisplay/XCreateWindow */
  (void)config;
#endif
#if defined(__ANDROID__)
  /* Android JNI hooks */
  /* This would bind to android_app context and EGL display */
  (void)config;
#endif
#if defined(__EMSCRIPTEN__)
  /* WebAssembly (Emscripten) canvas creation */
  /* Maps to Emscripten's HTML5 API (emscripten_set_canvas_element_size etc) */
  (void)config;
#endif
#if defined(CMP_USE_SDL3)
  /* Universal Fallback */
  /* window->sdl_window = SDL_CreateWindow(...) */
  (void)config;
#endif

  *out_window = window;
  return CMP_SUCCESS;
}

int cmp_window_set_drop_callback(cmp_window_t *window,
                                 cmp_window_drop_cb_t drop_cb,
                                 void *user_data) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  window->drop_cb = drop_cb;
  window->drop_user_data = user_data;
  return CMP_SUCCESS;
}
int cmp_window_show(cmp_window_t *window) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  ShowWindow(window->hwnd, SW_SHOW);
  UpdateWindow(window->hwnd);
#endif

  return CMP_SUCCESS;
}

int cmp_window_poll_events(cmp_window_t *window) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  {
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
    }
  }
#endif

  return CMP_SUCCESS;
}

int cmp_window_should_close(cmp_window_t *window) {
  if (window == NULL) {
    return 1;
  }
  return window->should_close;
}

int cmp_hardware_poll_gamepad(int index, cmp_gamepad_t *out_gamepad) {
  if (out_gamepad == NULL)
    return CMP_ERROR_INVALID_ARG;
  out_gamepad->id = index;
  out_gamepad->is_connected = 0; /* Stub */
  return CMP_SUCCESS;
}

int cmp_hardware_trigger_haptic(int index, float low_frequency,
                                float high_frequency, int duration_ms) {
  (void)index;
  (void)low_frequency;
  (void)high_frequency;
  (void)duration_ms;
  return CMP_SUCCESS; /* Stub */
}

int cmp_hardware_camera_start(int device_index, cmp_camera_t **out_camera) {
  cmp_camera_t *camera;
  if (out_camera == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_camera_t), (void **)&camera) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  camera->internal_handle = NULL;
  camera->width = 640;
  camera->height = 480;
  camera->is_capturing = 1;
  (void)device_index;
  *out_camera = camera;
  return CMP_SUCCESS;
}

int cmp_hardware_camera_read_frame(cmp_camera_t *camera,
                                   cmp_texture_t *target_texture) {
  if (camera == NULL || target_texture == NULL)
    return CMP_ERROR_INVALID_ARG;
  return CMP_SUCCESS;
}

int cmp_hardware_camera_stop(cmp_camera_t *camera) {
  if (camera == NULL)
    return CMP_ERROR_INVALID_ARG;
  camera->is_capturing = 0;
  CMP_FREE(camera);
  return CMP_SUCCESS;
}

int cmp_hardware_poll_sensors(cmp_sensor_data_t *out_data) {
  if (out_data == NULL)
    return CMP_ERROR_INVALID_ARG;
  memset(out_data, 0, sizeof(cmp_sensor_data_t));
  return CMP_SUCCESS;
}

int cmp_hardware_poll_geolocation(double *out_latitude, double *out_longitude) {
  if (out_latitude)
    *out_latitude = 0.0;
  if (out_longitude)
    *out_longitude = 0.0;
  return CMP_SUCCESS;
}

int cmp_window_set_pointer_lock(cmp_window_t *window,
                                cmp_pointer_lock_t lock_mode) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  if (lock_mode == CMP_POINTER_UNLOCKED) {
    ClipCursor(NULL);
    ShowCursor(TRUE);
  } else {
    RECT rect;
    GetClientRect(window->hwnd, &rect);
    ClientToScreen(window->hwnd, (POINT *)&rect.left);
    ClientToScreen(window->hwnd, (POINT *)&rect.right);
    ClipCursor(&rect);
    if (lock_mode == CMP_POINTER_LOCKED_HIDDEN) {
      ShowCursor(FALSE);
    }
  }
#else
  (void)lock_mode;
#endif

  return CMP_SUCCESS;
}

int cmp_window_render_test_frame(cmp_window_t *window) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  InvalidateRect(window->hwnd, NULL, TRUE);
#endif

  return CMP_SUCCESS;
}

int cmp_window_mac_init_menu_bar(void) {
#if defined(__APPLE__)
  /* Call out to objective-c NSMenu setup */
  return CMP_SUCCESS;
#else
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_mac_add_menu_item(const char *title, const char *key_equiv,
                                 void (*callback)(void)) {
  if (title == NULL || key_equiv == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__APPLE__)
  /* Call out to objective-c NSMenuItem allocation */
  (void)callback;
  return CMP_SUCCESS;
#else
  (void)callback;
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_apple_init_display_link(cmp_window_t *window, int refresh_rate) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__APPLE__)
  /* Setup CADisplayLink or CVDisplayLink depending on iOS/macOS */
  (void)refresh_rate;
  return CMP_SUCCESS;
#else
  (void)refresh_rate;
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_apple_enable_gestures(cmp_window_t *window, int enable_pinch,
                                     int enable_rotation, int enable_swipe) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__APPLE__)
  /* Bridge directly to UIGestureRecognizer/NSGestureRecognizer mapping logic */
  (void)enable_pinch;
  (void)enable_rotation;
  (void)enable_swipe;
  return CMP_SUCCESS;
#else
  (void)enable_pinch;
  (void)enable_rotation;
  (void)enable_swipe;
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_os_notify(const char *title, const char *body) {
  if (title == NULL || body == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__linux__) && !defined(__ANDROID__)
  /* Execute via libnotify or direct DBus message */
  return CMP_SUCCESS;
#else
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_set_clipboard_text(cmp_window_t *window,
                                  cmp_clipboard_type_t type, const char *text) {
  if (window == NULL || text == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__linux__) && !defined(__ANDROID__)
  /* X11 XSetSelectionOwner or Wayland zwp_primary_selection_device_v1 */
  (void)type;
  return CMP_SUCCESS;
#else
  (void)type;
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_get_clipboard_text(cmp_window_t *window,
                                  cmp_clipboard_type_t type,
                                  cmp_string_t *out_text) {
  if (window == NULL || out_text == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__linux__) && !defined(__ANDROID__)
  /* XConvertSelection or Wayland data offer receive */
  (void)type;
  return cmp_string_init(out_text);
#else
  (void)type;
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_linux_init_evdev(const char *event_device_path) {
  if (event_device_path == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__linux__) && !defined(__ANDROID__)
  /* Open /dev/input/X and attach libevdev processing to modality loop */
  return CMP_SUCCESS;
#else
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_android_init_hooks(void *app_state) {
  if (app_state == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__ANDROID__)
  /* Save the android_app state globally to intercept events */
  return CMP_SUCCESS;
#else
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_android_show_keyboard(cmp_window_t *window, int show) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__ANDROID__)
  /* Call NativeActivity->vm->AttachCurrentThread, find InputMethodManager,
   * toggleSoftInput */
  (void)show;
  return CMP_SUCCESS;
#else
  (void)show;
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_android_get_safe_area(cmp_window_t *window, int *out_top,
                                     int *out_bottom, int *out_left,
                                     int *out_right) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__ANDROID__)
  /* Call WindowInsets via JNI */
  if (out_top)
    *out_top = 0;
  if (out_bottom)
    *out_bottom = 0;
  if (out_left)
    *out_left = 0;
  if (out_right)
    *out_right = 0;
  return CMP_SUCCESS;
#else
  if (out_top)
    *out_top = 0;
  if (out_bottom)
    *out_bottom = 0;
  if (out_left)
    *out_left = 0;
  if (out_right)
    *out_right = 0;
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_wasm_init(const char *canvas_selector) {
  if (canvas_selector == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__EMSCRIPTEN__)
  /* Call emscripten_set_canvas_element_size and init bindings */
  return CMP_SUCCESS;
#else
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_wasm_resume_audio(void) {
#if defined(__EMSCRIPTEN__)
  /* Execute EM_ASM to resume AudioContext */
  return CMP_SUCCESS;
#else
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_window_wasm_set_main_loop(cmp_modality_t *mod,
                                  void (*main_loop)(void *), void *arg) {
  if (mod == NULL || main_loop == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(__EMSCRIPTEN__)
  /* emscripten_set_main_loop_arg */
  (void)arg;
  return CMP_SUCCESS;
#else
  (void)arg;
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_sdl3_fallback_init_subsystems(void) {
#if defined(CMP_USE_SDL3)
  /* SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_AUDIO) */
  return CMP_SUCCESS;
#else
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_sdl3_fallback_bridge_audio(void) {
#if defined(CMP_USE_SDL3)
  /* Open SDL_AudioDevice with mapping to cmp_audio_buffer formats */
  return CMP_SUCCESS;
#else
  return CMP_ERROR_NOT_FOUND;
#endif
}

int cmp_scripting_lua_init(void) {
  /* Initialize embedded Lua state and register CMP C functions */
  return CMP_SUCCESS;
}

int cmp_scripting_lua_execute_file(const char *script_path) {
  if (script_path == NULL)
    return CMP_ERROR_INVALID_ARG;
  /* Call luaL_dofile using resolved path */
  return CMP_SUCCESS;
}

int cmp_scripting_napi_init(void *env, void *exports) {
  if (env == NULL || exports == NULL)
    return CMP_ERROR_INVALID_ARG;
  /* Register napi_define_properties */
  return CMP_SUCCESS;
}

int cmp_scripting_python_generate_bindings(const char *output_path) {
  if (output_path == NULL)
    return CMP_ERROR_INVALID_ARG;
  /* Parse cmp.h and output ctypes format into output_path */
  return CMP_SUCCESS;
}

int cmp_window_destroy(cmp_window_t *window) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  if (window->hwnd) {
    SetWindowLongPtrA(window->hwnd, GWLP_USERDATA, 0);
    DestroyWindow(window->hwnd);
    window->hwnd = NULL;
  }
#endif

  CMP_FREE(window);
  return CMP_SUCCESS;
}

struct cmp_renderer {
  int initialized;
};

int cmp_renderer_create(cmp_window_t *window, cmp_render_backend_t backend,
                        cmp_renderer_t **out_renderer) {
  cmp_renderer_t *renderer;

  if (window == NULL || out_renderer == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_renderer_t), (void **)&renderer) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  /* Stub for Phase 18 layout logic */
  /* Real implementation would route backend configuration down via vtables */
  (void)backend;

  *out_renderer = renderer;
  window->renderer = renderer;
  return CMP_SUCCESS;
}

int cmp_renderer_destroy(cmp_renderer_t *renderer) {
  if (renderer == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Destroy textures, pipelines, then backend */
  CMP_FREE(renderer);
  return CMP_SUCCESS;
}

int cmp_renderer_begin_frame(cmp_renderer_t *renderer,
                             cmp_color_t clear_color) {
  if (renderer == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  (void)clear_color;
  return CMP_SUCCESS;
}

int cmp_renderer_end_frame(cmp_renderer_t *renderer) {
  if (renderer == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

int cmp_renderer_draw_sprite(cmp_renderer_t *renderer, cmp_texture_t *texture,
                             cmp_rect_t dest, cmp_rect_t *src,
                             cmp_color_t color) {
  if (renderer == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  (void)texture;
  (void)dest;
  (void)src;
  (void)color;
  return CMP_SUCCESS;
}

int cmp_renderer_set_shader(cmp_renderer_t *renderer, cmp_shader_t *shader) {
  if (renderer == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  (void)shader;
  return CMP_SUCCESS;
}

int cmp_renderer_set_render_target(cmp_renderer_t *renderer,
                                   cmp_texture_t *texture) {
  if (renderer == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  (void)texture;
  return CMP_SUCCESS;
}

int cmp_texture_create(cmp_renderer_t *renderer, int width, int height,
                       const void *pixels, cmp_texture_t **out_texture) {
  cmp_texture_t *texture;

  if (renderer == NULL || out_texture == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_texture_t), (void **)&texture) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  texture->internal_handle = NULL;
  texture->width = width;
  texture->height = height;
  (void)pixels;

  *out_texture = texture;
  return CMP_SUCCESS;
}

int cmp_texture_destroy(cmp_texture_t *texture) {
  if (texture == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(texture);
  return CMP_SUCCESS;
}

static int g_typography_initialized = 0;

int cmp_typography_init(void) {
  g_typography_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_typography_shutdown(void) {
  g_typography_initialized = 0;
  return CMP_SUCCESS;
}

int cmp_font_load(const char *virtual_path, float default_size,
                  cmp_font_t **out_font) {
  cmp_font_t *font;
  if (virtual_path == NULL || out_font == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_font_t), (void **)&font) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  font->internal_handle = NULL; /* Stub for stb_truetype / FreeType handle */
  font->default_size = default_size;
  *out_font = font;
  return CMP_SUCCESS;
}

int cmp_font_load_memory(const void *buffer, size_t size, float default_size,
                         cmp_font_t **out_font) {
  cmp_font_t *font;
  if (buffer == NULL || size == 0 || out_font == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_font_t), (void **)&font) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  font->internal_handle = NULL;
  font->default_size = default_size;
  *out_font = font;
  return CMP_SUCCESS;
}

int cmp_font_add_fallback(cmp_font_t *primary, cmp_font_t *fallback) {
  if (primary == NULL || fallback == NULL)
    return CMP_ERROR_INVALID_ARG;
  /* Stub for linking fallback chains */
  return CMP_SUCCESS;
}

int cmp_font_generate_sdf(cmp_font_t *font, uint32_t codepoint,
                          cmp_texture_t **out_texture) {
  if (font == NULL || out_texture == NULL)
    return CMP_ERROR_INVALID_ARG;
  (void)codepoint;
  return cmp_texture_create(NULL, 64, 64, NULL, out_texture);
}

int cmp_font_destroy(cmp_font_t *font) {
  if (font == NULL)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(font);
  return CMP_SUCCESS;
}

int cmp_text_shape(cmp_font_t *font, const char *text, float *out_width,
                   float *out_height) {
  if (font == NULL || text == NULL)
    return CMP_ERROR_INVALID_ARG;
  /* Basic stub calculation */
  if (out_width)
    *out_width = strlen(text) * (font->default_size * 0.5f);
  if (out_height)
    *out_height = font->default_size;
  return CMP_SUCCESS;
}

static int g_theme_initialized = 0;

int cmp_theme_init(void) {
  g_theme_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_theme_shutdown(void) {
  g_theme_initialized = 0;
  return CMP_SUCCESS;
}

int cmp_theme_generate_palette(cmp_color_t seed, cmp_palette_t *out_palette) {
  if (out_palette == NULL)
    return CMP_ERROR_INVALID_ARG;
  /* Stub for color space math (e.g. HCT space generation for Material 3) */
  memset(out_palette, 0, sizeof(cmp_palette_t));

  /* Primary */
  out_palette->primary = seed;
  out_palette->on_primary.r = 1.0f - seed.r;
  out_palette->on_primary.g = 1.0f - seed.g;
  out_palette->on_primary.b = 1.0f - seed.b;
  out_palette->on_primary.a = 1.0f;
  out_palette->on_primary.space = CMP_COLOR_SPACE_SRGB;

  out_palette->primary_container.r = seed.r * 0.8f;
  out_palette->primary_container.g = seed.g * 0.8f;
  out_palette->primary_container.b = seed.b * 0.8f;
  out_palette->primary_container.a = 1.0f;
  out_palette->primary_container.space = CMP_COLOR_SPACE_SRGB;

  out_palette->on_primary_container.r = 1.0f - seed.r * 0.8f;
  out_palette->on_primary_container.g = 1.0f - seed.g * 0.8f;
  out_palette->on_primary_container.b = 1.0f - seed.b * 0.8f;
  out_palette->on_primary_container.a = 1.0f;
  out_palette->on_primary_container.space = CMP_COLOR_SPACE_SRGB;

  /* Secondary */
  out_palette->secondary.r = seed.r * 0.9f;
  out_palette->secondary.g = seed.g * 0.9f;
  out_palette->secondary.b = seed.b * 0.9f;
  out_palette->secondary.a = 1.0f;
  out_palette->secondary.space = CMP_COLOR_SPACE_SRGB;

  out_palette->on_secondary.r = 1.0f - seed.r * 0.9f;
  out_palette->on_secondary.g = 1.0f - seed.g * 0.9f;
  out_palette->on_secondary.b = 1.0f - seed.b * 0.9f;
  out_palette->on_secondary.a = 1.0f;
  out_palette->on_secondary.space = CMP_COLOR_SPACE_SRGB;

  out_palette->secondary_container.r = seed.r * 0.7f;
  out_palette->secondary_container.g = seed.g * 0.7f;
  out_palette->secondary_container.b = seed.b * 0.7f;
  out_palette->secondary_container.a = 1.0f;
  out_palette->secondary_container.space = CMP_COLOR_SPACE_SRGB;

  out_palette->on_secondary_container.r = 1.0f - seed.r * 0.7f;
  out_palette->on_secondary_container.g = 1.0f - seed.g * 0.7f;
  out_palette->on_secondary_container.b = 1.0f - seed.b * 0.7f;
  out_palette->on_secondary_container.a = 1.0f;
  out_palette->on_secondary_container.space = CMP_COLOR_SPACE_SRGB;

  /* Neutral / Background / Surface */
  out_palette->background.r = 0.98f;
  out_palette->background.g = 0.98f;
  out_palette->background.b = 0.98f;
  out_palette->background.a = 1.0f;
  out_palette->background.space = CMP_COLOR_SPACE_SRGB;

  out_palette->on_background.r = 0.1f;
  out_palette->on_background.g = 0.1f;
  out_palette->on_background.b = 0.1f;
  out_palette->on_background.a = 1.0f;
  out_palette->on_background.space = CMP_COLOR_SPACE_SRGB;

  out_palette->surface.r = 1.0f;
  out_palette->surface.g = 1.0f;
  out_palette->surface.b = 1.0f;
  out_palette->surface.a = 1.0f;
  out_palette->surface.space = CMP_COLOR_SPACE_SRGB;

  out_palette->on_surface.r = 0.1f;
  out_palette->on_surface.g = 0.1f;
  out_palette->on_surface.b = 0.1f;
  out_palette->on_surface.a = 1.0f;
  out_palette->on_surface.space = CMP_COLOR_SPACE_SRGB;

  /* Error */
  out_palette->error.r = 0.8f;
  out_palette->error.g = 0.1f;
  out_palette->error.b = 0.1f;
  out_palette->error.a = 1.0f;
  out_palette->error.space = CMP_COLOR_SPACE_SRGB;

  out_palette->on_error.r = 1.0f;
  out_palette->on_error.g = 1.0f;
  out_palette->on_error.b = 1.0f;
  out_palette->on_error.a = 1.0f;
  out_palette->on_error.space = CMP_COLOR_SPACE_SRGB;

  return CMP_SUCCESS;
}

int cmp_window_set_theme(cmp_window_t *window, const cmp_theme_t *theme) {
  if (window == NULL || theme == NULL)
    return CMP_ERROR_INVALID_ARG;
  /* Store theme pointer on window or invalidate rect to trigger redraw with new
   * colors */
  return CMP_SUCCESS;
}

static int g_i18n_initialized = 0;
static cmp_text_direction_t g_bidi_dir = CMP_TEXT_DIR_LTR;

int cmp_i18n_init(void) {
  g_i18n_initialized = 1;
  return CMP_SUCCESS;
}

static int g_audio_initialized = 0;

int cmp_audio_init(void) {
  g_audio_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_audio_shutdown(void) {
  g_audio_initialized = 0;
  return CMP_SUCCESS;
}

int cmp_audio_buffer_load(const char *virtual_path,
                          cmp_audio_buffer_t **out_buffer) {
  cmp_audio_buffer_t *buffer;
  if (virtual_path == NULL || out_buffer == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_audio_buffer_t), (void **)&buffer) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  buffer->internal_handle = NULL;
  buffer->channels = 2;
  buffer->sample_rate = 44100;
  *out_buffer = buffer;
  return CMP_SUCCESS;
}

int cmp_audio_buffer_destroy(cmp_audio_buffer_t *buffer) {
  if (buffer == NULL)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(buffer);
  return CMP_SUCCESS;
}

int cmp_audio_source_create(cmp_audio_buffer_t *buffer,
                            cmp_audio_source_t **out_source) {
  cmp_audio_source_t *source;
  if (buffer == NULL || out_source == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_audio_source_t), (void **)&source) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  source->internal_handle = NULL;
  source->volume = 1.0f;
  source->pan_x = 0;
  source->pan_y = 0;
  source->pan_z = 0;
  source->is_playing = 0;
  source->is_looping = 0;
  *out_source = source;
  return CMP_SUCCESS;
}

int cmp_audio_source_play(cmp_audio_source_t *source) {
  if (source == NULL)
    return CMP_ERROR_INVALID_ARG;
  source->is_playing = 1;
  return CMP_SUCCESS;
}

int cmp_audio_source_set_position(cmp_audio_source_t *source, float x, float y,
                                  float z) {
  if (source == NULL)
    return CMP_ERROR_INVALID_ARG;
  source->pan_x = x;
  source->pan_y = y;
  source->pan_z = z;
  return CMP_SUCCESS;
}

int cmp_audio_source_destroy(cmp_audio_source_t *source) {
  if (source == NULL)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(source);
  return CMP_SUCCESS;
}

int cmp_video_decoder_open(const char *virtual_path,
                           cmp_video_decoder_t **out_decoder) {
  cmp_video_decoder_t *decoder;
  if (virtual_path == NULL || out_decoder == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_video_decoder_t), (void **)&decoder) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  decoder->internal_handle = NULL;
  decoder->width = 1920;
  decoder->height = 1080;
  decoder->framerate = 30.0f;
  decoder->is_playing = 0;
  *out_decoder = decoder;
  return CMP_SUCCESS;
}

int cmp_video_decoder_read_frame(cmp_video_decoder_t *decoder,
                                 cmp_texture_t *target_texture) {
  if (decoder == NULL || target_texture == NULL)
    return CMP_ERROR_INVALID_ARG;
  return CMP_SUCCESS;
}

int cmp_video_decoder_destroy(cmp_video_decoder_t *decoder) {
  if (decoder == NULL)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(decoder);
  return CMP_SUCCESS;
}

int cmp_i18n_detect_os_locale(cmp_string_t *out_locale) {
  if (out_locale == NULL)
    return CMP_ERROR_INVALID_ARG;
  cmp_string_init(out_locale);
  /* Stub: Use GetUserDefaultLocaleName on Win32, etc */
  cmp_string_append(out_locale, "en-US");
  return CMP_SUCCESS;
}

int cmp_i18n_load_catalog(const char *virtual_path, const char *locale) {
  if (virtual_path == NULL || locale == NULL)
    return CMP_ERROR_INVALID_ARG;
  return CMP_SUCCESS;
}

int cmp_i18n_translate(const char *key, cmp_string_t *out_translated) {
  if (key == NULL || out_translated == NULL)
    return CMP_ERROR_INVALID_ARG;
  cmp_string_init(out_translated);
  cmp_string_append(out_translated, key); /* Echo key as fallback */
  return CMP_SUCCESS;
}

int cmp_i18n_translate_plural(const char *key, int count,
                              cmp_string_t *out_translated) {
  if (key == NULL || out_translated == NULL)
    return CMP_ERROR_INVALID_ARG;
  cmp_string_init(out_translated);
  (void)count;
  cmp_string_append(out_translated, key);
  return CMP_SUCCESS;
}

int cmp_test_enable_headless(void) {
  /* Set a global flag that will make window creation skip opening an actual
   * HWND */
  return CMP_SUCCESS;
}

int cmp_test_simulate_input(const cmp_event_t *event) {
  if (event == NULL)
    return CMP_ERROR_INVALID_ARG;
  return cmp_event_push(event);
}

int cmp_test_capture_snapshot(cmp_window_t *window, void **out_pixels,
                              int *out_width, int *out_height) {
  if (window == NULL || out_pixels == NULL)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(window->config.width * window->config.height * 4,
                 out_pixels) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  /* Zero out memory */
  memset(*out_pixels, 0, window->config.width * window->config.height * 4);

  if (out_width)
    *out_width = window->config.width;
  if (out_height)
    *out_height = window->config.height;
  return CMP_SUCCESS;
}

int cmp_devtools_set_enabled(cmp_window_t *window, int enable) {
  if (window == NULL)
    return CMP_ERROR_INVALID_ARG;
  (void)enable; /* Toggle a flag in window struct later */
  return CMP_SUCCESS;
}

int cmp_i18n_set_bidi_direction(cmp_text_direction_t dir) {
  g_bidi_dir = dir;
  return CMP_SUCCESS;
}

int cmp_i18n_get_bidi_direction(void) { return (int)g_bidi_dir; }

int cmp_window_set_ui_tree(cmp_window_t *window, cmp_ui_node_t *tree) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  window->ui_tree = tree;
#if defined(_WIN32)
  if (window->hwnd) {
    InvalidateRect(window->hwnd, NULL, FALSE);
  }
#endif
  return CMP_SUCCESS;
}


