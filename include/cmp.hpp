#pragma once
#include "cmp.h"
#include <memory>
#include <stdexcept>
#include <string>

namespace cmp {

struct cmp_theme_deleter {
    void operator()(cmp_theme_t* t) const { cmp_theme_destroy(t); }
};

using Theme = std::unique_ptr<cmp_theme_t, cmp_theme_deleter>;

inline Theme make_theme() {
    cmp_theme_t* t = nullptr;
    if (cmp_theme_create(&t) != CMP_SUCCESS) {
        throw std::runtime_error("Failed to create cmp_theme");
    }
    return Theme(t);
}

} // namespace cmp
