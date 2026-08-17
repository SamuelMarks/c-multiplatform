/**
 * @file ui_event.h
 * @brief Generic event structures and enumerations.
 *
 * This header defines a unified event system handling various input types
 * such as mouse, keyboard, touch, pen, gamepad, and window events.
 */

#ifndef UI_EVENT_H
#define UI_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Types of events.
 */
enum ui_event_type {
  UI_EVENT_NONE = 0,            /**< No event. */
  UI_EVENT_WINDOW_CLOSE,        /**< Window close event. */
  UI_EVENT_WINDOW_RESIZE,       /**< Window resize event. */
  UI_EVENT_MOUSE_DOWN,          /**< Mouse down event. */
  UI_EVENT_MOUSE_UP,            /**< Mouse up event. */
  UI_EVENT_MOUSE_MOVE,          /**< Mouse move event. */
  UI_EVENT_MOUSE_WHEEL,         /**< Mouse wheel scroll event. */
  UI_EVENT_MOUSE_ENTER,         /**< Mouse enter element event. */
  UI_EVENT_MOUSE_LEAVE,         /**< Mouse leave element event. */
  UI_EVENT_CLICK,               /**< Mouse click event. */
  UI_EVENT_DBLCLICK,            /**< Mouse double click event. */
  UI_EVENT_CONTEXT_MENU,        /**< Context menu request event. */
  UI_EVENT_KEY_PRESS,           /**< Key press event. */
  UI_EVENT_KEY_DOWN,            /**< Key down event. */
  UI_EVENT_KEY_UP,              /**< Key up event. */
  UI_EVENT_TOUCH_START,         /**< Touch start event. */
  UI_EVENT_TOUCH_MOVE,          /**< Touch move event. */
  UI_EVENT_TOUCH_END,           /**< Touch end event. */
  UI_EVENT_TOUCH_CANCEL,        /**< Touch cancel event. */
  UI_EVENT_PEN_DOWN,            /**< Pen down event. */
  UI_EVENT_PEN_UP,              /**< Pen up event. */
  UI_EVENT_PEN_MOVE,            /**< Pen move event. */
  UI_EVENT_GAMEPAD_BUTTON_DOWN, /**< Gamepad button down event. */
  UI_EVENT_GAMEPAD_BUTTON_UP,   /**< Gamepad button up event. */
  UI_EVENT_GAMEPAD_AXIS,        /**< Gamepad axis move event. */
  UI_EVENT_OS_DEEP_LINK         /**< OS deep link event. */
};

/**
 * @brief Modifiers for input events.
 */
enum ui_modifier_flags {
  UI_MODIFIER_NONE = 0,       /**< No modifiers. */
  UI_MODIFIER_SHIFT = 1 << 0, /**< Shift modifier. */
  UI_MODIFIER_CTRL = 1 << 1,  /**< Control modifier. */
  UI_MODIFIER_ALT = 1 << 2,   /**< Alt modifier. */
  UI_MODIFIER_SUPER = 1 << 3  /**< Super/Meta modifier. */
};

/**
 * @brief Mouse event data.
 */
struct ui_mouse_event {
  int x;                  /**< X coordinate of the mouse. */
  int y;                  /**< Y coordinate of the mouse. */
  int button;             /**< 0: Left, 1: Right, 2: Middle. */
  float wheel_x;          /**< Horizontal scroll delta. */
  float wheel_y;          /**< Vertical scroll delta. */
  unsigned int modifiers; /**< Bitmask of `ui_modifier_flags`. */
};

/**
 * @brief Common key codes for cross-platform mapping.
 */
enum ui_key_code {
  UI_KEY_UNKNOWN = 0,     /**< Unknown key. */
  UI_KEY_SPACE = 32,      /**< Space key. */
  UI_KEY_ENTER = 257,     /**< Enter key. */
  UI_KEY_ESCAPE = 258,    /**< Escape key. */
  UI_KEY_BACKSPACE = 259, /**< Backspace key. */
  UI_KEY_TAB = 260,       /**< Tab key. */
  UI_KEY_UP = 261,        /**< Up arrow key. */
  UI_KEY_DOWN = 262,      /**< Down arrow key. */
  UI_KEY_LEFT = 263,      /**< Left arrow key. */
  UI_KEY_RIGHT = 264,     /**< Right arrow key. */
  UI_KEY_HOME = 265,      /**< Home key. */
  UI_KEY_END = 266        /**< End key. */
};

/**
 * @brief Keyboard event data.
 */
struct ui_keyboard_event {
  int key_code;           /**< The mapped key code. */
  int scan_code;          /**< The hardware scan code. */
  unsigned int modifiers; /**< Bitmask of `ui_modifier_flags`. */
  int is_repeat;          /**< 1 if the key repeats, 0 otherwise. */
};

/**
 * @brief Single touch point data.
 */
struct ui_touch_point {
  int id;         /**< The unique touch point ID. */
  int x;          /**< X coordinate of the touch point. */
  int y;          /**< Y coordinate of the touch point. */
  float pressure; /**< The pressure of the touch point. */
};

#define UI_MAX_TOUCH_POINTS                                                    \
  10 /**< Maximum number of simultaneous touch points. */

/**
 * @brief Touch event data.
 */
struct ui_touch_event {
  struct ui_touch_point
      points[UI_MAX_TOUCH_POINTS]; /**< Array of active touch points. */
  int num_points;                  /**< Number of active touch points. */
  unsigned int modifiers;          /**< Bitmask of `ui_modifier_flags`. */
};

/**
 * @brief Pen/Stylus event data.
 */
struct ui_pen_event {
  int x;                  /**< X coordinate of the pen. */
  int y;                  /**< Y coordinate of the pen. */
  float pressure;         /**< The pressure of the pen. */
  float tilt_x;           /**< Horizontal tilt. */
  float tilt_y;           /**< Vertical tilt. */
  unsigned int modifiers; /**< Bitmask of `ui_modifier_flags`. */
};

/**
 * @brief Gamepad event data.
 */
struct ui_gamepad_event {
  int gamepad_id; /**< The ID of the gamepad. */
  int button;     /**< The button ID. */
  int axis;       /**< The axis ID. */
  float value;    /**< The current value of the axis or button. */
};

/**
 * @brief Window event data.
 */
struct ui_window_event {
  int width;  /**< New window width. */
  int height; /**< New window height. */
};

/**
 * @brief OS Deep Link event data.
 */
struct ui_os_deep_link_event {
  char uri[1024]; /**< The deep link URI. */
};

/**
 * @brief Unified input event structure.
 */
struct ui_event {
  enum ui_event_type type; /**< The event type. */
  /** \brief Union holding the data for the specific event type. */
  union {
    struct ui_mouse_event mouse;            /**< Data for mouse events. */
    struct ui_keyboard_event keyboard;      /**< Data for keyboard events. */
    struct ui_touch_event touch;            /**< Data for touch events. */
    struct ui_pen_event pen;                /**< Data for pen events. */
    struct ui_gamepad_event gamepad;        /**< Data for gamepad events. */
    struct ui_window_event window;          /**< Data for window events. */
    struct ui_os_deep_link_event deep_link; /**< Data for deep link events. */
  } event_data;
};

/**
 * @brief State tracking for mouse interaction.
 */
struct ui_mouse_state {
  struct ui_layout_node
      *hovered_node;                  /**< The currently hovered layout node. */
  struct ui_layout_node *active_node; /**< The currently active layout node. */
  double
      last_click_time_ms; /**< Timestamp of the last click in milliseconds. */
  int click_count;        /**< Number of consecutive clicks. */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_EVENT_H */
