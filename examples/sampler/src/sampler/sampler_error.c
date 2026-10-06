/**
 * @file sampler_error.c
 * @brief Implementation of error translation for Compose Material Catalog.
 */

/* clang-format off */
#include "sampler/sampler_error.h"
/* clang-format on */

sampler_error_t sampler_error_to_string(sampler_error_t error,
                                        const char **out_str) {
  if (out_str == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  switch (error) {
  case SAMPLER_SUCCESS:
    *out_str = "Operation succeeded";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_NULL_POINTER:
    *out_str = "Null pointer passed for mandatory parameter";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_OUT_OF_MEMORY:
    *out_str = "Out of memory";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_INVALID_ARGUMENT:
    *out_str = "Invalid argument provided";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_OUT_OF_BOUNDS:
    *out_str = "Index out of bounds";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_ROUTE_NOT_FOUND:
    *out_str = "Route not found in navigation graph";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_STORAGE_FAILURE:
    *out_str = "Storage persistence operation failed";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_PARSE_FAILURE:
    *out_str = "Data parsing or deserialization failed";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_DOM_ATTACH_FAILED:
    *out_str = "Failed to attach node to DOM hierarchy";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_STYLE_PROP_FAILED:
    *out_str = "Failed to apply style property";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_LAYOUT_FAILED:
    *out_str = "Layout calculation failed";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_EVENT_DISPATCH_FAILED:
    *out_str = "Event dispatch failed";
    return SAMPLER_SUCCESS;
  case SAMPLER_ERROR_ANIMATION_FAILED:
    *out_str = "Animation step failed";
    return SAMPLER_SUCCESS;
  default:
    *out_str = "Unknown error";
    return SAMPLER_SUCCESS;
  }
}
