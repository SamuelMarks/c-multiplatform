/* clang-format off */
#include "cmp_waapi.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_waapi_doc_timeline_init(cmp_waapi_doc_timeline_t *tl) {
  if (!tl) {
    return -1;
  }
  tl->base.current_time = 0.0f;
  tl->origin_time = 0.0f;
  return 0;
}

int cmp_waapi_effect_init(cmp_waapi_effect_t *effect, void *target) {
  if (!effect) {
    return -1;
  }
  effect->target = target;
  effect->iteration_duration = 0.0f;
  effect->active_duration = 0.0f;
  effect->end_time = 0.0f;
  return 0;
}

int cmp_waapi_animation_init(cmp_waapi_animation_t *anim, const char *id) {
  size_t len;
  if (!anim) {
    return -1;
  }
  anim->id = NULL;
  if (id) {
    len = strlen(id);
    anim->id = (char *)malloc(len + 1);
    if (!anim->id) {
      return -1;
    }
    memcpy(anim->id, id, len + 1);
  }
  anim->effect = NULL;
  anim->timeline = NULL;
  anim->start_time = 0.0f;
  anim->current_time = 0.0f;
  anim->playback_rate = 1.0f;
  return 0;
}

int cmp_waapi_animation_free(cmp_waapi_animation_t *anim) {
  if (!anim) {
    return -1;
  }
  if (anim->id) {
    free(anim->id);
    anim->id = NULL;
  }
  return 0;
}
