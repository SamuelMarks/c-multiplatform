/**
 * @file md3_adaptive.h
 * @brief Material 3 Adaptive Scaffolds (ListDetailPaneScaffold,
 * SupportingPaneScaffold, NavigationSuiteScaffold).
 */

#ifndef MATERIAL3_MD3_ADAPTIVE_H
#define MATERIAL3_MD3_ADAPTIVE_H

/* clang-format off */
#include "ui_adaptive_pane_scaffold_base.h"
#include "material3/md3_navigation.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @brief Material 3 List-Detail Pane Scaffold wrapping
 * ui_adaptive_pane_scaffold_base.
 */
struct md3_list_detail_pane_scaffold;

/**
 * @brief Material 3 Supporting Pane Scaffold wrapping
 * ui_adaptive_pane_scaffold_base.
 */
struct md3_supporting_pane_scaffold;

/**
 * @brief Material 3 Navigation Suite Scaffold adapting between Bar, Rail, and
 * Drawer.
 */
struct md3_navigation_suite_scaffold;

/* ========================================================================= */
/* List-Detail Pane Scaffold                                                 */
/* ========================================================================= */

/**
 * @brief Creates a Material 3 List-Detail Pane Scaffold.
 *
 * @param engine Pointer to ui_engine.
 * @param out_scaffold Pointer receiving newly created scaffold instance.
 * @return UI_ERROR_NONE on success, or appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_list_detail_pane_scaffold_create(
    struct ui_engine *engine,
    struct md3_list_detail_pane_scaffold **out_scaffold);

/**
 * @brief Destroys a Material 3 List-Detail Pane Scaffold.
 *
 * @param scaffold Scaffold instance to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_list_detail_pane_scaffold_destroy(
    struct md3_list_detail_pane_scaffold *scaffold);

/**
 * @brief Retrieves underlying ui_adaptive_pane_scaffold_base handle.
 *
 * @param scaffold Scaffold instance.
 * @param out_base Pointer receiving base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_list_detail_pane_scaffold_get_base(
    struct md3_list_detail_pane_scaffold *scaffold,
    struct ui_adaptive_pane_scaffold_base **out_base);

/**
 * @brief Sets the primary list pane component.
 *
 * @param scaffold Scaffold instance.
 * @param list_component List pane component.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_detail_pane_scaffold_set_list_pane(
    struct md3_list_detail_pane_scaffold *scaffold,
    struct ui_component *list_component);

/**
 * @brief Sets the secondary detail pane component.
 *
 * @param scaffold Scaffold instance.
 * @param detail_component Detail pane component.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_detail_pane_scaffold_set_detail_pane(
    struct md3_list_detail_pane_scaffold *scaffold,
    struct ui_component *detail_component);

/**
 * @brief Sets an optional extra supporting pane component.
 *
 * @param scaffold Scaffold instance.
 * @param extra_component Extra pane component.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_detail_pane_scaffold_set_extra_pane(
    struct md3_list_detail_pane_scaffold *scaffold,
    struct ui_component *extra_component);

/* ========================================================================= */
/* Supporting Pane Scaffold                                                  */
/* ========================================================================= */

/**
 * @brief Creates a Material 3 Supporting Pane Scaffold.
 *
 * @param engine Pointer to ui_engine.
 * @param out_scaffold Pointer receiving newly created scaffold instance.
 * @return UI_ERROR_NONE on success, or appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_supporting_pane_scaffold_create(
    struct ui_engine *engine,
    struct md3_supporting_pane_scaffold **out_scaffold);

/**
 * @brief Destroys a Material 3 Supporting Pane Scaffold.
 *
 * @param scaffold Scaffold instance to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_supporting_pane_scaffold_destroy(
    struct md3_supporting_pane_scaffold *scaffold);

/**
 * @brief Retrieves underlying ui_adaptive_pane_scaffold_base handle.
 *
 * @param scaffold Scaffold instance.
 * @param out_base Pointer receiving base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_supporting_pane_scaffold_get_base(
    struct md3_supporting_pane_scaffold *scaffold,
    struct ui_adaptive_pane_scaffold_base **out_base);

/**
 * @brief Sets the primary main pane component.
 *
 * @param scaffold Scaffold instance.
 * @param main_component Main pane component.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_supporting_pane_scaffold_set_main_pane(
    struct md3_supporting_pane_scaffold *scaffold,
    struct ui_component *main_component);

/**
 * @brief Sets the secondary supporting pane component.
 *
 * @param scaffold Scaffold instance.
 * @param supporting_component Supporting pane component.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_supporting_pane_scaffold_set_supporting_pane(
    struct md3_supporting_pane_scaffold *scaffold,
    struct ui_component *supporting_component);

/* ========================================================================= */
/* Navigation Suite Scaffold                                                 */
/* ========================================================================= */

/**
 * @brief Creates a Material 3 Navigation Suite Scaffold adapting across window
 * sizes.
 *
 * @param engine Pointer to ui_engine.
 * @param out_scaffold Pointer receiving newly created scaffold instance.
 * @return UI_ERROR_NONE on success, or appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_suite_scaffold_create(
    struct ui_engine *engine,
    struct md3_navigation_suite_scaffold **out_scaffold);

/**
 * @brief Destroys a Material 3 Navigation Suite Scaffold.
 *
 * @param scaffold Scaffold instance to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_navigation_suite_scaffold_destroy(
    struct md3_navigation_suite_scaffold *scaffold);

/**
 * @brief Sets window width class for the navigation suite (adapts navigation
 * chrome).
 *
 * @param scaffold Scaffold instance.
 * @param width_class Compact, Medium, or Expanded.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_navigation_suite_scaffold_set_window_width_class(
    struct md3_navigation_suite_scaffold *scaffold,
    enum ui_adaptive_window_width_class width_class);

/**
 * @brief Retrieves underlying ui_component for the navigation suite.
 *
 * @param scaffold Scaffold instance.
 * @param out_component Pointer receiving component handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_navigation_suite_scaffold_get_component(
    struct md3_navigation_suite_scaffold *scaffold,
    struct ui_component **out_component);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_ADAPTIVE_H */
