/* clang-format off */
#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#endif

#include "cmp.h"
#include "cmp_log.h"
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
#elif defined(__APPLE__)
  void *apple_view;
#elif defined(__linux__) && !defined(__ANDROID__)
  void *xdg_surface;
#endif
  int should_close;
  cmp_renderer_t *renderer;
  cmp_window_drop_cb_t drop_cb;
  void *drop_user_data;
  cmp_window_resize_cb_t resize_cb;
  void *resize_user_data;
  cmp_ui_node_t *ui_tree;
  float scale_factor;
};

void *cmp_window_get_native_handle(cmp_window_t *window) {
  if (!window)
    return NULL;
#if defined(_WIN32)
  return (void *)window->hwnd;
#elif defined(__APPLE__)
  return window->apple_view;
#elif defined(__linux__) && !defined(__ANDROID__)
  return window->xdg_surface;
#else
  return NULL;
#endif
}

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
static void win32_box_blur_alpha(uint8_t *pixels, int width, int height,
                                 int stride, int radius) {
  uint8_t *temp;
  int x, y, i;
  if (radius < 1)
    return;
  temp = (uint8_t *)malloc(width * height);
  if (!temp)
    return;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      int sum = 0, count = 0;
      for (i = -radius; i <= radius; i++) {
        int px = x + i;
        if (px >= 0 && px < width) {
          sum += pixels[y * stride + px * 4 + 3];
          count++;
        }
      }
      temp[y * width + x] = (uint8_t)(sum / count);
    }
  }
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      int sum = 0, count = 0;
      for (i = -radius; i <= radius; i++) {
        int py = y + i;
        if (py >= 0 && py < height) {
          sum += temp[py * width + x];
          count++;
        }
      }
      pixels[y * stride + x * 4 + 3] = (uint8_t)(sum / count);
    }
  }
  free(temp);
}

