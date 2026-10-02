/**
 * @file cupertino_sf_symbols.c
 * @brief Apple SF Symbols Vector Engine and Optical Weight Matching
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_sf_symbols.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
static void evaluate_symbol_layers(struct cupertino_symbol *sym);
void test_cupertino_symbol_evaluate_layers(struct cupertino_symbol *sym);
void test_cupertino_symbol_evaluate_layers(struct cupertino_symbol *sym) {
  evaluate_symbol_layers(sym);
}
#endif

static const float s_weight_multipliers[CUPERTINO_SYMBOL_WEIGHT_COUNT] = {
    0.60f, /* UltraLight */
    0.75f, /* Thin */
    0.85f, /* Light */
    1.00f, /* Regular */
    1.15f, /* Medium */
    1.30f, /* SemiBold */
    1.50f, /* Bold */
    1.75f, /* Heavy */
    2.00f  /* Black */
};

static void evaluate_symbol_layers(struct cupertino_symbol *sym) {
  float base_thickness;
  int i;

  /* Resolve point size */
  if (sym->size == CUPERTINO_SYMBOL_SIZE_SMALL) {
    sym->point_size = 14.0f;
  } else if (sym->size == CUPERTINO_SYMBOL_SIZE_LARGE) {
    sym->point_size = 24.0f;
  } else {
    sym->point_size = 18.0f;
  }

  base_thickness =
      sym->point_size * 0.08f * s_weight_multipliers[(int)sym->weight];
  sym->stroke_thickness = base_thickness;
  sym->layer_count = CUPERTINO_SYMBOL_MAX_LAYERS;

  for (i = 0; i < CUPERTINO_SYMBOL_MAX_LAYERS; i++) {
    sym->layers[i].stroke_width = base_thickness;
  }

  switch (sym->rendering_mode) {
  case CUPERTINO_SYMBOL_RENDERING_HIERARCHICAL:
    sym->layers[0].color = sym->primary_color;
    sym->layers[0].opacity = 1.0f;
    sym->layers[1].color = sym->primary_color;
    sym->layers[1].opacity = 0.50f;
    sym->layers[2].color = sym->primary_color;
    sym->layers[2].opacity = 0.25f;
    break;

  case CUPERTINO_SYMBOL_RENDERING_PALETTE:
    sym->layers[0].color = sym->primary_color;
    sym->layers[0].opacity = 1.0f;
    sym->layers[1].color = sym->secondary_color;
    sym->layers[1].opacity = 1.0f;
    sym->layers[2].color = sym->tertiary_color;
    sym->layers[2].opacity = 1.0f;
    break;

  case CUPERTINO_SYMBOL_RENDERING_MULTICOLOR:
    /* Spec-intrinsic Apple Multicolor: primary Blue, secondary Yellow, tertiary
     * Green */
    sym->layers[0].color = UI_COLOR_ARGB(255, 0, 122, 255);
    sym->layers[0].opacity = 1.0f;
    sym->layers[1].color = UI_COLOR_ARGB(255, 255, 204, 0);
    sym->layers[1].opacity = 1.0f;
    sym->layers[2].color = UI_COLOR_ARGB(255, 52, 199, 89);
    sym->layers[2].opacity = 1.0f;
    break;

  case CUPERTINO_SYMBOL_RENDERING_MONOCHROME:
  default:
    sym->layers[0].color = sym->primary_color;
    sym->layers[0].opacity = 1.0f;
    sym->layers[1].color = sym->primary_color;
    sym->layers[1].opacity = 1.0f;
    sym->layers[2].color = sym->primary_color;
    sym->layers[2].opacity = 1.0f;
    break;
  }
}

ui_error_t
cupertino_symbol_create(struct ui_engine *engine,
                        const struct cupertino_symbol_descriptor *desc,
                        struct cupertino_symbol **out_symbol) {
  struct cupertino_symbol *sym;

  if (!engine || !desc || !out_symbol) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->rendering_mode < 0 || (int)desc->rendering_mode > 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->weight < 0 ||
      (int)desc->weight >= (int)CUPERTINO_SYMBOL_WEIGHT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->size < 0 || (int)desc->size > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sym = (struct cupertino_symbol *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_symbol));
  if (!sym) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(sym, 0, sizeof(*sym));
  sym->rendering_mode = desc->rendering_mode;
  sym->weight = desc->weight;
  sym->size = desc->size;

  sym->primary_color = (desc->primary_color != 0)
                           ? desc->primary_color
                           : UI_COLOR_ARGB(255, 0, 122, 255); /* SystemBlue */
  sym->secondary_color =
      (desc->secondary_color != 0)
          ? desc->secondary_color
          : UI_COLOR_ARGB(255, 142, 142, 147); /* SystemGray */
  sym->tertiary_color =
      (desc->tertiary_color != 0)
          ? desc->tertiary_color
          : UI_COLOR_ARGB(255, 199, 199, 204); /* SystemGray3 */

  if (desc->name) {
#if defined(_MSC_VER)
    strncpy_s(sym->name, sizeof(sym->name), desc->name, _TRUNCATE);
#else
    strncpy(sym->name, desc->name, sizeof(sym->name) - 1);
    sym->name[sizeof(sym->name) - 1] = '\0';
#endif
  } else {
#if defined(_MSC_VER)
    strcpy_s(sym->name, sizeof(sym->name), "star");
#else
    strcpy(sym->name, "star");
#endif
  }

  evaluate_symbol_layers(sym);

  *out_symbol = sym;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_symbol_destroy(struct cupertino_symbol *symbol) {
  if (!symbol) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(symbol);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_symbol_set_name(struct cupertino_symbol *symbol,
                                     const char *name) {
  if (!symbol || !name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(symbol->name, sizeof(symbol->name), name, _TRUNCATE);
#else
  strncpy(symbol->name, name, sizeof(symbol->name) - 1);
  symbol->name[sizeof(symbol->name) - 1] = '\0';
#endif

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_symbol_set_rendering_mode(struct cupertino_symbol *symbol,
                                    enum cupertino_symbol_rendering_mode mode) {
  if (!symbol || (int)mode < 0 || (int)mode > 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  symbol->rendering_mode = mode;
  evaluate_symbol_layers(symbol);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_symbol_set_weight(struct cupertino_symbol *symbol,
                                       enum cupertino_symbol_weight weight) {
  if (!symbol || (int)weight < 0 ||
      (int)weight >= (int)CUPERTINO_SYMBOL_WEIGHT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  symbol->weight = weight;
  evaluate_symbol_layers(symbol);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_symbol_set_size(struct cupertino_symbol *symbol,
                                     enum cupertino_symbol_size size) {
  if (!symbol || (int)size < 0 || (int)size > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  symbol->size = size;
  evaluate_symbol_layers(symbol);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_symbol_set_palette(struct cupertino_symbol *symbol,
                                        ui_color_t primary,
                                        ui_color_t secondary,
                                        ui_color_t tertiary) {
  if (!symbol) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  symbol->primary_color = primary;
  symbol->secondary_color = secondary;
  symbol->tertiary_color = tertiary;
  evaluate_symbol_layers(symbol);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_symbol_get_layer(const struct cupertino_symbol *symbol,
                           int layer_index,
                           struct cupertino_symbol_layer *out_layer) {
  if (!symbol || layer_index < 0 || layer_index >= symbol->layer_count ||
      !out_layer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_layer = symbol->layers[layer_index];
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_symbol_get_dimensions(const struct cupertino_symbol *symbol,
                                float *out_width, float *out_height) {
  if (!symbol || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = symbol->point_size;
  *out_height = symbol->point_size;
  return UI_ERROR_NONE;
}
