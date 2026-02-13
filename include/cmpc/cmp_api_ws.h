#ifndef CMP_API_WS_H
#define CMP_API_WS_H

/**
 * @file cmp_api_ws.h
 * @brief Window system abstraction ABI for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_math.h"
#include "cmpc/cmp_object.h"

/** @brief Window is resizable by the user. */
#define CMP_WS_WINDOW_RESIZABLE 0x01
/** @brief Window has no system border. */
#define CMP_WS_WINDOW_BORDERLESS 0x02
/** @brief Window starts in fullscreen mode. */
#define CMP_WS_WINDOW_FULLSCREEN 0x04
/** @brief Window requests high-DPI scaling. */
#define CMP_WS_WINDOW_HIGH_DPI 0x08

/** @brief Shift modifier is active. */
#define CMP_MOD_SHIFT 0x01
/** @brief Control modifier is active. */
#define CMP_MOD_CTRL 0x02
/** @brief Alt modifier is active. */
#define CMP_MOD_ALT 0x04
/** @brief Meta/Command modifier is active. */
#define CMP_MOD_META 0x08
/** @brief Caps lock is active. */
#define CMP_MOD_CAPS 0x10
/** @brief Num lock is active. */
#define CMP_MOD_NUM 0x20

/** @brief No input event. */
#define CMP_INPUT_NONE 0
/** @brief Pointer/touch press event. */
#define CMP_INPUT_POINTER_DOWN 1
/** @brief Pointer/touch release event. */
#define CMP_INPUT_POINTER_UP 2
/** @brief Pointer/touch move event. */
#define CMP_INPUT_POINTER_MOVE 3
/** @brief Pointer scroll/wheel event. */
#define CMP_INPUT_POINTER_SCROLL 4
/** @brief Key press event. */
#define CMP_INPUT_KEY_DOWN 5
/** @brief Key release event. */
#define CMP_INPUT_KEY_UP 6
/** @brief Text input event (UTF-8). */
#define CMP_INPUT_TEXT 7
/** @brief Window resize event. */
#define CMP_INPUT_WINDOW_RESIZE 8
/** @brief Window close request event. */
#define CMP_INPUT_WINDOW_CLOSE 9
/** @brief Window focus gained event. */
#define CMP_INPUT_WINDOW_FOCUS 10
/** @brief Window focus lost event. */
#define CMP_INPUT_WINDOW_BLUR 11
/** @brief UTF-8 text input event with full-length string. */
#define CMP_INPUT_TEXT_UTF8 12
/** @brief UTF-8 IME composition update event. */
#define CMP_INPUT_TEXT_EDIT 13
/** @brief Tap gesture event. */
#define CMP_INPUT_GESTURE_TAP 14
/** @brief Double tap gesture event. */
#define CMP_INPUT_GESTURE_DOUBLE_TAP 15
/** @brief Long press gesture event. */
#define CMP_INPUT_GESTURE_LONG_PRESS 16
/** @brief Drag gesture started. */
#define CMP_INPUT_GESTURE_DRAG_START 17
/** @brief Drag gesture update. */
#define CMP_INPUT_GESTURE_DRAG_UPDATE 18
/** @brief Drag gesture ended. */
#define CMP_INPUT_GESTURE_DRAG_END 19
/** @brief Fling gesture event. */
#define CMP_INPUT_GESTURE_FLING 20

/**
 * @brief Pointer input data.
 */
typedef struct CMPPointerEvent {
  cmp_i32 pointer_id; /**< Pointer or touch identifier. */
  cmp_i32 x;          /**< X position in window coordinates. */
  cmp_i32 y;          /**< Y position in window coordinates. */
  cmp_i32 buttons;    /**< Bitmask of pressed buttons. */
  cmp_i32 scroll_x;   /**< Horizontal scroll delta. */
  cmp_i32 scroll_y;   /**< Vertical scroll delta. */
} CMPPointerEvent;

/**
 * @brief Key input data.
 */
typedef struct CMPKeyEvent {
  cmp_u32 key_code;    /**< Logical key code. */
  cmp_u32 native_code; /**< Platform-specific key code. */
  CMPBool is_repeat;   /**< CMP_TRUE if this is an auto-repeat. */
} CMPKeyEvent;

/**
 * @brief UTF-8 text input data (null-terminated, short form).
 */
typedef struct CMPTextEvent {
  char utf8[8];   /**< UTF-8 bytes including the null terminator. */
  cmp_u32 length; /**< Number of bytes excluding the null terminator. */
} CMPTextEvent;

/**
 * @brief UTF-8 text input data (full-length).
 *
 * @note The utf8 pointer is owned by the backend and is only guaranteed to be
 *       valid until the next call to poll_event on the same window system.
 */
typedef struct CMPTextUtf8Event {
  const char *utf8; /**< UTF-8 bytes (may be NULL for empty). */
  cmp_usize length; /**< Number of bytes in utf8 (may be 0). */
} CMPTextUtf8Event;