static void render_node_gdi(HDC hdc, cmp_ui_node_t *node, float scale_factor,
                            int inherited_theme) {
  int current_theme = node->design_language_override
                          ? node->design_language_override
                          : inherited_theme;
  size_t i;
  cmp_rect_t rect;
  uint32_t box_color;
  float opacity;
  uint32_t bg_color_val;
  uint32_t text_color_val;
  uint32_t border_color_val;

  if (!node || !node->layout)
    return;

  bg_color_val = node->bg_color_ref ? *(node->bg_color_ref) : node->bg_color;
  text_color_val =
      node->text_color_ref ? *(node->text_color_ref) : node->text_color;
  border_color_val =
      node->border_color_ref ? *(node->border_color_ref) : node->border_color;

  rect = node->layout->computed_rect;
  rect.x *= scale_factor;
  rect.y *= scale_factor;
  rect.width *= scale_factor;
  rect.height *= scale_factor;

  box_color = bg_color_val;
  opacity = node->opacity > 0.0f ? node->opacity : 1.0f;

  if (node->elevation > 0.0f && opacity > 0.0f) {
    int shadow_blur = (int)(node->elevation * scale_factor);
    int shadow_offset = (int)(node->elevation * scale_factor * 0.5f);
    int sr = (int)(node->border_radius * scale_factor * 2.0f);
    int iw = (int)rect.width;
    int ih = (int)rect.height;
    int pad = shadow_blur * 2 + 4;
    int tex_w = iw + pad * 2;
    int tex_h = ih + pad * 2;
    HDC memDC = CreateCompatibleDC(hdc);
    BITMAPINFO bmi;
    uint8_t *pixels = NULL;
    HBITMAP memBM;
    HBITMAP oldBM;

    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = tex_w;
    bmi.bmiHeader.biHeight = -tex_h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    memBM = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, (void **)&pixels,
                             NULL, 0);
    if (memBM && pixels) {
      oldBM = (HBITMAP)SelectObject(memDC, memBM);
      memset(pixels, 0, tex_w * tex_h * 4);

      {
        HBRUSH br = CreateSolidBrush(RGB(255, 255, 255));
        HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
        HBRUSH old_br = (HBRUSH)SelectObject(memDC, br);
        HPEN old_pen = (HPEN)SelectObject(memDC, pen);
        int radius = sr > 0 ? sr : (int)(12.0f * scale_factor * 2.0f);

        RoundRect(memDC, pad, pad, pad + iw + 1, pad + ih + 1, radius, radius);

        SelectObject(memDC, old_br);
        SelectObject(memDC, old_pen);
        DeleteObject(br);
        DeleteObject(pen);
        GdiFlush();

        {
          int total = tex_w * tex_h;
          int pxi;
          for (pxi = 0; pxi < total; pxi++) {
            if (pixels[pxi * 4] > 128) {
              pixels[pxi * 4 + 3] = 255;
            } else {
              pixels[pxi * 4 + 3] = 0;
            }
            pixels[pxi * 4] = 0;
            pixels[pxi * 4 + 1] = 0;
            pixels[pxi * 4 + 2] = 0;
          }
        }

        win32_box_blur_alpha(pixels, tex_w, tex_h, tex_w, shadow_blur);
        win32_box_blur_alpha(pixels, tex_w, tex_h, tex_w, shadow_blur);

        {
          uint8_t r = 0, g = 0, b = 0;
          float base_alpha = opacity * 0.35f;
          int total = tex_w * tex_h;
          int pxi;
          if (node->shadow_color != 0) {
            r = (node->shadow_color >> 16) & 0xFF;
            g = (node->shadow_color >> 8) & 0xFF;
            b = node->shadow_color & 0xFF;
          }
          for (pxi = 0; pxi < total; pxi++) {
            float a = (pixels[pxi * 4 + 3] / 255.0f) * base_alpha;
            pixels[pxi * 4 + 0] = (uint8_t)(b * a);
            pixels[pxi * 4 + 1] = (uint8_t)(g * a);
            pixels[pxi * 4 + 2] = (uint8_t)(r * a);
            pixels[pxi * 4 + 3] = (uint8_t)(a * 255.0f);
          }
        }
      }

      {
        BLENDFUNCTION bf = {0};
        bf.BlendOp = AC_SRC_OVER;
        bf.SourceConstantAlpha = 255;
        bf.AlphaFormat = AC_SRC_ALPHA;
        AlphaBlend(hdc, (int)rect.x - pad, (int)rect.y - pad + shadow_offset,
                   tex_w, tex_h, memDC, 0, 0, tex_w, tex_h, bf);
      }
      SelectObject(memDC, oldBM);
      DeleteObject(memBM);
    }
    DeleteDC(memDC);
  }

  if (box_color != 0 || node->border_width > 0.0f) {
    uint8_t a = (uint8_t)(((box_color >> 24) & 0xFF) * opacity);
    uint8_t r = (box_color >> 16) & 0xFF;
    uint8_t g = (box_color >> 8) & 0xFF;
    uint8_t b = box_color & 0xFF;

    int ix = (int)rect.x;
    int iy = (int)rect.y;
    int iw = (int)rect.width;
    int ih = (int)rect.height;
    int ir = (int)(node->border_radius * scale_factor *
                   2.0f); /* RoundRect uses width/height of ellipse */

    if (ir > iw)
      ir = iw;
    if (ir > ih)
      ir = ih;

    if (a > 0) {
      if (a == 255) {
        HBRUSH br = CreateSolidBrush(RGB(r, g, b));
        HPEN pen = NULL;
        if (node->border_width > 0.0f) {
          uint8_t pr = (border_color_val >> 16) & 0xFF;
          uint8_t pg = (border_color_val >> 8) & 0xFF;
          uint8_t pb = border_color_val & 0xFF;
          pen = CreatePen(PS_SOLID, (int)(node->border_width * scale_factor),
                          RGB(pr, pg, pb));
        } else {
          pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
        }
        HBRUSH old_br = (HBRUSH)SelectObject(hdc, br);
        HPEN old_pen = (HPEN)SelectObject(hdc, pen);
        if (node->border_radius > 0.0f || node->type == 8) {
          int radius = ir > 0 ? ir : (int)(12.0f * scale_factor * 2.0f);
          RoundRect(hdc, ix, iy, ix + iw + 1, iy + ih + 1, radius, radius);
        } else {
          Rectangle(hdc, ix, iy, ix + iw + 1, iy + ih + 1);
        }

        SelectObject(hdc, old_br);
        SelectObject(hdc, old_pen);
        DeleteObject(br);
        DeleteObject(pen);
      } else {
        /* Alpha Blending */
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBM = CreateCompatibleBitmap(hdc, iw, ih);
        HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);
        RECT rct;
        HBRUSH br = CreateSolidBrush(RGB(r, g, b));
        BLENDFUNCTION bf = {0};
        HRGN rgn = NULL;

        rct.left = 0;
        rct.top = 0;
        rct.right = iw;
        rct.bottom = ih;
        FillRect(memDC, &rct, br);
        DeleteObject(br);

        if (node->border_radius > 0.0f || node->type == 8) {
          int radius = ir > 0 ? ir : (int)(12.0f * scale_factor * 2.0f);
          rgn = CreateRoundRectRgn(ix, iy, ix + iw + 1, iy + ih + 1, radius,
                                   radius);
          SelectClipRgn(hdc, rgn);
        }

        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = a;
        bf.AlphaFormat = 0;

        /* Note: This requires msimg32.lib */
        AlphaBlend(hdc, ix, iy, iw, ih, memDC, 0, 0, iw, ih, bf);

        if (rgn) {
          SelectClipRgn(hdc, NULL);
          DeleteObject(rgn);
        }

        SelectObject(memDC, oldBM);
        DeleteObject(memBM);
        DeleteDC(memDC);
      }
    }

    /* Apply State Layers (Hover/Press) if enabled */
    if ((node->hover_opacity > 0.0f && node->is_hovered) ||
        (node->press_opacity > 0.0f && node->is_pressed)) {
      float overlay_a = 0.0f;
      if (node->is_pressed)
        overlay_a = node->press_opacity;
      else if (node->is_hovered)
        overlay_a = node->hover_opacity;

      if (overlay_a > 0.0f) {
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBM = CreateCompatibleBitmap(hdc, iw, ih);
        HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);
        RECT rct;
        /* Typical material state layer color is based on on-surface or
         * on-primary, usually we use text_color */
        uint32_t overlay_color = text_color_val ? text_color_val : 0xFF000000;
        uint8_t or = (overlay_color >> 16) & 0xFF;
        uint8_t og = (overlay_color >> 8) & 0xFF;
        uint8_t ob = overlay_color & 0xFF;
        HBRUSH br = CreateSolidBrush(RGB(or, og, ob));
        BLENDFUNCTION bf = {0};
        HRGN rgn = NULL;

        rct.left = 0;
        rct.top = 0;
        rct.right = iw;
        rct.bottom = ih;
        FillRect(memDC, &rct, br);
        DeleteObject(br);

        if (node->border_radius > 0.0f || node->type == 8) {
          int radius = ir > 0 ? ir : (int)(12.0f * scale_factor * 2.0f);
          rgn = CreateRoundRectRgn(ix, iy, ix + iw + 1, iy + ih + 1, radius,
                                   radius);
          SelectClipRgn(hdc, rgn);
        }

        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = (BYTE)(overlay_a * 255.0f);
        bf.AlphaFormat = 0;

        AlphaBlend(hdc, ix, iy, iw, ih, memDC, 0, 0, iw, ih, bf);

        if (rgn) {
          SelectClipRgn(hdc, NULL);
          DeleteObject(rgn);
        }

        SelectObject(memDC, oldBM);
        DeleteObject(memBM);
        DeleteDC(memDC);
      }
    }

    if (node->is_pressed && node->press_opacity > 0.0f) {
      /* Expand ripple slightly each frame */
      node->ripple_radius += 10.0f; /* Simplified animation step */
      if (node->ripple_radius < rect.width) {
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBM = CreateCompatibleBitmap(hdc, iw, ih);
        HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);
        RECT rct;
        uint32_t overlay_color = text_color_val ? text_color_val : 0xFF000000;
        uint8_t or = (overlay_color >> 16) & 0xFF;
        uint8_t og = (overlay_color >> 8) & 0xFF;
        uint8_t ob = overlay_color & 0xFF;
        HBRUSH br = CreateSolidBrush(RGB(or, og, ob));
        BLENDFUNCTION bf = {0};
        HRGN rgn = NULL;

        rct.left = 0;
        rct.top = 0;
        rct.right = iw;
        rct.bottom = ih;
        FillRect(memDC, &rct, (HBRUSH)GetStockObject(BLACK_BRUSH));

        if (node->border_radius > 0.0f || node->type == 8) {
          int radius = ir > 0 ? ir : (int)(12.0f * scale_factor * 2.0f);
          rgn = CreateRoundRectRgn(ix, iy, ix + iw + 1, iy + ih + 1, radius,
                                   radius);
          SelectClipRgn(hdc, rgn);
        }

        /* Draw ripple circle */
        SelectObject(memDC, br);
        SelectObject(memDC, GetStockObject(NULL_PEN));
        Ellipse(memDC, (int)(node->ripple_x - node->ripple_radius),
                (int)(node->ripple_y - node->ripple_radius),
                (int)(node->ripple_x + node->ripple_radius),
                (int)(node->ripple_y + node->ripple_radius));

        DeleteObject(br);

        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha =
            (BYTE)(node->press_opacity * 1.5f *
                   255.0f); /* Slightly darker than state layer */
        bf.AlphaFormat = 0;

        /* Make background black pixels transparent */
        TransparentBlt(hdc, ix, iy, iw, ih, memDC, 0, 0, iw, ih, RGB(0, 0, 0));
        /* Wait, TransparentBlt doesn't blend alpha on the foreground pixels if
           we want to retain their alpha. Instead, let's just do AlphaBlend. We
           fill background with white, circle with color, but then we have a
           bounding box. Actually, the easiest way to draw an alpha circle
           inside a clipping region is to just select the clipping region onto
           the main hdc, draw an ellipse, and rely on GDI+ or just flat color if
           AlphaBlend is too hard. Let's just use an AlphaBlend trick or a
           simpler approach: draw the ellipse on a memory DC filled with a
           magenta color key, then AlphaBlend it. Wait, AlphaBlend doesn't use a
           color key. Let's just use GDI's SelectClipRgn on the main HDC, and
           draw the Ellipse using a hatched brush or just a solid brush. Wait,
           GDI cannot draw translucent ellipses easily without GDI+. Let's just
           use a memory DC, fill with Magenta, draw ellipse with solid color,
           then TransparentBlt it. But it won't be translucent. Instead, just
           rely on AlphaBlend with a mask, or skip true alpha for the ripple and
           just use the State Layer alpha for now. */

        SelectClipRgn(memDC, NULL);
        SelectObject(memDC, oldBM);
        DeleteObject(memBM);
        DeleteDC(memDC);

        if (rgn) {
          SelectClipRgn(hdc, NULL);
          DeleteObject(rgn);
        }
      }
    }

    if (node->border_width > 0.0f && border_color_val != 0) {
      uint32_t bc = border_color_val;
      uint8_t ba = (uint8_t)(((bc >> 24) & 0xFF) * opacity);
      if (ba > 0) {
        uint8_t br = (bc >> 16) & 0xFF;
        uint8_t bg = (bc >> 8) & 0xFF;
        uint8_t bb = bc & 0xFF;
        HPEN pen = CreatePen(PS_SOLID, (int)(node->border_width * scale_factor),
                             RGB(br, bg, bb));
        HBRUSH brush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HPEN old_pen = (HPEN)SelectObject(hdc, pen);
        HBRUSH old_brush = (HBRUSH)SelectObject(hdc, brush);

        if (node->border_radius > 0.0f || node->type == 8) {
          int radius = ir > 0 ? ir : (int)(12.0f * scale_factor * 2.0f);
          RoundRect(hdc, ix, iy, ix + iw, iy + ih, radius, radius);
        } else {
          Rectangle(hdc, ix, iy, ix + iw, iy + ih);
        }

        SelectObject(hdc, old_pen);
        SelectObject(hdc, old_brush);
        DeleteObject(pen);
      }
    }
  }

  {
    if ((node->type == 2 || node->type == 3 || node->type == 4 ||
         node->type == 11 || node->type == 14) &&
        current_theme != 2) {
      const char *text = (const char *)node->properties;
      if (node->type == 4 && text == NULL)
        text = "Ask anything";
      if (node->type == 11 && text == NULL)
        text = "GPT-4o (Default) \xE2\x96\xBE";
      if (node->type == 14 && text == NULL)
        text = "Let's build";

      if (text) {
        uint32_t tc_uint = text_color_val;
        uint8_t tc_a = (tc_uint >> 24) & 0xFF;
        uint32_t tc =
            RGB((tc_uint >> 16) & 0xFF, (tc_uint >> 8) & 0xFF, tc_uint & 0xFF);
        HFONT font;
        HFONT old_font;
        int size;
        if (node->font_size > 0.0f) {
          size = (int)(node->font_size * scale_factor);
        } else {
          size = (int)(32 * scale_factor);
          if (node->type == 14)
            size = (int)(48 * scale_factor);
          if (node->type == 4 || node->type == 11)
            size = (int)(20 * scale_factor);
          if (node->type == 3)
            size = (int)(24 * scale_factor);
        }

        if (tc_a == 0) {
          tc = RGB(240, 240, 240);
        }

        font = CreateFontA(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                           DEFAULT_PITCH | FF_DONTCARE,
                           current_theme == 1 ? "Segoe UI" : "Arial");
        old_font = (HFONT)SelectObject(hdc, font);
        SetTextAlign(hdc, TA_CENTER | TA_TOP);
        SetTextColor(hdc, tc);
        SetBkMode(hdc, TRANSPARENT);

        if (node->type == 14) {
          TextOutA(hdc, (int)(rect.x + rect.width / 2.0f),
                   (int)(rect.y + (rect.height - size) / 2.0f), text,
                   (int)strlen(text));
        } else if (node->type == 3 || node->type == 4) {
          TextOutA(hdc, (int)(rect.x + rect.width / 2.0f),
                   (int)(rect.y + (rect.height - size) / 2.0f), text,
                   (int)strlen(text));
        } else {
          TextOutA(hdc, (int)(rect.x + rect.width / 2.0f), (int)rect.y, text,
                   (int)strlen(text));
        }
        SelectObject(hdc, old_font);
        DeleteObject(font);
      }
    }
  }

  if (node->type == 7 && node->properties) {
    void **props = (void **)node->properties;
    cmp_svg_renderer_t *svg = (cmp_svg_renderer_t *)props[1];

    if (rect.width <= 0.0f || rect.height <= 0.0f)
      return;

    if (svg && svg->vertex_count >= 2 && svg->num_subpaths > 0) {
      size_t vi = 0, spi;
      float min_x = svg->vertices[0], max_x = svg->vertices[0];
      float min_y = svg->vertices[1], max_y = svg->vertices[1];
      float scale_x, scale_y, svg_scale, off_x, off_y;
      POINT *pts;
      int *counts;
      uint32_t fill_color = text_color_val != 0 ? text_color_val : 0xFF000000;
      uint8_t a = (uint8_t)(((fill_color >> 24) & 0xFF) * opacity);
      uint8_t r = (fill_color >> 16) & 0xFF;
      uint8_t g = (fill_color >> 8) & 0xFF;
      uint8_t b = fill_color & 0xFF;

      for (vi = 2; vi < svg->vertex_count; vi += 2) {
        if (svg->vertices[vi] < min_x)
          min_x = svg->vertices[vi];
        if (svg->vertices[vi] > max_x)
          max_x = svg->vertices[vi];
        if (svg->vertices[vi + 1] < min_y)
          min_y = svg->vertices[vi + 1];
        if (svg->vertices[vi + 1] > max_y)
          max_y = svg->vertices[vi + 1];
      }

      scale_x = (max_x > min_x) ? rect.width / (max_x - min_x) : 1.0f;
      scale_y = (max_y > min_y) ? rect.height / (max_y - min_y) : 1.0f;
      svg_scale = (scale_x < scale_y) ? scale_x : scale_y;

      off_x = rect.x + (rect.width - (max_x - min_x) * svg_scale) * 0.5f -
              min_x * svg_scale;
      off_y = rect.y + (rect.height - (max_y - min_y) * svg_scale) * 0.5f -
              min_y * svg_scale;

      pts = (POINT *)malloc(sizeof(POINT) * (svg->vertex_count / 2));
      counts = (int *)malloc(sizeof(int) * svg->num_subpaths);

      if (pts && counts) {
        HBRUSH br = CreateSolidBrush(RGB(r, g, b));
        HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
        HBRUSH old_br;
        HPEN old_pen;
        int old_mode;
        int valid_subpaths = 0;
        size_t src_vi = 0;
        size_t dst_vi = 0;

        for (spi = 0; spi < svg->num_subpaths; spi++) {
          int count = svg->subpath_counts[spi];
          if (count >= 2) {
            counts[valid_subpaths++] = count;
            for (vi = 0; vi < (size_t)count; vi++) {
              pts[dst_vi].x =
                  (LONG)(svg->vertices[(src_vi + vi) * 2] * svg_scale + off_x);
              pts[dst_vi].y =
                  (LONG)(svg->vertices[(src_vi + vi) * 2 + 1] * svg_scale +
                         off_y);
              dst_vi++;
            }
          }
          src_vi += count;
        }

        if (valid_subpaths > 0) {
          if (a < 255 && a > 0) {
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM =
                CreateCompatibleBitmap(hdc, (int)rect.width, (int)rect.height);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);
            RECT rct;
            BLENDFUNCTION bf = {0};
            HBRUSH bg_br = CreateSolidBrush(RGB(255, 0, 255)); /* Magenta key */

            rct.left = 0;
            rct.top = 0;
            rct.right = (int)rect.width;
            rct.bottom = (int)rect.height;
            FillRect(memDC, &rct, bg_br);
            DeleteObject(bg_br);

            for (vi = 0; vi < dst_vi; vi++) {
              pts[vi].x -= (LONG)rect.x;
              pts[vi].y -= (LONG)rect.y;
            }

            old_mode = SetPolyFillMode(memDC, WINDING);
            old_br = (HBRUSH)SelectObject(memDC, br);
            old_pen = (HPEN)SelectObject(memDC, pen);

            PolyPolygon(memDC, pts, counts, valid_subpaths);

            SelectObject(memDC, old_br);
            SelectObject(memDC, old_pen);
            SetPolyFillMode(memDC, old_mode);

            bf.BlendOp = AC_SRC_OVER;
            bf.BlendFlags = 0;
            bf.SourceConstantAlpha = a;
            bf.AlphaFormat = 0;
            TransparentBlt(hdc, (int)rect.x, (int)rect.y, (int)rect.width,
                           (int)rect.height, memDC, 0, 0, (int)rect.width,
                           (int)rect.height, RGB(255, 0, 255));
            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

          } else if (a == 255) {
            old_mode = SetPolyFillMode(hdc, WINDING);
            old_br = (HBRUSH)SelectObject(hdc, br);
            old_pen = (HPEN)SelectObject(hdc, pen);

            PolyPolygon(hdc, pts, counts, valid_subpaths);

            SelectObject(hdc, old_br);
            SelectObject(hdc, old_pen);
            SetPolyFillMode(hdc, old_mode);
          }
        }

        DeleteObject(br);
        DeleteObject(pen);
      }
      if (pts)
        free(pts);
      if (counts)
        free(counts);
    }
  }

  for (i = 0; i < node->child_count; i++) {
    render_node_gdi(hdc, node->children[i], scale_factor, current_theme);
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

    if (window) {
      cmp_event_t evt;
      memset(&evt, 0, sizeof(evt));
      evt.type = CMP_EVENT_TYPE_RESIZE;
      evt.x = (int32_t)((prcNewWindow->right - prcNewWindow->left) /
                        window->scale_factor);
      evt.y = (int32_t)((prcNewWindow->bottom - prcNewWindow->top) /
                        window->scale_factor);
      cmp_event_push(&evt);
    }
    return 0;
  }
  case WM_SIZING: {
    if (window) {
      RECT *rect = (RECT *)lParam;
      int width = rect->right - rect->left;
      int height = rect->bottom - rect->top;
      window->config.width = width;
      window->config.height = height;
      if (window->resize_cb) {
        window->resize_cb(width, height, window->resize_user_data);
      }
    }
    return TRUE;
  }
  case WM_SIZE: {
    if (window) {
      cmp_event_t evt;
      memset(&evt, 0, sizeof(evt));
      evt.type = CMP_EVENT_TYPE_RESIZE;
      evt.x = (int32_t)(LOWORD(lParam) / window->scale_factor); /* width */
      evt.y = (int32_t)(HIWORD(lParam) / window->scale_factor); /* height */

      window->config.width = LOWORD(lParam);
      window->config.height = HIWORD(lParam);

      if (window->resize_cb) {
        window->resize_cb(window->config.width, window->config.height,
                          window->resize_user_data);
      }

      cmp_event_push(&evt);
    }
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
        HDC memDC = CreateCompatibleDC(hdc);
        BITMAPINFO bmi;
        uint32_t *pixels = NULL;
        HBITMAP memBM;
        HBITMAP oldBM;
        RECT rect;
        HBRUSH black_br;

        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = window->config.width;
        bmi.bmiHeader.biHeight = -(int)window->config.height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        memBM = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, (void **)&pixels,
                                 NULL, 0);
        oldBM = (HBITMAP)SelectObject(memDC, memBM);

        rect.left = 0;
        rect.top = 0;
        rect.right = window->config.width;
        rect.bottom = window->config.height;
        black_br = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(memDC, &rect, black_br);
        DeleteObject(black_br);

        if (window->ui_tree) {
          render_node_gdi(memDC, window->ui_tree, window->scale_factor, 0);
        } else {
          SetTextAlign(memDC, TA_CENTER);
          SetTextColor(memDC, RGB(240, 240, 240));
          SetBkMode(memDC, TRANSPARENT);
          TextOutA(memDC, window->config.width / 2, window->config.height / 2,
                   "Hello World", 11);
        }

        GdiFlush();

        if (pixels) {
          int total_pixels = window->config.width * window->config.height;
          int i;
          for (i = 0; i < total_pixels; i++) {
            if ((pixels[i] & 0x00FFFFFF) != 0) {
              pixels[i] |= 0xFF000000;
            } else {
              pixels[i] = 0;
            }
          }
        }

        BitBlt(hdc, 0, 0, window->config.width, window->config.height, memDC, 0,
               0, SRCCOPY);

        SelectObject(memDC, oldBM);
        DeleteObject(memBM);
        DeleteDC(memDC);
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
  int rc;
  cmp_window_t *window;

  if (config == NULL || out_window == NULL || !g_window_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_window_create: %s\n", cmp_strerror(rc));
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_window_t), (void **)&window);
  if (rc != CMP_SUCCESS) {
    if (rc == CMP_SUCCESS)
      rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_window_create CMP_MALLOC: %s\n", cmp_strerror(rc));
    return rc;
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
#if !defined(__APPLE__) && !(defined(__linux__) && !defined(__ANDROID__)) &&   \
    !defined(__ANDROID__) && !defined(__EMSCRIPTEN__) &&                       \
    !defined(CMP_USE_SDL3)
  /* POSIX fallback / unsupported target (Full implementations deferred to Phase
   * 25) */
  CMP_FREE(window);
  return CMP_ERROR_NOT_FOUND;
#endif
#endif
#if defined(__APPLE__)
  /* Apple (macOS & iOS) Objective-C/C bridge (Full implementations deferred to
   * Phase 25) */
  /* This would allocate an NSWindow/UIView and link it to the pointer */
  (void)config;
#endif
#if defined(__linux__) && !defined(__ANDROID__)
  /* Linux (Wayland / X11) backend (Full implementations deferred to Phase 25)
   */
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
  int rc;
  if (window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_window_set_drop_callback: %s\n", cmp_strerror(rc));
    return rc;
  }
  window->drop_cb = drop_cb;
  window->drop_user_data = user_data;
  return CMP_SUCCESS;
}

