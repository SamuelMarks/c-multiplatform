/* clang-format off */
#include "../include/ui_web_bridge.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/ui_event.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
/** @brief internal */
#define WEB_CMD_BUFFER_SIZE 32
#else
/** @brief internal */
#define WEB_CMD_BUFFER_SIZE (1024 * 1024 * 4) /* 4MB */
#endif

static ui_uint32 *g_cmd_buffer = NULL;
static size_t g_cmd_pos = 0;
static size_t g_cmd_capacity = WEB_CMD_BUFFER_SIZE / sizeof(ui_uint32);

/*
 * @brief ensure_buffer.
 * @param words_needed Parameter words_needed.
 * @return Return value.
 */
static ui_error_t ensure_buffer(size_t words_needed) {
  if (!g_cmd_buffer) {
    g_cmd_buffer = (ui_uint32 *)C_MULTIPLATFORM_MALLOC((size_t)g_cmd_capacity *
                                                       sizeof(ui_uint32));
    if (!g_cmd_buffer) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    g_cmd_pos = 0;
  }

  if (g_cmd_pos + words_needed > g_cmd_capacity) {
    (void)ui_web_bridge_flush();
  }
  return UI_ERROR_NONE;
}

#if defined(__EMSCRIPTEN__)
/* EM_JS is the C89 safe way to do this in emscripten */
EM_JS(void, flush_to_js, (ui_uint32 * buf, ui_uint32 len), {
  const memBuf = new Uint32Array(wasmMemory.buffer, buf, len);
  const floatBuf = new Float32Array(wasmMemory.buffer, buf, len);
  let pos = 0;

  while (pos < len) {
    const opcode = memBuf[pos++];
    if (opcode == 1) {
      const id = memBuf[pos++];
      const strPtr = memBuf[pos++];
      const tag = UTF8ToString(strPtr);
      if (!window.uiNodeMap)
        window.uiNodeMap = new Map();
      let el;
      if (tag == 'svg' || tag == 'path' || tag == 'g' || tag == 'circle' ||
          tag == 'rect') {
        el = document.createElementNS('http://www.w3.org/2000/svg', tag);
      } else {
        el = document.createElement(tag);
      }
      window.uiNodeMap.set(id, el);
    } else if (opcode == 2) {
      const id = memBuf[pos++];
      if (window.uiNodeMap) {
        const el = window.uiNodeMap.get(id);
        if (el) {
          el.remove();
          window.uiNodeMap.delete(id);
        }
      }
    } else if (opcode == 3) {
      const id = memBuf[pos++];
      const strPtr = memBuf[pos++];
      const text = UTF8ToString(strPtr);
      if (window.uiNodeMap) {
        const el = window.uiNodeMap.get(id);
        if (el) {
          el.textContent = text;
        }
      }
    } else if (opcode == 4) {
      const parentId = memBuf[pos++];
      const childId = memBuf[pos++];
      if (window.uiNodeMap) {
        const parentEl = parentId == 0 ? document.getElementById('app-root')
                                       : window.uiNodeMap.get(parentId);
        const childEl = window.uiNodeMap.get(childId);
        if (parentEl && childEl)
          parentEl.appendChild(childEl);
      }
    } else if (opcode == 6) {
      const parentId = memBuf[pos++];
      const childId = memBuf[pos++];
      if (window.uiNodeMap) {
        const parentEl = parentId == 0 ? document.getElementById('app-root')
                                       : window.uiNodeMap.get(parentId);
        const childEl = window.uiNodeMap.get(childId);
        if (parentEl && childEl && childEl.parentNode == parentEl) {
          parentEl.removeChild(childEl);
        }
      }
    } else if (opcode == 7) {
      const id = memBuf[pos++];
      const x = floatBuf[pos++];
      const y = floatBuf[pos++];
      const w = floatBuf[pos++];
      const h = floatBuf[pos++];
      if (window.uiNodeMap) {
        const el = window.uiNodeMap.get(id);
        if (el) {
          el.style.left = x + 'px';
          el.style.top = y + 'px';
          el.style.width = w + 'px';
          el.style.height = h + 'px';
        }
      }
    } else if (opcode == 8) {
      const id = memBuf[pos++];
      const propPtr = memBuf[pos++];
      const valPtr = memBuf[pos++];
      const prop = UTF8ToString(propPtr);
      const val = UTF8ToString(valPtr);
      if (window.uiNodeMap) {
        const el = window.uiNodeMap.get(id);
        if (el)
          el.style.setProperty(prop, val);
      }
    } else if (opcode == 9) {
      const id = memBuf[pos++];
      const rolePtr = memBuf[pos++];
      const labelPtr = memBuf[pos++];
      const hidden = memBuf[pos++];
      const disabled = memBuf[pos++];
      const expanded = memBuf[pos++];
      const checked = memBuf[pos++];
      if (window.uiNodeMap) {
        const el = window.uiNodeMap.get(id);
        if (el) {
          if (rolePtr)
            el.setAttribute('role', UTF8ToString(rolePtr));
          if (labelPtr)
            el.setAttribute('aria-label', UTF8ToString(labelPtr));
          if (hidden)
            el.setAttribute('aria-hidden', 'true');
          if (disabled)
            el.setAttribute('aria-disabled', 'true');
          if (expanded != 0xFFFFFFFF)
            el.setAttribute('aria-expanded', expanded ? 'true' : 'false');
          if (checked != 0xFFFFFFFF)
            el.setAttribute('aria-checked', checked == 2
                                                ? 'mixed'
                                                : (checked ? 'true' : 'false'));
        }
      }
    } else if (opcode == 10) {
      const id = memBuf[pos++]; /* unused id */
      const strPtr = memBuf[pos++];
      const path = UTF8ToString(strPtr);
      window.history.pushState({}, "", path);
    } else if (opcode == 11) {
      const id = memBuf[pos++]; /* unused id */
      const strPtr = memBuf[pos++];
      const path = UTF8ToString(strPtr);
      window.history.replaceState({}, "", path);
    } else if (opcode == 12) {
      const id = memBuf[pos++];
      const namePtr = memBuf[pos++];
      const valPtr = memBuf[pos++];
      const name = UTF8ToString(namePtr);
      if (window.uiNodeMap) {
        const el = window.uiNodeMap.get(id);
        if (el) {
          if (valPtr == 0) {
            el.removeAttribute(name);
          } else {
            el.setAttribute(name, UTF8ToString(valPtr));
          }
        }
      }
    } else if (opcode == 13) {
      const id = memBuf[pos++];
      const namePtr = memBuf[pos++];
      const valPtr = memBuf[pos++];
      const name = UTF8ToString(namePtr);
      const valStr = valPtr ? UTF8ToString(valPtr) : "";
      if (window.uiNodeMap) {
        const el = window.uiNodeMap.get(id);
        if (el) {
          if (valStr == "true")
            el[name] = true;
          else if (valStr == "false")
            el[name] = false;
          else
            el[name] = valStr;
        }
      }
    } else {
      console.error("Unknown Wasm bridge opcode:", opcode);
      break;
    }
  }
})
#endif

