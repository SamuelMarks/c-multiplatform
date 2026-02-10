#ifndef M3_API_WS_H
#define M3_API_WS_H

/**
 * @file m3_api_ws.h
 * @brief Window system abstraction ABI for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_object.h"
#include "m3_math.h"

/** @brief Window is resizable by the user. */
#define M3_WS_WINDOW_RESIZABLE 0x01
/** @brief Window has no system border. */
#define M3_WS_WINDOW_BORDERLESS 0x02
/** @brief Window starts in fullscreen mode. */
#define M3_WS_WINDOW_FULLSCREEN 0x04
/** @brief Window requests high-DPI scaling. */
#define M3_WS_WINDOW_HIGH_DPI 0x08

/** @brief Shift modifier is active. */
#define M3_MOD_SHIFT 0x01
/** @brief Control modifier is active. */
#define M3_MOD_CTRL 0x02
/** @brief Alt modifier is active. */
#define M3_MOD_ALT 0x04
/** @brief Meta/Command modifier is active. */
#define M3_MOD_META 0x08
/** @brief Caps lock is active. */
#define M3_MOD_CAPS 0x10
/** @brief Num lock is active. */
#define M3_MOD_NUM 0x20

/** @brief No input event. */
#define M3_INPUT_NONE 0
/** @brief Pointer/touch press event. */
#define M3_INPUT_POINTER_DOWN 1
/** @brief Pointer/touch release event. */
#define M3_INPUT_POINTER_UP 2
/** @brief Pointer/touch move event. */
#define M3_INPUT_POINTER_MOVE 3
/** @brief Pointer scroll/wheel event. */
#define M3_INPUT_POINTER_SCROLL 4
/** @brief Key press event. */
#define M3_INPUT_KEY_DOWN 5
/** @brief Key release event. */
#define M3_INPUT_KEY_UP 6
/** @brief Text input event (UTF-8). */
#define M3_INPUT_TEXT 7
/** @brief Window resize event. */
#define M3_INPUT_WINDOW_RESIZE 8
/** @brief Window close request event. */
#define M3_INPUT_WINDOW_CLOSE 9
/** @brief Window focus gained event. */
#define M3_INPUT_WINDOW_FOCUS 10
/** @brief Window focus lost event. */
#define M3_INPUT_WINDOW_BLUR 11
/** @brief UTF-8 text input event with full-length string. */
#define M3_INPUT_TEXT_UTF8 12
/** @brief UTF-8 IME composition update event. */
#define M3_INPUT_TEXT_EDIT 13

/**
 * @brief Pointer input data.
 */
typedef struct M3PointerEvent {
    m3_i32 pointer_id; /**< Pointer or touch identifier. */
    m3_i32 x; /**< X position in window coordinates. */
    m3_i32 y; /**< Y position in window coordinates. */
    m3_i32 buttons; /**< Bitmask of pressed buttons. */
    m3_i32 scroll_x; /**< Horizontal scroll delta. */
    m3_i32 scroll_y; /**< Vertical scroll delta. */
} M3PointerEvent;

/**
 * @brief Key input data.
 */
typedef struct M3KeyEvent {
    m3_u32 key_code; /**< Logical key code. */
    m3_u32 native_code; /**< Platform-specific key code. */
    M3Bool is_repeat; /**< M3_TRUE if this is an auto-repeat. */
} M3KeyEvent;

/**
 * @brief UTF-8 text input data (null-terminated, short form).
 */
typedef struct M3TextEvent {
    char utf8[8]; /**< UTF-8 bytes including the null terminator. */
    m3_u32 length; /**< Number of bytes excluding the null terminator. */
} M3TextEvent;

/**
 * @brief UTF-8 text input data (full-length).
 *
 * @note The utf8 pointer is owned by the backend and is only guaranteed to be
 *       valid until the next call to poll_event on the same window system.
 */
typedef struct M3TextUtf8Event {
    const char *utf8; /**< UTF-8 bytes (may be NULL for empty). */
    m3_usize length; /**< Number of bytes in utf8 (may be 0). */
} M3TextUtf8Event;

/**
 * @brief IME composition (text editing) data.
 *
 * @note Offsets are byte offsets in the UTF-8 buffer.
 * @note The utf8 pointer is owned by the backend and is only guaranteed to be
 *       valid until the next call to poll_event on the same window system.
 */
typedef struct M3TextEditEvent {
    const char *utf8; /**< Current composition string in UTF-8 (may be NULL). */
    m3_usize length; /**< Number of bytes in utf8 (may be 0). */
    m3_i32 cursor; /**< Cursor byte offset in utf8. */
    m3_i32 selection_length; /**< Selection length in bytes. */
} M3TextEditEvent;

/**
 * @brief Window event data.
 */
typedef struct M3WindowEvent {
    m3_i32 width; /**< New window width in pixels. */
    m3_i32 height; /**< New window height in pixels. */
} M3WindowEvent;

/**
 * @brief Discriminated union for input events.
 */
typedef union M3InputEventData {
    M3PointerEvent pointer; /**< Pointer event payload. */
    M3KeyEvent key; /**< Key event payload. */
    M3TextEvent text; /**< Text input payload. */
    M3TextUtf8Event text_utf8; /**< Full UTF-8 text input payload. */
    M3TextEditEvent text_edit; /**< IME composition payload. */
    M3WindowEvent window; /**< Window event payload. */
} M3InputEventData;

