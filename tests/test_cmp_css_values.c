/* clang-format off */
#include "cmp_css_values.h"
#include "greatest.h"
/* clang-format on */

TEST test_val_num_parse(void) {
  cmp_val_num_t num;

  ASSERT_EQ(0, cmp_val_num_parse("123", &num));
  ASSERT_EQ(123.0f, num.value);
  ASSERT_EQ(1, num.is_integer);

  ASSERT_EQ(0, cmp_val_num_parse("12.3", &num));
  ASSERT_EQ(12.3f, num.value);
  ASSERT_EQ(0, num.is_integer);

  ASSERT_EQ(-1, cmp_val_num_parse("abc", &num));

  PASS();
}

TEST test_val_percent_parse(void) {
  cmp_val_percent_t pct;

  ASSERT_EQ(0, cmp_val_percent_parse("50%", &pct));
  ASSERT_EQ(50.0f, pct.value);

  ASSERT_EQ(-1, cmp_val_percent_parse("50", &pct));

  PASS();
}

TEST test_val_ratio_parse(void) {
  cmp_val_ratio_t ratio;

  ASSERT_EQ(0, cmp_val_ratio_parse("16/9", &ratio));
  ASSERT_EQ(16.0f, ratio.numerator);
  ASSERT_EQ(9.0f, ratio.denominator);

  ASSERT_EQ(0, cmp_val_ratio_parse("1 / 2", &ratio));
  ASSERT_EQ(1.0f, ratio.numerator);
  ASSERT_EQ(2.0f, ratio.denominator);

  ASSERT_EQ(-1, cmp_val_ratio_parse("1", &ratio));
  ASSERT_EQ(-1, cmp_val_ratio_parse("1/0", &ratio));

  PASS();
}

TEST test_val_flex_parse(void) {
  cmp_val_flex_t flex;

  ASSERT_EQ(0, cmp_val_flex_parse("1fr", &flex));
  ASSERT_EQ(1.0f, flex.value);

  ASSERT_EQ(0, cmp_val_flex_parse("2.5fr", &flex));
  ASSERT_EQ(2.5f, flex.value);

  ASSERT_EQ(-1, cmp_val_flex_parse("1px", &flex));

  PASS();
}

TEST test_val_string_parse(void) {
  cmp_val_string_t s;

  ASSERT_EQ(0, cmp_val_string_parse("\"hello\"", &s));
  ASSERT_EQ(5, s.length);
  ASSERT_EQ('h', s.value[0]);

  ASSERT_EQ(0, cmp_val_string_parse("'world'", &s));
  ASSERT_EQ(5, s.length);
  ASSERT_EQ('w', s.value[0]);

  ASSERT_EQ(-1, cmp_val_string_parse("hello", &s));

  PASS();
}

TEST test_val_custom_ident_parse(void) {
  cmp_val_custom_ident_t ident;

  ASSERT_EQ(0, cmp_val_custom_ident_parse("my-ident", &ident));
  ASSERT_EQ(8, ident.length);
  ASSERT_EQ('m', ident.value[0]);

  ASSERT_EQ(0, cmp_val_custom_ident_parse("--var-name", &ident));
  ASSERT_EQ(10, ident.length);
  ASSERT_EQ('-', ident.value[0]);

  ASSERT_EQ(-1, cmp_val_custom_ident_parse("123bad", &ident));
  ASSERT_EQ(-1, cmp_val_custom_ident_parse("", &ident));

  PASS();
}

TEST test_val_url_parse(void) {
  cmp_val_url_t u;

  ASSERT_EQ(0, cmp_val_url_parse("url(http://example.com)", &u));
  ASSERT_EQ(18, u.length);
  ASSERT_EQ('h', u.url[0]);

  ASSERT_EQ(0, cmp_val_url_parse("url(\"http://example.com\")", &u));
  ASSERT_EQ(18, u.length);
  ASSERT_EQ('h', u.url[0]);

  ASSERT_EQ(0, cmp_val_url_parse("url('http://example.com')", &u));
  ASSERT_EQ(18, u.length);

  ASSERT_EQ(-1, cmp_val_url_parse("http://example.com", &u));

  PASS();
}

