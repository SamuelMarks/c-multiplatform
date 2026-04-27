#ifndef CMP_EXAMPLE_APP_H
#define CMP_EXAMPLE_APP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "cmp_ui_app_bar.h"
#include "cmp_ui_icon_button.h"
#include "cmp_ui_tooltip.h"
#include "cmp_ui_badge.h"
/* clang-format on */

/**
 * \file cmp_example_app.h
 * \brief Modular, ejectable application shell for examples.
 */

/**
 * \brief The current design language.
 */
typedef enum {
  CMP_EXAMPLE_DESIGN_MATERIAL3 = 1,
  CMP_EXAMPLE_DESIGN_FLUENT2 = 2,
  CMP_EXAMPLE_DESIGN_CUPERTINO = 3,
  CMP_EXAMPLE_DESIGN_UNSTYLED = 4
} cmp_example_design_system_t;

/**
 * \brief Callbacks provided by specific examples.
 */
typedef struct cmp_example_app_callbacks_s {
  /**
   * \brief Called to build the body UI.
   * \param design_system The active design system.
   * \param body_container The root node to attach body elements to.
   * \return 0 on success, non-zero error code otherwise.
   */
  int (*build_body_ui)(cmp_example_design_system_t design_system,
                       cmp_ui_node_t *body_container);
} cmp_example_app_callbacks_t;

/**
 * \brief Runs the example application.
 *
 * \param title The window title.
 * \param callbacks Hooks for custom UI and logic.
 * \return 0 on success, non-zero error code otherwise.
 */
int cmp_example_app_run(const char *title,
                        const cmp_example_app_callbacks_t *callbacks);

/**
 * \brief Exposes a way to trigger a complete layout recalculation from example
 * code.
 * \return 0 on success, non-zero error code otherwise.
 */
int cmp_example_app_recalculate_layout(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_EXAMPLE_APP_H */
