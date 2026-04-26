/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_command_palette {
  cmp_command_item_t **items;
  size_t count;
  size_t capacity;
};

/**
 * @brief cmp_command_palette_create
 *
 * @param out_palette Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_palette_create(cmp_command_palette_t **out_palette) {
  cmp_command_palette_t *palette;
  int rc;

  if (!out_palette) {
    LOG_DEBUG("cmp_command_palette_create: out_palette is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_command_palette_t), (void **)&palette);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_command_palette_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  palette->capacity = 64;
  palette->count = 0;

  rc = CMP_MALLOC(palette->capacity * sizeof(cmp_command_item_t *),
                  (void **)&palette->items);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_command_palette_create: OOM items\n");
    CMP_FREE(palette);
    return CMP_ERROR_OOM;
  }

  *out_palette = palette;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_command_palette_destroy
 *
 * @param palette Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_palette_destroy(cmp_command_palette_t *palette) {
  size_t i;
  int rc;

  if (!palette) {
    LOG_DEBUG("cmp_command_palette_destroy: palette is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < palette->count; i++) {
    rc = CMP_FREE(palette->items[i]);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_command_palette_destroy: CMP_FREE item failed\n");
  }

  rc = CMP_FREE(palette->items);
  if (rc != CMP_SUCCESS)
    LOG_DEBUG("cmp_command_palette_destroy: CMP_FREE items failed\n");

  rc = CMP_FREE(palette);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_command_palette_destroy: CMP_FREE failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_command_palette_add_item
 *
 * @param palette Parameter description.
 * @param id Parameter description.
 * @param display_text Parameter description.
 * @param subtext Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_palette_add_item(cmp_command_palette_t *palette, const char *id,
                                 const char *display_text,
                                 const char *subtext) {
  cmp_command_item_t *item;
  cmp_command_item_t **new_array;
  int rc;

  if (!palette || !id || !display_text) {
    LOG_DEBUG("cmp_command_palette_add_item: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (palette->count == palette->capacity) {
    size_t new_cap = palette->capacity * 2;
    rc =
        CMP_MALLOC(new_cap * sizeof(cmp_command_item_t *), (void **)&new_array);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_command_palette_add_item: OOM items\n");
      return CMP_ERROR_OOM;
    }
    memcpy(new_array, palette->items,
           palette->count * sizeof(cmp_command_item_t *));
    CMP_FREE(palette->items);
    palette->items = new_array;
    palette->capacity = new_cap;
  }

  rc = CMP_MALLOC(sizeof(cmp_command_item_t), (void **)&item);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_command_palette_add_item: OOM item\n");
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  if (strncpy_s(item->id, sizeof(item->id), id, _TRUNCATE) != 0) {
    CMP_FREE(item);
    return CMP_ERROR_GENERAL;
  }
  if (strncpy_s(item->display_text, sizeof(item->display_text), display_text,
                _TRUNCATE) != 0) {
    CMP_FREE(item);
    return CMP_ERROR_GENERAL;
  }
  if (subtext) {
    if (strncpy_s(item->subtext, sizeof(item->subtext), subtext, _TRUNCATE) !=
        0) {
      CMP_FREE(item);
      return CMP_ERROR_GENERAL;
    }
  } else {
    item->subtext[0] = '\0';
  }
#else
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
#endif

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

/**
 * @brief cmp_command_palette_search
 *
 * @param palette Parameter description.
 * @param query Parameter description.
 * @param out_results Parameter description.
 * @param max_results Parameter description.
 * @param out_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_palette_search(cmp_command_palette_t *palette,
                               const char *query,
                               cmp_command_item_t **out_results,
                               size_t max_results, size_t *out_count) {
  size_t i, hits;

  if (!palette || !query || !out_results || !out_count) {
    LOG_DEBUG("cmp_command_palette_search: Invalid arg\n");
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