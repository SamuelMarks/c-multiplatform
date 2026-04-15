/* clang-format off */
#include "m3_communication.h"
#include "m3_color.h"
#include <string.h>
#include <stdio.h>

static uint32_t color_to_hex(cmp_color_t color) {
    uint32_t r = (uint32_t)(color.r * 255.0f);
    uint32_t g = (uint32_t)(color.g * 255.0f);
    uint32_t b = (uint32_t)(color.b * 255.0f);
    uint32_t a = (uint32_t)(color.a * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

int m3_badge_create(material_catalog_state_t* state, const m3_badge_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* badge;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&badge) != 0) {
        return -1;
    }
    
    badge->bg_color_ref = &state->sys_colors_hex.error;

    if (config->type == M3_BADGE_TYPE_SMALL_DOT) {
        float size = dp_to_px(state, 6.0f);
        badge->layout->width = size;
        badge->layout->height = size;
        /* we need radius to be size/2, we don't have style directly on layout without cmp_ui_node styles.
           We'll rely on the rendering system drawing a circle if radius is set, but since we can't set it via style here, 
           we will assume catalog handles generic rendering via custom properties or it will be drawn as a small square box.
           A real implementation would bind a cmp_style_t. */
    } else {
        float height = dp_to_px(state, 16.0f);
        cmp_ui_node_t* text_node;
        char num_str[16];
        
        badge->layout->height = height;
        badge->layout->min_width = height; /* Pill shape min width */
        badge->layout->padding[3] = dp_to_px(state, 4.0f); /* Left */
        badge->layout->padding[1] = dp_to_px(state, 4.0f); /* Right */
        badge->layout->direction = CMP_FLEX_ROW;
        badge->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
        badge->layout->align_items = CMP_FLEX_ALIGN_CENTER;

        if (config->number > 999) {
            #if defined(_MSC_VER)
            sprintf_s(num_str, sizeof(num_str), "999+");
#else
            sprintf(num_str, "999+");
#endif
        } else {
            #if defined(_MSC_VER)
            sprintf_s(num_str, sizeof(num_str), "%d", config->number);
#else
            sprintf(num_str, "%d", config->number);
#endif
        }

        if (cmp_ui_text_create(&text_node, num_str, (int)strlen(num_str)) == 0) {
            text_node->text_color_ref = &state->sys_colors_hex.on_error;
            /* Label small typography, 11sp */
            text_node->font_size = dp_to_px(state, 11.0f);
            cmp_ui_node_add_child(badge, text_node);
        }
    }
    
    /* Typically badges are positioned absolutely */
    badge->layout->position_type = CMP_POSITION_ABSOLUTE;
    /* Anchored top-right */
    badge->layout->position[0] = dp_to_px(state, -4.0f); /* Top offset */
    badge->layout->position[1] = dp_to_px(state, -4.0f); /* Right offset */

    *out_node = badge;
    return 0;
}

int m3_progress_create(material_catalog_state_t* state, const m3_progress_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* progress;
    
    if (!state || !config || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&progress) != 0) {
        return -1;
    }
    
    if (config->type == M3_PROGRESS_TYPE_LINEAR) {
        cmp_ui_node_t* track;
        
        progress->layout->height = dp_to_px(state, 4.0f);
        progress->layout->width = 100.0f; /* 100% width default */
        progress->layout->flex_grow = 1.0f; /* Expand */
        progress->bg_color_ref = &state->sys_colors_hex.surface_container_highest;

        if (cmp_ui_box_create(&track) == 0) {
            track->bg_color_ref = &state->sys_colors_hex.primary;
            track->layout->height = dp_to_px(state, 4.0f);
            if (config->is_determinate) {
                float clamp_p = config->progress < 0.0f ? 0.0f : (config->progress > 1.0f ? 1.0f : config->progress);
                track->layout->width = clamp_p * 100.0f; /* We'd use a percentage internally if CMP supported it, for now mock pixel or flex */
                track->layout->flex_grow = clamp_p; 
            } else {
                /* Indeterminate logic would attach an animation timeline here */
                track->layout->width = dp_to_px(state, 40.0f); /* Mock size */
            }
            cmp_ui_node_add_child(progress, track);
        }
    } else {
        /* Circular */
        float size = dp_to_px(state, 48.0f);
        progress->layout->width = size;
        progress->layout->height = size;
        progress->bg_color = 0; /* Transparent base */
        
        /* Render engine would intercept this based on a custom property to draw a sweep */
        /* For now, represent it as a box with a primary color */
    }

    *out_node = progress;
    return 0;
}