TEST test_length_phys_parse(void) {
  cmp_length_phys_t phys;

  ASSERT_EQ(0, cmp_length_phys_parse("10px", &phys));
  ASSERT_EQ(10.0f, phys.value);
  ASSERT_EQ(CMP_LENGTH_PX, phys.unit);

  ASSERT_EQ(0, cmp_length_phys_parse("2in", &phys));
  ASSERT_EQ(2.0f, phys.value);
  ASSERT_EQ(CMP_LENGTH_IN, phys.unit);

  ASSERT_EQ(0, cmp_length_phys_parse("3.5cm", &phys));
  ASSERT_EQ(3.5f, phys.value);
  ASSERT_EQ(CMP_LENGTH_CM, phys.unit);

  ASSERT_EQ(0, cmp_length_phys_parse("4mm", &phys));
  ASSERT_EQ(4.0f, phys.value);
  ASSERT_EQ(CMP_LENGTH_MM, phys.unit);

  ASSERT_EQ(0, cmp_length_phys_parse("5pt", &phys));
  ASSERT_EQ(5.0f, phys.value);
  ASSERT_EQ(CMP_LENGTH_PT, phys.unit);

  ASSERT_EQ(0, cmp_length_phys_parse("6pc", &phys));
  ASSERT_EQ(6.0f, phys.value);
  ASSERT_EQ(CMP_LENGTH_PC, phys.unit);

  ASSERT_EQ(-1, cmp_length_phys_parse("10", &phys));
  ASSERT_EQ(-1, cmp_length_phys_parse("10em", &phys));

  PASS();
}

TEST test_length_rel_parse(void) {
  cmp_length_rel_t rel;

  ASSERT_EQ(0, cmp_length_rel_parse("2em", &rel));
  ASSERT_EQ(2.0f, rel.value);
  ASSERT_EQ(CMP_LENGTH_EM, rel.unit);

  ASSERT_EQ(0, cmp_length_rel_parse("1.5rem", &rel));
  ASSERT_EQ(1.5f, rel.value);
  ASSERT_EQ(CMP_LENGTH_REM, rel.unit);

  ASSERT_EQ(0, cmp_length_rel_parse("3ex", &rel));
  ASSERT_EQ(3.0f, rel.value);
  ASSERT_EQ(CMP_LENGTH_EX, rel.unit);

  ASSERT_EQ(0, cmp_length_rel_parse("4ch", &rel));
  ASSERT_EQ(4.0f, rel.value);
  ASSERT_EQ(CMP_LENGTH_CH, rel.unit);

  ASSERT_EQ(-1, cmp_length_rel_parse("10px", &rel));

  PASS();
}

TEST test_length_rel_l4_parse(void) {
  cmp_length_rel_l4_t rel_l4;

  ASSERT_EQ(0, cmp_length_rel_l4_parse("2cap", &rel_l4));
  ASSERT_EQ(2.0f, rel_l4.value);
  ASSERT_EQ(CMP_LENGTH_CAP, rel_l4.unit);

  ASSERT_EQ(0, cmp_length_rel_l4_parse("1.5rcap", &rel_l4));
  ASSERT_EQ(1.5f, rel_l4.value);
  ASSERT_EQ(CMP_LENGTH_RCAP, rel_l4.unit);

  ASSERT_EQ(0, cmp_length_rel_l4_parse("3ic", &rel_l4));
  ASSERT_EQ(3.0f, rel_l4.value);
  ASSERT_EQ(CMP_LENGTH_IC, rel_l4.unit);

  ASSERT_EQ(0, cmp_length_rel_l4_parse("4ric", &rel_l4));
  ASSERT_EQ(4.0f, rel_l4.value);
  ASSERT_EQ(CMP_LENGTH_RIC, rel_l4.unit);

  ASSERT_EQ(0, cmp_length_rel_l4_parse("5lh", &rel_l4));
  ASSERT_EQ(5.0f, rel_l4.value);
  ASSERT_EQ(CMP_LENGTH_LH, rel_l4.unit);

  ASSERT_EQ(0, cmp_length_rel_l4_parse("6rlh", &rel_l4));
  ASSERT_EQ(6.0f, rel_l4.value);
  ASSERT_EQ(CMP_LENGTH_RLH, rel_l4.unit);

  ASSERT_EQ(0, cmp_length_rel_l4_parse("7rex", &rel_l4));
  ASSERT_EQ(7.0f, rel_l4.value);
  ASSERT_EQ(CMP_LENGTH_REX, rel_l4.unit);

  ASSERT_EQ(0, cmp_length_rel_l4_parse("8rch", &rel_l4));
  ASSERT_EQ(8.0f, rel_l4.value);
  ASSERT_EQ(CMP_LENGTH_RCH, rel_l4.unit);

  ASSERT_EQ(-1, cmp_length_rel_l4_parse("10em", &rel_l4));

  PASS();
}

