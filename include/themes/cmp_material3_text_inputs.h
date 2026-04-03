#ifndef CMP_MATERIAL3_TEXT_INPUTS_H
#define CMP_MATERIAL3_TEXT_INPUTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_material3_sys.h"
/* clang-format on */

/**
 * @brief Material 3 Text Field Variants
 */
typedef enum cmp_m3_text_field_variant {
  CMP_M3_TEXT_FIELD_FILLED,
  CMP_M3_TEXT_FIELD_OUTLINED
} cmp_m3_text_field_variant_t;

/**
 * @brief Evaluated constraints for a Material 3 Text Field
 */
typedef struct cmp_m3_text_field_metrics {
  float container_height_single_line;
  cmp_m3_shape_family_t shape;
  cmp_m3_shape_modifiers_t shape_modifiers;
  float border_thickness_unfocused;
  float border_thickness_focused;
  float icon_size;
  float padding_horizontal;
  float gap_with_icon;
  int has_floating_label;
} cmp_m3_text_field_metrics_t;

CMP_API int cmp_m3_text_field_resolve(cmp_m3_text_field_variant_t variant,
                                      cmp_m3_text_field_metrics_t *out_metrics);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_TEXT_INPUTS_H */