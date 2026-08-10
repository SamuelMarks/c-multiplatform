/* clang-format off */
#include "ui_css_ui.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "strtok_posix.h"
/* clang-format on */

#if defined(_MSC_VER)
#define UI_STRTOK(str, delim, ctx) strtok_s((str), (delim), (ctx))
#else
#define UI_STRTOK(str, delim, ctx) strtok_r((str), (delim), (ctx))
#endif

static ui_error_t skip_whitespace(const char **p_str) {
  while (isspace((unsigned char)**p_str)) {
    (*p_str)++;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_css_parse_outline_width(const char *str,
                                      struct ui_css_value *out_width) {
  if (!str || !out_width)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    ui_error_t sw_rc = skip_whitespace(&str);
    (void)sw_rc;
  }

  if (strcmp(str, "thin") == 0) {
    out_width->unit = UI_CSS_UNIT_PX;
    out_width->value = 1.0f;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "medium") == 0) {
    out_width->unit = UI_CSS_UNIT_PX;
    out_width->value = 3.0f;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "thick") == 0) {
    out_width->unit = UI_CSS_UNIT_PX;
    out_width->value = 5.0f;
    return UI_ERROR_NONE;
  }

  return ui_css_parse_value(str, out_width);
}

ui_error_t ui_css_parse_outline_style(const char *str,
                                      enum ui_css_outline_style *out_style) {
  if (!str || !out_style)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    ui_error_t sw_rc = skip_whitespace(&str);
    (void)sw_rc;
  }

  if (strcmp(str, "none") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_NONE;
  else if (strcmp(str, "hidden") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_HIDDEN;
  else if (strcmp(str, "dotted") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_DOTTED;
  else if (strcmp(str, "dashed") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_DASHED;
  else if (strcmp(str, "solid") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_SOLID;
  else if (strcmp(str, "double") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_DOUBLE;
  else if (strcmp(str, "groove") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_GROOVE;
  else if (strcmp(str, "ridge") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_RIDGE;
  else if (strcmp(str, "inset") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_INSET;
  else if (strcmp(str, "outset") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_OUTSET;
  else if (strcmp(str, "auto") == 0)
    *out_style = UI_CSS_OUTLINE_STYLE_AUTO;
  else
    return UI_ERROR_PARSE_FAILED;

  return UI_ERROR_NONE;
}

ui_error_t ui_css_parse_outline(const char *str,
                                struct ui_css_outline *out_outline) {
  char token_buf[256];
  char *token;
  char *next_token = NULL;
  size_t len;
  int has_width = 0;
  int has_style = 0;
  int has_color = 0;

  if (!str || !out_outline) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_outline->width.unit = UI_CSS_UNIT_PX;
  out_outline->width.value = 0.0f;
  out_outline->style = UI_CSS_OUTLINE_STYLE_NONE;
  out_outline->color.space = UI_CSS_COLOR_SPACE_SRGB;
  out_outline->color.components[0] = 0.0f;
  out_outline->color.components[1] = 0.0f;
  out_outline->color.components[2] = 0.0f;
  out_outline->color.components[3] = 1.0f;
  out_outline->offset.unit = UI_CSS_UNIT_PX;
  out_outline->offset.value = 0.0f;
  out_outline->is_color_invert = 0;

  {
    ui_error_t sw_rc = skip_whitespace(&str);
    (void)sw_rc;
  }

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  len = strlen(str);
  if (len >= sizeof(token_buf)) {
    len = sizeof(token_buf) - 1;
  }
  memcpy(token_buf, str, len);
  token_buf[len] = '\0';

  token = UI_STRTOK(token_buf, " ", &next_token);
  while (token) {
    if (!has_style) {
      ui_error_t st_rc = ui_css_parse_outline_style(token, &out_outline->style);
      (void)st_rc;
      if (st_rc == UI_ERROR_NONE) {
        has_style = 1;
        token = UI_STRTOK(NULL, " ", &next_token);
        continue;
      }
    }
    if (!has_width) {
      ui_error_t w_rc = ui_css_parse_outline_width(token, &out_outline->width);
      (void)w_rc;
      if (w_rc == UI_ERROR_NONE) {
        has_width = 1;
        token = UI_STRTOK(NULL, " ", &next_token);
        continue;
      }
    }
    if (!has_color) {
      if (strcmp(token, "invert") == 0) {
        out_outline->is_color_invert = 1;
        has_color = 1;
      } else {
        ui_error_t c_rc = ui_css_parse_color(token, &out_outline->color);
        if (c_rc != UI_ERROR_NONE) {
          return c_rc;
        }
        has_color = 1;
      }
    } else {
      return UI_ERROR_PARSE_FAILED;
    }
    token = UI_STRTOK(NULL, " ", &next_token);
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_css_cursor_destroy(struct ui_css_cursor *cursor) {
  if (!cursor)
    return UI_ERROR_INVALID_ARGUMENT;

  if (cursor->images) {
    struct ui_css_cursor_image *current = cursor->images;
    while (current) {
      struct ui_css_cursor_image *next = current->next;
      C_MULTIPLATFORM_FREE(current);
      current = next;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_css_parse_cursor(const char *str,
                               struct ui_css_cursor *out_cursor) {
  char token_buf[1024];
  char *token;
  char *next_token = NULL;
  struct ui_css_cursor_image *head = NULL;
  struct ui_css_cursor_image *tail = NULL;
  size_t len;

  if (!str || !out_cursor)
    return UI_ERROR_INVALID_ARGUMENT;

  out_cursor->images = NULL;
  out_cursor->keyword = UI_CSS_CURSOR_AUTO;

  {
    ui_error_t sw_rc = skip_whitespace(&str);
    (void)sw_rc;
  }

  len = strlen(str);
  if (len >= sizeof(token_buf)) {
    len = sizeof(token_buf) - 1;
  }
  memcpy(token_buf, str, len);
  token_buf[len] = '\0';

  token = UI_STRTOK(token_buf, ",", &next_token);
  while (token) {
    char *p = token;
    while (isspace((unsigned char)*p))
      p++;

    if (strncmp(p, "url(", 4) == 0 || strncmp(p, "image-set(", 10) == 0 ||
        strncmp(p, "-webkit-image-set(", 18) == 0) {
      struct ui_css_cursor_image *img =
          (struct ui_css_cursor_image *)C_MULTIPLATFORM_MALLOC(
              sizeof(struct ui_css_cursor_image));
      if (!img) {
        ui_error_t destroy_rc;
        out_cursor->images = head;
        ui_css_cursor_destroy(out_cursor);
        return UI_ERROR_OUT_OF_MEMORY;
      }
      img->has_hotspot = 0;
      img->hotspot_x = 0.0f;
      img->hotspot_y = 0.0f;
      img->next = NULL;

      /* Could have x y appended like: url(cur.png) 10 10 */
      {
        char *close_paren = strrchr(p, ')');
        if (close_paren) {
          char url_str[512];
          size_t ulen = close_paren - p + 1;
          ui_error_t img_rc;
          if (ulen >= sizeof(url_str)) {
            ulen = sizeof(url_str) - 1;
          }
          memcpy(url_str, p, ulen);
          url_str[ulen] = '\0';

          img_rc = ui_css_parse_image(url_str, &img->image);
          if (img_rc != UI_ERROR_NONE) {
            if (0)
              return img_rc;
            C_MULTIPLATFORM_FREE(img);
            token = UI_STRTOK(NULL, ",", &next_token);
            continue;
          }
          {
            char *coords = close_paren + 1;
            while (isspace((unsigned char)*coords))
              coords++;
            if (*coords) {
              if (sscanf(coords, "%f %f", &img->hotspot_x, &img->hotspot_y) ==
                  2) {
                img->has_hotspot = 1;
              }
            }

            if (tail)
              tail->next = img;
            else
              head = img;
            tail = img;
          }
        } else {
          C_MULTIPLATFORM_FREE(img);
        }
      }
    } else {
      /* Keyword */
      if (strcmp(p, "auto") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_AUTO;
      else if (strcmp(p, "default") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_DEFAULT;
      else if (strcmp(p, "none") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_NONE;
      else if (strcmp(p, "context-menu") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_CONTEXT_MENU;
      else if (strcmp(p, "help") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_HELP;
      else if (strcmp(p, "pointer") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_POINTER;
      else if (strcmp(p, "progress") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_PROGRESS;
      else if (strcmp(p, "wait") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_WAIT;
      else if (strcmp(p, "cell") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_CELL;
      else if (strcmp(p, "crosshair") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_CROSSHAIR;
      else if (strcmp(p, "text") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_TEXT;
      else if (strcmp(p, "vertical-text") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_VERTICAL_TEXT;
      else if (strcmp(p, "alias") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_ALIAS;
      else if (strcmp(p, "copy") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_COPY;
      else if (strcmp(p, "move") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_MOVE;
      else if (strcmp(p, "no-drop") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_NO_DROP;
      else if (strcmp(p, "not-allowed") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_NOT_ALLOWED;
      else if (strcmp(p, "grab") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_GRAB;
      else if (strcmp(p, "grabbing") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_GRABBING;
      else if (strcmp(p, "e-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_E_RESIZE;
      else if (strcmp(p, "n-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_N_RESIZE;
      else if (strcmp(p, "ne-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_NE_RESIZE;
      else if (strcmp(p, "nw-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_NW_RESIZE;
      else if (strcmp(p, "s-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_S_RESIZE;
      else if (strcmp(p, "se-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_SE_RESIZE;
      else if (strcmp(p, "sw-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_SW_RESIZE;
      else if (strcmp(p, "w-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_W_RESIZE;
      else if (strcmp(p, "ew-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_EW_RESIZE;
      else if (strcmp(p, "ns-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_NS_RESIZE;
      else if (strcmp(p, "nesw-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_NESW_RESIZE;
      else if (strcmp(p, "nwse-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_NWSE_RESIZE;
      else if (strcmp(p, "col-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_COL_RESIZE;
      else if (strcmp(p, "row-resize") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_ROW_RESIZE;
      else if (strcmp(p, "all-scroll") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_ALL_SCROLL;
      else if (strcmp(p, "zoom-in") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_ZOOM_IN;
      else if (strcmp(p, "zoom-out") == 0)
        out_cursor->keyword = UI_CSS_CURSOR_ZOOM_OUT;
      else {
        ui_error_t destroy_rc;
        out_cursor->images = head;
        ui_css_cursor_destroy(out_cursor);
        return UI_ERROR_PARSE_FAILED;
      }
    }

    token = UI_STRTOK(NULL, ",", &next_token);
  }

  out_cursor->images = head;
  return UI_ERROR_NONE;
}

ui_error_t ui_css_parse_user_select(const char *str,
                                    enum ui_css_user_select *out_select) {
  if (!str || !out_select)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    ui_error_t sw_rc = skip_whitespace(&str);
    (void)sw_rc;
  }

  if (strcmp(str, "auto") == 0)
    *out_select = UI_CSS_USER_SELECT_AUTO;
  else if (strcmp(str, "none") == 0)
    *out_select = UI_CSS_USER_SELECT_NONE;
  else if (strcmp(str, "text") == 0)
    *out_select = UI_CSS_USER_SELECT_TEXT;
  else if (strcmp(str, "all") == 0)
    *out_select = UI_CSS_USER_SELECT_ALL;
  else if (strcmp(str, "contain") == 0)
    *out_select = UI_CSS_USER_SELECT_CONTAIN;
  else
    return UI_ERROR_PARSE_FAILED;

  return UI_ERROR_NONE;
}

ui_error_t ui_css_parse_appearance(const char *str,
                                   enum ui_css_appearance *out_appearance) {
  if (!str || !out_appearance)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    ui_error_t sw_rc = skip_whitespace(&str);
    (void)sw_rc;
  }

  if (strcmp(str, "none") == 0)
    *out_appearance = UI_CSS_APPEARANCE_NONE;
  else if (strcmp(str, "auto") == 0)
    *out_appearance = UI_CSS_APPEARANCE_AUTO;
  else if (strcmp(str, "menulist-button") == 0)
    *out_appearance = UI_CSS_APPEARANCE_MENULIST_BUTTON;
  else if (strcmp(str, "textfield") == 0)
    *out_appearance = UI_CSS_APPEARANCE_TEXTFIELD;
  else if (strcmp(str, "button") == 0)
    *out_appearance = UI_CSS_APPEARANCE_BUTTON;
  else if (strcmp(str, "searchfield") == 0)
    *out_appearance = UI_CSS_APPEARANCE_SEARCHFIELD;
  else if (strcmp(str, "textarea") == 0)
    *out_appearance = UI_CSS_APPEARANCE_TEXTAREA;
  else if (strcmp(str, "push-button") == 0)
    *out_appearance = UI_CSS_APPEARANCE_PUSH_BUTTON;
  else if (strcmp(str, "slider-horizontal") == 0)
    *out_appearance = UI_CSS_APPEARANCE_SLIDER_HORIZONTAL;
  else if (strcmp(str, "checkbox") == 0)
    *out_appearance = UI_CSS_APPEARANCE_CHECKBOX;
  else if (strcmp(str, "radio") == 0)
    *out_appearance = UI_CSS_APPEARANCE_RADIO;
  else if (strcmp(str, "square-button") == 0)
    *out_appearance = UI_CSS_APPEARANCE_SQUARE_BUTTON;
  else if (strcmp(str, "menulist") == 0)
    *out_appearance = UI_CSS_APPEARANCE_MENULIST;
  else if (strcmp(str, "listbox") == 0)
    *out_appearance = UI_CSS_APPEARANCE_LISTBOX;
  else if (strcmp(str, "meter") == 0)
    *out_appearance = UI_CSS_APPEARANCE_METER;
  else if (strcmp(str, "progress-bar") == 0)
    *out_appearance = UI_CSS_APPEARANCE_PROGRESS_BAR;
  else
    return UI_ERROR_PARSE_FAILED;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_css_parse_pointer_events(const char *str,
                                       enum ui_css_pointer_events *out_events) {
  if (!str || !out_events)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    ui_error_t sw_rc = skip_whitespace(&str);
    (void)sw_rc;
  }

  if (strcmp(str, "auto") == 0)
    *out_events = UI_CSS_POINTER_EVENTS_AUTO;
  else if (strcmp(str, "none") == 0)
    *out_events = UI_CSS_POINTER_EVENTS_NONE;
  else if (strcmp(str, "visiblePainted") == 0)
    *out_events = UI_CSS_POINTER_EVENTS_VISIBLE_PAINTED;
  else if (strcmp(str, "visibleFill") == 0)
    *out_events = UI_CSS_POINTER_EVENTS_VISIBLE_FILL;
  else if (strcmp(str, "visibleStroke") == 0)
    *out_events = UI_CSS_POINTER_EVENTS_VISIBLE_STROKE;
  else if (strcmp(str, "visible") == 0)
    *out_events = UI_CSS_POINTER_EVENTS_VISIBLE;
  else if (strcmp(str, "painted") == 0)
    *out_events = UI_CSS_POINTER_EVENTS_PAINTED;
  else if (strcmp(str, "fill") == 0)
    *out_events = UI_CSS_POINTER_EVENTS_FILL;
  else if (strcmp(str, "stroke") == 0)
    *out_events = UI_CSS_POINTER_EVENTS_STROKE;
  else if (strcmp(str, "all") == 0)
    *out_events = UI_CSS_POINTER_EVENTS_ALL;
  else
    return UI_ERROR_PARSE_FAILED;

  return UI_ERROR_NONE;
}

/** \brief ui_css_parse_overscroll_behavior_axis */
ui_error_t ui_css_parse_overscroll_behavior_axis(
    const char *str, enum ui_css_overscroll_behavior *out_behavior) {
  if (!str || !out_behavior)
    return UI_ERROR_INVALID_ARGUMENT;

  {
    ui_error_t sw_rc = skip_whitespace(&str);
    (void)sw_rc;
  }

  if (strcmp(str, "auto") == 0) {
    *out_behavior = UI_CSS_OVERSCROLL_BEHAVIOR_AUTO;
  } else if (strcmp(str, "contain") == 0) {
    *out_behavior = UI_CSS_OVERSCROLL_BEHAVIOR_CONTAIN;
  } else if (strcmp(str, "none") == 0) {
    *out_behavior = UI_CSS_OVERSCROLL_BEHAVIOR_NONE;
  } else {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_css_parse_overscroll_behavior */
ui_error_t ui_css_parse_overscroll_behavior(
    const char *str,
    struct ui_css_overscroll_behavior_shorthand *out_shorthand) {
  char buffer[256];
  char *token;
  char *next_token = NULL;
  enum ui_css_overscroll_behavior parsed_x = UI_CSS_OVERSCROLL_BEHAVIOR_AUTO;
  enum ui_css_overscroll_behavior parsed_y = UI_CSS_OVERSCROLL_BEHAVIOR_AUTO;
  int count = 0;
  ui_error_t err;
  size_t len;

  if (!str || !out_shorthand)
    return UI_ERROR_INVALID_ARGUMENT;

  len = strlen(str);
  if (len >= sizeof(buffer)) {
    len = sizeof(buffer) - 1;
  }
  memcpy(buffer, str, len);
  buffer[len] = '\0';

  token = UI_STRTOK(buffer, " \t\r\n", &next_token);
  while (token != NULL) {
    if (count == 0) {
      err = ui_css_parse_overscroll_behavior_axis(token, &parsed_x);
      if (err != UI_ERROR_NONE)
        return err;
      count++;
    } else if (count == 1) {
      err = ui_css_parse_overscroll_behavior_axis(token, &parsed_y);
      if (err != UI_ERROR_NONE)
        return err;
      count++;
    } else {
      return UI_ERROR_PARSE_FAILED; /* Too many values */
    }
    token = UI_STRTOK(NULL, " \t\r\n", &next_token);
  }

  if (count == 0) {
    return UI_ERROR_PARSE_FAILED;
  } else if (count == 1) {
    parsed_y = parsed_x;
  }

  out_shorthand->x = parsed_x;
  out_shorthand->y = parsed_y;

  return UI_ERROR_NONE;
}