TEST test_length_vw_parse(void) {
  cmp_length_vw_t vw;

  ASSERT_EQ(0, cmp_length_vw_parse("50vw", &vw));
  ASSERT_EQ(50.0f, vw.value);
  ASSERT_EQ(CMP_LENGTH_VW, vw.unit);

  ASSERT_EQ(0, cmp_length_vw_parse("100vh", &vw));
  ASSERT_EQ(100.0f, vw.value);
  ASSERT_EQ(CMP_LENGTH_VH, vw.unit);

  ASSERT_EQ(0, cmp_length_vw_parse("25vmin", &vw));
  ASSERT_EQ(25.0f, vw.value);
  ASSERT_EQ(CMP_LENGTH_VMIN, vw.unit);

  ASSERT_EQ(0, cmp_length_vw_parse("75.5vmax", &vw));
  ASSERT_EQ(75.5f, vw.value);
  ASSERT_EQ(CMP_LENGTH_VMAX, vw.unit);

  ASSERT_EQ(-1, cmp_length_vw_parse("10px", &vw));

  PASS();
}

TEST test_length_viewport_l4_parse(void) {
  cmp_length_viewport_l4_t vw_l4;

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("10dvh", &vw_l4));
  ASSERT_EQ(10.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_DVH, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("20dvw", &vw_l4));
  ASSERT_EQ(20.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_DVW, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("30lvh", &vw_l4));
  ASSERT_EQ(30.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_LVH, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("40lvw", &vw_l4));
  ASSERT_EQ(40.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_LVW, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("50svh", &vw_l4));
  ASSERT_EQ(50.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_SVH, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("60svw", &vw_l4));
  ASSERT_EQ(60.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_SVW, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("70vi", &vw_l4));
  ASSERT_EQ(70.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_VI, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("80vb", &vw_l4));
  ASSERT_EQ(80.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_VB, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("90dvi", &vw_l4));
  ASSERT_EQ(90.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_DVI, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("100dvb", &vw_l4));
  ASSERT_EQ(100.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_DVB, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("110lvi", &vw_l4));
  ASSERT_EQ(110.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_LVI, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("120lvb", &vw_l4));
  ASSERT_EQ(120.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_LVB, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("130svi", &vw_l4));
  ASSERT_EQ(130.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_SVI, vw_l4.unit);

  ASSERT_EQ(0, cmp_length_viewport_l4_parse("140svb", &vw_l4));
  ASSERT_EQ(140.0f, vw_l4.value);
  ASSERT_EQ(CMP_LENGTH_SVB, vw_l4.unit);

  ASSERT_EQ(-1, cmp_length_viewport_l4_parse("10vw", &vw_l4));

  PASS();
}

TEST test_length_cq_parse(void) {
  cmp_length_cq_t cq;

  ASSERT_EQ(0, cmp_length_cq_parse("10cqw", &cq));
  ASSERT_EQ(10.0f, cq.value);
  ASSERT_EQ(CMP_LENGTH_CQW, cq.unit);

  ASSERT_EQ(0, cmp_length_cq_parse("20cqh", &cq));
  ASSERT_EQ(20.0f, cq.value);
  ASSERT_EQ(CMP_LENGTH_CQH, cq.unit);

  ASSERT_EQ(0, cmp_length_cq_parse("30cqi", &cq));
  ASSERT_EQ(30.0f, cq.value);
  ASSERT_EQ(CMP_LENGTH_CQI, cq.unit);

  ASSERT_EQ(0, cmp_length_cq_parse("40cqb", &cq));
  ASSERT_EQ(40.0f, cq.value);
  ASSERT_EQ(CMP_LENGTH_CQB, cq.unit);

  ASSERT_EQ(0, cmp_length_cq_parse("50cqmin", &cq));
  ASSERT_EQ(50.0f, cq.value);
  ASSERT_EQ(CMP_LENGTH_CQMIN, cq.unit);

  ASSERT_EQ(0, cmp_length_cq_parse("60cqmax", &cq));
  ASSERT_EQ(60.0f, cq.value);
  ASSERT_EQ(CMP_LENGTH_CQMAX, cq.unit);

  ASSERT_EQ(-1, cmp_length_cq_parse("10px", &cq));

  PASS();
}

