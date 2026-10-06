/**
 * @file sampler_models.h
 * @brief Catalog component, example models, and registry for Compose Material
 * Catalog.
 */

#ifndef SAMPLER_MODELS_H
#define SAMPLER_MODELS_H

/* clang-format off */
#include "sampler/sampler_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct ui_dom_node;

struct ui_engine;

/**
 * @brief Render callback function pointer for an interactive example.
 * @param engine Pointer to the UI engine.
 * @param container DOM node serving as the mounting container for the sample.
 * @return SAMPLER_SUCCESS on success, or appropriate error code.
 */
typedef sampler_error_t (*sampler_example_content_fn)(
    struct ui_engine *engine, struct ui_dom_node *container);

/**
 * @struct sampler_example
 * @brief Metadata and execution descriptor for a single component sample.
 */
struct sampler_example {
  /** @brief Unique numeric identifier for the example within its component. */
  int id;
  /** @brief Display title for the example. */
  const char *name;
  /** @brief Explanatory summary of what the example demonstrates. */
  const char *description;
  /** @brief Nonzero if this example highlights Material 3 Expressive features.
   */
  int is_expressive;
  /** @brief URL pointing to reference source code on GitHub. */
  const char *source_url;
  /** @brief Callback to render the interactive content into a DOM container. */
  sampler_example_content_fn content;
};

/**
 * @struct sampler_component
 * @brief High-level metadata descriptor for a Material component category.
 */
struct sampler_component {
  /** @brief Globally unique numeric identifier for the component. */
  int id;
  /** @brief Display name of the component (e.g., "Buttons", "Cards"). */
  const char *name;
  /** @brief Detailed description of component purpose and usage. */
  const char *description;
  /** @brief Identifier for the component vector icon. */
  int icon_id;
  /** @brief Nonzero if the icon should be tinted with onSurface / primary
   * color. */
  int tint_icon;
  /** @brief URL pointing to official design guidelines (m3.material.io). */
  const char *guidelines_url;
  /** @brief URL pointing to developer API reference documentation. */
  const char *docs_url;
  /** @brief URL pointing to component source implementation. */
  const char *source_url;
  /** @brief Nonzero if this component has at least one expressive example. */
  int has_expressive_examples;
  /** @brief Array of interactive examples under this component. */
  const struct sampler_example *examples;
  /** @brief Number of examples in the examples array. */
  size_t examples_count;
};

/**
 * @brief Total number of catalog components in the reference inventory.
 */
#define SAMPLER_TOTAL_COMPONENTS 41

/**
 * @brief Retrieve the global list of all catalog components.
 * @param out_components Pointer receiving the array of component pointers.
 * @param out_count Pointer receiving the total number of components.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_catalog_get_components(
    const struct sampler_component *const **out_components, size_t *out_count);

/**
 * @brief Find a component by its numeric identifier.
 * @param id Numeric ID to look up.
 * @param out_component Pointer receiving the component pointer if found.
 * @return SAMPLER_SUCCESS if found, SAMPLER_ERROR_ROUTE_NOT_FOUND if not found,
 *         or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_catalog_find_component_by_id(
    int id, const struct sampler_component **out_component);

/**
 * @brief Find a component by its exact or case-insensitive name.
 * @param name Name string to match.
 * @param out_component Pointer receiving the component pointer if found.
 * @return SAMPLER_SUCCESS if found, SAMPLER_ERROR_ROUTE_NOT_FOUND if not found,
 *         or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_catalog_find_component_by_name(
    const char *name, const struct sampler_component **out_component);

/**
 * @brief Filter components matching an optional search text and expressive
 * filter.
 * @param search_query Substring to search within name or description (or NULL /
 * empty for all).
 * @param show_only_expressive Nonzero to filter to components with expressive
 * examples.
 * @param out_components Array of component pointers to receive matches.
 * @param max_results Maximum number of pointers out_components can store.
 * @param out_count Pointer receiving actual number of matching components
 * populated.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t
sampler_catalog_filter(const char *search_query, int show_only_expressive,
                       const struct sampler_component **out_components,
                       size_t max_results, size_t *out_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SAMPLER_MODELS_H */
