ui_error_t ui_css_parse_transition(const char *str,
                                   struct ui_css_transition **out_transitions) {
  struct ui_css_transition *head = NULL;
  struct ui_css_transition *tail = NULL;

  if (!str || !out_transitions)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_transitions = NULL;
  { skip_whitespace(&str); }

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE; /* NULL list represents none */
  }

  while (*str) {
    struct ui_css_transition *tr = NULL;
    char segment[512];
    const char *comma = str;
    size_t len;
    const char *p;
    int time_count = 0;
    int paren_depth = 0;

    /* Find next comma not inside parentheses */
    while (*comma) {
      if (*comma == '(')
        paren_depth++;
      else if (*comma == ')')
        paren_depth--;
      else if (*comma == ',' && paren_depth == 0)
        break;
      comma++;
    }

    len = (size_t)(comma - str);

    if (len >= sizeof(segment)) {
      len = sizeof(segment) - 1;
    }
    memcpy(segment, str, len);
    segment[len] = '\0';

    tr = (struct ui_css_transition *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_css_transition));
    if (!tr) {
      {
        ui_css_transition_destroy(head);
      }
      return UI_ERROR_OUT_OF_MEMORY;
    }

    /* Defaults for unspecified components */
    UI_STRCPY(tr->property_name, sizeof(tr->property_name), "all");
    tr->duration.value = 0.0f;
    tr->duration.unit = UI_CSS_UNIT_S;
    tr->delay.value = 0.0f;
    tr->delay.unit = UI_CSS_UNIT_S;
    tr->timing_function.type = UI_CSS_EASING_EASE;
    tr->next = NULL;

    p = segment;
    while (*p) {
      {
        skip_whitespace(&p);
      }
      if (!*p)
        break;

      /* Check for complex easing functions which might contain spaces */
      if (strncmp(p, "cubic-bezier(", 13) == 0 ||
          strncmp(p, "steps(", 6) == 0) {
        const char *end = strchr(p, ')');
        if (end) {
          char easing_str[64];
          size_t e_len = (size_t)(end - p) + 1;
          ui_error_t rc;
          if (e_len >= sizeof(easing_str)) {
            e_len = sizeof(easing_str) - 1;
          }
          memcpy(easing_str, p, e_len);
          easing_str[e_len] = '\0';
          rc = ui_css_parse_easing_function(easing_str, &tr->timing_function);
          if (rc != UI_ERROR_NONE) {
            if (0)
              return rc;
          } else {
            p = end + 1;
            continue;
          }
        }
      }

      /* Simple token fallback */
      {
        struct ui_css_easing_function ef;
        char tmp_token[64];
        const char *space = p;
        size_t t_len;
        ui_error_t rc;
        while (*space && !isspace((unsigned char)*space))
          space++;
        t_len = (size_t)(space - p);
        if (t_len >= sizeof(tmp_token)) {
          t_len = sizeof(tmp_token) - 1;
        }
        memcpy(tmp_token, p, t_len);
        tmp_token[t_len] = '\0';

        /* Try matching easing function */
        rc = ui_css_parse_easing_function(tmp_token, &ef);
        if (rc != UI_ERROR_NONE) {
          if (0)
            return rc;
        } else {
          tr->timing_function = ef;
          p = space;
          continue;
        }

        /* Try parsing time (duration or delay) */
        {
          struct ui_css_value val;
          rc = ui_css_parse_value(tmp_token, &val);
          if (rc != UI_ERROR_NONE) {
            if (0)
              return rc;
          } else if (val.unit == UI_CSS_UNIT_S || val.unit == UI_CSS_UNIT_MS) {
            if (time_count == 0) {
              tr->duration = val;
            } else if (time_count == 1) {
              tr->delay = val;
            }
            time_count++;
            p = space;
            continue;
          }
        }

        /* Fallback: Must be a property name */
        memcpy(tr->property_name, tmp_token, t_len);
        tr->property_name[t_len] = '\0';
        p = space;
      }
    }

    if (tail) {
      tail->next = tr;
    } else {
      head = tr;
    }
    tail = tr;

    if (*comma == ',') {
      str = comma + 1;
    } else {
      break;
    }
  }

  *out_transitions = head;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_css_animation_destroy.
 * @param animations Parameter animations.
 * @return Return value.
 */
ui_error_t ui_css_animation_destroy(struct ui_css_animation *animations) {
  while (animations) {
    struct ui_css_animation *next = animations->next;
    C_MULTIPLATFORM_FREE(animations);
    animations = next;
  }
  return UI_ERROR_NONE;
}

/*
 * @brief ui_css_parse_animation.
 * @param str Parameter str.
 * @param out_animations Parameter out_animations.
 * @return Return value.
 */
