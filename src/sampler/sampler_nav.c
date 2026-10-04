/**
 * @file sampler_nav.c
 * @brief Implementation of navigation stack and routing for Compose Material
 * Catalog.
 */

/* clang-format off */
#include "sampler/sampler_nav.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define SAMPLER_NAV_INITIAL_CAPACITY 8

struct sampler_nav_entry {
  char *route;
};

struct sampler_nav {
  struct sampler_nav_entry *entries;
  size_t count;
  size_t capacity;
};

sampler_error_t sampler_nav_create(struct sampler_nav **out_nav) {
  struct sampler_nav *nav;

  if (out_nav == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  nav =
      (struct sampler_nav *)C_MULTIPLATFORM_MALLOC(sizeof(struct sampler_nav));
  if (nav == NULL) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  nav->entries = (struct sampler_nav_entry *)C_MULTIPLATFORM_MALLOC(
      SAMPLER_NAV_INITIAL_CAPACITY * sizeof(struct sampler_nav_entry));
  if (nav->entries == NULL) {
    C_MULTIPLATFORM_FREE(nav);
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  nav->count = 0;
  nav->capacity = SAMPLER_NAV_INITIAL_CAPACITY;

  *out_nav = nav;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_nav_clear(struct sampler_nav *nav) {
  size_t i;

  if (nav == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  for (i = 0; i < nav->count; ++i) {
    if (nav->entries[i].route != NULL) {
      C_MULTIPLATFORM_FREE(nav->entries[i].route);
      nav->entries[i].route = NULL;
    }
  }
  nav->count = 0;

  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_nav_destroy(struct sampler_nav **nav) {
  sampler_error_t rc;

  if (nav == NULL || *nav == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  rc = sampler_nav_clear(*nav);
  if (rc != SAMPLER_SUCCESS) {
    return rc;
  }

  if ((*nav)->entries != NULL) {
    C_MULTIPLATFORM_FREE((*nav)->entries);
    (*nav)->entries = NULL;
  }

  C_MULTIPLATFORM_FREE(*nav);
  *nav = NULL;

  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_nav_navigate(struct sampler_nav *nav,
                                     const char *route) {
  size_t len;
  char *route_copy;

  if (nav == NULL || route == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  if (nav->count >= nav->capacity) {
    size_t new_cap = nav->capacity * 2;
    struct sampler_nav_entry *new_entries =
        (struct sampler_nav_entry *)C_MULTIPLATFORM_REALLOC(
            nav->entries, new_cap * sizeof(struct sampler_nav_entry));
    if (new_entries == NULL) {
      return SAMPLER_ERROR_OUT_OF_MEMORY;
    }
    nav->entries = new_entries;
    nav->capacity = new_cap;
  }

  len = strlen(route);
  route_copy = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (route_copy == NULL) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(route_copy, len + 1, route);
#else
  strcpy(route_copy, route);
#endif

  nav->entries[nav->count].route = route_copy;
  nav->count++;

  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_nav_pop(struct sampler_nav *nav, int *out_popped) {
  if (nav == NULL || out_popped == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  if (nav->count <= 1) {
    *out_popped = 0;
    return SAMPLER_SUCCESS;
  }

  nav->count--;
  if (nav->entries[nav->count].route != NULL) {
    C_MULTIPLATFORM_FREE(nav->entries[nav->count].route);
    nav->entries[nav->count].route = NULL;
  }

  *out_popped = 1;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_nav_get_current_route(const struct sampler_nav *nav,
                                              const char **out_route) {
  if (nav == NULL || out_route == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  if (nav->count == 0) {
    *out_route = NULL;
    return SAMPLER_ERROR_ROUTE_NOT_FOUND;
  }

  *out_route = nav->entries[nav->count - 1].route;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_nav_get_stack_depth(const struct sampler_nav *nav,
                                            size_t *out_depth) {
  if (nav == NULL || out_depth == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  *out_depth = nav->count;
  return SAMPLER_SUCCESS;
}
