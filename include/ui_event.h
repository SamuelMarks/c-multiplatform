#ifndef UI_EVENT_H
#define UI_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Types of events.
 */
enum ui_event_type {
  UI_EVENT_NONE = 0,
  UI_EVENT_WINDOW_CLOSE,
  UI_EVENT_WINDOW_RESIZE,
  UI_EVENT_MOUSE_DOWN,
  UI_EVENT_MOUSE_UP,
  UI_EVENT_MOUSE_MOVE,
  UI_EVENT_MOUSE_WHEEL,
  UI_EVENT_MOUSE_ENTER,
  UI_EVENT_MOUSE_LEAVE,
  UI_EVENT_CLICK,
  UI_EVENT_DBLCLICK,
  UI_EVENT_CONTEXT_MENU,
  UI_EVENT_KEY_PRESS,
  UI_EVENT_KEY_DOWN,
  UI_EVENT_KEY_UP,
  UI_EVENT_TOUCH_START,
  UI_EVENT_TOUCH_MOVE,
  UI_EVENT_TOUCH_END,
  UI_EVENT_TOUCH_CANCEL,
  UI_EVENT_PEN_DOWN,
  UI_EVENT_PEN_UP,
  UI_EVENT_PEN_MOVE,
  UI_EVENT_GAMEPAD_BUTTON_DOWN,
  UI_EVENT_GAMEPAD_BUTTON_UP,
  UI_EVENT_GAMEPAD_AXIS,
  UI_EVENT_OS_DEEP_LINK
};

/**
 * @brief Modifiers for input events.
 */
enum ui_modifier_flags {
  UI_MODIFIER_NONE = 0,
  UI_MODIFIER_SHIFT = 1 << 0,
  UI_MODIFIER_CTRL = 1 << 1,
  UI_MODIFIER_ALT = 1 << 2,
  UI_MODIFIER_SUPER = 1 << 3
};

/**
 * @brief Mouse event data.
 */
struct ui_mouse_event {
  int x;
  int y;
  int button; /* 0: Left, 1: Right, 2: Middle */
  float wheel_x;
  float wheel_y;
  unsigned int modifiers;
};

/**
 * @brief Common key codes for cross-platform mapping.
 */
enum ui_key_code {
  UI_KEY_UNKNOWN = 0,
  UI_KEY_SPACE = 32,
  UI_KEY_ENTER = 257,
  UI_KEY_ESCAPE = 258,
  UI_KEY_BACKSPACE = 259,
  UI_KEY_TAB = 260,
  UI_KEY_UP = 261,
  UI_KEY_DOWN = 262,
  UI_KEY_LEFT = 263,
  UI_KEY_RIGHT = 264,
  UI_KEY_HOME = 265,
  UI_KEY_END = 266
};

/**
 * @brief Keyboard event data.
 */
struct ui_keyboard_event {
  int key_code;
  int scan_code;
  unsigned int modifiers;
  int is_repeat;
};

/**
 * @brief Single touch point data.
 */
struct ui_touch_point {
  int id;
  int x;
  int y;
  float pressure;
};

#define UI_MAX_TOUCH_POINTS 10

/**
 * @brief Touch event data.
 */
struct ui_touch_event {
  struct ui_touch_point points[UI_MAX_TOUCH_POINTS];
  int num_points;
  unsigned int modifiers;
};

/**
 * @brief Pen/Stylus event data.
 */
struct ui_pen_event {
  int x;
  int y;
  float pressure;
  float tilt_x;
  float tilt_y;
  unsigned int modifiers;
};

/**
 * @brief Gamepad event data.
 */
struct ui_gamepad_event {
  int gamepad_id;
  int button;
  int axis;
  float value;
};

/**
 * @brief Window event data.
 */
struct ui_window_event {
  int width;
  int height;
};

/**
 * @brief OS Deep Link event data.
 */
struct ui_os_deep_link_event {
  char uri[1024];
};

/**
 * @brief Unified input event structure.
 */
struct ui_event {
  enum ui_event_type type;
  /** \brief union */
  union {
    struct ui_mouse_event mouse;
    struct ui_keyboard_event keyboard;
    struct ui_touch_event touch;
    struct ui_pen_event pen;
    struct ui_gamepad_event gamepad;
    struct ui_window_event window;
    struct ui_os_deep_link_event deep_link;
  } event_data;
};

struct ui_mouse_state {
  struct ui_layout_node *hovered_node;
  struct ui_layout_node *active_node;
  double last_click_time_ms;
  int click_count;
};
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_EVENT_H */