/**
 * @brief Top-level input event.
 */
typedef struct M3InputEvent {
    m3_u32 type; /**< One of M3_INPUT_*. */
    m3_u32 modifiers; /**< Modifier key bitmask (M3_MOD_*). */
    m3_u32 reserved; /**< Reserved for future use. */
    m3_u32 time_ms; /**< Timestamp in milliseconds. */
    M3Handle window; /**< Originating window handle. */
    M3InputEventData data; /**< Event payload. */
} M3InputEvent;

/**
 * @brief Window creation configuration.
 */
typedef struct M3WSWindowConfig {
    m3_i32 width; /**< Initial width in pixels. */
    m3_i32 height; /**< Initial height in pixels. */
    const char *utf8_title; /**< Window title in UTF-8. */
    m3_u32 flags; /**< Window flags (M3_WS_WINDOW_*). */
} M3WSWindowConfig;

/**
 * @brief Window system configuration.
 */
typedef struct M3WSConfig {
    const char *utf8_app_name; /**< Application name in UTF-8. */
    const char *utf8_app_id; /**< Application identifier in UTF-8. */
    m3_u32 reserved; /**< Reserved for future use. */
} M3WSConfig;

/**
 * @brief Window system virtual table.
 */
typedef struct M3WSVTable {
    /**
     * @brief Initialize the window system.
     * @param ws Window system instance.
     * @param config Configuration to apply.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *init)(void *ws, const M3WSConfig *config);
    /**
     * @brief Shut down the window system.
     * @param ws Window system instance.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *shutdown)(void *ws);
    /**
     * @brief Create a window.
     * @param ws Window system instance.
     * @param config Window configuration.
     * @param out_window Receives the created window handle.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *create_window)(void *ws, const M3WSWindowConfig *config, M3Handle *out_window);
    /**
     * @brief Destroy a window.
     * @param ws Window system instance.
     * @param window Window handle to destroy.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *destroy_window)(void *ws, M3Handle window);
    /**
     * @brief Show a window.
     * @param ws Window system instance.
     * @param window Window handle to show.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *show_window)(void *ws, M3Handle window);
    /**
     * @brief Hide a window.
     * @param ws Window system instance.
     * @param window Window handle to hide.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *hide_window)(void *ws, M3Handle window);
    /**
     * @brief Set the window title.
     * @param ws Window system instance.
     * @param window Window handle to update.
     * @param utf8_title New window title in UTF-8.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *set_window_title)(void *ws, M3Handle window, const char *utf8_title);
    /**
     * @brief Set the window size.
     * @param ws Window system instance.
     * @param window Window handle to update.
     * @param width New width in pixels.
     * @param height New height in pixels.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *set_window_size)(void *ws, M3Handle window, m3_i32 width, m3_i32 height);
    /**
     * @brief Get the window size.
     * @param ws Window system instance.
     * @param window Window handle to query.
     * @param out_width Receives the width in pixels.
     * @param out_height Receives the height in pixels.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *get_window_size)(void *ws, M3Handle window, m3_i32 *out_width, m3_i32 *out_height);
    /**
     * @brief Set the window DPI scale.
     * @param ws Window system instance.
     * @param window Window handle to update.
     * @param scale DPI scale factor.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *set_window_dpi_scale)(void *ws, M3Handle window, M3Scalar scale);
    /**
     * @brief Get the window DPI scale.
     * @param ws Window system instance.
     * @param window Window handle to query.
     * @param out_scale Receives the DPI scale factor.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *get_window_dpi_scale)(void *ws, M3Handle window, M3Scalar *out_scale);
    /**
     * @brief Set the clipboard text.
     * @param ws Window system instance.
     * @param utf8_text Text to place on the clipboard.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *set_clipboard_text)(void *ws, const char *utf8_text);
    /**
     * @brief Get the clipboard text.
     * @param ws Window system instance.
     * @param buffer Output buffer for UTF-8 text.
     * @param buffer_size Size of the output buffer in bytes.
     * @param out_length Receives the number of bytes written.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *get_clipboard_text)(void *ws, char *buffer, m3_usize buffer_size, m3_usize *out_length);
    /**
     * @brief Poll for a single input event.
     * @param ws Window system instance.
     * @param out_event Receives the next event.
     * @param out_has_event Receives M3_TRUE if an event was returned.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *poll_event)(void *ws, M3InputEvent *out_event, M3Bool *out_has_event);
    /**
     * @brief Pump pending events without returning them.
     * @param ws Window system instance.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *pump_events)(void *ws);
    /**
     * @brief Get the current time in milliseconds.
     * @param ws Window system instance.
     * @param out_time_ms Receives the time in milliseconds.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *get_time_ms)(void *ws, m3_u32 *out_time_ms);
} M3WSVTable;

/**
 * @brief Window system interface.
 */
typedef struct M3WS {
    void *ctx; /**< Backend context pointer. */
    const M3WSVTable *vtable; /**< Window system virtual table. */
} M3WS;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_API_WS_H */
