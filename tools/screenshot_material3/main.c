/**
 * @file main.c
 * @brief CLI Tool for automatically generating Material 3 component
 * screenshots.
 */

/* clang-format off */
#include "components_render.h"
#include "rasterizer.h"
#include "material3/md3_color.h"
#include "ui_error.h"
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
  struct render_options opts;
  struct md3_color_scheme scheme;
  ui_color_t seed_color = UI_COLOR_ARGB(255, 103, 80, 164); /* M3 Baseline */
  const char *explicit_font = NULL;
  const char *loaded_font = NULL;
  const char *candidate_fonts[9];
  ui_error_t rc;
  int i;

  candidate_fonts[0] = "/System/Library/Fonts/Supplemental/Arial.ttf";
  candidate_fonts[1] = "/Library/Fonts/Arial.ttf";
  candidate_fonts[2] = "/System/Library/Fonts/SFNS.ttf";
  candidate_fonts[3] = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
  candidate_fonts[4] =
      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf";
  candidate_fonts[5] = "/usr/share/fonts/truetype/freefont/FreeSans.ttf";
  candidate_fonts[6] = "C:\\Windows\\Fonts\\arial.ttf";
  candidate_fonts[7] = "tests/minimal.ttf";
  candidate_fonts[8] = NULL;

  opts.output_dir = "../cc0-assets/c-multiplatform/material3";
  opts.is_dark = 0;
  opts.dpi_scale = 1.0f;

  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--output-dir") == 0 && i + 1 < argc) {
      opts.output_dir = argv[++i];
    } else if (strcmp(argv[i], "--font") == 0 && i + 1 < argc) {
      explicit_font = argv[++i];
    } else if (strcmp(argv[i], "--dpi-scale") == 0 && i + 1 < argc) {
      opts.dpi_scale = (float)atof(argv[++i]);
      if (opts.dpi_scale <= 0.0f) {
        opts.dpi_scale = 1.0f;
      }
    } else if (strcmp(argv[i], "--dark") == 0) {
      opts.is_dark = 1;
    } else if (strcmp(argv[i], "--light") == 0) {
      opts.is_dark = 0;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printf("Usage: %s [OPTIONS]\n", argv[0]);
      printf("  --output-dir <path>  Target directory (default: "
             "../cc0-assets/c-multiplatform/material3)\n");
      printf("  --font <path>        Path to TrueType font file (.ttf)\n");
      printf("  --dpi-scale <float>  Device Pixel Ratio scale factor (default: "
             "1.0)\n");
      printf("  --dark               Generate dark mode screenshots\n");
      printf(
          "  --light              Generate light mode screenshots (default)\n");
      printf("  --help, -h           Show this help\n");
      return 0;
    }
  }

  if (explicit_font) {
    if (rasterizer_load_font(explicit_font) == UI_ERROR_NONE) {
      loaded_font = explicit_font;
    }
  } else {
    for (i = 0; candidate_fonts[i]; ++i) {
      if (rasterizer_load_font(candidate_fonts[i]) == UI_ERROR_NONE) {
        loaded_font = candidate_fonts[i];
        break;
      }
    }
  }

  printf("====================================================\n");
  printf("  Material 3 Automated Screenshot Tool              \n");
  printf("====================================================\n");
  printf("Target Output Directory: %s\n", opts.output_dir);
  printf("Theme Mode: %s\n", opts.is_dark ? "Dark" : "Light");
  printf("DPI Scale: %.2f\n", (double)opts.dpi_scale);
  printf("Typography Engine: %s\n",
         loaded_font ? loaded_font : "Anti-aliased Fallback Engine");

  rc = ensure_dir_exists(opts.output_dir);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to create directory %s (rc=%d)\n", opts.output_dir,
            (int)rc);
    return (int)rc;
  }

  rc = rasterizer_set_dpi_scale(opts.dpi_scale);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to set rasterizer DPI scale (rc=%d)\n", (int)rc);
    return (int)rc;
  }

  rc = md3_color_scheme_create(seed_color, opts.is_dark,
                               MD3_PALETTE_MODE_TONAL_SPOT, &scheme);
  if (rc != UI_ERROR_NONE) {
    fprintf(stderr, "Failed to initialize Material 3 color scheme (rc=%d)\n",
            (int)rc);
    return (int)rc;
  }

  printf("\n[1/10] Rendering Buttons...\n");
  rc = render_all_buttons(&scheme, &opts);
  if (rc != UI_ERROR_NONE)
    return (int)rc;

  printf("[2/10] Rendering Datepickers & Timepickers...\n");
  rc = render_all_pickers(&scheme, &opts);
  if (rc != UI_ERROR_NONE)
    return (int)rc;

  printf("[3/10] Rendering Selection Controls & Inputs...\n");
  rc = render_all_selection_controls(&scheme, &opts);
  if (rc != UI_ERROR_NONE)
    return (int)rc;

  printf("[4/10] Rendering Containment & Cards...\n");
  rc = render_all_containment_and_cards(&scheme, &opts);
  if (rc != UI_ERROR_NONE)
    return (int)rc;

  printf("[5/10] Rendering Navigation Architecture...\n");
  rc = render_all_navigation(&scheme, &opts);
  if (rc != UI_ERROR_NONE)
    return (int)rc;

  printf("[6/10] Rendering Overlays, Dialogs & Menus...\n");
  rc = render_all_overlays_and_dialogs(&scheme, &opts);
  if (rc != UI_ERROR_NONE)
    return (int)rc;

  printf("[7/10] Rendering Data Tables & Trees...\n");
  rc = render_all_data_and_hierarchy(&scheme, &opts);
  if (rc != UI_ERROR_NONE)
    return (int)rc;

  printf("[8/10] Rendering Media & Workspace Shells...\n");
  rc = render_all_media_and_workspace(&scheme, &opts);
  if (rc != UI_ERROR_NONE)
    return (int)rc;

  printf("[9/10] Rendering Progress & Expressive Shapes...\n");
  rc = render_all_progress_and_shapes(&scheme, &opts);
  if (rc != UI_ERROR_NONE)
    return (int)rc;

  printf("[10/10] Rendering Catalog Hero Overview...\n");
  rc = render_catalog_overview(&scheme, &opts);
  if (rc != UI_ERROR_NONE)
    return (int)rc;

  printf("\n[SUCCESS] All Material 3 component screenshots generated in %s\n",
         opts.output_dir);
  return 0;
}
