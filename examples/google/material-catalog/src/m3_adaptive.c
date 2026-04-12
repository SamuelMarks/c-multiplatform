/* clang-format off */
#include "m3_adaptive.h"

int m3_adaptive_get_window_size_class(float width_dp, m3_window_size_class_t* out_class) {
    if (!out_class) {
        return -1;
    }

    if (width_dp < 600.0f) {
        *out_class = M3_WINDOW_SIZE_CLASS_COMPACT;
    } else if (width_dp < 840.0f) {
        *out_class = M3_WINDOW_SIZE_CLASS_MEDIUM;
    } else {
        *out_class = M3_WINDOW_SIZE_CLASS_EXPANDED;
    }

    return 0;
}

int m3_adaptive_get_nav_scaffolding(m3_window_size_class_t size_class, m3_nav_scaffolding_t* out_nav) {
    if (!out_nav) {
        return -1;
    }

    switch (size_class) {
        case M3_WINDOW_SIZE_CLASS_COMPACT:
            *out_nav = M3_NAV_SCAFFOLDING_BOTTOM_BAR;
            break;
        case M3_WINDOW_SIZE_CLASS_MEDIUM:
            *out_nav = M3_NAV_SCAFFOLDING_NAVIGATION_RAIL;
            break;
        case M3_WINDOW_SIZE_CLASS_EXPANDED:
            *out_nav = M3_NAV_SCAFFOLDING_PERMANENT_DRAWER;
            break;
        default:
            return -1;
    }

    return 0;
}

int m3_adaptive_get_list_detail_layout(m3_window_size_class_t size_class, m3_list_detail_layout_t* out_layout) {
    if (!out_layout) {
        return -1;
    }

    switch (size_class) {
        case M3_WINDOW_SIZE_CLASS_COMPACT:
            *out_layout = M3_LIST_DETAIL_STACKED;
            break;
        case M3_WINDOW_SIZE_CLASS_MEDIUM:
        case M3_WINDOW_SIZE_CLASS_EXPANDED:
            *out_layout = M3_LIST_DETAIL_SIDE_BY_SIDE;
            break;
        default:
            return -1;
    }

    return 0;
}

int m3_adaptive_get_supporting_pane_layout(m3_window_size_class_t size_class, m3_supporting_pane_layout_t* out_layout) {
    if (!out_layout) {
        return -1;
    }

    switch (size_class) {
        case M3_WINDOW_SIZE_CLASS_COMPACT:
            *out_layout = M3_SUPPORTING_PANE_HIDDEN_BOTTOM_SHEET;
            break;
        case M3_WINDOW_SIZE_CLASS_MEDIUM:
        case M3_WINDOW_SIZE_CLASS_EXPANDED:
            *out_layout = M3_SUPPORTING_PANE_VISIBLE_SIDE_PANEL;
            break;
        default:
            return -1;
    }

    return 0;
}

int m3_adaptive_get_feed_layout(m3_window_size_class_t size_class, m3_feed_layout_t* out_layout) {
    if (!out_layout) {
        return -1;
    }

    switch (size_class) {
        case M3_WINDOW_SIZE_CLASS_COMPACT:
            *out_layout = M3_FEED_SINGLE_COLUMN;
            break;
        case M3_WINDOW_SIZE_CLASS_MEDIUM:
        case M3_WINDOW_SIZE_CLASS_EXPANDED:
            *out_layout = M3_FEED_RESPONSIVE_GRID;
            break;
        default:
            return -1;
    }

    return 0;
}
/* clang-format on */