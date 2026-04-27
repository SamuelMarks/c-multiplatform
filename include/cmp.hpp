#pragma once
/* clang-format off */
#include "cmp.h"
#include <memory>
#include <stdexcept>
#include <string>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


CMP_EXEMPT(/* namespace */
#define CMP_NS_BEGIN namespace cmp {
CMP_NS_BEGIN)

struct cmp_theme_deleter {
    CMP_EXEMPT(void operator()(cmp_theme_t* t) const) { cmp_theme_destroy(t); }
};

using Theme = std::unique_ptr<cmp_theme_t, cmp_theme_deleter>;

CMP_EXEMPT(inline Theme make_theme()) {
    cmp_theme_t* t = nullptr;
    if (cmp_theme_create(&t) != CMP_SUCCESS) {
        throw std::runtime_error("Failed to create cmp_theme");
    }
    return Theme(t);
}

} /* namespace cmp */
