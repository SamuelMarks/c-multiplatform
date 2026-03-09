#ifndef F2_AVATAR_H
#define F2_AVATAR_H

/**
 * @file f2_avatar.h
 * @brief Microsoft Fluent 2 Avatar widget.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "f2/f2_badge.h"
#include "f2/f2_color.h"

/** @brief Default avatar size. */
#define F2_AVATAR_DEFAULT_SIZE 32.0f

struct F2Avatar;

/**
 * @brief Fluent 2 Avatar style descriptor.
 */
typedef struct F2AvatarStyle {
  CMPScalar size;            /**< Diameter or width/height of the avatar. */
  CMPColor background_color; /**< Background color for initials. */
  CMPColor text_color;       /**< Text color for initials. */
  CMPTextStyle text_style;   /**< Typography for initials. */
  CMPBool is_circular;       /**< True for circular, false for rounded rect. */
  CMPScalar corner_radius;   /**< Corner radius if not circular. */
  F2PresenceBadgeStyle badge_style; /**< Style for attached presence badge. */
} F2AvatarStyle;

/**
 * @brief Fluent 2 Avatar widget.
 */
typedef struct F2Avatar {
  CMPWidget widget;          /**< Widget interface. */
  F2AvatarStyle style;       /**< Current style. */
  const char *utf8_initials; /**< UTF-8 initials text (up to 2 chars). */
  cmp_usize initials_len;    /**< Length of initials text. */
  CMPHandle image_handle;    /**< Optional image texture handle. */
  CMPRect bounds;            /**< Layout bounds. */
  F2PresenceBadge badge;     /**< Attached presence badge instance. */
  CMPBool has_badge;         /**< True if the badge should be rendered. */
} F2Avatar;

/**
 * @brief Initialize a default Fluent 2 avatar style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_avatar_style_init(F2AvatarStyle *style);

/**
 * @brief Initialize a Fluent 2 avatar widget with initials.
 * @param avatar Avatar instance.
 * @param style Style descriptor.
 * @param utf8_initials Text initials (e.g., "AB").
 * @param initials_len Length of initials.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_avatar_init_initials(F2Avatar *avatar,
                                             const F2AvatarStyle *style,
                                             const char *utf8_initials,
                                             cmp_usize initials_len);

/**
 * @brief Initialize a Fluent 2 avatar widget with an image.
 * @param avatar Avatar instance.
 * @param style Style descriptor.
 * @param image_handle Texture handle for the image.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_avatar_init_image(F2Avatar *avatar,
                                          const F2AvatarStyle *style,
                                          CMPHandle image_handle);

/**
 * @brief Set or update the presence badge state on the avatar.
 * @param avatar Avatar instance.
 * @param state Presence state.
 * @param show True to show the badge, false to hide.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_avatar_set_presence(F2Avatar *avatar,
                                            F2PresenceState state,
                                            CMPBool show);

#ifdef __cplusplus
}
#endif

#endif /* F2_AVATAR_H */
