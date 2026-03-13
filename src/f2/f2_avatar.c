/* clang-format off */
#include "f2/f2_avatar.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL f2_avatar_style_init(F2AvatarStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  style->size = F2_AVATAR_DEFAULT_SIZE;
  style->background_color = cmp_color_rgba(0.0f, 0.36f, 0.77f, 1.0f); /* Blue */
  style->text_color = cmp_color_rgba(1.0f, 1.0f, 1.0f, 1.0f);
  style->is_circular = CMP_TRUE;
  style->corner_radius = 4.0f; /* Used if not circular */

  style->text_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->text_style.size_px = 12;
  style->text_style.weight = 400;
  style->text_style.color = style->text_color;

  f2_presence_badge_style_init(&style->badge_style);

  return CMP_OK;
}

CMP_API int CMP_CALL f2_avatar_init_initials(F2Avatar *avatar,
                                             const F2AvatarStyle *style,
                                             const char *utf8_initials,
                                             cmp_usize initials_len) {
  if (!avatar || !style)
    return CMP_ERR_INVALID_ARGUMENT;
  memset(avatar, 0, sizeof(*avatar));
  avatar->style = *style;
  avatar->utf8_initials = utf8_initials;
  avatar->initials_len = initials_len;
  avatar->image_handle.id = 0; /* Invalid/null handle */
  avatar->has_badge = CMP_FALSE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_avatar_init_image(F2Avatar *avatar,
                                          const F2AvatarStyle *style,
                                          CMPHandle image_handle) {
  if (!avatar || !style)
    return CMP_ERR_INVALID_ARGUMENT;
  memset(avatar, 0, sizeof(*avatar));
  avatar->style = *style;
  avatar->image_handle = image_handle;
  avatar->has_badge = CMP_FALSE;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_avatar_set_presence(F2Avatar *avatar,
                                            F2PresenceState state,
                                            CMPBool show) {
  int res;
  if (!avatar)
    return CMP_ERR_INVALID_ARGUMENT;

  avatar->has_badge = show;
  if (show) {
    res = f2_presence_badge_init(&avatar->badge, &avatar->style.badge_style,
                                 state);
    if (res != CMP_OK)
      return res;
  }

  return CMP_OK;
}
