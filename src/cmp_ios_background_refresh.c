/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ios_background_refresh {
  int is_active;
  int current_task_id;
};

int cmp_ios_background_refresh_create(
    cmp_ios_background_refresh_t **out_refresh) {
  cmp_ios_background_refresh_t *r;

  if (!out_refresh)
    return CMP_ERROR_INVALID_ARG;

  r = (cmp_ios_background_refresh_t *)malloc(
      sizeof(cmp_ios_background_refresh_t));
  if (!r)
    return CMP_ERROR_OOM;

  r->is_active = 0;
  r->current_task_id = 0;
  *out_refresh = r;

  return CMP_SUCCESS;
}

int cmp_ios_background_refresh_destroy(cmp_ios_background_refresh_t *refresh) {
  if (!refresh)
    return CMP_ERROR_INVALID_ARG;
  free(refresh);
  return CMP_SUCCESS;
}

int cmp_ios_background_refresh_begin_task(cmp_ios_background_refresh_t *refresh,
                                          int *out_task_id) {
  if (!refresh || !out_task_id)
    return CMP_ERROR_INVALID_ARG;

  /* Mock: In reality this calls [[UIApplication sharedApplication]
   * beginBackgroundTaskWithExpirationHandler] */
  refresh->current_task_id++;
  refresh->is_active = 1;
  *out_task_id = refresh->current_task_id;

  return CMP_SUCCESS;
}

int cmp_ios_background_refresh_end_task(cmp_ios_background_refresh_t *refresh,
                                        int task_id) {
  if (!refresh)
    return CMP_ERROR_INVALID_ARG;

  /* Mock: In reality this calls [[UIApplication sharedApplication]
   * endBackgroundTask] */
  if (refresh->current_task_id == task_id) {
    refresh->is_active = 0;
  }

  return CMP_SUCCESS;
}
