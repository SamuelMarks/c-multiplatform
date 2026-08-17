/* clang-format off */
#include "ui_syntax_surface_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
#include <math.h>
/* clang-format on */

/**
 * \file ui_syntax_surface_base.c
 * \brief Syntax surface base component implementation.
 */

#define UI_SYNTAX_MAX_FOLDS 256

/**
 * \brief ui_syntax_surface_base structure.
 * \details Internal state for the syntax surface base component.
 */
struct ui_syntax_surface_base {
  struct ui_arena *arena;
  struct ui_rich_text_base *base_rich_text;

  struct ui_syntax_fold_region folds[UI_SYNTAX_MAX_FOLDS];
  int num_folds;

  int active_line;
  struct ui_syntax_bracket_match bracket_match;

  ui_signal_t *fold_changed_signal;
  ui_signal_t *active_line_signal;
};

/**
 * \brief Equality function for void pointers.
 * \param a First payload.
 * \param b Second payload.
 * \param out_equal Pointer to store the result.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t void_equality(union ui_signal_payload a,
                                union ui_signal_payload b,
                                ui_bool_t *out_equal) {
  (void)a;
  (void)b;
  *out_equal = UI_FALSE; /* Always trigger fold signal */
  return UI_ERROR_NONE;
}

/**
 * \brief Equality function for integers.
 * \param a First payload.
 * \param b Second payload.
 * \param out_equal Pointer to store the result.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t int_equality(union ui_signal_payload a,
                               union ui_signal_payload b,
                               ui_bool_t *out_equal) {
  *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a new syntax surface base component.
 * \param arena The arena to allocate from.
 * \param base_rich_text The base rich text component.
 * \param out_surface Pointer to store the component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_syntax_surface_base_create(struct ui_arena *arena,
                              struct ui_rich_text_base *base_rich_text,
                              struct ui_syntax_surface_base **out_surface) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !out_surface) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_syntax_surface_base), 8, &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  *out_surface = (struct ui_syntax_surface_base *)ptr;
  (*out_surface)->arena = arena;
  (*out_surface)->base_rich_text = base_rich_text;
  (*out_surface)->num_folds = 0;
  (*out_surface)->active_line = -1;

  (*out_surface)->bracket_match.has_match = UI_FALSE;

  initial_payload.ptr_val = NULL;
  err = ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_POINTER,
                         void_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_surface)->fold_changed_signal);
  if (err != UI_ERROR_NONE)
    return err;

  initial_payload.int_val = -1;
  err = ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_INT32,
                         int_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_surface)->active_line_signal);
  if (err != UI_ERROR_NONE)
    return err;

  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a syntax surface base component.
 * \param surface The component to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_syntax_surface_base_destroy(struct ui_syntax_surface_base *surface) {
  if (!surface) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_signal_destroy(surface->fold_changed_signal);
  (void)ui_signal_destroy(surface->active_line_signal);

  return UI_ERROR_NONE;
}

/**
 * \brief Calculates the width of the line number gutter.
 * \param surface The syntax surface component.
 * \param total_lines The total number of lines.
 * \param char_width The width of a single character.
 * \param out_width Pointer to store the calculated width.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_syntax_surface_base_calculate_gutter_width(
    const struct ui_syntax_surface_base *surface, int total_lines,
    float char_width, float *out_width) {
  int num_digits = 1;
  int temp = total_lines;

  if (!surface || !out_width || total_lines < 0 || char_width < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Calculate number of digits required to display total_lines */
  while (temp >= 10) {
    temp /= 10;
    num_digits++;
  }

  /* Base width + small padding */
  *out_width = (num_digits * char_width) + (char_width * 2.0f);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets a fold region.
 * \param surface The syntax surface component.
 * \param start_line The start line of the fold.
 * \param end_line The end line of the fold.
 * \param collapse Whether the fold is collapsed.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_syntax_surface_base_set_fold_region(struct ui_syntax_surface_base *surface,
                                       int start_line, int end_line,
                                       ui_bool_t collapse) {
  int i;
  union ui_signal_payload payload;

  if (!surface || start_line < 0 || end_line <= start_line) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Find existing fold */
  for (i = 0; i < surface->num_folds; ++i) {
    if (surface->folds[i].start_line == start_line) {
      surface->folds[i].end_line = end_line;
      surface->folds[i].is_collapsed = collapse;
      payload.ptr_val = NULL;
      return ui_signal_set(surface->fold_changed_signal, payload);
    }
  }

  /* Create new fold */
  if (surface->num_folds >= UI_SYNTAX_MAX_FOLDS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  surface->folds[surface->num_folds].start_line = start_line;
  surface->folds[surface->num_folds].end_line = end_line;
  surface->folds[surface->num_folds].is_collapsed = collapse;
  surface->num_folds++;

  payload.ptr_val = NULL;
  return ui_signal_set(surface->fold_changed_signal, payload);
}

/**
 * \brief Gets the fold changed signal.
 * \param surface The syntax surface component.
 * \param out_signal Pointer to store the signal.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_syntax_surface_base_get_fold_changed_signal(
    struct ui_syntax_surface_base *surface, ui_signal_t **out_signal) {
  if (!surface || !out_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_signal = surface->fold_changed_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets the visual line index for a given absolute line.
 * \param surface The syntax surface component.
 * \param absolute_line The absolute line number.
 * \param out_visual_index Pointer to store the visual line index.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_syntax_surface_base_get_visual_line_index(
    const struct ui_syntax_surface_base *surface, int absolute_line,
    int *out_visual_index) {
  int i;
  int visual_index = absolute_line;

  if (!surface || !out_visual_index || absolute_line < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Subtract lines hidden within collapsed folds that appear BEFORE
   * absolute_line */
  for (i = 0; i < surface->num_folds; ++i) {
    if (surface->folds[i].is_collapsed) {
      /* If the requested line is inside a collapsed fold (not the header), it
       * is hidden */
      if (absolute_line > surface->folds[i].start_line &&
          absolute_line <= surface->folds[i].end_line) {
        *out_visual_index = -1;
        return UI_ERROR_NONE;
      }

      /* If the fold occurs entirely before the requested line, subtract the
       * folded length */
      if (surface->folds[i].end_line < absolute_line) {
        visual_index -=
            (surface->folds[i].end_line - surface->folds[i].start_line);
      }
    }
  }

  *out_visual_index = visual_index;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the active line.
 * \param surface The syntax surface component.
 * \param absolute_line The absolute line number.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_syntax_surface_base_set_active_line(struct ui_syntax_surface_base *surface,
                                       int absolute_line) {
  union ui_signal_payload payload;

  if (!surface) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  surface->active_line = absolute_line;
  payload.int_val = absolute_line;
  return ui_signal_set(surface->active_line_signal, payload);
}

/**
 * \brief Gets the active line signal.
 * \param surface The syntax surface component.
 * \param out_signal Pointer to store the signal.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_syntax_surface_base_get_active_line_signal(
    struct ui_syntax_surface_base *surface, ui_signal_t **out_signal) {
  if (!surface || !out_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_signal = surface->active_line_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the bracket match region.
 * \param surface The syntax surface component.
 * \param match The bracket match info.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_syntax_surface_base_set_bracket_match(
    struct ui_syntax_surface_base *surface,
    const struct ui_syntax_bracket_match *match) {
  if (!surface || !match) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  surface->bracket_match = *match;
  return UI_ERROR_NONE;
}
