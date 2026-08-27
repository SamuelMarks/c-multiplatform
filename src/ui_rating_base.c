/*
 * \file ui_rating_base.c
 * \brief Implementation of the UI Rating Base component.
 */

/* clang-format off */
#include "ui_rating_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @struct ui_rating_base
 * \brief Internal structure representing a rating component.
 */
struct ui_rating_base {
  unsigned int max_rating;         /**< Maximum rating (number of items) */
  float value;                     /**< Current fractional rating value */
  struct ui_icon_base *full_icon;  /**< Icon for full state */
  struct ui_icon_base *half_icon;  /**< Icon for half state */
  struct ui_icon_base *empty_icon; /**< Icon for empty state */

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data); /**< CVA change callback */
  void *cva_on_change_user_data;                /**< CVA change user data */

  ui_error_t (*cva_on_touched)(void *user_data); /**< CVA touched callback */
  void *cva_on_touched_user_data;                /**< CVA touched user data */

  int is_disabled; /**< Non-zero if disabled */
};

/*
 * \brief Triggers the CVA change callback.
 *
 * \param rating The rating component.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t trigger_cva_change(struct ui_rating_base *rating) {
  if (rating->cva_on_change) {
    union ui_signal_payload payload;
    payload.float_val = rating->value;
    return rating->cva_on_change(payload, rating->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Triggers the CVA touched callback.
 *
 * \param rating The rating component.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t trigger_cva_touched(struct ui_rating_base *rating) {
  if (rating->cva_on_touched) {
    return rating->cva_on_touched(rating->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

/*
 * \brief CVA method to write a value.
 *
 * \param component The rating component.
 * \param value The payload containing the float value.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t rating_cva_write_value(void *component,
                                         union ui_signal_payload value) {
  struct ui_rating_base *rating = (struct ui_rating_base *)component;

  if (!rating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)ui_rating_base_set_value(rating, value.float_val);
  return UI_ERROR_NONE;
}

/*
 * \brief CVA method to register an on-change callback.
 *
 * \param component The rating component.
 * \param callback The callback.
 * \param user_data Opaque user data.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t rating_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_rating_base *rating = (struct ui_rating_base *)component;
  if (!rating)
    return UI_ERROR_INVALID_ARGUMENT;
  rating->cva_on_change = callback;
  rating->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/*
 * \brief CVA method to register an on-touched callback.
 *
 * \param component The rating component.
 * \param callback The callback.
 * \param user_data Opaque user data.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t rating_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_rating_base *rating = (struct ui_rating_base *)component;
  if (!rating)
    return UI_ERROR_INVALID_ARGUMENT;
  rating->cva_on_touched = callback;
  rating->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/*
 * \brief CVA method to set the disabled state.
 *
 * \param component The rating component.
 * \param is_disabled The disabled state.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t rating_cva_set_disabled_state(void *component,
                                                int is_disabled) {
  struct ui_rating_base *rating = (struct ui_rating_base *)component;
  if (!rating)
    return UI_ERROR_INVALID_ARGUMENT;
  rating->is_disabled = is_disabled;
  return UI_ERROR_NONE;
}

/*
 * \brief Creates a new unstyled rating component.
 *
 * \param out_rating Pointer to receive the allocated rating base.
 * \param out_cva Optional pointer to receive the CVA interface.
 * \return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_rating_base_create(struct ui_rating_base **out_rating,
                                 struct ui_control_value_accessor *out_cva) {
  struct ui_rating_base *rating;
  ui_error_t rc;

  if (!out_rating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rating = (struct ui_rating_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_rating_base));
  if (!rating) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rating->max_rating = 5; /* Default max rating */
  rating->value = 0.0f;
  rating->full_icon = NULL;
  rating->half_icon = NULL;
  rating->empty_icon = NULL;
  rating->cva_on_change = NULL;
  rating->cva_on_change_user_data = NULL;
  rating->cva_on_touched = NULL;
  rating->cva_on_touched_user_data = NULL;
  rating->is_disabled = 0;

  rc = ui_icon_base_create(&rating->full_icon);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_icon_base_create(&rating->half_icon);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_icon_base_create(&rating->empty_icon);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  if (out_cva) {
    out_cva->write_value = rating_cva_write_value;
    out_cva->register_on_change = rating_cva_register_on_change;
    out_cva->register_on_touched = rating_cva_register_on_touched;
    out_cva->set_disabled_state = rating_cva_set_disabled_state;
  }

  *out_rating = rating;
  return UI_ERROR_NONE;

cleanup:
  (void)ui_rating_base_destroy(rating);
  return rc;
}

