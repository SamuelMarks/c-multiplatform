#ifndef TEST_VISUAL_REGRESSION_UTILS_H
#define TEST_VISUAL_REGRESSION_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmp_test_write_bmp(const char *filename, const unsigned char *pixels,
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
    return -1;

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
  return 0;
}

static int cmp_test_compare_golden(const char *golden_path,
                                   const unsigned char *pixels, int width,
                                   int height) {
  FILE *f = fopen(golden_path, "rb");
  int data_size = width * height * 4;
  unsigned char *golden_pixels;
  int i, diff_count = 0;

  if (!f) {
    /* Golden image doesn't exist, generate it */
    printf("Generating golden image: %s\n", golden_path);
    return cmp_test_write_bmp(golden_path, pixels, width, height);
  }

  fseek(f, 54, SEEK_SET); /* Skip header */
  golden_pixels = (unsigned char *)malloc(data_size);
  if (!golden_pixels) {
    fclose(f);
    return -1;
  }

  /* Note: BMP stores pixels bottom-up natively unless height is negative, our
     writer uses negative height, so it's top-down. We'll assume the BMP is
     top-down and matches our write_bmp format perfectly. */
  fread(golden_pixels, 1, data_size, f);
  fclose(f);

  /* Compare pixels */
  for (i = 0; i < width * height; i++) {
    unsigned char b = golden_pixels[i * 4 + 0];
    unsigned char g = golden_pixels[i * 4 + 1];
    unsigned char r = golden_pixels[i * 4 + 2];
    unsigned char a = golden_pixels[i * 4 + 3];

    if (pixels[i * 4 + 0] != r || pixels[i * 4 + 1] != g ||
        pixels[i * 4 + 2] != b || pixels[i * 4 + 3] != a) {
      diff_count++;
    }
  }

  free(golden_pixels);

  if (diff_count > 0) {
    printf("Visual regression detected in %s! %d pixels differ.\n", golden_path,
           diff_count);
    return -1; /* Fail */
  }

  return 0; /* Pass */
}

#endif /* TEST_VISUAL_REGRESSION_UTILS_H */