/* clang-format off */
#include "ui_avatar_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_avatar_base {
  char *name;
  char *image_url;
  char initials[9]; /* Max 2 UTF-8 chars (4 bytes each) + null terminator */
  struct ui_icon_base *fallback_icon;
  struct ui_signal *src_signal;
};

static enum ui_error get_utf8_len(unsigned char c, int *out_len) {
  if ((c & 0x80) == 0)
    *out_len = 1;
  else if ((c & 0xE0) == 0xC0)
    *out_len = 2;
  else if ((c & 0xF0) == 0xE0)
    *out_len = 3;
  else if ((c & 0xF8) == 0xF0)
    *out_len = 4;
  else
    *out_len = 1; /* Fallback */
  return UI_ERROR_NONE;
}

static enum ui_error extract_initials(const char *name, char *initials_out) {
  int i = 0;
  int first_initial_idx = -1;
  int last_initial_idx = -1;
  int in_word = 0;

  initials_out[0] = '\0';

  while (name[i] != '\0') {
    if (name[i] == ' ' || name[i] == '\t' || name[i] == '\n' ||
        name[i] == '\r') {
      in_word = 0;
      i++;
    } else {
      if (!in_word) {
        if (first_initial_idx == -1) {
          first_initial_idx = i;
        } else {
          last_initial_idx = i;
        }
        in_word = 1;
      }
      i++;
    }
  }

  if (first_initial_idx != -1) {
    int out_idx = 0;
    int l = 1;
    int j;
    (void)get_utf8_len((unsigned char)name[first_initial_idx], &l);
    for (j = 0; j < l && name[first_initial_idx + j] != '\0'; j++) {
      initials_out[out_idx++] = name[first_initial_idx + j];
    }

    if (last_initial_idx != -1) {
      (void)get_utf8_len((unsigned char)name[last_initial_idx], &l);
      for (j = 0; j < l && name[last_initial_idx + j] != '\0'; j++) {
        initials_out[out_idx++] = name[last_initial_idx + j];
      }
    }
    initials_out[out_idx] = '\0';
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_avatar_base_create(struct ui_avatar_base **out_avatar) {
  struct ui_avatar_base *avatar;
  enum ui_error rc = UI_ERROR_NONE;

  if (!out_avatar) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  avatar = (struct ui_avatar_base *)UI_MALLOC(sizeof(struct ui_avatar_base));
  if (!avatar) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  avatar->name = NULL;
  avatar->image_url = NULL;
  avatar->initials[0] = '\0';
  avatar->fallback_icon = NULL;

  *out_avatar = avatar;

cleanup:
  return rc;
}

void ui_avatar_base_destroy(struct ui_avatar_base *avatar) {
  if (!avatar) {
    return;
  }
  UI_FREE(avatar->name);
  UI_FREE(avatar->image_url);
  if (avatar->fallback_icon) {
    ui_icon_base_destroy(avatar->fallback_icon);
  }
  UI_FREE(avatar);
}

enum ui_error ui_avatar_base_set_name(struct ui_avatar_base *avatar,
                                      const char *name) {
  enum ui_error rc = UI_ERROR_NONE;
  size_t len;
  char *new_name = NULL;

  if (!avatar || !name) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  len = strlen(name);
  new_name = (char *)UI_MALLOC(len + 1);
  if (!new_name) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  if (UI_STRCPY(new_name, len + 1, name) != 0) {
    UI_FREE(new_name);
    rc = UI_ERROR_UNKNOWN;
    goto cleanup;
  }

  UI_FREE(avatar->name);

  avatar->name = new_name;
  (void)extract_initials(avatar->name, avatar->initials);

cleanup:
  return rc;
}

enum ui_error ui_avatar_base_get_name(const struct ui_avatar_base *avatar,
                                      const char **out_name) {
  if (!avatar || !out_name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_name = avatar->name;
  return UI_ERROR_NONE;
}

enum ui_error ui_avatar_base_get_initials(const struct ui_avatar_base *avatar,
                                          const char **out_initials) {
  if (!avatar || !out_initials) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_initials = avatar->initials;
  return UI_ERROR_NONE;
}

enum ui_error ui_avatar_base_set_image_url(struct ui_avatar_base *avatar,
                                           const char *image_url) {
  enum ui_error rc = UI_ERROR_NONE;
  size_t len;
  char *new_url = NULL;

  if (!avatar || !image_url) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  len = strlen(image_url);
  new_url = (char *)UI_MALLOC(len + 1);
  if (!new_url) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  if (UI_STRCPY(new_url, len + 1, image_url) != 0) {
    UI_FREE(new_url);
    rc = UI_ERROR_UNKNOWN;
    goto cleanup;
  }

  UI_FREE(avatar->image_url);

  avatar->image_url = new_url;

cleanup:
  return rc;
}

enum ui_error ui_avatar_base_get_image_url(const struct ui_avatar_base *avatar,
                                           const char **out_image_url) {
  if (!avatar || !out_image_url) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_image_url = avatar->image_url;
  return UI_ERROR_NONE;
}

enum ui_error ui_avatar_base_set_fallback_icon(struct ui_avatar_base *avatar,
                                               struct ui_icon_base *icon) {
  if (!avatar || !icon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (avatar->fallback_icon) {
    ui_icon_base_destroy(avatar->fallback_icon);
  }

  avatar->fallback_icon = icon;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_avatar_base_get_fallback_icon(const struct ui_avatar_base *avatar,
                                 struct ui_icon_base **out_icon) {
  if (!avatar || !out_icon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_icon = avatar->fallback_icon;
  return UI_ERROR_NONE;
}

enum ui_error ui_avatar_base_get_type(const struct ui_avatar_base *avatar,
                                      enum ui_avatar_type *out_type) {
  if (!avatar || !out_type) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (avatar->image_url) {
    *out_type = UI_AVATAR_TYPE_IMAGE;
  } else if (avatar->initials[0] != '\0') {
    *out_type = UI_AVATAR_TYPE_INITIALS;
  } else {
    *out_type = UI_AVATAR_TYPE_FALLBACK_ICON;
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_avatar_base_bind_src(struct ui_avatar_base *widget,
                                      struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->src_signal = signal;
  return UI_ERROR_NONE;
}
