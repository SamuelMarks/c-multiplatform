/**
 * @file main.c
 * @brief Extensible multi-design-system automated screenshot tool CLI.
 */

/* clang-format off */
#include "design_system_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define MKDIR_CMD(p) _mkdir(p)
#else
#define MKDIR_CMD(p) mkdir(p, 0755)
#endif
/* clang-format on */

static ui_error_t ensure_dir_exists(const char *path) {
  char temp[512];
  char *p = NULL;
  size_t len;

  if (!path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(temp, sizeof(temp), path, _TRUNCATE);
#else
  strncpy(temp, path, sizeof(temp) - 1);
  temp[sizeof(temp) - 1] = '\0';
#endif

  len = strlen(temp);
  if (len > 0 && (temp[len - 1] == '/' || temp[len - 1] == '\\')) {
    temp[len - 1] = '\0';
  }

  for (p = temp + 1; *p; p++) {
    if (*p == '/' || *p == '\\') {
      *p = '\0';
      MKDIR_CMD(temp);
      *p = '/';
    }
  }
  MKDIR_CMD(temp);
  return UI_ERROR_NONE;
}

int main(int argc, char **argv) {
  const char *target_system = "all";
  const char *output_base = "../cc0-assets/c-multiplatform";
  const char *explicit_output_dir = NULL;
  int is_dark = 0;
  const struct design_system_driver *const *drivers;
  size_t driver_count = 0;
  size_t i;
  int processed = 0;
  ui_error_t rc;

  /* Register built-in design systems */
  rc = ds_registry_register(&g_driver_material3);
  if (rc != UI_ERROR_NONE) {
    return (int)rc;
  }
  rc = ds_registry_register(&g_driver_cupertino);
  if (rc != UI_ERROR_NONE) {
    return (int)rc;
  }
  rc = ds_registry_register(&g_driver_fluent2);
  if (rc != UI_ERROR_NONE) {
    return (int)rc;
  }

  rc = ds_registry_get_all(&drivers, &driver_count);
  if (rc != UI_ERROR_NONE) {
    return (int)rc;
  }

  for (i = 1; i < (size_t)argc; ++i) {
    if ((strcmp(argv[i], "--system") == 0 ||
         strcmp(argv[i], "--design-system") == 0 ||
         strcmp(argv[i], "-s") == 0) &&
        i + 1 < (size_t)argc) {
      target_system = argv[++i];
    } else if (strcmp(argv[i], "--output-base") == 0 && i + 1 < (size_t)argc) {
      output_base = argv[++i];
    } else if (strcmp(argv[i], "--output-dir") == 0 && i + 1 < (size_t)argc) {
      explicit_output_dir = argv[++i];
    } else if (strcmp(argv[i], "--dark") == 0) {
      is_dark = 1;
    } else if (strcmp(argv[i], "--light") == 0) {
      is_dark = 0;
    } else if (strcmp(argv[i], "--list") == 0) {
      printf("Available Design Systems:\n");
      for (i = 0; i < driver_count; ++i) {
        printf("  - %-12s : %s\n", drivers[i]->name, drivers[i]->title);
      }
      return 0;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printf("Usage: %s [OPTIONS]\n\n", argv[0]);
      printf("Options:\n");
      printf("  -s, --system <name>       Target design system (material3, "
             "cupertino, fluent2, or all; default: all)\n");
      printf("  --output-base <path>      Base directory (default: "
             "../cc0-assets/c-multiplatform)\n");
      printf("  --output-dir <path>       Explicit output directory (overrides "
             "output-base for single system)\n");
      printf("  --dark                    Render dark mode variant\n");
      printf(
          "  --light                   Render light mode variant (default)\n");
      printf("  --list                    List all available design systems\n");
      printf("  -h, --help                Show this help message\n");
      return 0;
    }
  }

  printf("====================================================\n");
  printf("  Multi-Platform Design System Screenshot Tool     \n");
  printf("====================================================\n");
  printf("Target Design System : %s\n", target_system);
  printf("Output Base          : %s\n", output_base);
  printf("Theme Mode           : %s\n\n", is_dark ? "Dark" : "Light");

  for (i = 0; i < driver_count; ++i) {
    char dir_buf[512];
    struct design_system_options opts;

    if (strcmp(target_system, "all") != 0 &&
        strcmp(target_system, drivers[i]->name) != 0) {
      continue;
    }

    if (explicit_output_dir) {
      opts.output_dir = explicit_output_dir;
    } else {
#if defined(_MSC_VER)
      sprintf_s(dir_buf, sizeof(dir_buf), "%s/%s", output_base,
                drivers[i]->name);
#else
      snprintf(dir_buf, sizeof(dir_buf), "%s/%s", output_base,
               drivers[i]->name);
#endif
      opts.output_dir = dir_buf;
    }
    opts.is_dark = is_dark;

    printf("==> [%s] Generating %s screenshots...\n", drivers[i]->name,
           drivers[i]->title);
    printf("    Target directory: %s\n", opts.output_dir);
    rc = ensure_dir_exists(opts.output_dir);
    if (rc != UI_ERROR_NONE) {
      fprintf(stderr, "Failed to create directory %s (rc=%d)\n",
              opts.output_dir, (int)rc);
      return (int)rc;
    }

    rc = drivers[i]->render_all(&opts);
    if (rc != UI_ERROR_NONE) {
      fprintf(stderr, "Error rendering %s screenshots (rc=%d)\n",
              drivers[i]->name, (int)rc);
      return (int)rc;
    }
    printf("    [PASS] %s complete.\n\n", drivers[i]->title);
    processed++;
  }

  if (processed == 0) {
    fprintf(stderr,
            "Unknown design system: %s. Use --list to see available systems.\n",
            target_system);
    return 1;
  }

  printf("[SUCCESS] All design system screenshots generated successfully!\n");
  return 0;
}
