#include "cmpc/cmp_anim.h"
#include <stdio.h>
int main() {
  CMPScalar val;
  cmp_anim_test_apply_ease(CMP_ANIM_EASE_STANDARD_ACCELERATE, 0.3f, &val);
  printf("done\n");
  return 0;
}
