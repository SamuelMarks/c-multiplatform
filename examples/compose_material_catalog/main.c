/**
 * @file main.c
 * @brief Entry point for the Compose Material Catalog replica
 * (c-multiplatform).
 */

/* clang-format off */
#include "sampler/sampler_error.h"
#include "sampler/sampler_models.h"
#include "sampler/sampler_nav.h"
#include "sampler/sampler_preferences.h"
#include "sampler/sampler_theme.h"
#include "sampler/sampler_top_app_bar.h"
#include "sampler/sampler_specification_screen.h"
#include "ui_dom_node.h"
#include "ui_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Initialize and run the Compose Material Catalog application.
 * @return SAMPLER_SUCCESS on clean completion, or nonzero error code.
 */
static sampler_error_t run_catalog(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct sampler_preferences *prefs = NULL;
  struct sampler_nav *nav = NULL;
  struct sampler_theme theme;
  const struct sampler_component *const *components = NULL;
  size_t component_count = 0;
  char favorite_route[256];
  int has_favorite = 0;
  int has_theme = 0;
  const char *active_route = NULL;
  sampler_error_t rc;
  ui_error_t u_rc;

  /* Initialize ui_engine */
  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  u_rc = ui_engine_create(&engine_cfg, &engine);
  if (u_rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to initialize ui_engine: %d\x0a", (int)u_rc);
    return SAMPLER_ERROR_LAYOUT_FAILED;
  }

  /* Initialize preferences */
  rc = sampler_preferences_create_in_memory(&prefs);
  if (rc != SAMPLER_SUCCESS) {
    fprintf(stderr, "Failed to initialize preferences: %d\x0a", (int)rc);
    ui_engine_destroy(engine);
    return rc;
  }

  /* Query theme */
  rc = sampler_preferences_get_theme(prefs, &theme, &has_theme);
  if (rc != SAMPLER_SUCCESS) {
    fprintf(stderr, "Failed to load theme: %d\x0a", (int)rc);
    sampler_preferences_destroy(&prefs);
    return rc;
  }

  /* Initialize navigation router */
  rc = sampler_nav_create(&nav);
  if (rc != SAMPLER_SUCCESS) {
    fprintf(stderr, "Failed to initialize router: %d\x0a", (int)rc);
    sampler_preferences_destroy(&prefs);
    return rc;
  }

  /* Query all 41 components */
  rc = sampler_catalog_get_components(&components, &component_count);
  if (rc != SAMPLER_SUCCESS) {
    fprintf(stderr, "Failed to load catalog components: %d\x0a", (int)rc);
    sampler_nav_destroy(&nav);
    sampler_preferences_destroy(&prefs);
    return rc;
  }

  printf("Compose Material Catalog initialized successfully.\x0a");
  printf("Loaded %d Material components.\x0a", (int)component_count);

  /* Check for saved favorite route */
  rc = sampler_preferences_get_favorite_route(
      prefs, favorite_route, sizeof(favorite_route), &has_favorite);
  if (rc != SAMPLER_SUCCESS) {
    fprintf(stderr, "Failed to query favorite route: %d\x0a", (int)rc);
    sampler_nav_destroy(&nav);
    sampler_preferences_destroy(&prefs);
    return rc;
  }

  if (has_favorite && favorite_route[0] != '\0') {
    printf("Navigating to pinned favorite: %s\x0a", favorite_route);
    rc = sampler_nav_navigate(nav, favorite_route);
    if (rc != SAMPLER_SUCCESS) {
      fprintf(stderr, "Failed to navigate to favorite: %d\x0a", (int)rc);
      sampler_nav_destroy(&nav);
      sampler_preferences_destroy(&prefs);
      return rc;
    }
  } else {
    printf("Navigating to specification screen.\x0a");
    rc = sampler_nav_navigate(nav, SAMPLER_ROUTE_SPECIFICATION);
    if (rc != SAMPLER_SUCCESS) {
      fprintf(stderr, "Failed to navigate to specification: %d\x0a", (int)rc);
      sampler_nav_destroy(&nav);
      sampler_preferences_destroy(&prefs);
      return rc;
    }
  }

  rc = sampler_nav_get_current_route(nav, &active_route);
  if (rc != SAMPLER_SUCCESS) {
    fprintf(stderr, "Failed to get active route: %d\x0a", (int)rc);
    sampler_nav_destroy(&nav);
    sampler_preferences_destroy(&prefs);
    return rc;
  }
  printf("Active route: %s\x0a", active_route);

  /* Render specification screen strictly composing M3 widgets */
  {
    struct sampler_specification_screen *screen = NULL;

    rc = sampler_specification_screen_create(engine, nav, &screen);
    if (rc != SAMPLER_SUCCESS) {
      fprintf(stderr, "Failed to create specification screen: %d\x0a", (int)rc);
      sampler_nav_destroy(&nav);
      sampler_preferences_destroy(&prefs);
      ui_engine_destroy(engine);
      return rc;
    }
    printf("Specification screen M3 widgets composed successfully.\x0a");

    rc = sampler_specification_screen_destroy(&screen);
    if (rc != SAMPLER_SUCCESS) {
      fprintf(stderr, "Failed to destroy specification screen: %d\x0a",
              (int)rc);
      sampler_nav_destroy(&nav);
      sampler_preferences_destroy(&prefs);
      ui_engine_destroy(engine);
      return rc;
    }
  }

  /* Cleanup */
  rc = sampler_nav_destroy(&nav);
  if (rc != SAMPLER_SUCCESS) {
    fprintf(stderr, "Failed to destroy router: %d\x0a", (int)rc);
    sampler_preferences_destroy(&prefs);
    ui_engine_destroy(engine);
    return rc;
  }

  rc = sampler_preferences_destroy(&prefs);
  if (rc != SAMPLER_SUCCESS) {
    fprintf(stderr, "Failed to destroy preferences: %d\x0a", (int)rc);
    ui_engine_destroy(engine);
    return rc;
  }

  u_rc = ui_engine_destroy(engine);
  if (u_rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to destroy ui_engine: %d\x0a", (int)u_rc);
    return SAMPLER_ERROR_LAYOUT_FAILED;
  }

  return SAMPLER_SUCCESS;
}

int main(int argc, char **argv) {
  sampler_error_t rc;
  if (argc > 1 && argv != NULL) {
    /* CLI flags parsed if needed */
  }

  rc = run_catalog();
  if (rc != SAMPLER_SUCCESS) {
    return (int)rc;
  }

  return 0;
}
