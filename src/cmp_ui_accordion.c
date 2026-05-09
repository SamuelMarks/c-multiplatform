/* clang-format off */
#include "cmp_ui_accordion.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Accordion widget.
 */
struct cmp_ui_accordion {
  /** @brief The root box node of the accordion */
  cmp_ui_node_t *node_root;
  /** @brief The text node containing the accordion title */
  cmp_ui_node_t *node_title;
  /** @brief The raw string of the title */
  char *title;
  /** @brief Indicates if the accordion is expanded (1) or collapsed (0) */
  int is_expanded;
};

/**
 * @brief cmp_ui_accordion_create
 *
 * @param out_accordion Pointer to output the newly created accordion.
 * @param title The title text for the accordion.
 * @param bg_color The background color.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_accordion_create(cmp_ui_accordion_t **out_accordion,
                            const char *title, uint32_t bg_color) {
  cmp_ui_accordion_t *accordion;
  int rc = CMP_SUCCESS;
  size_t len;
  cmp_string_t translated = {NULL, 0, 0};
  const char *final_title;

  if (!out_accordion) {
    LOG_DEBUG("cmp_ui_accordion_create: out_accordion is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_accordion_t), (void **)&accordion);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_accordion_create: OOM\n");

    return rc;
  }

  memset(accordion, 0, sizeof(cmp_ui_accordion_t));

  final_title = title;

  if (title) {
    rc = cmp_i18n_translate(title, &translated);
    if (rc == CMP_SUCCESS && translated.data) {
      final_title = translated.data;
    } else if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_accordion_create: cmp_i18n_translate failed\n");
    }
  }

  accordion->title = NULL;
  if (final_title) {
    len = strlen(final_title);
    rc = CMP_MALLOC(len + 1, (void **)&accordion->title);
    if (rc == CMP_SUCCESS) {
#if defined(_MSC_VER)
      if (memcpy_s(accordion->title, len + 1, final_title, len + 1) != 0) {
        LOG_DEBUG("cmp_ui_accordion_create: memcpy_s failed\n");
        rc = CMP_FREE(accordion->title);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_accordion_create: CMP_FREE failed\n");
        }
        rc = CMP_FREE(accordion);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_accordion_create: CMP_FREE failed\n");
        }
        if (translated.data) {
          rc = cmp_string_destroy(&translated);
          if (rc != CMP_SUCCESS) {
            LOG_DEBUG("cmp_ui_accordion_create: cmp_string_destroy failed\n");
          }
        }
        return CMP_ERROR_GENERAL;
      }
#else
      memcpy(accordion->title, final_title, len + 1);
#endif
    } else {
      LOG_DEBUG("cmp_ui_accordion_create: OOM title\n");
      rc = CMP_FREE(accordion);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_accordion_create: CMP_FREE failed\n");
      }
      if (translated.data) {
        rc = cmp_string_destroy(&translated);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_ui_accordion_create: cmp_string_destroy failed\n");
        }
      }
      return CMP_ERROR_OOM;
    }
  }

  if (translated.data) {
    rc = cmp_string_destroy(&translated);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_accordion_create: cmp_string_destroy failed\n");
    }
  }

  accordion->is_expanded = 0;

  rc = cmp_ui_box_create(&accordion->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_accordion_create: cmp_ui_box_create failed\n");
    if (accordion->title) {
      rc = CMP_FREE(accordion->title);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_accordion_create: CMP_FREE title failed\n");
      }
    }
    rc = CMP_FREE(accordion);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_accordion_create: CMP_FREE failed\n");
    }

    return rc;
  }

  accordion->node_root->bg_color = bg_color;

  rc = cmp_ui_text_create(&accordion->node_title,
                          accordion->title ? accordion->title : "", -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_accordion_create: cmp_ui_text_create failed\n");
    rc = cmp_ui_node_destroy(accordion->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_accordion_create: cmp_ui_node_destroy failed\n");
    }
    if (accordion->title) {
      rc = CMP_FREE(accordion->title);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_ui_accordion_create: CMP_FREE title failed\n");
      }
    }
    rc = CMP_FREE(accordion);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_accordion_create: CMP_FREE failed\n");
    }

    return rc;
  }

  rc = cmp_ui_node_add_child(accordion->node_root, accordion->node_title);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_accordion_create: cmp_ui_node_add_child failed\n");
    /* We handle it, but continue */
  }

  *out_accordion = accordion;
  return rc;
}

/**
 * @brief cmp_ui_accordion_destroy
 *
 * @param accordion The accordion to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_accordion_destroy(cmp_ui_accordion_t *accordion) {
  int rc = CMP_SUCCESS;

  if (!accordion) {
    LOG_DEBUG("cmp_ui_accordion_destroy: accordion is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (accordion->title) {
    rc = CMP_FREE(accordion->title);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_accordion_destroy: CMP_FREE title failed\n");
    }
  }
  if (accordion->node_root) {
    rc = cmp_ui_node_destroy(accordion->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_accordion_destroy: cmp_ui_node_destroy failed\n");
    }
  }

  rc = CMP_FREE(accordion);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_accordion_destroy: CMP_FREE failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ui_accordion_get_node
 *
 * @param accordion The accordion.
 * @param out_node Pointer to receive the node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_accordion_get_node(cmp_ui_accordion_t *accordion,
                              cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!accordion || !out_node) {
    LOG_DEBUG("cmp_ui_accordion_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = accordion->node_root;

  return rc;
}

/**
 * @brief cmp_ui_accordion_set_expanded
 *
 * @param accordion The accordion.
 * @param is_expanded 1 for expanded, 0 for collapsed.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_accordion_set_expanded(cmp_ui_accordion_t *accordion,
                                  int is_expanded) {
  int rc = CMP_SUCCESS;
  if (!accordion) {
    LOG_DEBUG("cmp_ui_accordion_set_expanded: accordion is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }
  accordion->is_expanded = is_expanded;

  return rc;
}
