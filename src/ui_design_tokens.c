/* clang-format off */
#include "ui_design_tokens.h"
#include <string.h>
/* clang-format on */

/** @cond */
#define UI_MAX_ALIAS_DEPTH 32
/** @endcond */
/** @cond */
#define UI_TOKEN_INITIAL_CAPACITY 64
/** @endcond */

/*
 * @brief ui_design_token_dict_init.
 * @param arena Parameter arena.
 * @param out_dict Parameter out_dict.
 * @return Return value.
 */
ui_error_t ui_design_token_dict_init(struct ui_arena *arena,
                                     struct ui_design_token_dict *out_dict) {
  void *ptr;
  ui_error_t err;

  if (!arena || !out_dict) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_dict->arena = arena;
  out_dict->capacity = UI_TOKEN_INITIAL_CAPACITY;
  out_dict->count = 0;

  err = ui_arena_alloc(
      arena, sizeof(struct ui_design_token) * (size_t)out_dict->capacity, 8,
      &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  out_dict->tokens = (struct ui_design_token *)ptr;

  return UI_ERROR_NONE;
}

/*
 * @brief find_token.
 * @param dict Parameter dict.
 * @param name Parameter name.
 * @return Return value.
 */
static struct ui_design_token *
find_token(const struct ui_design_token_dict *dict, const char *name) {
  ui_uint32 i;
  for (i = 0; i < dict->count; ++i) {
    if (strcmp(dict->tokens[i].name, name) == 0) {
      return &dict->tokens[i];
    }
  }
  return NULL;
}

/*
 * @brief duplicate_string.
 * @param arena Parameter arena.
 * @param str Parameter str.
 * @param out_copy Parameter out_copy.
 * @return Return value.
 */
static ui_error_t duplicate_string(struct ui_arena *arena, const char *str,
                                   const char **out_copy) {
  size_t len = strlen(str);
  void *ptr;
  char *copy;
  ui_error_t err;

  *out_copy = NULL;
  err = ui_arena_alloc(arena, len + 1, 1, &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  copy = (char *)ptr;
  /* Use strcpy_s if available or just strcpy. We can use memcpy to avoid crt
   * hassle */
  memcpy(copy, str, len);
  copy[len] = '\0';

  *out_copy = copy;
  return UI_ERROR_NONE;
}

/*
 * @brief ensure_capacity.
 * @param dict Parameter dict.
 * @return Return value.
 */
static ui_error_t ensure_capacity(struct ui_design_token_dict *dict) {
  ui_uint32 new_capacity;
  struct ui_design_token *new_tokens;
  ui_uint32 i;
  void *ptr;
  ui_error_t err;

  if (dict->count < dict->capacity) {
    return UI_ERROR_NONE;
  }

  new_capacity = dict->capacity * 2;
  err = ui_arena_alloc(dict->arena,
                       sizeof(struct ui_design_token) * (size_t)new_capacity, 8,
                       &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  new_tokens = (struct ui_design_token *)ptr;

  for (i = 0; i < dict->count; ++i) {
    new_tokens[i] = dict->tokens[i];
  }

  dict->tokens = new_tokens;
  dict->capacity = new_capacity;

  return UI_ERROR_NONE;
}

/*
 * @brief get_or_create_token.
 * @param dict Parameter dict.
 * @param name Parameter name.
 * @param out_token Parameter out_token.
 * @return Return value.
 */
static ui_error_t get_or_create_token(struct ui_design_token_dict *dict,
                                      const char *name,
                                      struct ui_design_token **out_token) {
  struct ui_design_token *token = find_token(dict, name);

  if (token) {
    *out_token = token;
    return UI_ERROR_NONE;
  }

  {
    ui_error_t rc = ensure_capacity(dict);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  token = &dict->tokens[dict->count++];
  {
    ui_error_t dup_err = duplicate_string(dict->arena, name, &token->name);
    if (dup_err != UI_ERROR_NONE) {
      dict->count--; /* Revert */
      return dup_err;
    }
  }

  *out_token = token;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_design_token_set_color.
 * @param dict Parameter dict.
 * @param name Parameter name.
 * @param color Parameter color.
 * @return Return value.
 */
ui_error_t ui_design_token_set_color(struct ui_design_token_dict *dict,
                                     const char *name, ui_color_t color) {
  ui_error_t err;
  struct ui_design_token *token;

  if (!dict || !name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = get_or_create_token(dict, name, &token);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  token->type = UI_TOKEN_TYPE_COLOR;
  token->value.color_val = color;

  return UI_ERROR_NONE;
}

/*
 * @brief ui_design_token_set_number.
 * @param dict Parameter dict.
 * @param name Parameter name.
 * @param number Parameter number.
 * @return Return value.
 */
ui_error_t ui_design_token_set_number(struct ui_design_token_dict *dict,
                                      const char *name, float number) {
  ui_error_t err;
  struct ui_design_token *token;

  if (!dict || !name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = get_or_create_token(dict, name, &token);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  token->type = UI_TOKEN_TYPE_NUMBER;
  token->value.number_val = number;

  return UI_ERROR_NONE;
}

/*
 * @brief ui_design_token_set_string.
 * @param dict Parameter dict.
 * @param name Parameter name.
 * @param str Parameter str.
 * @return Return value.
 */
ui_error_t ui_design_token_set_string(struct ui_design_token_dict *dict,
                                      const char *name, const char *str) {
  ui_error_t err;
  struct ui_design_token *token;
  const char *dup_str;

  if (!dict || !name || !str) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = duplicate_string(dict->arena, str, &dup_str);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = get_or_create_token(dict, name, &token);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  token->type = UI_TOKEN_TYPE_STRING;
  token->value.string_val = dup_str;

  return UI_ERROR_NONE;
}

/*
 * @brief ui_design_token_set_alias.
 * @param dict Parameter dict.
 * @param name Parameter name.
 * @param target Parameter target.
 * @return Return value.
 */
ui_error_t ui_design_token_set_alias(struct ui_design_token_dict *dict,
                                     const char *name, const char *target) {
  ui_error_t err;
  struct ui_design_token *token;
  const char *dup_target;

  if (!dict || !name || !target) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = duplicate_string(dict->arena, target, &dup_target);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = get_or_create_token(dict, name, &token);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  token->type = UI_TOKEN_TYPE_ALIAS;
  token->value.alias_val = dup_target;

  return UI_ERROR_NONE;
}

/*
 * @brief resolve_token.
 * @param dict Parameter dict.
 * @param name Parameter name.
 * @param out_token Parameter out_token.
 * @return Return value.
 */
static ui_error_t resolve_token(const struct ui_design_token_dict *dict,
                                const char *name,
                                struct ui_design_token **out_token) {
  ui_uint32 depth = 0;
  struct ui_design_token *token;
  const char *current_name = name;

  *out_token = NULL;

  while (depth < UI_MAX_ALIAS_DEPTH) {
    token = find_token(dict, current_name);
    if (!token) {
      return UI_ERROR_NOT_FOUND;
    }

    if (token->type != UI_TOKEN_TYPE_ALIAS) {
      *out_token = token;
      return UI_ERROR_NONE;
    }

    current_name = token->value.alias_val;
    depth++;
  }

  return UI_ERROR_UNKNOWN; /* Cycle or max depth reached */
}

/*
 * @brief ui_design_token_get_color.
 * @param dict Parameter dict.
 * @param name Parameter name.
 * @param out_color Parameter out_color.
 * @return Return value.
 */
ui_error_t ui_design_token_get_color(const struct ui_design_token_dict *dict,
                                     const char *name, ui_color_t *out_color) {
  ui_error_t err;
  struct ui_design_token *token;

  if (!dict || !name || !out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = resolve_token(dict, name, &token);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (token->type != UI_TOKEN_TYPE_COLOR) {
    return UI_ERROR_UNSUPPORTED; /* Mismatched type */
  }

  *out_color = token->value.color_val;
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t ui_design_token_get_number(const struct ui_design_token_dict *dict,
                                      const char *name, float *out_number) {
  ui_error_t err;
  struct ui_design_token *token;

  if (!dict || !name || !out_number) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = resolve_token(dict, name, &token);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  if (token->type != UI_TOKEN_TYPE_NUMBER) {
    return UI_ERROR_UNSUPPORTED; /* Mismatched type */
  }

  *out_number = token->value.number_val;
  return UI_ERROR_NONE;
}
