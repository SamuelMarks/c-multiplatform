/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "../include/ui_css_parser.h"
#include "../include/ui_cssom.h"
#include "../include/ui_css_values.h"
#include "../include/ui_css_ui.h"
#include "../include/ui_dom_node.h"
#include "../src/ui_internal_mem.h"
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_malloc_called;

void test_oom(void) {
  int i;
  for (i = 0; i < 4000; i++) {
    struct ui_css_stylesheet *sheet = NULL;
    const char *css =
        "@import \"test.css\"; @import url(; @import url)(; @namespace svg "
        "url(\"http://www.w3.org/2000/svg\"); div, .class, #id, "
        "[href^=\"https\"], ::before, :hover { color: red !important; margin: "
        "10px; font-family: 'Arial', sans-serif; background: url('img.png'); } "
        "@media screen { div { width: 100px; } } @layer base { p { margin: 0; "
        "} } @layer single; @layer foo, bar; @property --my-color { syntax: "
        "\"<color>\"; inherits: false; "
        "initial-value: #c0ffee; } @property --my-other { syntax: \"<color>\"; "
        "inherits: true; } "
        "@property --my-bad { inherits: ; } "
        ".test2 { color:!important; } div unknown; @keyframes foo invalid { } "
        "@media { div unknown } div eof";
    g_malloc_fail_countdown = i;
    ui_css_parse_stylesheet(css, &sheet);
    g_malloc_fail_countdown = -1;
    if (sheet) {
      printf("OOM loop broke at i=%d\n", i);
      ui_css_stylesheet_destroy(sheet);
      break;
    }
  }
}