/**
 * @brief IME composition (text editing) data.
 *
 * @note Offsets are byte offsets in the UTF-8 buffer.
 * @note The utf8 pointer is owned by the backend and is only guaranteed to be
 *       valid until the next call to poll_event on the same window system.
 */
typedef struct CMPTextEditEvent {
  const char *utf8; /**< Current composition string in UTF-8 (may be NULL). */
  cmp_usize length; /**< Number of bytes in utf8 (may be 0). */
  cmp_i32 cursor;   /**< Cursor byte offset in utf8. */
  cmp_i32 selection_length; /**< Selection length in bytes. */
} CMPTextEditEvent;

/**
 * @brief Gesture input data.
 */
typedef struct CMPGestureEvent {
  cmp_i32 pointer_id;   /**< Pointer or touch identifier. */
  CMPScalar x;          /**< Current X position in window coordinates. */
  CMPScalar y;          /**< Current Y position in window coordinates. */
  CMPScalar start_x;    /**< Gesture start X position in window coordinates. */
  CMPScalar start_y;    /**< Gesture start Y position in window coordinates. */
  CMPScalar delta_x;    /**< Delta X since the previous gesture event. */
  CMPScalar delta_y;    /**< Delta Y since the previous gesture event. */
  CMPScalar total_x;    /**< Total X offset since the gesture start. */
  CMPScalar total_y;    /**< Total Y offset since the gesture start. */
  CMPScalar velocity_x; /**< Velocity along X in pixels per second. */
  CMPScalar velocity_y; /**< Velocity along Y in pixels per second. */
  cmp_u32 tap_count;    /**< Tap count (1 for tap, 2 for double tap). */
} CMPGestureEvent;

/**
 * @brief Window event data.
 */
typedef struct CMPWindowEvent {
  cmp_i32 width;  /**< New window width in pixels. */
  cmp_i32 height; /**< New window height in pixels. */
} CMPWindowEvent;

/**
 * @brief Discriminated union for input events.
 */
typedef union CMPInputEventData {
  CMPPointerEvent pointer;    /**< Pointer event payload. */
  CMPKeyEvent key;            /**< Key event payload. */
  CMPTextEvent text;          /**< Text input payload. */
  CMPTextUtf8Event text_utf8; /**< Full UTF-8 text input payload. */
  CMPTextEditEvent text_edit; /**< IME composition payload. */
  CMPGestureEvent gesture;    /**< Gesture payload. */
  CMPWindowEvent window;      /**< Window event payload. */
} CMPInputEventData;

/**
 * @brief Top-level input event.
 */
typedef struct CMPInputEvent {
  cmp_u32 type;           /**< One of CMP_INPUT_*. */
  cmp_u32 modifiers;      /**< Modifier key bitmask (CMP_MOD_*). */
  cmp_u32 reserved;       /**< Reserved for future use. */
  cmp_u32 time_ms;        /**< Timestamp in milliseconds. */
  CMPHandle window;       /**< Originating window handle. */
  CMPInputEventData data; /**< Event payload. */
} CMPInputEvent;

/**
 * @brief Window creation configuration.
 */
typedef struct CMPWSWindowConfig {
  cmp_i32 width;          /**< Initial width in pixels. */
  cmp_i32 height;         /**< Initial height in pixels. */
  const char *utf8_title; /**< Window title in UTF-8. */
  cmp_u32 flags;          /**< Window flags (CMP_WS_WINDOW_*). */
} CMPWSWindowConfig;

/**
 * @brief Window system configuration.
 */
typedef struct CMPWSConfig {
  const char *utf8_app_name; /**< Application name in UTF-8. */
  const char *utf8_app_id;   /**< Application identifier in UTF-8. */
  cmp_u32 reserved;          /**< Reserved for future use. */
} CMPWSConfig;

/**
 * @brief Window system virtual table.
 */
