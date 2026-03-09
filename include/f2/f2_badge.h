#ifndef F2_BADGE_H
#define F2_BADGE_H

/**
 * @file f2_badge.h
 * @brief Microsoft Fluent 2 Badge and PresenceBadge widgets.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "f2/f2_color.h"

/** @brief Default dot badge diameter. */
#define F2_BADGE_DEFAULT_DOT_DIAMETER 8.0f
/** @brief Default labeled badge minimum width. */
#define F2_BADGE_DEFAULT_MIN_WIDTH 16.0f
/** @brief Default labeled badge height. */
#define F2_BADGE_DEFAULT_HEIGHT 16.0f
/** @brief Default badge horizontal padding. */
#define F2_BADGE_DEFAULT_PADDING_X 4.0f

/** @brief Small presence badge size. */
#define F2_PRESENCE_BADGE_SIZE_SMALL 10.0f
/** @brief Medium presence badge size. */
#define F2_PRESENCE_BADGE_SIZE_MEDIUM 12.0f
/** @brief Large presence badge size. */
#define F2_PRESENCE_BADGE_SIZE_LARGE 16.0f

/** @brief Presence badge states. */
typedef enum F2PresenceState {
  F2_PRESENCE_AVAILABLE = 0,
  F2_PRESENCE_AWAY,
  F2_PRESENCE_BUSY,
  F2_PRESENCE_DO_NOT_DISTURB,
  F2_PRESENCE_OFFLINE,
  F2_PRESENCE_OUT_OF_OFFICE,
  F2_PRESENCE_UNKNOWN
} F2PresenceState;

/**
 * @brief Fluent 2 Badge style descriptor.
 */
typedef struct F2BadgeStyle {
  CMPColor background_color; /**< Badge fill color. */
  CMPColor text_color;       /**< Text color. */
  CMPTextStyle text_style;   /**< Text style for labeled badges. */
  CMPScalar dot_diameter;    /**< Diameter for a dot badge. */
  CMPScalar min_width;       /**< Min width for a labeled badge. */
  CMPScalar height;          /**< Height for a labeled badge. */
  CMPScalar padding_x;       /**< Horizontal padding. */
} F2BadgeStyle;

/**
 * @brief Fluent 2 Badge widget.
 */
typedef struct F2Badge {
  CMPWidget widget;       /**< Widget interface. */
  F2BadgeStyle style;     /**< Current style. */
  const char *utf8_label; /**< UTF-8 label text (NULL for dot). */
  cmp_usize utf8_len;     /**< Label text length. */
  CMPRect bounds;         /**< Layout bounds. */
} F2Badge;

/**
 * @brief Initialize a default Fluent 2 badge style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_badge_style_init(F2BadgeStyle *style);

/**
 * @brief Initialize a Fluent 2 badge widget.
 * @param badge Badge instance.
 * @param style Style descriptor.
 * @param utf8_label UTF-8 label text (NULL for a dot badge).
 * @param utf8_len Length of the label.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_badge_init(F2Badge *badge, const F2BadgeStyle *style,
                                   const char *utf8_label, cmp_usize utf8_len);

/**
 * @brief Update the badge label.
 * @param badge Badge instance.
 * @param utf8_label UTF-8 label text.
 * @param utf8_len Length of the label.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_badge_set_label(F2Badge *badge, const char *utf8_label,
                                        cmp_usize utf8_len);

/**
 * @brief Fluent 2 Presence Badge style descriptor.
 */
typedef struct F2PresenceBadgeStyle {
  CMPScalar size;           /**< Outer diameter of presence badge. */
  CMPScalar outline_width;  /**< Outline cut-out width around the badge. */
  CMPColor available_color; /**< Color for available state. */
  CMPColor away_color;      /**< Color for away state. */
  CMPColor busy_color;      /**< Color for busy/DND state. */
  CMPColor offline_color;   /**< Color for offline/OOF state. */
  CMPColor outline_color;   /**< Background cut-out color. */
} F2PresenceBadgeStyle;

/**
 * @brief Fluent 2 Presence Badge widget.
 */
typedef struct F2PresenceBadge {
  CMPWidget widget;           /**< Widget interface. */
  F2PresenceBadgeStyle style; /**< Current style. */
  F2PresenceState state;      /**< Presence state. */
  CMPRect bounds;             /**< Layout bounds. */
} F2PresenceBadge;

/**
 * @brief Initialize a default Fluent 2 presence badge style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_presence_badge_style_init(F2PresenceBadgeStyle *style);

/**
 * @brief Initialize a Fluent 2 presence badge widget.
 * @param badge Presence badge instance.
 * @param style Style descriptor.
 * @param state Initial presence state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_presence_badge_init(F2PresenceBadge *badge,
                                            const F2PresenceBadgeStyle *style,
                                            F2PresenceState state);

/**
 * @brief Update the presence badge state.
 * @param badge Presence badge instance.
 * @param state New state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_presence_badge_set_state(F2PresenceBadge *badge,
                                                 F2PresenceState state);

#ifdef __cplusplus
}
#endif

#endif /* F2_BADGE_H */
