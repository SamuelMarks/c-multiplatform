#include "f2/f2_badge.h"
#include "cmpc/cmp_core.h"
#include <string.h>

CMP_API int CMP_CALL f2_badge_style_init(F2BadgeStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));
  style->background_color =
      (CMPColor){0.8f, 0.0f, 0.0f, 1.0f}; /* Fluent default red for alerts */
  style->text_color = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};

  style->text_style.utf8_family =
      "Segoe UI Variable, system-ui, -apple-system, Roboto, Ubuntu";
  style->text_style.size_px = 10;
  style->text_style.weight = 600;
  style->text_style.color = style->text_color;

  style->dot_diameter = F2_BADGE_DEFAULT_DOT_DIAMETER;
  style->min_width = F2_BADGE_DEFAULT_MIN_WIDTH;
  style->height = F2_BADGE_DEFAULT_HEIGHT;
  style->padding_x = F2_BADGE_DEFAULT_PADDING_X;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_badge_init(F2Badge *badge, const F2BadgeStyle *style,
                                   const char *utf8_label, cmp_usize utf8_len) {
  if (!badge || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(badge, 0, sizeof(*badge));
  badge->style = *style;
  badge->utf8_label = utf8_label;
  badge->utf8_len = utf8_len;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_badge_set_label(F2Badge *badge, const char *utf8_label,
                                        cmp_usize utf8_len) {
  if (!badge)
    return CMP_ERR_INVALID_ARGUMENT;
  badge->utf8_label = utf8_label;
  badge->utf8_len = utf8_len;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_presence_badge_style_init(F2PresenceBadgeStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));
  style->size = F2_PRESENCE_BADGE_SIZE_MEDIUM;
  style->outline_width = 2.0f;

  style->available_color = (CMPColor){0.0f, 0.8f, 0.0f, 1.0f}; /* Green */
  style->away_color = (CMPColor){1.0f, 0.8f, 0.0f, 1.0f};      /* Yellow */
  style->busy_color = (CMPColor){0.8f, 0.0f, 0.0f, 1.0f};      /* Red */
  style->offline_color = (CMPColor){0.6f, 0.6f, 0.6f, 1.0f};   /* Gray */
  style->outline_color = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f}; /* White cut-out */

  return CMP_OK;
}

CMP_API int CMP_CALL f2_presence_badge_init(F2PresenceBadge *badge,
                                            const F2PresenceBadgeStyle *style,
                                            F2PresenceState state) {
  if (!badge || !style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(badge, 0, sizeof(*badge));
  badge->style = *style;
  badge->state = state;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_presence_badge_set_state(F2PresenceBadge *badge,
                                                 F2PresenceState state) {
  if (!badge)
    return CMP_ERR_INVALID_ARGUMENT;
  badge->state = state;
  return CMP_OK;
}