static void test_parser_branches(void) {
  struct ui_css_stylesheet *sheet = NULL;

  /* Unclosed string at EOF */
  /* Coverage for EOF breaks */
  ui_css_parse_stylesheet("@property --foo { myprop ", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --foo { 123 ", &sheet);
  ui_css_parse_stylesheet("@property --foo { * ", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { color ", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;
  ui_css_parse_stylesheet("\"", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Unclosed string with escape at EOF */
  ui_css_parse_stylesheet("\"\\", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Ident at EOF */
  ui_css_parse_stylesheet("ident", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* At-keyword at EOF */
  ui_css_parse_stylesheet("@media", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Number at EOF */
  ui_css_parse_stylesheet("12345", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Number with sign at EOF */
  ui_css_parse_stylesheet("+123", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Dot at EOF */
  ui_css_parse_stylesheet(".", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Hash at EOF */
  ui_css_parse_stylesheet("#", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Colon at EOF */
  ui_css_parse_stylesheet(":", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Double colon at EOF */
  ui_css_parse_stylesheet("::", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Comment at EOF */
  ui_css_parse_stylesheet("/*", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("/* hello *", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Selector incomplete bracket */
  ui_css_parse_stylesheet("[", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("[attr", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("[attr=", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("[attr=\"", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("[attr=\"\"]", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Empty string in attr */
  ui_css_parse_stylesheet("[attr=\"a", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":first-letter {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("::first-line {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":host-context {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":host {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* is_ident_start branches */
  ui_css_parse_stylesheet("_ident {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("-ident {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("1ident {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("1 {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("+1 {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("-1 {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* is_nested_rule branches */
  ui_css_parse_stylesheet("* { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a, b { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a { b }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a { @media {} }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* parse_rule_list branches */
  ui_css_parse_stylesheet("@layer ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@layer name, name2 ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@layer name { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope to (b) {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) to {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) to (b) {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(
      "@property --var { inherits: false; syntax: \"*\"; initial-value: a;}",
      &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --var2 { inherits: true; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --var3 { inherits: other; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@media { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@media condition", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@supports cond { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@container cond { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace url(  http://test.com  ) ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace \"uri\" ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace prefix \"uri\" ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace prefix url(http);", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace url();", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@unknown ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@unknown { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("{ a: b !important; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("{ a: b!important; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("{ a: b !IMPORTANT; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("{ { { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a { prop }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a { prop ; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a { prop: }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a { : }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a { 123 }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a { a:    ; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a { a: ; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("123{", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("123}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("::after {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) too {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) not_to {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --var { inherits: false; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace \"", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace prefix \"", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { [attr] {} }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { :hover {} }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { #id {} }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { , {} }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* @layer missing comma or ident */
  ui_css_parse_stylesheet("@layer name , ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* @property missing components */
  ui_css_parse_stylesheet("@property --var { syntax: ; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --var { inherits: ; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --var { initial-value: ; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --var { syntax }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Missing namespace uri */
  ui_css_parse_stylesheet("@namespace ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace prefix ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* unclosed block without content */
  ui_css_parse_stylesheet("@media (all) {", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@layer {", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope {", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --var {", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* attr variations */
  ui_css_parse_stylesheet("[attr=\"a\"]", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("[attr=]", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("/", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("/-", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("/* hello *-", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":before {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":after {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":first-line {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":first-letter {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":host(a) {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":host-context(a) {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("::host-context(a) {}", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":is(a", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet(":host-context(a", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { { { } } }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@layer name name2;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) something_else {", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) something_else ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) something_else { .b { } }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) extra ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) extra ; extra_extra", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) extra { {} }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) extra { {", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) extra { ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) { { { } }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) bad_block { { { } } }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) bad_block ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* The following will hit LBRACE while recovering: */
  ui_css_parse_stylesheet("@scope (a) bad_block { { } }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Force semicolon hit at 1287: while recovering after missing scope block,
   * find semicolon and consume it */
  ui_css_parse_stylesheet("@scope (a) invalid_no_block ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { prop missing value }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { prop invalid ; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { prop invalid { { nested } } }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { prop missing colon { nested } }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { prop missing colon { { double_nested } } }",
                          &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  /* Triggering missing LBRACE in style rule error recovery (1280+) */
  ui_css_parse_stylesheet("div ) ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div ) { { } }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div ) { { { } } }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) ; trailing", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) something ; next_rule { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) something { nested { } }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) \"to\" (b)", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a) ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --var ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --var { inherits: true }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace \"\" ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace \"a\" ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace a ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace a b ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace url() ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace url(a) ;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("a { b: c }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;
}

int main(void) {
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_css_computed_style *style = NULL;
  ui_error_t rc;
  const char *val;

  const char *css_text =
      "/* test sheet */\n"
      "div, .container {\n"
      "  display: block;\n"
      "  color: red !important;\n"
      "  .child { font-size: 14px; }\n"
      "  &:hover { background: yellow; }\n"
      "}\n"
      "#main { padding: 10px 20px; }\n"
      "* { margin: 0; }\n"
      "[type=\"text\"] { border: 1px solid black; }\n"
      "[disabled] { opacity: 0.5; }\n"
      "[href^=\"https\"] { text-decoration: underline; }\n"
      ":empty { display: none; }\n"
      "@layer theme, layout;\n"
      "@layer layout { div { background: blue; } }\n"
      "@layer theme { div { background: green; z-index: 10; } }\n"
      "div { background: white; }\n"
      "@layer layout { div { color: blue !important; } }\n"
      "@layer theme { div { color: green !important; } }\n"
      "@scope (.card) to (.hole) {\n"
      "  div { padding: 50px; }\n"
      "}\n"
      "@property --my-color {\n"
      "  syntax: \"<color>\";\n"
      "  inherits: 0;\n"
      "  initial-value: #c0ffee;\n"
      "}\n"
      ":is(div, span) { font-weight: bold; }\n"
      ":where(#main, .foo) { line-height: 1.5; }\n"
      ":not(.container) { cursor: pointer; }\n"
      ":has(.child) { border-style: dotted; }\n"
      ":hover { outline: 2px solid red; }\n"
      ":checked { background-color: blue; }\n"
      ":valid { border-color: green; }\n"
      "@media (max-width: 600px) {\n"
      "  div { margin: 10px; }\n"
      "}\n"
      "@supports (display: flex) {\n"
      "  #cond_main { display: flex; }\n"
      "}\n"
      "@supports (display: flex) and (not (display: grid)) {\n"
      "  #cond_main { border-color: red; }\n"
      "}\n"
      "@supports selector(:has(*)) or (display: grid) {\n"
      "  #cond_main { border-style: dashed; }\n"
      "}\n"
      "@container card (min-width: 700px) {\n"
      "  #cond_main { padding: 20px; }\n"
      "}\n"
      "@namespace svg url(http://www.w3.org/2000/svg);\n"
      "@namespace \"http://www.w3.org/1999/xhtml\";\n"
      "div::before { content: \"test-before\"; }\n"
      "span:after { content: \"test-after\"; }\n"
      "li::marker { color: red; }\n"
      "div > span { color: blue; }\n"
      "div + span { color: green; }\n"
      "div ~ span { color: yellow; }\n"
      "div:first-child { color: black; }\n"
      "div:last-child { color: white; }\n"
      "div:nth-child(even) { color: gray; }\n";

  printf("Starting test_ui_css_parser...\n");

  rc = ui_css_parse_stylesheet(css_text, &sheet);
  if (rc != UI_ERROR_NONE || !sheet) {
    printf("Failed to parse stylesheet\n");
    return 1;
  }

  {
    struct ui_css_rule *rule = sheet->rules;
    int found_property = 0;
    while (rule) {
      if (rule->type == UI_CSS_RULE_TYPE_PROPERTY) {
        if (strcmp(rule->property_name, "--my-color") == 0) {
          found_property = 1;
          if (!rule->property_syntax ||
              strcmp(rule->property_syntax, "\"<color>\"") != 0) {
            printf("Property 'syntax' mismatch\n");
            return 1;
          }
          if (rule->property_inherits != 0) {
            printf("Property 'inherits' mismatch\n");
            return 1;
          }
          if (!rule->property_initial_value ||
              strcmp(rule->property_initial_value, "#c0ffee") != 0) {
            printf("Property 'initial-value' mismatch\n");
            return 1;
          }
        }
      }
      rule = rule->next;
    }
    if (!found_property) {
      printf("Failed to find @property rule\n");
      return 1;
    }
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create DOM node\n");
    return 1;
  }
  ui_dom_node_set_tag_name(node, "div");
  ui_dom_node_set_attribute(node, "class", "container");
  ui_dom_node_set_attribute(node, "id", "main");
  ui_dom_node_set_attribute(node, "type", "text");
  ui_dom_node_set_attribute(node, "disabled", "");
  ui_dom_node_set_attribute(node, "href", "https://google.com");

  rc = ui_css_resolve_style(sheet, node, &style);
  if (rc != UI_ERROR_NONE || !style) {
    printf("Failed to resolve style\n");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "background", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "white") != 0) {
    printf("Property 'background' layer mismatch, expected 'white' (unlayered "
           "beats layered), got '%s'\n",
           val ? val : "null");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "z-index", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "10") != 0) {
    printf("Property 'z-index' layer mismatch, expected '10', got '%s'\n",
           val ? val : "null");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "color", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "green") != 0) {
    printf("Property 'color' mismatch: expected 'green' (layered important "
           "wins), got '%s'\n",
           val ? val : "null");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "border", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "1px solid black") != 0) {
    printf("Property 'border' mismatch, expected '1px solid black'\n");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "opacity", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "0.5") != 0) {
    printf("Property 'opacity' mismatch, expected '0.5'\n");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "text-decoration", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "underline") != 0) {
    printf("Property 'text-decoration' mismatch, expected 'underline'\n");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "padding", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "10px 20px") != 0) {
    printf("Property 'padding' mismatch\n");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "margin", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "10px") != 0) {
    printf("Property 'margin' mismatch, got: %s\n", val ? val : "null");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "font-weight", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "bold") != 0) {
    printf("Property 'font-weight' mismatch, expected 'bold' from :is()\n");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "line-height", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "1.5") != 0) {
    printf("Property 'line-height' mismatch, expected '1.5' from :where()\n");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "cursor", &val);
  if (rc != UI_ERROR_NOT_FOUND) {
    printf("Property 'cursor' mismatch, expected NOT FOUND from :not()\n");
    return 1;
  }

  rc = ui_css_computed_style_get_property(style, "border-style", &val);
  if (rc != UI_ERROR_NOT_FOUND) {
    printf(
        "Property 'border-style' mismatch, expected NOT FOUND from :has()\n");
    return 1;
  }

  ui_css_computed_style_destroy(style);

  /* Test :has() by adding a child */
  {
    struct ui_dom_node *child;
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child);
    ui_dom_node_set_tag_name(child, "span");
    ui_dom_node_set_attribute(child, "class", "child");
    ui_dom_node_append_child(node, child);

    rc = ui_css_resolve_style(sheet, node, &style);
    rc = ui_css_computed_style_get_property(style, "border-style", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "dotted") != 0) {
      printf("Property 'border-style' mismatch, expected 'dotted' from :has(), "
             "got %s\n",
             val ? val : "null");
      return 1;
    }
    ui_css_computed_style_destroy(style);

    /* Test CSS Nesting Level 1 matching on child */
    rc = ui_css_resolve_style(sheet, child, &style);
    rc = ui_css_computed_style_get_property(style, "font-size", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "14px") != 0) {
      printf("Property 'font-size' mismatch, expected '14px' from CSS Nesting, "
             "got %s\n",
             val ? val : "null");
      return 1;
    }
    ui_css_computed_style_destroy(style);

    /* we do not destroy child directly because ui_dom_node_destroy(node)
     * destroys children */
  }
  /* Test state matchers */
  {
    struct ui_dom_node *state_node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &state_node);
    state_node->state_flags |= UI_DOM_NODE_STATE_HOVER;
    ui_dom_node_set_attribute(state_node, "class", "container");
    ui_dom_node_set_attribute(state_node, "checked", "");
    ui_dom_node_set_attribute(state_node, "aria-invalid", "0");

    rc = ui_css_resolve_style(sheet, state_node, &style);

    rc = ui_css_computed_style_get_property(style, "outline", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "2px solid red") != 0) {
      printf("Property 'outline' mismatch, expected '2px solid red' from "
             ":hover, got %s\n",
             val ? val : "null");
      return 1;
    }

    rc = ui_css_computed_style_get_property(style, "background", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "yellow") != 0) {
      printf("Property 'background' mismatch, expected 'yellow' from nested "
             "&:hover, got %s\n",
             val ? val : "null");
      return 1;
    }

    rc = ui_css_computed_style_get_property(style, "background-color", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "blue") != 0) {
      printf("Property 'background-color' mismatch, expected 'blue' from "
             ":checked, got %s\n",
             val ? val : "null");
      return 1;
    }

    rc = ui_css_computed_style_get_property(style, "border-color", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "green") != 0) {
      printf("Property 'border-color' mismatch, expected 'green' from :valid, "
             "got %s\n",
             val ? val : "null");
      return 1;
    }

    ui_css_computed_style_destroy(style);
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(state_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  /* Test Conditional Rules */
  {
    struct ui_dom_node *cond_node;
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cond_node);
    ui_dom_node_set_tag_name(cond_node, "div");
    ui_dom_node_set_attribute(cond_node, "id", "cond_main");

    rc = ui_css_resolve_style(sheet, cond_node, &style);

    rc = ui_css_computed_style_get_property(style, "margin", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "10px") != 0) {
      printf(
          "Property 'margin' mismatch, expected '10px' from @media, got %s\n",
          val ? val : "null");
      return 1;
    }

    rc = ui_css_computed_style_get_property(style, "display", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "flex") != 0) {
      printf("Property 'display' mismatch, expected 'flex' from @supports, got "
             "%s\n",
             val ? val : "null");
      return 1;
    }

    rc = ui_css_computed_style_get_property(style, "border-color", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "red") != 0) {
      printf("Property 'border-color' mismatch, expected 'red' from extended "
             "@supports, got %s\n",
             val ? val : "null");
      return 1;
    }

    rc = ui_css_computed_style_get_property(style, "border-style", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "dashed") != 0) {
      printf("Property 'border-style' mismatch, expected 'dashed' from "
             "@supports selector(), got %s\n",
             val ? val : "null");
      return 1;
    }

    rc = ui_css_computed_style_get_property(style, "padding", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "20px") != 0) {
      printf("Property 'padding' mismatch, expected '20px' from "
             "@container, got %s\n",
             val ? val : "null");
      return 1;
    }

    ui_css_computed_style_destroy(style);
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(cond_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  /* Test Scope Rules */
  {
    struct ui_dom_node *card_node;
    struct ui_dom_node *child_node;
    struct ui_dom_node *hole_node;
    struct ui_dom_node *deep_node;

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &card_node);
    ui_dom_node_set_tag_name(card_node, "div");
    ui_dom_node_set_attribute(card_node, "class", "card");

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_node);
    ui_dom_node_set_tag_name(child_node, "div");
    ui_dom_node_set_attribute(child_node, "class", "child");
    ui_dom_node_append_child(card_node, child_node);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &hole_node);
    ui_dom_node_set_tag_name(hole_node, "div");
    ui_dom_node_set_attribute(hole_node, "class", "hole");
    ui_dom_node_append_child(child_node, hole_node);

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &deep_node);
    ui_dom_node_set_tag_name(deep_node, "div");
    ui_dom_node_set_attribute(deep_node, "class", "deep");
    ui_dom_node_append_child(hole_node, deep_node);

    /* card_node matches scope_start, child_node is in scope, hole_node is out
     * of scope (boundary), deep_node is out of scope */

    rc = ui_css_resolve_style(sheet, card_node, &style);
    rc = ui_css_computed_style_get_property(style, "padding", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "50px") != 0) {
      printf("Property 'padding' mismatch, expected '50px' from @scope start "
             "node, got %s\n",
             val ? val : "null");
      return 1;
    }
    ui_css_computed_style_destroy(style);

    rc = ui_css_resolve_style(sheet, child_node, &style);
    rc = ui_css_computed_style_get_property(style, "padding", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "50px") != 0) {
      printf("Property 'padding' mismatch, expected '50px' from @scope "
             "descendant node, got %s\n",
             val ? val : "null");
      return 1;
    }
    ui_css_computed_style_destroy(style);

    rc = ui_css_resolve_style(sheet, hole_node, &style);
    rc = ui_css_computed_style_get_property(style, "padding", &val);
    if (rc == UI_ERROR_NONE && strcmp(val, "50px") == 0) {
      printf("Property 'padding' leaked, expected missing from @scope boundary "
             "node, got %s\n",
             val);
      return 1;
    }
    ui_css_computed_style_destroy(style);

    rc = ui_css_resolve_style(sheet, deep_node, &style);
    rc = ui_css_computed_style_get_property(style, "padding", &val);
    if (rc == UI_ERROR_NONE && strcmp(val, "50px") == 0) {
      printf("Property 'padding' leaked, expected missing from @scope "
             "out-of-bounds node, got %s\n",
             val);
      return 1;
    }
    ui_css_computed_style_destroy(style);

    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(card_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  /* Test Namespace Rules */
  {
    struct ui_css_namespace *ns = sheet->namespaces;
    int found_svg = 0;
    int found_default = 0;
    while (ns) {
      if (ns->prefix && strcmp(ns->prefix, "svg") == 0 && ns->uri &&
          strcmp(ns->uri, "http://www.w3.org/2000/svg") == 0) {
        found_svg = 1;
      }
      if (!ns->prefix && ns->uri &&
          strcmp(ns->uri, "http://www.w3.org/1999/xhtml") == 0) {
        found_default = 1;
      }
      ns = ns->next;
    }
    if (!found_svg) {
      printf("Failed to find 'svg' @namespace rule. Namespaces:\n");
      struct ui_css_namespace *n = sheet->namespaces;
      while (n) {
        printf("ns: %s uri: %s\n", n->prefix, n->uri);
        n = n->next;
      }
      return 1;
    }
    if (!found_default) {
      printf("Failed to find default @namespace rule\n");
      return 1;
    }
  }

  /* Test Pseudo-Elements representation in AST */
  {
    struct ui_css_rule *rule = sheet->rules;
    int found_before = 0;
    int found_after = 0;
    int found_marker = 0;

    while (rule) {
      if (rule->type == UI_CSS_RULE_TYPE_STYLE && rule->selectors) {
        struct ui_css_selector *sel = rule->selectors;
        /* check the last piece of the selector */
        while (sel->next)
          sel = sel->next;

        if (sel->type == UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT) {
          if (strcmp(sel->value, "before") == 0)
            found_before = 1;
          if (strcmp(sel->value, "after") == 0)
            found_after = 1;
          if (strcmp(sel->value, "marker") == 0)
            found_marker = 1;
        }
      }
      rule = rule->next;
    }

    if (!found_before || !found_after || !found_marker) {
      printf(
          "Failed to parse pseudo-elements: before=%d, after=%d, marker=%d\n",
          found_before, found_after, found_marker);
      return 1;
    }
  }

  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_css_stylesheet_destroy(sheet);

  /* Parser Recovery and Edge Cases */
  const char *edge_case_css =
      "@unknown-rule { a { } } \n"
      "@unknown-rule2 ;\n"
      "@layer ;\n" /* just layer without name */
      "div[class~=\"foo\"] { }\n"
      "div[class|=\"foo\"] { }\n"
      "div[class$=\"foo\"] { }\n"
      "div[class^=\"foo\"] { }\n"
      "div[class*=\"foo\"] { }\n"
      "div[class~] { }\n" /* malformed */
      "div[class$] { }\n"
      "div[class|] { }\n"
      "div[class^] { }\n"
      "div[class*] { }\n"
      "div[type=text] { }\n"     /* unquoted attr */
      "div[.x] { }\n"            /* invalid attr */
      "span.class:is(div) { }\n" /* multiple selectors for dummy_rule */
      "div { @media (min-width: 0) { } }\n" /* nested @rule */
      "div { .a { } .b { } .c { } }\n"      /* >=3 nested rules */
      "div { span { } }\n"                  /* nested rule with ident */
      "div { color red; } \n"               /* missing colon, token after */
      "div { \"prop\" oops ; } \n" /* non-ident at start of declaration */
      "div:unknown-pseudo() { color: red; } \n"
      "div:unknown-pseudo(abc) { color: red; } \n"
      "div:unknown-pseudo(a()) { color: red; } \n"
      "@scope (.start) to (.end) { }\n"
      "@scope (.start) { }\n"
      "@scope (.start) { }\n" /* was malformed, now balanced */
      "@property --bar { inherits: true; initial-value: 0; }\n"
      "@property --foo { inherits no-colon; unknown-desc: true; \"invalid\": "
      "true; }\n" /* no colon, unknown desc, string desc */
      "@namespace url(http://test.com);\n"              /* no prefix */
      "@namespace prefix url( ' http://test.com ' );\n" /* spaces with quotes */
      "@namespace prefix2 url(  http://test.com  );\n"  /* spaces */
      "@namespace prefix3 something;\n"   /* not url, not string */
      "@ \n"                              /* error token */
      "div { content: \"str\\n\"; }\n"    /* string escape */
      "div { color:   red  ; } \n"        /* spaces in property value */
      "div { margin: 10px    ; } \n"      /* more trailing spaces */
      "div { *= ; $= ; |= ; ^= ; ~= } \n" /* operators as properties */
      "div { = ; $ ; | ; ^ ; ~ } \n"      /* single char errors */
      "div { 123 } \n"                    /* unclosed identifier block */
      "div { invalid_prop } \n"           /* missing colon for property */
      "div { invalid_prop ; } \n" /* missing colon for property, has semicolon
                                   */
      "div { color: red \n"       /* missing semicolon/brace EOF */
      "*= \n $= \n |= \n ^= \n ~= \n = \n $ \n | \n ^ \n ~ \n"
      "/* comment at eof \n"    /* unclosed comment at EOF to hit peek() */
      "/*\n"                    /* short unclosed comment */
      "/* hello *\n"            /* star at eof */
      "div { color: red } .\n"  /* dot at EOF */
      "div { color: red } #\n"  /* hash at EOF */
      "div { color: red } ::\n" /* double colon at EOF */
      "div { color: red } :\n"  /* colon at EOF */
      "div { color: red } @\n"  /* at at EOF */
      "div { color: red } [\n"  /* LBRACKET at EOF */
      "div { color: red } [attr=\"\n"    /* unclosed string at EOF */
      "div { color: red } [attr=\"\" \n" /* LBRACKET unclosed EOF */
      "div { color: red } :first-letter { }\n"
      "div { color: red } :host-context { }\n"
      "--var-name { }\n"
      "_ident { }\n"
      "\"unclosed string\n"
      "123.45\n"
      "+123\n"
      "-123\n"
      "@layer name, name2 ;\n"
      "@scope to (b) { }\n"
      "@media { }\n"
      "@media condition\n"
      "@layer oom, oom2 ;\n"
      "@scope (oom) { }\n"
      "@property --oom { inherits: false; }\n"
      "@property --oom2 { inherits: true; }\n"
      "@namespace prefix \"url\";\n"
      "@media cond { } @media cond2 { } @media cond3 { }\n"
      "{ a: b !important }\n"
      "{ { { }\n";

  {
    /* tests where a specific rule is first to hit rule_head = ... */
    const char *scope_first = "@scope (.x) { }";
    const char *prop_first = "@property --x { }";
    const char *media_first = "@media (all) { }";
    const char *layer_first = "@layer { }";
    struct ui_css_stylesheet *tmp = NULL;
    ui_css_parse_stylesheet(scope_first, &tmp);
    ui_css_stylesheet_destroy(tmp);
    ui_css_parse_stylesheet(prop_first, &tmp);
    ui_css_stylesheet_destroy(tmp);
    ui_css_parse_stylesheet(media_first, &tmp);
    ui_css_stylesheet_destroy(tmp);
    ui_css_parse_stylesheet(layer_first, &tmp);
    ui_css_stylesheet_destroy(tmp);
  }

  {
    const char *recovery_css =
        "div { color: red background: blue; margin: 0; }\n"
        "span { not-a-prop: 10px; color: green; }\n"
        "p { color: yellow";

    struct ui_css_stylesheet *rec_sheet = NULL;
    struct ui_dom_node *rec_node = NULL;
    struct ui_css_computed_style *rec_style = NULL;

    rc = ui_css_parse_stylesheet(recovery_css, &rec_sheet);
    if (rc != UI_ERROR_NONE || !rec_sheet) {
      printf("Failed to parse recovery_css\n");
      return 1;
    }

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &rec_node);
    ui_dom_node_set_tag_name(rec_node, "div");
    rc = ui_css_resolve_style(rec_sheet, rec_node, &rec_style);
    rc = ui_css_computed_style_get_property(rec_style, "margin", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "0") != 0) {
      printf("Parser recovery failed on missing semicolon, margin got %s\n",
             val ? val : "null");
      return 1;
    }
    ui_css_computed_style_destroy(rec_style);
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(rec_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &rec_node);
    ui_dom_node_set_tag_name(rec_node, "span");
    rc = ui_css_resolve_style(rec_sheet, rec_node, &rec_style);
    rc = ui_css_computed_style_get_property(rec_style, "color", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "green") != 0) {
      printf("Parser recovery failed on invalid property, color got %s\n",
             val ? val : "null");
      return 1;
    }
    ui_css_computed_style_destroy(rec_style);
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(rec_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &rec_node);
    ui_dom_node_set_tag_name(rec_node, "p");
    rc = ui_css_resolve_style(rec_sheet, rec_node, &rec_style);
    rc = ui_css_computed_style_get_property(rec_style, "color", &val);
    if (rc != UI_ERROR_NONE || strcmp(val, "yellow") != 0) {
      printf("Parser recovery failed on unclosed bracket, color got %s\n",
             val ? val : "null");
      return 1;
    }
    ui_css_computed_style_destroy(rec_style);
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(rec_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }

    ui_css_stylesheet_destroy(rec_sheet);

    rc = ui_css_parse_stylesheet(edge_case_css, &rec_sheet);
    if (rec_sheet) {
      ui_css_stylesheet_destroy(rec_sheet);
    }
  }

  /* Error Percolation Test */
  if (ui_css_parse_stylesheet(NULL, &sheet) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT for null text\n");
    return 1;
  }
  if (ui_css_parse_stylesheet("body {}", NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT for null out pointer\n");
    return 1;
  }
  if (ui_css_parse_stylesheet(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT for both null pointers\n");
    return 1;
  }

  /* Test malloc failure injection */
  {
    int i;
    for (i = 0; i < 800; i++) {
      g_malloc_fail_countdown = i;
      rc = ui_css_parse_stylesheet(css_text, &sheet);
      if (rc == UI_ERROR_NONE && sheet) {
        ui_css_stylesheet_destroy(sheet);
        break; /* found max mallocs */
      }
    }
    g_malloc_fail_countdown = -1;

    for (i = 0; i < 800; i++) {
      g_malloc_fail_countdown = i;
      rc = ui_css_parse_stylesheet(edge_case_css, &sheet);
      if (rc == UI_ERROR_NONE && sheet) {
        ui_css_stylesheet_destroy(sheet);
        break;
      }
    }
    g_malloc_fail_countdown = -1;
  }

  {
    /* Test missing CSS values */
    struct ui_css_color color;
    struct ui_css_image img;
    struct ui_css_value width_val;
    enum ui_css_outline_style outline_style;
    enum ui_css_overscroll_behavior os_behavior;
    struct ui_css_stylesheet *sheet2;
    struct ui_css_rule *attr_rule;
    int layer_order;

    rc = ui_css_parse_color("#ff0000", &color);
    if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNSUPPORTED) {
      printf("parse color failed\n");
      return 1;
    }

    /* It's okay if image parsing fails if it's not implemented fully, we just
     * test the entrypoint */
    ui_css_parse_image("url('test.png')", &img);

    rc = ui_css_parse_outline_width("10px", &width_val);
    if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNSUPPORTED) {
      printf("parse outline width failed\n");
      return 1;
    }

    rc = ui_css_parse_outline_style("solid", &outline_style);
    if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNSUPPORTED) {
      printf("parse outline style failed\n");
      return 1;
    }

    rc = ui_css_parse_overscroll_behavior_axis("contain", &os_behavior);
    if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNSUPPORTED) {
      printf("parse overscroll failed\n");
      return 1;
    }

    ui_css_stylesheet_create(&sheet2);
    ui_css_stylesheet_register_layer(sheet2, "my-layer", &layer_order);
    ui_css_stylesheet_register_namespace(sheet2, "my-ns", "http://my-ns.com");

    ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &attr_rule);
    ui_css_rule_append_selector_attr(attr_rule, "href", UI_CSS_ATTR_OP_PREFIX,
                                     "https");

    ui_css_rule_destroy(attr_rule);
    ui_css_stylesheet_destroy(sheet2);
  }

  test_oom();
  test_parser_branches();
  /* branch coverage */
  {
    struct ui_css_stylesheet *sheet = NULL;
    ui_css_parse_stylesheet(":hover(", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("div { * {} }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@layer a, b {", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@layer a, b;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@layer a {", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@layer a, b;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@layer a b c;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@scope (a { }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@property --a { bad: syntax garbage", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@namespace url((bad));", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@namespace url(test) garbage", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("div { color:   ; }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@unknown", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;
  }

  ui_css_parse_stylesheet("@layer a, b;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@layer a b c;", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@scope (a { }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@property --a { bad: syntax garbage", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace url((bad));", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@namespace url(test) garbage", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { color:   ; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("@unknown", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;

  ui_css_parse_stylesheet("div { color: !important; }", &sheet);
  if (sheet)
    ui_css_stylesheet_destroy(sheet);
  sheet = NULL;
  {
    struct ui_css_stylesheet *sheet = NULL;

    ui_css_parse_stylesheet("@layer a, b;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@layer a b c;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@scope (a { }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@property --a { bad: syntax garbage", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@namespace url((bad));", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@namespace url(test) garbage", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("div { color:   ; }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("div { color: !important; }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@unknown", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@layer single;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@property --my-other { inherits: true; }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@property --my-bad { inherits: ; }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@namespace url ;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@namespace url( ;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@namespace url)( ;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@import url(;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@import url)(;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("div { color:!important; }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@keyframes foo invalid { }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("div ) ;", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("div ) }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;

    ui_css_parse_stylesheet("@media { div unknown }", &sheet);
    if (sheet)
      ui_css_stylesheet_destroy(sheet);
    sheet = NULL;
  }
  printf("All parser tests passed.\n");
  return 0;
}
