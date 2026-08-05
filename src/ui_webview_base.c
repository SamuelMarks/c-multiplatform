/* clang-format off */
#include "ui_webview_base.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

struct ui_webview_base {
  struct ui_component *component;
  struct ui_signal *url_signal;
  ui_webview_ipc_callback ipc_callback;
  void *ipc_user_data;
  char *current_url;
  char *current_html;
};

ui_error_t ui_webview_base_create(struct ui_webview_base **out_webview) {
  struct ui_webview_base *webview;
  struct ui_dom_node *root_node = NULL;
  ui_error_t rc;

  if (!out_webview) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  webview = (struct ui_webview_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_webview_base));
  if (!webview) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(webview, 0, sizeof(struct ui_webview_base));

  rc = ui_component_create(&webview->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(webview);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    (void)ui_component_destroy(webview->component);
    C_MULTIPLATFORM_FREE(webview);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(root_node, "iframe");
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(root_node);
    (void)ui_component_destroy(webview->component);
    C_MULTIPLATFORM_FREE(webview);
    return rc;
  }
  rc = ui_dom_node_set_attribute(root_node, "role", "application");
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(root_node);
    (void)ui_component_destroy(webview->component);
    C_MULTIPLATFORM_FREE(webview);
    return rc;
  }
  webview->component->shadow_root = root_node;

  *out_webview = webview;
  return UI_ERROR_NONE;
}

ui_error_t ui_webview_base_destroy(struct ui_webview_base *webview) {
  if (!webview) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  (void)ui_component_destroy(webview->component);
  if (webview->current_url) {
    C_MULTIPLATFORM_FREE(webview->current_url);
  }
  if (webview->current_html) {
    C_MULTIPLATFORM_FREE(webview->current_html);
  }
  C_MULTIPLATFORM_FREE(webview);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_webview_base_get_component(struct ui_webview_base *webview,
                                         struct ui_component **out_component) {
  if (!webview || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = webview->component;
  return UI_ERROR_NONE;
}

static ui_error_t duplicate_string(const char *str, char **out_copy) {
  size_t len;
  char *dup;
  *out_copy = NULL;
  len = strlen(str);
  dup = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!dup)
    return UI_ERROR_OUT_OF_MEMORY;
  memcpy(dup, str, len);
  dup[len] = '\0';
  *out_copy = dup;
  return UI_ERROR_NONE;
}

ui_error_t ui_webview_base_set_url(struct ui_webview_base *webview,
                                   const char *url) {
  if (!webview) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (webview->current_url) {
    C_MULTIPLATFORM_FREE(webview->current_url);
    webview->current_url = NULL;
  }
  if (url) {
    char *tmp = NULL;
    ui_error_t err = duplicate_string(url, &tmp);
    if (err != UI_ERROR_NONE)
      return err;
    webview->current_url = tmp;
  }
  /* Update the iframe src attribute */
  if (url) {
    return ui_dom_node_set_attribute(webview->component->shadow_root, "src",
                                     url);
  } else {
    return ui_dom_node_remove_attribute(webview->component->shadow_root, "src");
  }
}

ui_error_t ui_webview_base_set_html(struct ui_webview_base *webview,
                                    const char *html) {
  if (!webview) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (webview->current_html) {
    C_MULTIPLATFORM_FREE(webview->current_html);
    webview->current_html = NULL;
  }
  if (html) {
    char *tmp = NULL;
    ui_error_t err = duplicate_string(html, &tmp);
    if (err != UI_ERROR_NONE)
      return err;
    webview->current_html = tmp;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_webview_base_bind_url(struct ui_webview_base *webview,
                                    struct ui_signal *signal) {
  if (!webview) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  webview->url_signal = signal;
  return UI_ERROR_NONE;
}

ui_error_t ui_webview_base_evaluate_js(struct ui_webview_base *webview,
                                       const char *script) {
  if (!webview || !script) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* In the CDK unstyled base, this is a no-op or simulated bridge. */
  return UI_ERROR_NONE;
}

ui_error_t ui_webview_base_set_ipc_callback(struct ui_webview_base *webview,
                                            ui_webview_ipc_callback callback,
                                            void *user_data) {
  if (!webview) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  webview->ipc_callback = callback;
  webview->ipc_user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_webview_base_dispatch_ipc_message(struct ui_webview_base *webview,
                                                const char *message) {
  if (!webview || !message) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (webview->ipc_callback) {
    ui_error_t rc =
        webview->ipc_callback(webview, message, webview->ipc_user_data);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}