/*
 * @brief ui_web_bridge_shutdown.
 * @return Return value.
 */
ui_error_t ui_web_bridge_shutdown(void) {
  if (g_cmd_buffer) {
    C_MULTIPLATFORM_FREE(g_cmd_buffer);
    g_cmd_buffer = NULL;
  }
  g_cmd_pos = 0;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_flush.
 * @return Return value.
 */
ui_error_t ui_web_bridge_flush(void) {
#if defined(__EMSCRIPTEN__)
  if (g_cmd_pos > 0 && g_cmd_buffer) {
    flush_to_js(g_cmd_buffer, (ui_uint32)g_cmd_pos);
  }
#endif
  g_cmd_pos = 0;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_create_node.
 * @param id Parameter id.
 * @param tag_name Parameter tag_name.
 * @return Return value.
 */
ui_error_t ui_web_bridge_create_node(ui_uint32 id, const char *tag_name) {
  ui_error_t rc = ensure_buffer(3);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = CMD_CREATE_NODE;
  g_cmd_buffer[g_cmd_pos++] = id;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)tag_name;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_destroy_node.
 * @param id Parameter id.
 * @return Return value.
 */
ui_error_t ui_web_bridge_destroy_node(ui_uint32 id) {
  ui_error_t rc = ensure_buffer(2);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = CMD_DESTROY_NODE;
  g_cmd_buffer[g_cmd_pos++] = id;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_set_text.
 * @param id Parameter id.
 * @param text Parameter text.
 * @return Return value.
 */
ui_error_t ui_web_bridge_set_text(ui_uint32 id, const char *text) {
  ui_error_t rc = ensure_buffer(3);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = CMD_SET_TEXT;
  g_cmd_buffer[g_cmd_pos++] = id;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)text;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_append_child.
 * @param parent_id Parameter parent_id.
 * @param child_id Parameter child_id.
 * @return Return value.
 */
ui_error_t ui_web_bridge_append_child(ui_uint32 parent_id, ui_uint32 child_id) {
  ui_error_t rc = ensure_buffer(3);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = CMD_APPEND_CHILD;
  g_cmd_buffer[g_cmd_pos++] = parent_id;
  g_cmd_buffer[g_cmd_pos++] = child_id;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_remove_child.
 * @param parent_id Parameter parent_id.
 * @param child_id Parameter child_id.
 * @return Return value.
 */
ui_error_t ui_web_bridge_remove_child(ui_uint32 parent_id, ui_uint32 child_id) {
  ui_error_t rc = ensure_buffer(3);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = CMD_REMOVE_CHILD;
  g_cmd_buffer[g_cmd_pos++] = parent_id;
  g_cmd_buffer[g_cmd_pos++] = child_id;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_set_bounds.
 * @param id Parameter id.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param w Parameter w.
 * @param h Parameter h.
 * @return Return value.
 */
ui_error_t ui_web_bridge_set_bounds(ui_uint32 id, float x, float y, float w,
                                    float h) {
  ui_error_t rc = ensure_buffer(6);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = CMD_SET_BOUNDS;
  g_cmd_buffer[g_cmd_pos++] = id;

  memcpy(&g_cmd_buffer[g_cmd_pos], &x, sizeof(float));
  g_cmd_pos++;
  memcpy(&g_cmd_buffer[g_cmd_pos], &y, sizeof(float));
  g_cmd_pos++;
  memcpy(&g_cmd_buffer[g_cmd_pos], &w, sizeof(float));
  g_cmd_pos++;
  memcpy(&g_cmd_buffer[g_cmd_pos], &h, sizeof(float));
  g_cmd_pos++;

  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_insert_before.
 * @param parent_id Parameter parent_id.
 * @param child_id Parameter child_id.
 * @param reference_id Parameter reference_id.
 * @return Return value.
 */
ui_error_t ui_web_bridge_insert_before(ui_uint32 parent_id, ui_uint32 child_id,
                                       ui_uint32 reference_id) {
  ui_error_t rc = ensure_buffer(4);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = CMD_INSERT_BEFORE;
  g_cmd_buffer[g_cmd_pos++] = parent_id;
  g_cmd_buffer[g_cmd_pos++] = child_id;
  g_cmd_buffer[g_cmd_pos++] = reference_id;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_set_style.
 * @param id Parameter id.
 * @param property Parameter property.
 * @param value Parameter value.
 * @return Return value.
 */
ui_error_t ui_web_bridge_set_style(ui_uint32 id, const char *property,
                                   const char *value) {
  ui_error_t rc = ensure_buffer(4);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = 8;
  g_cmd_buffer[g_cmd_pos++] = id;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)property;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)value;
  return UI_ERROR_NONE;
}

/* Phase 5 implementations (JS -> C event translation) */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief ui_web_bridge_dispatch_event.
 * @param type Parameter type.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param buttons Parameter buttons.
 * @return Return value.
 */
ui_error_t ui_web_bridge_dispatch_event(int type, float x, float y,
                                        int buttons) {
  struct ui_event event;
  memset(&event, 0, sizeof(event));

  if (type == 1)
    event.type = UI_EVENT_MOUSE_DOWN;
  else if (type == 2)
    event.type = UI_EVENT_MOUSE_UP;
  else if (type == 3)
    event.type = UI_EVENT_MOUSE_MOVE;
  else if (type == 5)
    event.type = UI_EVENT_MOUSE_WHEEL;
  else if (type == 30)
    event.type = UI_EVENT_OS_DEEP_LINK;

  if (type >= 1 && type <= 4) {
    event.event_data.mouse.x = (int)x;
    event.event_data.mouse.y = (int)y;
    event.event_data.mouse.button = buttons;
  } else if (type == 5) {
    event.event_data.mouse.wheel_x = x;
    event.event_data.mouse.wheel_y = y;
  } else if (type == 30) {
    const char *uri_ptr = (const char *)(ui_uintptr)x;
    if (uri_ptr) {
      /* Use strncpy since uri is a char[1024] array */
      strncpy(event.event_data.deep_link.uri, uri_ptr,
              sizeof(event.event_data.deep_link.uri) - 1);
      event.event_data.deep_link
          .uri[sizeof(event.event_data.deep_link.uri) - 1] = '\0';
    }
  }

  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_dispatch_resize.
 * @param w Parameter w.
 * @param h Parameter h.
 * @param dpr Parameter dpr.
 * @return Return value.
 */
ui_error_t ui_web_bridge_dispatch_resize(float w, float h, float dpr) {
  struct ui_event event;
  memset(&event, 0, sizeof(event));
  event.type = UI_EVENT_WINDOW_RESIZE;
  event.event_data.window.width = (int)w;
  event.event_data.window.height = (int)h;
  (void)dpr;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_dispatch_key.
 * @param type Parameter type.
 * @param key Parameter key.
 * @param modifiers Parameter modifiers.
 * @return Return value.
 */
ui_error_t ui_web_bridge_dispatch_key(int type, const char *key,
                                      int modifiers) {
  struct ui_event event;
  memset(&event, 0, sizeof(event));

  if (type == 10)
    event.type = UI_EVENT_KEY_DOWN;
  else
    event.type = UI_EVENT_KEY_UP;

  /* Naive mapping for demo */
  event.event_data.keyboard.key_code = key[0];
  event.event_data.keyboard.modifiers = (unsigned int)modifiers;

  return UI_ERROR_NONE;
}

#ifdef __cplusplus
}
#endif

/*
 * @brief ui_web_bridge_set_aria.
 * @param id Parameter id.
 * @param role Parameter role.
 * @param label Parameter label.
 * @param hidden Parameter hidden.
 * @param disabled Parameter disabled.
 * @param expanded Parameter expanded.
 * @param checked Parameter checked.
 * @return Return value.
 */
ui_error_t ui_web_bridge_set_aria(ui_uint32 id, const char *role,
                                  const char *label, int hidden, int disabled,
                                  int expanded, int checked) {
  ui_error_t rc = ensure_buffer(8);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = 9;
  g_cmd_buffer[g_cmd_pos++] = id;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)role;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)label;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)hidden;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)disabled;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)expanded;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)checked;

  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_push_state.
 * @param path Parameter path.
 * @return Return value.
 */
ui_error_t ui_web_bridge_push_state(const char *path) {
  ui_error_t rc = ensure_buffer(3);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = 10;
  g_cmd_buffer[g_cmd_pos++] = 0; /* unused id slot for alignment */
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)path;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_web_bridge_replace_state.
 * @param path Parameter path.
 * @return Return value.
 */
ui_error_t ui_web_bridge_replace_state(const char *path) {
  ui_error_t rc = ensure_buffer(3);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = 11;
  g_cmd_buffer[g_cmd_pos++] = 0; /* unused id slot for alignment */
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)path;
  return UI_ERROR_NONE;
}
/*
 * @brief ui_web_bridge_set_attribute.
 * @param id Parameter id.
 * @param name Parameter name.
 * @param value Parameter value.
 * @return Return value.
 */
ui_error_t ui_web_bridge_set_attribute(ui_uint32 id, const char *name,
                                       const char *value) {
  ui_error_t rc = ensure_buffer(4);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = CMD_SET_ATTRIBUTE;
  g_cmd_buffer[g_cmd_pos++] = id;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)name;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)value;
  return UI_ERROR_NONE;
}
/*
 * @brief ui_web_bridge_set_property.
 * @param id Parameter id.
 * @param name Parameter name.
 * @param value Parameter value.
 * @return Return value.
 */
ui_error_t ui_web_bridge_set_property(ui_uint32 id, const char *name,
                                      const char *value) {
  ui_error_t rc = ensure_buffer(4);
  if (rc != UI_ERROR_NONE)
    return rc;

  g_cmd_buffer[g_cmd_pos++] = CMD_SET_PROPERTY;
  g_cmd_buffer[g_cmd_pos++] = id;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)name;
  g_cmd_buffer[g_cmd_pos++] = (ui_uint32)(ui_uintptr)value;
  return UI_ERROR_NONE;
}
