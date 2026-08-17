/* clang-format off */
#ifndef C_CI_NO_DISCARD_H
#define C_CI_NO_DISCARD_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__cplusplus) && __cplusplus >= 201703L
#define NO_DISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#define NO_DISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
#define NO_DISCARD _Check_return_
#else
#define NO_DISCARD
#endif

#ifdef __cplusplus
}
#endif

#endif /* C_CI_NO_DISCARD_H */
/* clang-format on */