typedef struct CMPWSVTable {
/**
 * @brief Initialize the window system.
 * @param ws Window system instance.
 * @param config Configuration to apply.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int init(void *ws, const CMPWSConfig *config);
#else
  int(CMP_CALL *init)(void *ws, const CMPWSConfig *config);
#endif
/**
 * @brief Shut down the window system.
 * @param ws Window system instance.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int shutdown(void *ws);
#else
  int(CMP_CALL *shutdown)(void *ws);
#endif
/**
 * @brief Create a window.
 * @param ws Window system instance.
 * @param config Window configuration.
 * @param out_window Receives the created window handle.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int create_window(void *ws, const CMPWSWindowConfig *config,
                    CMPHandle *out_window);
#else
  int(CMP_CALL *create_window)(void *ws, const CMPWSWindowConfig *config,
                               CMPHandle *out_window);
#endif
/**
 * @brief Destroy a window.
 * @param ws Window system instance.
 * @param window Window handle to destroy.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int destroy_window(void *ws, CMPHandle window);
#else
  int(CMP_CALL *destroy_window)(void *ws, CMPHandle window);
#endif
/**
 * @brief Show a window.
 * @param ws Window system instance.
 * @param window Window handle to show.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int show_window(void *ws, CMPHandle window);
#else
  int(CMP_CALL *show_window)(void *ws, CMPHandle window);
#endif
/**
 * @brief Hide a window.
 * @param ws Window system instance.
 * @param window Window handle to hide.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int hide_window(void *ws, CMPHandle window);
#else
  int(CMP_CALL *hide_window)(void *ws, CMPHandle window);
#endif
/**
 * @brief Set the window title.
 * @param ws Window system instance.
 * @param window Window handle to update.
 * @param utf8_title New window title in UTF-8.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int set_window_title(void *ws, CMPHandle window, const char *utf8_title);
#else
  int(CMP_CALL *set_window_title)(void *ws, CMPHandle window,
                                  const char *utf8_title);
#endif
/**
 * @brief Set the window size.
 * @param ws Window system instance.
 * @param window Window handle to update.
 * @param width New width in pixels.
 * @param height New height in pixels.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int set_window_size(void *ws, CMPHandle window, cmp_i32 width,
                      cmp_i32 height);
#else
  int(CMP_CALL *set_window_size)(void *ws, CMPHandle window, cmp_i32 width,
                                 cmp_i32 height);
#endif
/**
 * @brief Get the window size.
 * @param ws Window system instance.
 * @param window Window handle to query.
 * @param out_width Receives the width in pixels.
 * @param out_height Receives the height in pixels.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int get_window_size(void *ws, CMPHandle window, cmp_i32 *out_width,
                      cmp_i32 *out_height);
#else
  int(CMP_CALL *get_window_size)(void *ws, CMPHandle window, cmp_i32 *out_width,
                                 cmp_i32 *out_height);
#endif
/**
 * @brief Set the window DPI scale.
 * @param ws Window system instance.
 * @param window Window handle to update.
 * @param scale DPI scale factor.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int set_window_dpi_scale(void *ws, CMPHandle window, CMPScalar scale);
#else
  int(CMP_CALL *set_window_dpi_scale)(void *ws, CMPHandle window,
                                      CMPScalar scale);
#endif
/**
 * @brief Get the window DPI scale.
 * @param ws Window system instance.
 * @param window Window handle to query.
 * @param out_scale Receives the DPI scale factor.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int get_window_dpi_scale(void *ws, CMPHandle window, CMPScalar *out_scale);
#else
  int(CMP_CALL *get_window_dpi_scale)(void *ws, CMPHandle window,
                                      CMPScalar *out_scale);
#endif
/**
 * @brief Set the clipboard text.
 * @param ws Window system instance.
 * @param utf8_text Text to place on the clipboard.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int set_clipboard_text(void *ws, const char *utf8_text);
#else
  int(CMP_CALL *set_clipboard_text)(void *ws, const char *utf8_text);
#endif
/**
 * @brief Get the clipboard text.
 * @param ws Window system instance.
 * @param buffer Output buffer for UTF-8 text.
 * @param buffer_size Size of the output buffer in bytes.
 * @param out_length Receives the number of bytes written.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int get_clipboard_text(void *ws, char *buffer, cmp_usize buffer_size,
                         cmp_usize *out_length);
#else
  int(CMP_CALL *get_clipboard_text)(void *ws, char *buffer,
                                    cmp_usize buffer_size,
                                    cmp_usize *out_length);
#endif
/**
 * @brief Poll for a single input event.
 * @param ws Window system instance.
 * @param out_event Receives the next event.
 * @param out_has_event Receives CMP_TRUE if an event was returned.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int poll_event(void *ws, CMPInputEvent *out_event, CMPBool *out_has_event);
#else
  int(CMP_CALL *poll_event)(void *ws, CMPInputEvent *out_event,
                            CMPBool *out_has_event);
#endif
/**
 * @brief Pump pending events without returning them.
 * @param ws Window system instance.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int pump_events(void *ws);
#else
  int(CMP_CALL *pump_events)(void *ws);
#endif
/**
 * @brief Get the current time in milliseconds.
 * @param ws Window system instance.
 * @param out_time_ms Receives the time in milliseconds.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int get_time_ms(void *ws, cmp_u32 *out_time_ms);
#else
  int(CMP_CALL *get_time_ms)(void *ws, cmp_u32 *out_time_ms);
#endif
} CMPWSVTable;

/**
 * @brief Window system interface.
 */
typedef struct CMPWS {
  void *ctx;                 /**< Backend context pointer. */
  const CMPWSVTable *vtable; /**< Window system virtual table. */
} CMPWS;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_API_WS_H */