TEST test_angle_parse(void) {
  cmp_angle_t a;

  ASSERT_EQ(0, cmp_angle_parse("90deg", &a));
  ASSERT_EQ(90.0f, a.value);
  ASSERT_EQ(CMP_ANGLE_DEG, a.unit);

  ASSERT_EQ(0, cmp_angle_parse("100grad", &a));
  ASSERT_EQ(100.0f, a.value);
  ASSERT_EQ(CMP_ANGLE_GRAD, a.unit);

  ASSERT_EQ(0, cmp_angle_parse("3.14rad", &a));
  ASSERT_EQ(3.14f, a.value);
  ASSERT_EQ(CMP_ANGLE_RAD, a.unit);

  ASSERT_EQ(0, cmp_angle_parse("0.5turn", &a));
  ASSERT_EQ(0.5f, a.value);
  ASSERT_EQ(CMP_ANGLE_TURN, a.unit);

  ASSERT_EQ(-1, cmp_angle_parse("10px", &a));

  PASS();
}

TEST test_time_parse(void) {
  cmp_time_t t;

  ASSERT_EQ(0, cmp_time_parse("1.5s", &t));
  ASSERT_EQ(1.5f, t.value);
  ASSERT_EQ(CMP_TIME_S, t.unit);

  ASSERT_EQ(0, cmp_time_parse("250ms", &t));
  ASSERT_EQ(250.0f, t.value);
  ASSERT_EQ(CMP_TIME_MS, t.unit);

  ASSERT_EQ(-1, cmp_time_parse("10px", &t));

  PASS();
}

TEST test_resolution_parse(void) {
  cmp_resolution_t r;

  ASSERT_EQ(0, cmp_resolution_parse("300dpi", &r));
  ASSERT_EQ(300.0f, r.value);
  ASSERT_EQ(CMP_RESOLUTION_DPI, r.unit);

  ASSERT_EQ(0, cmp_resolution_parse("118dpcm", &r));
  ASSERT_EQ(118.0f, r.value);
  ASSERT_EQ(CMP_RESOLUTION_DPCM, r.unit);

  ASSERT_EQ(0, cmp_resolution_parse("2dppx", &r));
  ASSERT_EQ(2.0f, r.value);
  ASSERT_EQ(CMP_RESOLUTION_DPPX, r.unit);

  ASSERT_EQ(0, cmp_resolution_parse("2x", &r));
  ASSERT_EQ(2.0f, r.value);
  ASSERT_EQ(CMP_RESOLUTION_X, r.unit);

  ASSERT_EQ(-1, cmp_resolution_parse("10px", &r));

  PASS();
}

TEST test_frequency_parse(void) {
  cmp_frequency_t f;

  ASSERT_EQ(0, cmp_frequency_parse("60Hz", &f));
  ASSERT_EQ(60.0f, f.value);
  ASSERT_EQ(CMP_FREQUENCY_HZ, f.unit);

  ASSERT_EQ(0, cmp_frequency_parse("1.5kHz", &f));
  ASSERT_EQ(1.5f, f.value);
  ASSERT_EQ(CMP_FREQUENCY_KHZ, f.unit);

  ASSERT_EQ(-1, cmp_frequency_parse("10px", &f));

  PASS();
}

SUITE(css_values_suite) {
  RUN_TEST(test_val_num_parse);
  RUN_TEST(test_val_percent_parse);
  RUN_TEST(test_val_ratio_parse);
  RUN_TEST(test_val_flex_parse);
  RUN_TEST(test_val_string_parse);
  RUN_TEST(test_val_custom_ident_parse);
  RUN_TEST(test_val_url_parse);
  RUN_TEST(test_length_phys_parse);
  RUN_TEST(test_length_rel_parse);
  RUN_TEST(test_length_rel_l4_parse);
  RUN_TEST(test_length_vw_parse);
  RUN_TEST(test_length_viewport_l4_parse);
  RUN_TEST(test_length_cq_parse);
  RUN_TEST(test_angle_parse);
  RUN_TEST(test_time_parse);
  RUN_TEST(test_resolution_parse);
  RUN_TEST(test_frequency_parse);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(css_values_suite);
  GREATEST_MAIN_END();
}
