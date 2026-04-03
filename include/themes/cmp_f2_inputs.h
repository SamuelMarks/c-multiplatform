#ifndef CMP_F2_INPUTS_H
#define CMP_F2_INPUTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

typedef enum cmp_f2_label_position {
  CMP_F2_LABEL_POSITION_AFTER = 0, /* Default: Right of box */
  CMP_F2_LABEL_POSITION_BEFORE     /* Left of box */
} cmp_f2_label_position_t;

/* 2.6 Checkbox */
typedef enum cmp_f2_checkbox_size {
  CMP_F2_CHECKBOX_SIZE_MEDIUM = 0, /* 16px */
  CMP_F2_CHECKBOX_SIZE_LARGE       /* 20px */
} cmp_f2_checkbox_size_t;

typedef enum cmp_f2_checkbox_state {
  CMP_F2_CHECKBOX_STATE_UNCHECKED = 0,
  CMP_F2_CHECKBOX_STATE_CHECKED,
  CMP_F2_CHECKBOX_STATE_INDETERMINATE
} cmp_f2_checkbox_state_t;

/** \brief Documented */
typedef struct cmp_f2_checkbox_s {
  /** \brief Documented */
  cmp_f2_checkbox_size_t size;
  /** \brief Documented */
  cmp_f2_checkbox_state_t state;
  /** \brief Documented */
  cmp_f2_label_position_t label_position;
  /** \brief Documented */
  int is_disabled;
} cmp_f2_checkbox_t;

/**
 * @brief Initialize a Fluent 2 checkbox_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_checkbox_create(cmp_ui_node_t **out_node, const char *label);
CMP_API int cmp_f2_checkbox_set_size(cmp_ui_node_t *node,
                                     cmp_f2_checkbox_size_t size);
CMP_API int cmp_f2_checkbox_set_state(cmp_ui_node_t *node,
                                      cmp_f2_checkbox_state_t state);
CMP_API int cmp_f2_checkbox_set_label_position(cmp_ui_node_t *node,
                                               cmp_f2_label_position_t pos);
CMP_API int cmp_f2_checkbox_set_disabled(cmp_ui_node_t *node, int disabled);

/* 2.7 RadioGroup & RadioButton */
typedef enum cmp_f2_radio_size {
  CMP_F2_RADIO_SIZE_MEDIUM = 0, /* 16px */
  CMP_F2_RADIO_SIZE_LARGE       /* 20px */
} cmp_f2_radio_size_t;

/** \brief Documented */
typedef struct cmp_f2_radio_s {
  /** \brief Documented */
  cmp_f2_radio_size_t size;
  /** \brief Documented */
  cmp_f2_label_position_t label_position;
  /** \brief Documented */
  int is_selected;
  /** \brief Documented */
  int is_disabled;
} cmp_f2_radio_t;

/**
 * @brief Initialize a Fluent 2 radio_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_radio_create(cmp_ui_node_t **out_node, const char *label);
CMP_API int cmp_f2_radio_set_size(cmp_ui_node_t *node,
                                  cmp_f2_radio_size_t size);
CMP_API int cmp_f2_radio_set_selected(cmp_ui_node_t *node, int selected);
CMP_API int cmp_f2_radio_set_disabled(cmp_ui_node_t *node, int disabled);

/**
 * @brief Initialize a Fluent 2 radio_group_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_radio_group_create(cmp_ui_node_t **out_node, int horizontal);

/* 2.8 Switch / Toggle */
/** \brief Documented */
typedef struct cmp_f2_toggle_s {
  /** \brief Documented */
  int is_on;
  /** \brief Documented */
  int is_disabled;
  /** \brief Documented */
  cmp_f2_label_position_t label_position;
} cmp_f2_toggle_t;

/**
 * @brief Initialize a Fluent 2 toggle_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_toggle_create(cmp_ui_node_t **out_node, const char *label);
CMP_API int cmp_f2_toggle_set_on(cmp_ui_node_t *node, int is_on);
CMP_API int cmp_f2_toggle_set_disabled(cmp_ui_node_t *node, int disabled);

/* 2.9 Slider */
typedef enum cmp_f2_slider_orientation {
  CMP_F2_SLIDER_HORIZONTAL = 0,
  CMP_F2_SLIDER_VERTICAL
} cmp_f2_slider_orientation_t;

/** \brief Documented */
typedef struct cmp_f2_slider_s {
  /** \brief Documented */
  cmp_f2_slider_orientation_t orientation;
  /** \brief Documented */
  int is_range;
  /** \brief Documented */
  int is_stepped;
  /** \brief Documented */
  float min_val;
  /** \brief Documented */
  float max_val;
  /** \brief Documented */
  float step_val;
  /** \brief Documented */
  float value1;
  /** \brief Documented */
  float value2;
  /** \brief Documented */
  int is_disabled;
} cmp_f2_slider_t;

/**
 * @brief Initialize a Fluent 2 slider_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_slider_create(cmp_ui_node_t **out_node,
                                 cmp_f2_slider_orientation_t orientation);
CMP_API int cmp_f2_slider_set_range_mode(cmp_ui_node_t *node, int is_range);
CMP_API int cmp_f2_slider_set_bounds(cmp_ui_node_t *node, float min_val,
                                     float max_val, float step_val);
CMP_API int cmp_f2_slider_set_value(cmp_ui_node_t *node, float val1,
                                    float val2);
CMP_API int cmp_f2_slider_set_disabled(cmp_ui_node_t *node, int disabled);

/* 2.10 SpinButton */
/** \brief Documented */
typedef struct cmp_f2_spin_button_s {
  /** \brief Documented */
  float value;
  /** \brief Documented */
  float min_val;
  /** \brief Documented */
  float max_val;
  /** \brief Documented */
  float step_val;
  /** \brief Documented */
  int is_disabled;
} cmp_f2_spin_button_t;

/**
 * @brief Initialize a Fluent 2 spin_button_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_spin_button_create(cmp_ui_node_t **out_node,
                                      float initial_val);
CMP_API int cmp_f2_spin_button_set_bounds(cmp_ui_node_t *node, float min_val,
                                          float max_val, float step_val);
CMP_API int cmp_f2_spin_button_set_value(cmp_ui_node_t *node, float val);
CMP_API int cmp_f2_spin_button_set_disabled(cmp_ui_node_t *node, int disabled);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_INPUTS_H */
