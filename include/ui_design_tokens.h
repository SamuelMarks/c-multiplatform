/**
 * @file ui_design_tokens.h
 * @brief Design token management and resolution.
 *
 * This header defines the structures and functions necessary to store, query,
 * and resolve design tokens such as colors, numbers, and strings, along with
 * support for alias references.
 */

#ifndef UI_DESIGN_TOKENS_H
#define UI_DESIGN_TOKENS_H

/* clang-format off */
#include "ui_arena.h"
#include "ui_color_space.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type of a design token.
 */
enum ui_design_token_type {
  UI_TOKEN_TYPE_COLOR,  /**< Color token type. */
  UI_TOKEN_TYPE_NUMBER, /**< Numeric token type. */
  UI_TOKEN_TYPE_STRING, /**< String token type. */
  UI_TOKEN_TYPE_ALIAS   /**< Alias token type pointing to another token. */
};

/**
 * @brief Represents a single design token.
 */
struct ui_design_token {
  const char *name;               /**< The name of the design token. */
  enum ui_design_token_type type; /**< The type of the token. */
  /** \brief Union holding the token value based on its type. */
  union {
    ui_color_t color_val;   /**< Value for a color token. */
    float number_val;       /**< Value for a numeric token. */
    const char *string_val; /**< Value for a string token. */
    const char *alias_val;  /**< Value for an alias token (the target name). */
  } value;                  /**< The union value */
};

/**
 * @brief A dictionary of design tokens.
 */
struct ui_design_token_dict {
  struct ui_design_token *tokens; /**< Array of design tokens. */
  ui_uint32 count;                /**< The current number of tokens. */
  ui_uint32 capacity; /**< The allocated capacity of the tokens array. */
  struct ui_arena
      *arena; /**< Memory arena used for string allocations if needed. */
};

/**
 * @brief Initializes a new token dictionary.
 *
 * @param arena Pointer to the memory arena to allocate from.
 * @param out_dict Pointer to output the initialized dictionary structure.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_design_token_dict_init(struct ui_arena *arena,
                                     struct ui_design_token_dict *out_dict);

/**
 * @brief Sets a color token in the dictionary.
 *
 * @param dict Pointer to the token dictionary.
 * @param name Token name.
 * @param color The color value.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_design_token_set_color(struct ui_design_token_dict *dict,
                                     const char *name, ui_color_t color);

/**
 * @brief Sets a number token in the dictionary.
 *
 * @param dict Pointer to the token dictionary.
 * @param name Token name.
 * @param number The numeric value.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_design_token_set_number(struct ui_design_token_dict *dict,
                                      const char *name, float number);

/**
 * @brief Sets a string token in the dictionary.
 *
 * @param dict Pointer to the token dictionary.
 * @param name Token name.
 * @param str The string value.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_design_token_set_string(struct ui_design_token_dict *dict,
                                      const char *name, const char *str);

/**
 * @brief Sets an alias token in the dictionary.
 *
 * @param dict Pointer to the token dictionary.
 * @param name Token name.
 * @param target The target token name being aliased.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_design_token_set_alias(struct ui_design_token_dict *dict,
                                     const char *name, const char *target);

/**
 * @brief Retrieves a resolved color token from the dictionary.
 * Resolves aliases recursively.
 *
 * @param dict Pointer to the token dictionary.
 * @param name Token name to look up.
 * @param out_color Pointer to store the resolved color.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_NOT_FOUND` if missing, or
 * mismatched type.
 */
ui_error_t ui_design_token_get_color(const struct ui_design_token_dict *dict,
                                     const char *name, ui_color_t *out_color);

/**
 * @brief Retrieves a resolved number token from the dictionary.
 * Resolves aliases recursively.
 *
 * @param dict Pointer to the token dictionary.
 * @param name Token name to look up.
 * @param out_number Pointer to store the resolved numeric value.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_NOT_FOUND` if missing, or
 * mismatched type.
 */
ui_error_t ui_design_token_get_number(const struct ui_design_token_dict *dict,
                                      const char *name, float *out_number);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DESIGN_TOKENS_H */
