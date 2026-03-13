/* clang-format off */
#include "cmpc/cmp_text_field.h"
#include "test_utils.h"
#include <string.h>
/* clang-format on */

int main(void) {
  CMP_TEST_EXPECT(cmp_text_field_test_validate_backend(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMPTextBackend b;
  b.vtable = NULL;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_backend(&b),
                  CMP_ERR_INVALID_ARGUMENT);

  CMPTextVTable v;
  memset(&v, 0, sizeof(v));
  b.vtable = &v;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_backend(&b),
                  CMP_ERR_UNSUPPORTED);

  CMPMeasureSpec ms = {99, 0};
  CMP_TEST_EXPECT(cmp_text_field_test_validate_measure_spec(ms),
                  CMP_ERR_INVALID_ARGUMENT);
  ms.mode = CMP_MEASURE_EXACTLY;
  ms.size = -1.0f;
  CMP_TEST_EXPECT(cmp_text_field_test_validate_measure_spec(ms), CMP_ERR_RANGE);

  CMPTextField field;
  memset(&field, 0, sizeof(field));
  field.text_backend = b;
  CMP_TEST_EXPECT(cmp_text_field_test_update_placeholder_metrics(&field),
                  CMP_ERR_UNSUPPORTED);

  return CMP_OK;
}
