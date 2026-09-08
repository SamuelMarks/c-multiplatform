/**
 * @file ui_web_bridge.c
 * @brief ui_web_bridge.c implementation.
 */
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

/** @brief Global command buffer */
static ui_uint32 *g_cmd_buffer = NULL;
/** @brief Global command position */
static size_t g_cmd_pos = 0;
/** @brief Global command capacity */
static size_t g_cmd_capacity = WEB_CMD_BUFFER_SIZE / sizeof(ui_uint32);

/**
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
EM_JS(void, js_create_node, (ui_uint32 id, const char *t_ptr), {
  const g = typeof window != = 'undefined' ? window : globalThis;
  if (!g.uiNodeMap)
    g.uiNodeMap = new Map();
  if (typeof document == = 'undefined') {
    g.uiNodeMap.set(id, {});
    return;
  }
  const t = UTF8ToString(t_ptr);
  const isSvg =
      (t == 'svg' || t == 'path' || t == 'g' || t == 'circle' || t == 'rect');
  const el = isSvg ? document.createElementNS('http://www.w3.org/2000/svg', t)
                   : document.createElement(t);
  g.uiNodeMap.set(id, el);
})

EM_JS(void, js_destroy_node, (ui_uint32 id), {
  const g = typeof window != = 'undefined' ? window : globalThis;
  if (g.uiNodeMap) {
    const el = g.uiNodeMap.get(id);
    if (el) {
      if (el.remove)
        el.remove();
      g.uiNodeMap.delete(id);
    }
  }
})

EM_JS(void, js_set_text, (ui_uint32 id, const char *text_ptr), {
  const g = typeof window != = 'undefined' ? window : globalThis;
  if (g.uiNodeMap) {
    const el = g.uiNodeMap.get(id);
    if (el)
      el.textContent = UTF8ToString(text_ptr);
  }
})

EM_JS(void, js_append_child, (ui_uint32 parent_id, ui_uint32 child_id), {
  const g = typeof window != = 'undefined' ? window : globalThis;
  if (g.uiNodeMap &&typeof document != = 'undefined') {
    const parentEl = parent_id == 0 ? document.getElementById('app-root')
                                    : g.uiNodeMap.get(parent_id);
    const childEl = g.uiNodeMap.get(child_id);
    if (parentEl && childEl && parentEl.appendChild)
      parentEl.appendChild(childEl);
  }
})

EM_JS(void, js_remove_child, (ui_uint32 parent_id, ui_uint32 child_id), {
  const g = typeof window != = 'undefined' ? window : globalThis;
  if (g.uiNodeMap &&typeof document != = 'undefined') {
    const parentEl = parent_id == 0 ? document.getElementById('app-root')
                                    : g.uiNodeMap.get(parent_id);
    const childEl = g.uiNodeMap.get(child_id);
    if (parentEl && childEl && childEl.parentNode == parentEl &&
        parentEl.removeChild) {
      parentEl.removeChild(childEl);
    }
  }
})

EM_JS(void, js_set_bounds, (ui_uint32 id, float x, float y, float w, float h), {
  const g = typeof window != = 'undefined' ? window : globalThis;
  if (g.uiNodeMap) {
    const el = g.uiNodeMap.get(id);
    if (el && el.style) {
      el.style.left = x + 'px';
      el.style.top = y + 'px';
      el.style.width = w + 'px';
      el.style.height = h + 'px';
    }
  }
})

EM_JS(void, js_set_style,
      (ui_uint32 id, const char *prop_ptr, const char *val_ptr), {
        const g = typeof window != = 'undefined' ? window : globalThis;
        if (g.uiNodeMap) {
          const el = g.uiNodeMap.get(id);
          if (el && el.style && el.style.setProperty)
            el.style.setProperty(UTF8ToString(prop_ptr), UTF8ToString(val_ptr));
        }
      })

EM_JS(void, js_set_aria_role_label,
      (ui_uint32 id, const char *role, const char *label), {
        const g = typeof window != = 'undefined' ? window : globalThis;
        const el = g.uiNodeMap ? g.uiNodeMap.get(id) : null;
        if (!el || !el.setAttribute)
          return;
        if (role)
          el.setAttribute('role', UTF8ToString(role));
        if (label)
          el.setAttribute('aria-label', UTF8ToString(label));
      })

EM_JS(void, js_set_aria_state, (ui_uint32 id, int h, int d, int exp, int chk), {
  const g = typeof window != = 'undefined' ? window : globalThis;
  const el = g.uiNodeMap ? g.uiNodeMap.get(id) : null;
  if (!el || !el.setAttribute)
    return;
  if (h)
    el.setAttribute('aria-hidden', 'true');
  if (d)
    el.setAttribute('aria-disabled', 'true');
  if (exp != -1)
    el.setAttribute('aria-expanded', exp ? 'true' : 'false');
  if (chk != -1)
    el.setAttribute('aria-checked',
                    chk == 2 ? 'mixed' : (chk ? 'true' : 'false'));
})

EM_JS(void, js_push_state, (const char *path_ptr), {
  if (typeof window != = 'undefined' && window.history)
    window.history.pushState({}, "", UTF8ToString(path_ptr));
})

EM_JS(void, js_replace_state, (const char *path_ptr), {
  if (typeof window != = 'undefined' && window.history)
    window.history.replaceState({}, "", UTF8ToString(path_ptr));
})

EM_JS(void, js_set_attribute,
      (ui_uint32 id, const char *name_ptr, const char *val_ptr), {
        const g = typeof window != = 'undefined' ? window : globalThis;
        if (g.uiNodeMap) {
          const el = g.uiNodeMap.get(id);
          if (el) {
            if (val_ptr == 0 && el.removeAttribute) {
              el.removeAttribute(UTF8ToString(name_ptr));
            } else if (el.setAttribute) {
              el.setAttribute(UTF8ToString(name_ptr), UTF8ToString(val_ptr));
            }
          }
        }
      })

EM_JS(void, js_set_property,
      (ui_uint32 id, const char *name_ptr, const char *val_ptr), {
        const g = typeof window != = 'undefined' ? window : globalThis;
        if (g.uiNodeMap) {
          const el = g.uiNodeMap.get(id);
          if (el) {
            const name = UTF8ToString(name_ptr);
            const valStr = val_ptr ? UTF8ToString(val_ptr) : "";
            if (valStr == "true")
              el[name] = true;
            else if (valStr == "false")
              el[name] = false;
            else
              el[name] = valStr;
          }
        }
      })

static void flush_to_js(ui_uint32 *buf, ui_uint32 len) {
  ui_uint32 pos = 0;
  float *float_buf = (float *)buf;
  ui_uint32 opcode, id, parent_id, child_id;
  const char *str_ptr, *prop_ptr, *val_ptr, *role_ptr, *label_ptr, *name_ptr;
  float x, y, w, h;
  int hidden, disabled, expanded, checked;

  while (pos < len) {
    opcode = buf[pos++];
    if (opcode == 1) {
      id = buf[pos++];
      str_ptr = (const char *)(ui_uintptr)buf[pos++];
      js_create_node(id, str_ptr);
    } else if (opcode == 2) {
      id = buf[pos++];
      js_destroy_node(id);
    } else if (opcode == 3) {
      id = buf[pos++];
      str_ptr = (const char *)(ui_uintptr)buf[pos++];
      js_set_text(id, str_ptr);
    } else if (opcode == 4) {
      parent_id = buf[pos++];
      child_id = buf[pos++];
      js_append_child(parent_id, child_id);
    } else if (opcode == 6) {
      parent_id = buf[pos++];
      child_id = buf[pos++];
      js_remove_child(parent_id, child_id);
    } else if (opcode == 7) {
      id = buf[pos++];
      x = float_buf[pos++];
      y = float_buf[pos++];
      w = float_buf[pos++];
      h = float_buf[pos++];
      js_set_bounds(id, x, y, w, h);
    } else if (opcode == 8) {
      id = buf[pos++];
      prop_ptr = (const char *)(ui_uintptr)buf[pos++];
      val_ptr = (const char *)(ui_uintptr)buf[pos++];
      js_set_style(id, prop_ptr, val_ptr);
    } else if (opcode == 9) {
      id = buf[pos++];
      role_ptr = (const char *)(ui_uintptr)buf[pos++];
      label_ptr = (const char *)(ui_uintptr)buf[pos++];
      hidden = (int)buf[pos++];
      disabled = (int)buf[pos++];
      expanded = (int)buf[pos++];
      checked = (int)buf[pos++];
      js_set_aria_role_label(id, role_ptr, label_ptr);
      js_set_aria_state(id, hidden, disabled, expanded, checked);
    } else if (opcode == 10) {
      pos++; /* unused id */
      str_ptr = (const char *)(ui_uintptr)buf[pos++];
      js_push_state(str_ptr);
    } else if (opcode == 11) {
      pos++; /* unused id */
      str_ptr = (const char *)(ui_uintptr)buf[pos++];
      js_replace_state(str_ptr);
    } else if (opcode == 12) {
      id = buf[pos++];
      name_ptr = (const char *)(ui_uintptr)buf[pos++];
      val_ptr = (const char *)(ui_uintptr)buf[pos++];
      js_set_attribute(id, name_ptr, val_ptr);
    } else if (opcode == 13) {
      id = buf[pos++];
      name_ptr = (const char *)(ui_uintptr)buf[pos++];
      val_ptr = (const char *)(ui_uintptr)buf[pos++];
      js_set_property(id, name_ptr, val_ptr);
    } else {
      break;
    }
  }
}
#endif

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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
#if defined(_MSC_VER)
      strncpy_s(event.event_data.deep_link.uri,
                sizeof(event.event_data.deep_link.uri), uri_ptr, _TRUNCATE);
#else
      /* Use strncpy since uri is a char[1024] array */
      strncpy(event.event_data.deep_link.uri, uri_ptr,
              sizeof(event.event_data.deep_link.uri) - 1);
      event.event_data.deep_link
          .uri[sizeof(event.event_data.deep_link.uri) - 1] = '\0';
#endif
    }
  }

  return UI_ERROR_NONE;
}

/**
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

/**
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
  if (key) {
    event.event_data.keyboard.key_code = key[0];
  } else {
    event.event_data.keyboard.key_code = 0;
  }
  event.event_data.keyboard.modifiers = (unsigned int)modifiers;

  return UI_ERROR_NONE;
}

#ifdef __cplusplus
}
#endif

/**
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

/**
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

/**
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
/**
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
/**
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
