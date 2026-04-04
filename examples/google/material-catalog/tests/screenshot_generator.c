/* clang-format off */
#include "material_catalog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Helper to write an RGBA buffer to a BMP format image.
 */
static void write_bmp(const char *filename, const unsigned char *pixels,
                      int width, int height) {
  FILE *f = fopen(filename, "wb");
  int i;
  int row_size = width * 4;
  int data_size = row_size * height;
  int file_size = 54 + data_size;
  unsigned char header[54] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0,
                              40,  0,   0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 1, 0,
                              32,  0,   0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,
                              0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0,  0};
  if (!f)
    return;

  header[2] = (unsigned char)(file_size);
  header[3] = (unsigned char)(file_size >> 8);
  header[4] = (unsigned char)(file_size >> 16);
  header[5] = (unsigned char)(file_size >> 24);

  header[18] = (unsigned char)(width);
  header[19] = (unsigned char)(width >> 8);
  header[20] = (unsigned char)(width >> 16);
  header[21] = (unsigned char)(width >> 24);

  header[22] = (unsigned char)(-height);
  header[23] = (unsigned char)((-height) >> 8);
  header[24] = (unsigned char)((-height) >> 16);
  header[25] = (unsigned char)((-height) >> 24);

  fwrite(header, 1, 54, f);
  for (i = 0; i < width * height; i++) {
    unsigned char pixel[4];
    pixel[0] = pixels[i * 4 + 2]; /* B */
    pixel[1] = pixels[i * 4 + 1]; /* G */
    pixel[2] = pixels[i * 4 + 0]; /* R */
    pixel[3] = pixels[i * 4 + 3]; /* A */
    fwrite(pixel, 1, 4, f);
  }
  fclose(f);
}

int main(void) {
  cmp_window_config_t config;
  material_catalog_state_t state;
  void *pixels;
  int w, h;
  int comp_count = 0;
  int i, j;
  const material_component_t *comps;

  memset(&config, 0, sizeof(config));
  config.width = 800;
  config.height = 600;

  cmp_window_system_init();

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    printf("Failed to init state\n");
    return 1;
  }

  if (material_catalog_create_ui(&state) != MATERIAL_CATALOG_SUCCESS) {
    printf("Failed to create UI\n");
    return 1;
  }

  comps = material_catalog_get_components(&comp_count);

  for (i = 0; i < comp_count; i++) {
    int ex_count = 0;
    material_catalog_get_examples(comps[i].id, &ex_count);

    for (j = 0; j < ex_count; j++) {
      char filename[256];
      sprintf(filename, "screenshot_%d_%d.bmp", comps[i].id, j);

      material_catalog_navigate_to(&state, CATALOG_SCREEN_EXAMPLE_VIEWER,
                                   comps[i].id, j);
      material_catalog_recompose_ui(&state);

      /* Give the layout engine a frame to render */
      cmp_window_render_test_frame(state.window);

      {
        int res = cmp_test_capture_snapshot(state.window, &pixels, &w, &h);
        if (res == 0 && pixels != NULL) {
          printf("Captured snapshot %dx%d for %s\n", w, h, filename);
          write_bmp(filename, (const unsigned char *)pixels, w, h);
        } else {
          printf("Failed to capture snapshot: res=%d, pixels=%p\n", res,
                 pixels);
        }
      }
    }
  }

  material_catalog_cleanup(&state);
  cmp_window_system_shutdown();
  return 0;
}
