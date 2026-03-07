#include "f2/f2_card.h"
#include "cmpc/cmp_core.h"
#include <string.h>

CMP_API int CMP_CALL f2_card_style_init(F2CardStyle *style) {
    if (!style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(style, 0, sizeof(*style));
    
    style->is_clickable = CMP_FALSE;
    style->padding = F2_CARD_DEFAULT_PADDING;
    style->corner_radius = F2_CARD_DEFAULT_CORNER_RADIUS;
    style->min_width = F2_CARD_DEFAULT_MIN_SIZE;
    style->min_height = F2_CARD_DEFAULT_MIN_SIZE;
    style->border_width = 1.0f;
    
    style->background_color = (CMPColor){1.0f, 1.0f, 1.0f, 1.0f};
    style->border_color = (CMPColor){0.9f, 0.9f, 0.9f, 1.0f};
    style->hover_border_color = (CMPColor){0.8f, 0.8f, 0.8f, 1.0f};
    style->pressed_color = (CMPColor){0.95f, 0.95f, 0.95f, 1.0f};
    
    return CMP_OK;
}

CMP_API int CMP_CALL f2_card_init(F2Card *card, const F2CardStyle *style) {
    if (!card || !style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(card, 0, sizeof(*card));
    card->style = *style;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_card_set_on_click(F2Card *card, F2CardOnClick on_click, void *ctx) {
    if (!card) return CMP_ERR_INVALID_ARGUMENT;
    card->on_click = on_click;
    card->on_click_ctx = ctx;
    card->style.is_clickable = (on_click != NULL);
    return CMP_OK;
}