ui_error_t ui_css_parse_animation(const char *str,
                                  struct ui_css_animation **out_animations) {
  struct ui_css_animation *head = NULL;
  struct ui_css_animation *tail = NULL;

  if (!str || !out_animations)
    return UI_ERROR_INVALID_ARGUMENT;

  *out_animations = NULL;
  { skip_whitespace(&str); }

  if (strcmp(str, "none") == 0) {
    return UI_ERROR_NONE;
  }

  while (*str) {
    struct ui_css_animation *anim = NULL;
    char segment[512];
    const char *comma = str;
    size_t len;
    const char *p;
    int time_count = 0;
    int paren_depth = 0;

    /* Find next comma not inside parentheses */
    while (*comma) {
      if (*comma == '(')
        paren_depth++;
      else if (*comma == ')')
        paren_depth--;
      else if (*comma == ',' && paren_depth == 0)
        break;
      comma++;
    }

    len = (size_t)(comma - str);

    if (len >= sizeof(segment)) {
      len = sizeof(segment) - 1;
    }
    memcpy(segment, str, len);
    segment[len] = '\0';

    anim = (struct ui_css_animation *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_css_animation));
    if (!anim) {
      {
        ui_css_animation_destroy(head);
      }
      return UI_ERROR_OUT_OF_MEMORY;
    }

    /* Defaults */
    UI_STRCPY(anim->name, sizeof(anim->name), "none");
    anim->duration.value = 0.0f;
    anim->duration.unit = UI_CSS_UNIT_S;
    anim->timing_function.type = UI_CSS_EASING_EASE;
    anim->delay.value = 0.0f;
    anim->delay.unit = UI_CSS_UNIT_S;
    anim->iteration_count = 1.0f;
    anim->direction = UI_CSS_ANIMATION_DIRECTION_NORMAL;
    anim->fill_mode = UI_CSS_ANIMATION_FILL_MODE_NONE;
    anim->play_state = UI_CSS_ANIMATION_PLAY_STATE_RUNNING;
    anim->next = NULL;

    p = segment;
    while (*p) {
      {
        skip_whitespace(&p);
      }
      if (!*p)
        break;

      if (strncmp(p, "cubic-bezier(", 13) == 0 ||
          strncmp(p, "steps(", 6) == 0) {
        const char *end = strchr(p, ')');
        if (end) {
          char easing_str[64];
          size_t e_len = (size_t)(end - p) + 1;
          ui_error_t rc;
          if (e_len >= sizeof(easing_str)) {
            e_len = sizeof(easing_str) - 1;
          }
          memcpy(easing_str, p, e_len);
          easing_str[e_len] = '\0';
          rc = ui_css_parse_easing_function(easing_str, &anim->timing_function);
          if (rc != UI_ERROR_NONE) {
            if (0)
              return rc;
          } else {
            p = end + 1;
            continue;
          }
        }
      }

      {
        struct ui_css_easing_function ef;
        char tmp_token[64];
        const char *space = p;
        size_t t_len;
        ui_error_t rc;
        while (*space && !isspace((unsigned char)*space))
          space++;
        t_len = (size_t)(space - p);
        if (t_len >= sizeof(tmp_token)) {
          t_len = sizeof(tmp_token) - 1;
        }
        memcpy(tmp_token, p, t_len);
        tmp_token[t_len] = '\0';

        rc = ui_css_parse_easing_function(tmp_token, &ef);
        if (rc != UI_ERROR_NONE) {
          if (0)
            return rc;
        } else {
          anim->timing_function = ef;
          p = space;
          continue;
        }

        if (strcmp(tmp_token, "infinite") == 0) {
          anim->iteration_count = -1.0f;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "normal") == 0) {
          /* Can be direction */
          anim->direction = UI_CSS_ANIMATION_DIRECTION_NORMAL;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "reverse") == 0) {
          anim->direction = UI_CSS_ANIMATION_DIRECTION_REVERSE;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "alternate") == 0) {
          anim->direction = UI_CSS_ANIMATION_DIRECTION_ALTERNATE;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "alternate-reverse") == 0) {
          anim->direction = UI_CSS_ANIMATION_DIRECTION_ALTERNATE_REVERSE;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "forwards") == 0) {
          anim->fill_mode = UI_CSS_ANIMATION_FILL_MODE_FORWARDS;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "backwards") == 0) {
          anim->fill_mode = UI_CSS_ANIMATION_FILL_MODE_BACKWARDS;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "both") == 0) {
          anim->fill_mode = UI_CSS_ANIMATION_FILL_MODE_BOTH;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "running") == 0) {
          anim->play_state = UI_CSS_ANIMATION_PLAY_STATE_RUNNING;
          p = space;
          continue;
        } else if (strcmp(tmp_token, "paused") == 0) {
          anim->play_state = UI_CSS_ANIMATION_PLAY_STATE_PAUSED;
          p = space;
          continue;
        }

        {
          struct ui_css_value val;
          ui_error_t val_rc = ui_css_parse_value(tmp_token, &val);
          if (val_rc != UI_ERROR_NONE) {
            if (0)
              return val_rc;
          } else {
            if (val.unit == UI_CSS_UNIT_S || val.unit == UI_CSS_UNIT_MS) {
              if (time_count == 0)
                anim->duration = val;
              else if (time_count == 1)
                anim->delay = val;
              time_count++;
              p = space;
              continue;
            } else if (val.unit == UI_CSS_UNIT_NONE) {
              anim->iteration_count = val.value;
              p = space;
              continue;
            }
          }
        }

        if (strcmp(tmp_token, "none") != 0) {
          memcpy(anim->name, tmp_token, t_len);
          anim->name[t_len] = '\0';
        }
        p = space;
      }
    }

    if (tail) {
      tail->next = anim;
    } else {
      head = anim;
    }
    tail = anim;

    if (*comma == ',') {
      str = comma + 1;
    } else {
      break;
    }
  }

  *out_animations = head;
  return UI_ERROR_NONE;
}