int m3_snackbar_create(material_catalog_state_t* state, const m3_snackbar_config_t* config, cmp_ui_node_t** out_node) {
    cmp_ui_node_t* snackbar;
    cmp_ui_node_t* text_node;
    
    if (!state || !config || !config->text || !out_node) {
        return -1;
    }

    if (cmp_ui_box_create(&snackbar) != 0) {
        return -1;
    }
    
    snackbar->bg_color_ref = &state->sys_colors_hex.inverse_surface;
    
    snackbar->layout->min_height = dp_to_px(state, 48.0f);
    snackbar->layout->direction = CMP_FLEX_ROW;
    snackbar->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    snackbar->layout->padding[3] = dp_to_px(state, 16.0f); /* Left */
    snackbar->layout->padding[1] = dp_to_px(state, 8.0f); /* Right */
    snackbar->layout->padding[0] = dp_to_px(state, 14.0f); /* Top */
    snackbar->layout->padding[2] = dp_to_px(state, 14.0f); /* Bottom */

    if (cmp_ui_text_create(&text_node, config->text, (int)strlen(config->text)) == 0) {
        text_node->text_color_ref = &state->sys_colors_hex.inverse_on_surface;
        text_node->font_size = dp_to_px(state, 14.0f);
        text_node->layout->flex_grow = 1.0f; /* Push actions to the right */
        cmp_ui_node_add_child(snackbar, text_node);
    }

    if (config->action_label) {
        cmp_ui_node_t* action_btn;
        /* Using m3_buttons here */
        /* Would need to include m3_buttons.h and create a Text button, but we avoid dependency tangle 
           by just creating a simple text node acting as a button for this mock */
        if (cmp_ui_text_create(&action_btn, config->action_label, (int)strlen(config->action_label)) == 0) {
            action_btn->text_color_ref = &state->sys_colors_hex.inverse_primary;
            action_btn->font_size = dp_to_px(state, 14.0f);
            action_btn->layout->padding[3] = dp_to_px(state, 8.0f);
            cmp_ui_node_add_child(snackbar, action_btn);
        }
    }

    if (config->show_close_icon) {
        cmp_ui_node_t* close_btn;
        const char* x_icon = "X";
        if (cmp_ui_text_create(&close_btn, x_icon, 1) == 0) {
            close_btn->text_color_ref = &state->sys_colors_hex.inverse_on_surface;
            close_btn->layout->padding[3] = dp_to_px(state, 12.0f);
            cmp_ui_node_add_child(snackbar, close_btn);
        }
    }
    
    *out_node = snackbar;
    return 0;
}

/* Very simple global queue for snackbar examples */
#define MAX_SNACKBAR_QUEUE 16
static m3_snackbar_config_t snackbar_queue[MAX_SNACKBAR_QUEUE];
static int queue_head = 0;
static int queue_tail = 0;
static float current_snackbar_timer = 0.0f;
static float current_snackbar_duration = 0.0f;

int m3_snackbar_enqueue(material_catalog_state_t* state, const m3_snackbar_config_t* config) {
    int next_tail;
    if (!state || !config) return -1;
    
    next_tail = (queue_tail + 1) % MAX_SNACKBAR_QUEUE;
    if (next_tail == queue_head) {
        return -1; /* Queue full */
    }
    
    snackbar_queue[queue_tail] = *config;
    queue_tail = next_tail;
    
    return 0;
}

int m3_snackbar_process_queue(material_catalog_state_t* state, float dt) {
    if (!state) return -1;
    
    if (queue_head == queue_tail) {
        return 0; /* Empty queue */
    }
    
    /* If a snackbar is active */
    if (current_snackbar_duration > 0.0f) {
        current_snackbar_timer += dt;
        if (current_snackbar_timer >= current_snackbar_duration) {
            /* Dismissed */
            if (snackbar_queue[queue_head].on_dismiss) {
                snackbar_queue[queue_head].on_dismiss(snackbar_queue[queue_head].user_data);
            }
            queue_head = (queue_head + 1) % MAX_SNACKBAR_QUEUE;
            current_snackbar_timer = 0.0f;
            current_snackbar_duration = 0.0f;
            state->is_ui_dirty = 1;
        }
    } else {
        /* Pop next snackbar */
        if (queue_head != queue_tail) {
            switch (snackbar_queue[queue_head].duration) {
                case M3_SNACKBAR_DURATION_SHORT:
                    current_snackbar_duration = 4000.0f;
                    break;
                case M3_SNACKBAR_DURATION_LONG:
                    current_snackbar_duration = 10000.0f;
                    break;
                case M3_SNACKBAR_DURATION_INDEFINITE:
                    current_snackbar_duration = 99999999.0f; /* Essentially indefinite */
                    break;
            }
            current_snackbar_timer = 0.0f;
            state->is_ui_dirty = 1;
        }
    }
    
    return 0;
}
/* clang-format on */