/*
 * \brief Destroys a rating component and frees all resources.
 *
 * \param rating The rating component to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_rating_base_destroy(struct ui_rating_base *rating) {
  if (!rating) {
    return UI_ERROR_NONE;
  }

  if (rating->full_icon) {
    (void)ui_icon_base_destroy(rating->full_icon);
  }
  if (rating->half_icon) {
    (void)ui_icon_base_destroy(rating->half_icon);
  }
  if (rating->empty_icon) {
    (void)ui_icon_base_destroy(rating->empty_icon);
  }

  C_MULTIPLATFORM_FREE(rating);
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the maximum rating bound (number of items/stars).
 *
 * \param rating The rating component.
 * \param max_rating The maximum rating (must be > 0).
 * \return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_set_max(struct ui_rating_base *rating,
                                  unsigned int max_rating) {
  if (!rating || max_rating == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  rating->max_rating = max_rating;
  if (rating->value > (float)max_rating) {
    rating->value = (float)max_rating;
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Gets the maximum rating bound.
 *
 * \param rating The rating component.
 * \param out_max Pointer to receive the maximum rating.
 * \return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_get_max(const struct ui_rating_base *rating,
                                  unsigned int *out_max) {
  if (!rating || !out_max) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_max = rating->max_rating;
  return UI_ERROR_NONE;
}

/*
 * \brief Sets the current fractional rating value.
 *
 * The value will be clamped between 0.0 and max_rating.
 *
 * \param rating The rating component.
 * \param value The rating value.
 * \return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_set_value(struct ui_rating_base *rating,
                                    float value) {
  float old_val;

  if (!rating || value < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  old_val = rating->value;

  if (value > (float)rating->max_rating) {
    rating->value = (float)rating->max_rating;
  } else {
    rating->value = value;
  }

  if (rating->value != old_val) {
    (void)trigger_cva_change(rating);
    (void)trigger_cva_touched(rating);
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Gets the current fractional rating value.
 *
 * \param rating The rating component.
 * \param out_value Pointer to receive the rating value.
 * \return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_get_value(const struct ui_rating_base *rating,
                                    float *out_value) {
  if (!rating || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_value = rating->value;
  return UI_ERROR_NONE;
}

/*
 * \brief Gets the fraction to render for a given item index based on the
 * current rating.
 *
 * This function calculates whether the item at the zero-based index should be
 * rendered as full (1.0), empty (0.0), or fractional (between 0.0 and 1.0).
 *
 * \param rating The rating component.
 * \param index The zero-based index of the item (must be < max_rating).
 * \param out_fraction Pointer to receive the fraction.
 * \return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters or index out of bounds.
 */
ui_error_t ui_rating_base_get_item_fraction(const struct ui_rating_base *rating,
                                            unsigned int index,
                                            float *out_fraction) {
  float item_start;
  float item_end;

  if (!rating || !out_fraction || index >= rating->max_rating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item_start = (float)index;
  item_end = item_start + 1.0f;

  if (rating->value >= item_end) {
    *out_fraction = 1.0f;
  } else if (rating->value <= item_start) {
    *out_fraction = 0.0f;
  } else {
    *out_fraction = rating->value - item_start;
  }

  return UI_ERROR_NONE;
}

/*
 * \brief Retrieves the icon component used to represent a fully filled rating
 * item.
 *
 * The returned icon is owned by the rating component and can be modified
 * directly.
 *
 * \param rating The rating component.
 * \param out_icon Pointer to receive the icon.
 * \return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_get_full_icon(struct ui_rating_base *rating,
                                        struct ui_icon_base **out_icon) {
  if (!rating || !out_icon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_icon = rating->full_icon;
  return UI_ERROR_NONE;
}

/*
 * \brief Retrieves the icon component used to represent a partially filled
 * (half) rating item.
 *
 * The returned icon is owned by the rating component and can be modified
 * directly.
 *
 * \param rating The rating component.
 * \param out_icon Pointer to receive the icon.
 * \return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_get_half_icon(struct ui_rating_base *rating,
                                        struct ui_icon_base **out_icon) {
  if (!rating || !out_icon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_icon = rating->half_icon;
  return UI_ERROR_NONE;
}

/*
 * \brief Retrieves the icon component used to represent an empty rating item.
 *
 * The returned icon is owned by the rating component and can be modified
 * directly.
 *
 * \param rating The rating component.
 * \param out_icon Pointer to receive the icon.
 * \return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_get_empty_icon(struct ui_rating_base *rating,
                                         struct ui_icon_base **out_icon) {
  if (!rating || !out_icon) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_icon = rating->empty_icon;
  return UI_ERROR_NONE;
}
