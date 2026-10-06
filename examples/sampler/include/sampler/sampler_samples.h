/**
 * @file sampler_samples.h
 * @brief Declarations for interactive Compose Material Catalog samples.
 */

#ifndef SAMPLER_SAMPLES_H
#define SAMPLER_SAMPLES_H

/* clang-format off */
#include "sampler/sampler_models.h"
#include "ui_dom_node.h"
#include "ui_engine.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/* Buttons */
extern sampler_error_t sample_filled_button(struct ui_engine *engine,
                                            struct ui_dom_node *container);
extern sampler_error_t sample_elevated_button(struct ui_engine *engine,
                                              struct ui_dom_node *container);
extern sampler_error_t
sample_filled_tonal_button(struct ui_engine *engine,
                           struct ui_dom_node *container);
extern sampler_error_t sample_outlined_button(struct ui_engine *engine,
                                              struct ui_dom_node *container);
extern sampler_error_t sample_text_button(struct ui_engine *engine,
                                          struct ui_dom_node *container);
extern sampler_error_t sample_button_with_icon(struct ui_engine *engine,
                                               struct ui_dom_node *container);
extern sampler_error_t
sample_button_animated_shape(struct ui_engine *engine,
                             struct ui_dom_node *container);

/* Stub */
extern sampler_error_t sample_stub_create(struct ui_engine *engine,
                                          struct ui_dom_node *container);

#ifdef __cplusplus
}
#endif

#endif /* SAMPLER_SAMPLES_H */