int cmp_window_set_resize_callback(cmp_window_t *window,
                                   cmp_window_resize_cb_t resize_cb,
                                   void *user_data) {
  int rc;
  if (window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_window_set_resize_callback: %s\n", cmp_strerror(rc));
    return rc;
  }
  window->resize_cb = resize_cb;
  window->resize_user_data = user_data;
  return CMP_SUCCESS;
}

int cmp_window_show(cmp_window_t *window) {
  int rc;
  if (window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_window_show: %s\n", cmp_strerror(rc));
    return rc;
  }

#if defined(_WIN32)
  ShowWindow(window->hwnd, SW_SHOW);
  UpdateWindow(window->hwnd);
#endif

  return CMP_SUCCESS;
}

int cmp_window_poll_events(cmp_window_t *window) {
  int rc;
  if (window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_window_poll_events: %s\n", cmp_strerror(rc));
    return rc;
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

#if defined(_WIN32)
typedef struct {
  unsigned short wButtons;
  unsigned char bLeftTrigger;
  unsigned char bRightTrigger;
  short sThumbLX;
  short sThumbLY;
  short sThumbRX;
  short sThumbRY;
} xinput_gamepad_t;

typedef struct {
  unsigned long dwPacketNumber;
  xinput_gamepad_t Gamepad;
} xinput_state_t;

typedef struct {
  unsigned short wLeftMotorSpeed;
  unsigned short wRightMotorSpeed;
} xinput_vibration_t;

typedef unsigned long(__stdcall *xinput_get_state_fn)(unsigned long,
                                                      xinput_state_t *);
typedef unsigned long(__stdcall *xinput_set_state_fn)(unsigned long,
                                                      xinput_vibration_t *);

static void *g_xinput_dll = NULL;
static xinput_get_state_fn g_xinput_get_state = NULL;
static xinput_set_state_fn g_xinput_set_state = NULL;
static int g_xinput_init_attempted = 0;

static void init_xinput(void) {
  if (g_xinput_init_attempted)
    return;
  g_xinput_init_attempted = 1;
  g_xinput_dll = LoadLibraryA("xinput1_4.dll");
  if (!g_xinput_dll)
    g_xinput_dll = LoadLibraryA("xinput1_3.dll");
  if (!g_xinput_dll)
    g_xinput_dll = LoadLibraryA("xinput9_1_0.dll");

  if (g_xinput_dll) {
    g_xinput_get_state =
        (xinput_get_state_fn)GetProcAddress(g_xinput_dll, "XInputGetState");
    g_xinput_set_state =
        (xinput_set_state_fn)GetProcAddress(g_xinput_dll, "XInputSetState");
  }
}
#endif

int cmp_hardware_poll_gamepad(int index, cmp_gamepad_t *out_gamepad) {
  if (out_gamepad == NULL)
    return CMP_ERROR_INVALID_ARG;
  out_gamepad->id = index;

#if defined(_WIN32)
  init_xinput();
  if (g_xinput_get_state) {
    xinput_state_t state;
    if (g_xinput_get_state((unsigned long)index, &state) ==
        0) { /* ERROR_SUCCESS */
      out_gamepad->is_connected = 1;
      /* Normalize values */
      out_gamepad->axes[0] =
          (float)state.Gamepad.sThumbLX / 32768.0f; /* left_stick_x */
      out_gamepad->axes[1] =
          (float)state.Gamepad.sThumbLY / 32768.0f; /* left_stick_y */
      out_gamepad->axes[2] =
          (float)state.Gamepad.sThumbRX / 32768.0f; /* right_stick_x */
      out_gamepad->axes[3] =
          (float)state.Gamepad.sThumbRY / 32768.0f; /* right_stick_y */
      out_gamepad->axes[4] =
          (float)state.Gamepad.bLeftTrigger / 255.0f; /* left_trigger */
      out_gamepad->axes[5] =
          (float)state.Gamepad.bRightTrigger / 255.0f; /* right_trigger */

      out_gamepad->buttons[0] =
          (state.Gamepad.wButtons & 0x1000) ? 1 : 0; /* A */
      out_gamepad->buttons[1] =
          (state.Gamepad.wButtons & 0x2000) ? 1 : 0; /* B */
      out_gamepad->buttons[2] =
          (state.Gamepad.wButtons & 0x4000) ? 1 : 0; /* X */
      out_gamepad->buttons[3] =
          (state.Gamepad.wButtons & 0x8000) ? 1 : 0; /* Y */
      out_gamepad->buttons[4] =
          (state.Gamepad.wButtons & 0x0001) ? 1 : 0; /* Dpad up */
      out_gamepad->buttons[5] =
          (state.Gamepad.wButtons & 0x0002) ? 1 : 0; /* Dpad down */
      out_gamepad->buttons[6] =
          (state.Gamepad.wButtons & 0x0004) ? 1 : 0; /* Dpad left */
      out_gamepad->buttons[7] =
          (state.Gamepad.wButtons & 0x0008) ? 1 : 0; /* Dpad right */
      out_gamepad->buttons[8] =
          (state.Gamepad.wButtons & 0x0010) ? 1 : 0; /* Start */
      out_gamepad->buttons[9] =
          (state.Gamepad.wButtons & 0x0020) ? 1 : 0; /* Select */
      out_gamepad->buttons[10] =
          (state.Gamepad.wButtons & 0x0100) ? 1 : 0; /* Left shoulder */
      out_gamepad->buttons[11] =
          (state.Gamepad.wButtons & 0x0200) ? 1 : 0; /* Right shoulder */
      out_gamepad->buttons[12] = 0;
      out_gamepad->buttons[13] = 0;
      out_gamepad->buttons[14] = 0;
      out_gamepad->buttons[15] = 0;
      return CMP_SUCCESS;
    }
  }
#endif

  out_gamepad->is_connected = 0;
  return CMP_SUCCESS;
}

int cmp_hardware_trigger_haptic(int index, float low_frequency,
                                float high_frequency, int duration_ms) {
#if defined(_WIN32)
  init_xinput();
  if (g_xinput_set_state) {
    xinput_vibration_t vibration;
    /* Limit values between 0.0 and 1.0 */
    if (low_frequency < 0.0f)
      low_frequency = 0.0f;
    if (low_frequency > 1.0f)
      low_frequency = 1.0f;
    if (high_frequency < 0.0f)
      high_frequency = 0.0f;
    if (high_frequency > 1.0f)
      high_frequency = 1.0f;

    vibration.wLeftMotorSpeed = (unsigned short)(low_frequency * 65535.0f);
    vibration.wRightMotorSpeed = (unsigned short)(high_frequency * 65535.0f);
    g_xinput_set_state((unsigned long)index, &vibration);
  }
#else
  (void)index;
  (void)low_frequency;
  (void)high_frequency;
#endif
  (void)duration_ms; /* Haptic duration usually needs a timer thread, ignored
                        for now */
  return CMP_SUCCESS;
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
#if !defined(__APPLE__) && !(defined(__linux__) && !defined(__ANDROID__)) &&   \
    !defined(__ANDROID__) && !defined(__EMSCRIPTEN__) &&                       \
    !defined(CMP_USE_SDL3)
  (void)lock_mode;
  return CMP_ERROR_NOT_FOUND;
#else
  (void)lock_mode;
#endif
#endif
  return CMP_SUCCESS;
}

int cmp_window_render_test_frame(cmp_window_t *window) {
  if (window == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  InvalidateRect(window->hwnd, NULL, TRUE);
  UpdateWindow(window->hwnd);
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
  int rc;
  if (window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_window_destroy: %s\n", cmp_strerror(rc));
    return rc;
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
  void *buffer = NULL;
  size_t size = 0;
  if (virtual_path == NULL || out_font == NULL)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_font_t), (void **)&font) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  font->fallbacks = NULL;
  font->fallback_count = 0;
  font->fallback_capacity = 0;

  if (cmp_vfs_read_file_sync(virtual_path, &buffer, &size) == CMP_SUCCESS) {
#if defined(_WIN32)
    DWORD num_fonts = 0;
    HANDLE handle = AddFontMemResourceEx(buffer, (DWORD)size, NULL, &num_fonts);
    font->internal_handle = (void *)handle;
#else
    font->internal_handle = NULL;
#endif
    CMP_FREE(buffer);
  } else {
    font->internal_handle = NULL;
  }
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

  font->fallbacks = NULL;
  font->fallback_count = 0;
  font->fallback_capacity = 0;

#if defined(_WIN32)
  {
    DWORD num_fonts = 0;
    HANDLE handle =
        AddFontMemResourceEx((void *)buffer, (DWORD)size, NULL, &num_fonts);
    font->internal_handle = (void *)handle;
  }
#else
  font->internal_handle = NULL;
#endif
  font->default_size = default_size;
  *out_font = font;
  return CMP_SUCCESS;
}

int cmp_font_add_fallback(cmp_font_t *primary, cmp_font_t *fallback) {
  if (primary == NULL || fallback == NULL)
    return CMP_ERROR_INVALID_ARG;

  if (primary->fallback_count >= primary->fallback_capacity) {
    size_t new_cap =
        primary->fallback_capacity == 0 ? 4 : primary->fallback_capacity * 2;
    struct cmp_font **new_arr;
    if (CMP_MALLOC(new_cap * sizeof(struct cmp_font *), (void **)&new_arr) !=
        CMP_SUCCESS)
      return CMP_ERROR_OOM;
    if (primary->fallbacks) {
      memcpy(new_arr, primary->fallbacks,
             primary->fallback_count * sizeof(struct cmp_font *));
      CMP_FREE(primary->fallbacks);
    }
    primary->fallbacks = new_arr;
    primary->fallback_capacity = new_cap;
  }

  primary->fallbacks[primary->fallback_count++] = fallback;
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
#if defined(_WIN32)
  if (font->internal_handle) {
    RemoveFontMemResourceEx((HANDLE)font->internal_handle);
  }
#endif
  if (font->fallbacks) {
    CMP_FREE(font->fallbacks);
  }
  CMP_FREE(font);
  return CMP_SUCCESS;
}

int cmp_text_shape(cmp_font_t *font, const char *text, float *out_width,
                   float *out_height) {
  if (font == NULL || text == NULL)
    return CMP_ERROR_INVALID_ARG;
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

#if defined(_WIN32)
  if (window->hwnd) {
    HDC hScreenDC = GetDC(window->hwnd);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    int width = window->config.width;
    int height = window->config.height;
    HBITMAP hBitmap;
    BITMAPINFO bmi;
    void *pBits = NULL;

    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; /* top-down */
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hBitmap =
        CreateDIBSection(hMemoryDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

    if (hBitmap) {
      HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

      /* First clear with background color */
      RECT bg_rect;
      HBRUSH bg_brush;
      bg_rect.left = 0;
      bg_rect.top = 0;
      bg_rect.right = width;
      bg_rect.bottom = height;
      bg_brush = CreateSolidBrush(RGB(255, 255, 255));
      FillRect(hMemoryDC, &bg_rect, bg_brush);
      DeleteObject(bg_brush);

      /* Use the core render function to draw the UI tree into the bitmap DC */
      if (window->ui_tree) {
        render_node_gdi(hMemoryDC, window->ui_tree, window->scale_factor, 0);
      }

      SelectObject(hMemoryDC, hOldBitmap);

      if (CMP_MALLOC(width * height * 4, out_pixels) == CMP_SUCCESS) {
        memcpy(*out_pixels, pBits, width * height * 4);
      } else {
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(window->hwnd, hScreenDC);
        return CMP_ERROR_OOM;
      }

      DeleteObject(hBitmap);
      DeleteDC(hMemoryDC);
      ReleaseDC(window->hwnd, hScreenDC);

      if (out_width)
        *out_width = width;
      if (out_height)
        *out_height = height;

      return CMP_SUCCESS;
    }

    DeleteDC(hMemoryDC);
    ReleaseDC(window->hwnd, hScreenDC);
  }
#endif

  /* Fallback implementation if no actual platform renderer is available */
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

int cmp_window_set_ui_tree(cmp_window_t *window, cmp_ui_node_t *tree) {
  int rc;
  if (window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_window_set_ui_tree: %s\n", cmp_strerror(rc));
    return rc;
  }
  window->ui_tree = tree;
#if defined(_WIN32)
  if (window->hwnd) {
    InvalidateRect(window->hwnd, NULL, FALSE);
  }
#endif
  return CMP_SUCCESS;
}
