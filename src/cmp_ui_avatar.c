/* clang-format off */
#include "cmp_ui_avatar.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_avatar {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_text;
  char *initials;
};

/**
 * @brief cmp_ui_avatar_create
 *
 * @param out_avatar Parameter description.
 * @param initials Parameter description.
 * @param bg_color Parameter description.
 * @param text_color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_avatar_create(cmp_ui_avatar_t **out_avatar, const char *initials,
                         uint32_t bg_color, uint32_t text_color) {
  cmp_ui_avatar_t *avatar;
  int err;
  size_t len;

  if (!out_avatar) {
    LOG_DEBUG("cmp_ui_avatar_create: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_avatar_t), (void **)&avatar);
  if (err != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_avatar_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  avatar->initials = NULL;
  if (initials) {
    len = strlen(initials);
    err = CMP_MALLOC(len + 1, (void **)&avatar->initials);
    if (err == CMP_SUCCESS) {
#if defined(_MSC_VER)
      if (memcpy_s(avatar->initials, len + 1, initials, len + 1) != 0) {
        LOG_DEBUG("cmp_ui_avatar_create: memcpy_s failed\n");
        CMP_FREE(avatar->initials);
        CMP_FREE(avatar);
        return CMP_ERROR_GENERAL;
      }
#else
      memcpy(avatar->initials, initials, len + 1);
#endif
    } else {
      LOG_DEBUG("cmp_ui_avatar_create: OOM initials\n");
      CMP_FREE(avatar);
      return CMP_ERROR_OOM;
    }
  }

  err = cmp_ui_box_create(&avatar->node_root);
  if (err != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_avatar_create: cmp_ui_box_create failed\n");
    if (avatar->initials)
      CMP_FREE(avatar->initials);
    CMP_FREE(avatar);
    return err;
  }

  avatar->node_root->bg_color = bg_color;

  err = cmp_ui_text_create(&avatar->node_text,
                           avatar->initials ? avatar->initials : "", -1);
  if (err != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_avatar_create: cmp_ui_text_create failed\n");
    err = cmp_ui_node_destroy(avatar->node_root);
    if (err != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_avatar_create: cmp_ui_node_destroy failed\n");
    if (avatar->initials)
      CMP_FREE(avatar->initials);
    CMP_FREE(avatar);
    return err;
  }

  avatar->node_text->text_color = text_color;

  err = cmp_ui_node_add_child(avatar->node_root, avatar->node_text);
  if (err != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_avatar_create: cmp_ui_node_add_child failed\n");
  }

  *out_avatar = avatar;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_avatar_destroy
 *
 * @param avatar Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_avatar_destroy(cmp_ui_avatar_t *avatar) {
  int rc;
  if (!avatar) {
    LOG_DEBUG("cmp_ui_avatar_destroy: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (avatar->initials) {
    rc = CMP_FREE(avatar->initials);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_avatar_destroy: CMP_FREE initials failed\n");
  }
  if (avatar->node_root) {
    rc = cmp_ui_node_destroy(avatar->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_avatar_destroy: cmp_ui_node_destroy failed\n");
  }
  rc = CMP_FREE(avatar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_avatar_destroy: CMP_FREE failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_avatar_get_node
 *
 * @param avatar Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_avatar_get_node(cmp_ui_avatar_t *avatar, cmp_ui_node_t **out_node) {
  if (!avatar || !out_node) {
    LOG_DEBUG("cmp_ui_avatar_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = avatar->node_root;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_avatar_set_initials
 *
 * @param avatar Parameter description.
 * @param initials Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_avatar_set_initials(cmp_ui_avatar_t *avatar, const char *initials) {
  size_t len;
  int rc;

  if (!avatar) {
    LOG_DEBUG("cmp_ui_avatar_set_initials: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (avatar->initials) {
    rc = CMP_FREE(avatar->initials);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_avatar_set_initials: CMP_FREE initials failed\n");
    avatar->initials = NULL;
  }

  if (initials) {
    len = strlen(initials);
    rc = CMP_MALLOC(len + 1, (void **)&avatar->initials);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_avatar_set_initials: OOM\n");
      return CMP_ERROR_OOM;
    }
#if defined(_MSC_VER)
    if (memcpy_s(avatar->initials, len + 1, initials, len + 1) != 0) {
      LOG_DEBUG("cmp_ui_avatar_set_initials: memcpy_s failed\n");
      CMP_FREE(avatar->initials);
      avatar->initials = NULL;
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(avatar->initials, initials, len + 1);
#endif
  }

  return CMP_SUCCESS;
}