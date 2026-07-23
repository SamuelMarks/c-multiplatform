#ifndef UI_CSS_PARSER_H
#define UI_CSS_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_cssom.h"
/* clang-format on */

/**
 * @brief Parses CSS text into a stylesheet AST.
 *
 * Performs strict C89 state-machine tokenization and AST generation
 * to map the provided CSS text into the ui_css_stylesheet object model.
 *
 * @param css_text Null-terminated string containing the CSS source.
 * @param out_stylesheet Pointer to receive the parsed stylesheet.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_css_parse_stylesheet(const char *css_text,
                        struct ui_css_stylesheet **out_stylesheet);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_PARSER_H */
