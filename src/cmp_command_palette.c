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
 * @brief Creates a command palette context.
 *
 * @param out_palette Pointer to store the newly allocated command palette
 * context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_palette_create(cmp_command_palette_t **out_palette) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_command_palette_t *palette = NULL;

  if (out_palette == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_command_palette_create: Invalid argument (out_palette=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_command_palette_t), (void **)&palette);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_palette_create: Out of memory: %s\n", err_str);

    return rc;
  }

  palette->capacity = 64;
  palette->count = 0;
  palette->items = NULL;

  rc = CMP_MALLOC(palette->capacity * sizeof(cmp_command_item_t *),
                  (void **)&palette->items);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_command_palette_create: Out of memory for items array: %s\n",
        err_str);

    rc = CMP_FREE(palette);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_command_palette_create: Failed cleanup on error\n");
    }
    return CMP_ERROR_OOM;
  }

  *out_palette = palette;
  cmp_log_debug(
      "cmp_command_palette_create: Successfully created palette context\n");
  return rc;
}

/**
 * @brief Destroys a command palette context.
 *
 * @param palette Pointer to the command palette context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_palette_destroy(cmp_command_palette_t *palette) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t i;

  if (palette == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_palette_destroy: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  if (palette->items != NULL) {
    for (i = 0; i < palette->count; i++) {
      if (palette->items[i] != NULL) {
        rc = CMP_FREE(palette->items[i]);
        if (rc != CMP_SUCCESS) {
          cmp_log_debug(
              "cmp_command_palette_destroy: CMP_FREE item failed at index %d\n",
              (int)i);
        }
      }
    }
    rc = CMP_FREE(palette->items);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_command_palette_destroy: CMP_FREE items array failed\n");
    }
  }

  rc = CMP_FREE(palette);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_command_palette_destroy: CMP_FREE failed\n");

    return rc;
  }

  cmp_log_debug(
      "cmp_command_palette_destroy: Successfully destroyed palette context\n");
  return rc;
}

/**
 * @brief Adds an item to the command palette.
 *
 * @param palette Pointer to the command palette context.
 * @param id Unique string identifier for the command.
 * @param display_text The primary text to display for the command.
 * @param subtext Optional secondary text to display (or NULL).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_palette_add_item(cmp_command_palette_t *palette, const char *id,
                                 const char *display_text,
                                 const char *subtext) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_command_item_t *item = NULL;
  cmp_command_item_t **new_array = NULL;
  size_t new_cap;

  if (palette == NULL || id == NULL || display_text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_palette_add_item: Invalid arg: %s\n", err_str);

    return rc;
  }

  if (palette->count == palette->capacity) {
    new_cap = palette->capacity * 2;
    rc =
        CMP_MALLOC(new_cap * sizeof(cmp_command_item_t *), (void **)&new_array);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_command_palette_add_item: Out of memory growing items "
                    "array: %s\n",
                    err_str);

      return rc;
    }
    if (palette->items != NULL) {
      memcpy(new_array, palette->items,
             palette->count * sizeof(cmp_command_item_t *));
      rc = CMP_FREE(palette->items);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug(
            "cmp_command_palette_add_item: Failed freeing old array\n");
      }
    }
    palette->items = new_array;
    palette->capacity = new_cap;
  }

  rc = CMP_MALLOC(sizeof(cmp_command_item_t), (void **)&item);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_command_palette_add_item: Out of memory allocating item: %s\n",
        err_str);

    return rc;
  }
#if defined(_MSC_VER)
  if (strncpy_s(item->id, sizeof(item->id), id, _TRUNCATE) != 0) {
    cmp_log_debug("cmp_command_palette_add_item: strncpy_s (id) failed\n");
    CMP_FREE(item);
    return CMP_ERROR_GENERAL;
  }
  if (strncpy_s(item->display_text, sizeof(item->display_text), display_text,
                _TRUNCATE) != 0) {
    cmp_log_debug(
        "cmp_command_palette_add_item: strncpy_s (display_text) failed\n");
    CMP_FREE(item);
    return CMP_ERROR_GENERAL;
  }
  if (subtext != NULL) {
    if (strncpy_s(item->subtext, sizeof(item->subtext), subtext, _TRUNCATE) !=
        0) {
      cmp_log_debug(
          "cmp_command_palette_add_item: strncpy_s (subtext) failed\n");
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

  if (subtext != NULL) {
    strncpy(item->subtext, subtext, sizeof(item->subtext) - 1);
    item->subtext[sizeof(item->subtext) - 1] = '\0';
  } else {
    item->subtext[0] = '\0';
  }
#endif

  item->score = 0;
  palette->items[palette->count++] = item;

  cmp_log_debug("cmp_command_palette_add_item: Added item id=%s\n", id);
  return rc;
}

/* Very basic fuzzy matching score algorithm for strings. */
static int fuzzy_score(const char *text, const char *query) {
  int rc = CMP_SUCCESS;
  const char *t = text;
  const char *q = query;
  int score = 0;
  int consecutive_matches = 0;
  char t_char, q_char;

  if (*q == '\0') {
    return 1; /* Empty query matches everything but loosely */
  }

  /* Exact match check first */
  if (strcmp(text, query) == 0) {
    return 1000;
  }

  while (*t != '\0' && *q != '\0') {
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
  if (*q != '\0') {
    return rc;
  }

  /* Add a bonus if we matched the whole text string entirely (exact match) */
  if (*t == '\0') {
    score += 50;
  } else {
    /* Penalize slightly for leftover characters to push shorter/exact matches
     * to the top */
    score -= (int)strlen(t);
  }

  rc = score > 0 ? score : 1; /* Minimum score of 1 if it matched */
  return rc;
}

/* Compare func for qsort descending */
static int compare_items(const void *a, const void *b) {
  int rc = CMP_SUCCESS;
  const cmp_command_item_t *item_a = *(const cmp_command_item_t *const *)a;
  const cmp_command_item_t *item_b = *(const cmp_command_item_t *const *)b;

  if (item_a->score < item_b->score) {
    return 1;
  }
  if (item_a->score > item_b->score) {
    return -1;
  }

  /* Fallback to length to favor shorter strings */
  if (strlen(item_a->display_text) > strlen(item_b->display_text)) {
    return 1;
  }
  if (strlen(item_a->display_text) < strlen(item_b->display_text)) {
    return -1;
  }

  return rc;
}

/**
 * @brief Searches the command palette for items matching the query using fuzzy
 * matching.
 *
 * @param palette Pointer to the command palette context.
 * @param query The search query string.
 * @param out_results Array of pointers to store the matching command items.
 * @param max_results Maximum number of results to return.
 * @param out_count Pointer to store the number of matching items found.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_command_palette_search(cmp_command_palette_t *palette,
                               const char *query,
                               cmp_command_item_t **out_results,
                               size_t max_results, size_t *out_count) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t i, hits;

  if (palette == NULL || query == NULL || out_results == NULL ||
      out_count == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_command_palette_search: Invalid argument: %s\n",
                  err_str);

    return rc;
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
  if (hits > 0) {
    qsort(out_results, hits, sizeof(cmp_command_item_t *), compare_items);
  }

  *out_count = hits;
  cmp_log_debug("cmp_command_palette_search: Search returned %d items\n",
                (int)hits);

  return rc;
}
