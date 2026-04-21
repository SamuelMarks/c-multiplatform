/* clang-format off */
#ifndef M3_EVENTS_H
#define M3_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmp.h>
#include "material_catalog.h"

/* Material 3 specific interaction flags */
#define M3_INTERACTION_HOVERED  (1 << 0)
#define M3_INTERACTION_FOCUSED  (1 << 1)
#define M3_INTERACTION_PRESSED  (1 << 2)
#define M3_INTERACTION_DRAGGED  (1 << 3)
#define M3_INTERACTION_DISABLED (1 << 4)

/* M3 Slop default threshold (dp) */
#define M3_TOUCH_SLOP_DP 8.0f

/**
 * \struct m3_interaction_state_t
 * \brief Node-level state holding interaction flags.
 */
typedef struct m3_interaction_state_t {
    uint32_t flags;
    float ripple_radius;
    float ripple_opacity;
    float hover_opacity;
    float focus_opacity;
    float pressed_opacity;
} m3_interaction_state_t;

/**
 * \struct m3_event_engine_t
 * \brief Tracks global input state, velocity, and modality.
 */
typedef struct m3_event_engine_t {
    uint32_t active_modality; /* CMP_EVENT_TYPE_MOUSE, CMP_EVENT_TYPE_TOUCH, CMP_EVENT_TYPE_KEYBOARD, etc. */
    
    cmp_ui_node_t *hovered_node;
    cmp_ui_node_t *focused_node;
    cmp_ui_node_t *pressed_node;
    
    int is_focus_ring_visible; /* Only true if focus was acquired via Keyboard */
    
    /* Touch / Drag tracking */
    int is_dragging;
    float start_x;
    float start_y;
    float current_x;
    float current_y;
    
    /* Velocity tracking */
    float velocity_x;
    float velocity_y;
    uint32_t last_event_time_ms;
    
    /* Gamepad specific (placeholder for D-Pad focus navigation) */
    int gamepad_connected;
} m3_event_engine_t;

/**
 * \brief Initializes the M3 event pipeline and touch slop tracker.
 * \param engine The event engine context.
 * \return 0 on success, non-zero on error.
 */
int m3_event_engine_init(m3_event_engine_t *engine);

/**
 * \brief Cleans up the event engine.
 * \param engine The event engine context.
 * \return 0 on success, non-zero on error.
 */
int m3_event_engine_cleanup(m3_event_engine_t *engine);

/**
 * \brief Global entry point for processing an input event, updating modality and velocity.
 * \param engine The event engine context.
 * \param state The Material Catalog state context.
 * \param event The incoming CMP event.
 * \param current_time_ms The current timestamp.
 * \return 0 on success, non-zero on error.
 */
int m3_event_process(m3_event_engine_t *engine, material_catalog_state_t *state, const cmp_event_t *event, uint32_t current_time_ms);

/**
 * \brief Retrieves the active input modality (Mouse, Touch, Keyboard, Stylus, Gamepad).
 * \param engine The event engine context.
 * \param out_modality Pointer to receive the modality enum value.
 * \return 0 on success, non-zero on error.
 */
int m3_event_get_active_modality(const m3_event_engine_t *engine, uint32_t *out_modality);

/**
 * \brief Checks if the minimum touch slop distance has been exceeded.
 * \param engine The event engine context.
 * \param dpi_scale The active monitor's DPI scaling factor.
 * \param out_exceeded Pointer to receive boolean flag (1 if exceeded, 0 if not).
 * \return 0 on success, non-zero on error.
 */
int m3_event_check_touch_slop(const m3_event_engine_t *engine, float dpi_scale, int *out_exceeded);

/**
 * \brief Calculates the current scroll/fling velocity.
 * \param engine The event engine context.
 * \param out_vx Pointer to receive X velocity (pixels/ms).
 * \param out_vy Pointer to receive Y velocity (pixels/ms).
 * \return 0 on success, non-zero on error.
 */
int m3_event_get_velocity(const m3_event_engine_t *engine, float *out_vx, float *out_vy);

/**
 * \brief Focuses a node programmatically. Adjusts focus ring visibility based on modality.
 * \param engine The event engine context.
 * \param node The node to focus.
 * \return 0 on success, non-zero on error.
 */
int m3_event_set_focus(m3_event_engine_t *engine, cmp_ui_node_t *node);

#ifdef __cplusplus
}
#endif

#endif /* M3_EVENTS_H */
/* clang-format on */