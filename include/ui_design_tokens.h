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
  UI_TOKEN_TYPE_COLOR,
  UI_TOKEN_TYPE_NUMBER,
  UI_TOKEN_TYPE_STRING,
  UI_TOKEN_TYPE_ALIAS
};

/**
 * @brief Represents a single design token.
 */
struct ui_design_token {
  const char *name;
  enum ui_design_token_type type;
  /** \brief union */
  union {
    ui_color_t color_val;
    float number_val;
    const char *string_val;
    const char *alias_val;
  } value;
};

/**
 * @brief A dictionary of design tokens.
 */
struct ui_design_token_dict {
  struct ui_design_token *tokens;
  ui_uint32 count;
  ui_uint32 capacity;
  struct ui_arena *arena; /* Used for string allocations if needed */
};

/**
 * @brief Initializes a new token dictionary.
 *
 * @param arena The memory arena to allocate from.
 * @param out_dict Pointer to output the initialized dictionary.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_design_token_dict_init(struct ui_arena *arena,
                                        struct ui_design_token_dict *out_dict);

/**
 * @brief Sets a color token in the dictionary.
 *
 * @param dict The dictionary.
 * @param name Token name.
 * @param color The color value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_design_token_set_color(struct ui_design_token_dict *dict,
                                        const char *name, ui_color_t color);

/**
 * @brief Sets a number token in the dictionary.
 *
 * @param dict The dictionary.
 * @param name Token name.
 * @param number The number value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_design_token_set_number(struct ui_design_token_dict *dict,
                                         const char *name, float number);

/**
 * @brief Sets a string token in the dictionary.
 *
 * @param dict The dictionary.
 * @param name Token name.
 * @param str The string value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_design_token_set_string(struct ui_design_token_dict *dict,
                                         const char *name, const char *str);

/**
 * @brief Sets an alias token in the dictionary.
 *
 * @param dict The dictionary.
 * @param name Token name.
 * @param target The target token name being aliased.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_design_token_set_alias(struct ui_design_token_dict *dict,
                                        const char *name, const char *target);

/**
 * @brief Retrieves a resolved color token from the dictionary.
 * Resolves aliases recursively.
 *
 * @param dict The dictionary.
 * @param name Token name.
 * @param out_color Pointer to store the resolved color.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if missing or mismatched
 * type.
 */
enum ui_error ui_design_token_get_color(const struct ui_design_token_dict *dict,
                                        const char *name,
                                        ui_color_t *out_color);

/**
 * @brief Retrieves a resolved number token from the dictionary.
 * Resolves aliases recursively.
 *
 * @param dict The dictionary.
 * @param name Token name.
 * @param out_number Pointer to store the resolved number.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if missing or mismatched
 * type.
 */
enum ui_error
ui_design_token_get_number(const struct ui_design_token_dict *dict,
                           const char *name, float *out_number);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DESIGN_TOKENS_H */
