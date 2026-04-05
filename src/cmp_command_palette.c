/* clang-format off */
#include "cmp.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_command_palette {
  cmp_command_item_t **items;
  size_t count;
  size_t capacity;
};

int cmp_command_palette_create(cmp_command_palette_t **out_palette) {
  cmp_command_palette_t *palette;
  if (!out_palette) {
    return CMP_ERROR_INVALID_ARG;
  }

  palette = (cmp_command_palette_t *)malloc(sizeof(cmp_command_palette_t));
  if (!palette) {
    return CMP_ERROR_OOM;
  }

  palette->capacity = 64;
  palette->count = 0;
  palette->items = (cmp_command_item_t **)malloc(palette->capacity *
                                                 sizeof(cmp_command_item_t *));
  if (!palette->items) {
    free(palette);
    return CMP_ERROR_OOM;
  }

  *out_palette = palette;
  return CMP_SUCCESS;
}

int cmp_command_palette_destroy(cmp_command_palette_t *palette) {
  size_t i;
  if (!palette) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < palette->count; i++) {
    free(palette->items[i]);
  }
  free(palette->items);
  free(palette);
  return CMP_SUCCESS;
}

int cmp_command_palette_add_item(cmp_command_palette_t *palette, const char *id,
                                 const char *display_text,
                                 const char *subtext) {
  cmp_command_item_t *item;
  cmp_command_item_t **new_array;

  if (!palette || !id || !display_text) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (palette->count == palette->capacity) {
    palette->capacity *= 2;
    new_array = (cmp_command_item_t **)realloc(
        palette->items, palette->capacity * sizeof(cmp_command_item_t *));
    if (!new_array) {
      return CMP_ERROR_OOM;
    }
    palette->items = new_array;
  }

  item = (cmp_command_item_t *)malloc(sizeof(cmp_command_item_t));
  if (!item) {
    return CMP_ERROR_OOM;
  }

  strncpy(item->id, id, sizeof(item->id) - 1);
  item->id[sizeof(item->id) - 1] = '\0';

  strncpy(item->display_text, display_text, sizeof(item->display_text) - 1);
  item->display_text[sizeof(item->display_text) - 1] = '\0';

  if (subtext) {
    strncpy(item->subtext, subtext, sizeof(item->subtext) - 1);
    item->subtext[sizeof(item->subtext) - 1] = '\0';
  } else {
    item->subtext[0] = '\0';
  }

  item->score = 0;
  palette->items[palette->count++] = item;

  return CMP_SUCCESS;
}

/* Very basic fuzzy matching score algorithm for strings. */
static int fuzzy_score(const char *text, const char *query) {
  const char *t = text;
  const char *q = query;
  int score = 0;
  int consecutive_matches = 0;
  char t_char, q_char;

  if (!*q) {
    return 1; /* Empty query matches everything but loosely */
  }

  /* Exact match check first */
  if (strcmp(text, query) == 0) {
    return 1000;
  }

  while (*t && *q) {
    t_char = (char)tolower((unsigned char)*t);
    q_char = (char)tolower((unsigned char)*q);

    if (t_char == q_char) {
      score += 10 + (consecutive_matches * 5); /* Bonus for consecutive chars */
      /* Small bonus if it matches at the beginning of the string or right after
       * a boundary */
      if (t == text || *(t - 1) == '/' || *(t - 1) == '\\' || *(t - 1) == ' ' ||
          *(t - 1) == '_' || *(t - 1) == '.') {
        score += 20;
      }
      consecutive_matches++;
      q++;
    } else {
      consecutive_matches = 0;
    }
    t++;
  }

  /* Must match all chars in query to be a valid fuzzy hit */
  if (*q) {
    return 0;
  }

  /* Add a bonus if we matched the whole text string entirely (exact match) */
  if (!*t) {
    score += 50;
  } else {
    /* Penalize slightly for leftover characters to push shorter/exact matches
     * to the top */
    score -= (int)strlen(t);
  }

  return score > 0 ? score : 1; /* Minimum score of 1 if it matched */
}

/* Compare func for qsort descending */
static int compare_items(const void *a, const void *b) {
  const cmp_command_item_t *item_a = *(const cmp_command_item_t **)a;
  const cmp_command_item_t *item_b = *(const cmp_command_item_t **)b;

  if (item_a->score < item_b->score)
    return 1;
  if (item_a->score > item_b->score)
    return -1;

  /* Fallback to length to favor shorter strings */
  if (strlen(item_a->display_text) > strlen(item_b->display_text))
    return 1;
  if (strlen(item_a->display_text) < strlen(item_b->display_text))
    return -1;

  return 0;
}

int cmp_command_palette_search(cmp_command_palette_t *palette,
                               const char *query,
                               cmp_command_item_t **out_results,
                               size_t max_results, size_t *out_count) {
  size_t i, hits;

  if (!palette || !query || !out_results || !out_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  hits = 0;
  /* Calculate score for all */
  for (i = 0; i < palette->count; i++) {
    palette->items[i]->score =
        fuzzy_score(palette->items[i]->display_text, query);

    /* If the score is 0 on text, maybe try subtext? */
    if (palette->items[i]->score == 0 &&
        palette->items[i]->subtext[0] != '\0') {
      palette->items[i]->score = fuzzy_score(palette->items[i]->subtext, query);
    }
  }

  /* Filter and add to results */
  for (i = 0; i < palette->count; i++) {
    if (palette->items[i]->score > 0) {
      if (hits < max_results) {
        out_results[hits++] = palette->items[i];
      }
    }
  }

  /* Sort the results array descending */
  qsort(out_results, hits, sizeof(cmp_command_item_t *), compare_items);

  *out_count = hits;
  return CMP_SUCCESS;
}
