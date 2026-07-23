/* clang-format off */
#include "ui_design_tokens.h"
#include <string.h>
/* clang-format on */

#define UI_MAX_ALIAS_DEPTH 32
#define UI_TOKEN_INITIAL_CAPACITY 64

enum ui_error ui_design_token_dict_init(struct ui_arena *arena,
                                        struct ui_design_token_dict *out_dict) {
  void *ptr;
  enum ui_error err;

  if (!arena || !out_dict) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_dict->arena = arena;
  out_dict->capacity = UI_TOKEN_INITIAL_CAPACITY;
  out_dict->count = 0;

  err = ui_arena_alloc(
      arena, sizeof(struct ui_design_token) * out_dict->capacity, 8, &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  out_dict->tokens = (struct ui_design_token *)ptr;

  return UI_ERROR_NONE;
}

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

static enum ui_error duplicate_string(struct ui_arena *arena, const char *str,
                                      const char **out_copy) {
  size_t len = strlen(str);
  void *ptr;
  char *copy;
  enum ui_error err;

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

static enum ui_error ensure_capacity(struct ui_design_token_dict *dict) {
  ui_uint32 new_capacity;
  struct ui_design_token *new_tokens;
  ui_uint32 i;
  void *ptr;
  enum ui_error err;

  if (dict->count < dict->capacity) {
    return UI_ERROR_NONE;
  }

  new_capacity = dict->capacity * 2;
  err = ui_arena_alloc(dict->arena,
                       sizeof(struct ui_design_token) * new_capacity, 8, &ptr);
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

static struct ui_design_token *
get_or_create_token(struct ui_design_token_dict *dict, const char *name,
                    enum ui_error *out_err) {
  struct ui_design_token *token = find_token(dict, name);

  *out_err = UI_ERROR_NONE;

  if (token) {
    return token;
  }

  *out_err = ensure_capacity(dict);
  if (*out_err != UI_ERROR_NONE) {
    return NULL;
  }

  token = &dict->tokens[dict->count++];
  {
    enum ui_error dup_err = duplicate_string(dict->arena, name, &token->name);
    if (dup_err != UI_ERROR_NONE) {
      dict->count--; /* Revert */
      *out_err = UI_ERROR_OUT_OF_MEMORY;
      return NULL;
    }
  }

  return token;
}

enum ui_error ui_design_token_set_color(struct ui_design_token_dict *dict,
                                        const char *name, ui_color_t color) {
  enum ui_error err;
  struct ui_design_token *token;

  if (!dict || !name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  token = get_or_create_token(dict, name, &err);
  if (!token) {
    return err;
  }

  token->type = UI_TOKEN_TYPE_COLOR;
  token->value.color_val = color;

  return UI_ERROR_NONE;
}

enum ui_error ui_design_token_set_number(struct ui_design_token_dict *dict,
                                         const char *name, float number) {
  enum ui_error err;
  struct ui_design_token *token;

  if (!dict || !name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  token = get_or_create_token(dict, name, &err);
  if (!token) {
    return err;
  }

  token->type = UI_TOKEN_TYPE_NUMBER;
  token->value.number_val = number;

  return UI_ERROR_NONE;
}

enum ui_error ui_design_token_set_string(struct ui_design_token_dict *dict,
                                         const char *name, const char *str) {
  enum ui_error err;
  struct ui_design_token *token;
  const char *dup_str;

  if (!dict || !name || !str) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (duplicate_string(dict->arena, str, &dup_str) != UI_ERROR_NONE) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  token = get_or_create_token(dict, name, &err);
  if (!token) {
    return err;
  }

  token->type = UI_TOKEN_TYPE_STRING;
  token->value.string_val = dup_str;

  return UI_ERROR_NONE;
}

enum ui_error ui_design_token_set_alias(struct ui_design_token_dict *dict,
                                        const char *name, const char *target) {
  enum ui_error err;
  struct ui_design_token *token;
  const char *dup_target;

  if (!dict || !name || !target) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (duplicate_string(dict->arena, target, &dup_target) != UI_ERROR_NONE) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  token = get_or_create_token(dict, name, &err);
  if (!token) {
    return err;
  }

  token->type = UI_TOKEN_TYPE_ALIAS;
  token->value.alias_val = dup_target;

  return UI_ERROR_NONE;
}

static enum ui_error resolve_token(const struct ui_design_token_dict *dict,
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

enum ui_error ui_design_token_get_color(const struct ui_design_token_dict *dict,
                                        const char *name,
                                        ui_color_t *out_color) {
  enum ui_error err;
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

/** \brief ui_error */
enum ui_error
ui_design_token_get_number(const struct ui_design_token_dict *dict,
                           const char *name, float *out_number) {
  enum ui_error err;
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
