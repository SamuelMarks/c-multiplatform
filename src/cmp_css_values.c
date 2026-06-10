/* clang-format off */
#include "cmp_css_values.h"

#include <string.h>
#include <stdlib.h>
/* clang-format on */

static float parse_float(const char *str, const char **endptr) {
  /* Simple float parser */
  return (float)strtod(str, (char **)endptr);
}

int cmp_val_num_parse(const char *str, cmp_val_num_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1; /* No digits */
  /* Check if integer (no decimal point or exponent) */
  out_val->is_integer = 1;
  while (str < end) {
    if (*str == '.' || *str == 'e' || *str == 'E') {
      out_val->is_integer = 0;
      break;
    }
    str++;
  }
  return 0;
}

int cmp_val_percent_parse(const char *str, cmp_val_percent_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str || *end != '%')
    return -1;
  return 0;
}

int cmp_val_ratio_parse(const char *str, cmp_val_ratio_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->numerator = parse_float(str, &end);
  if (end == str)
    return -1;
  while (*end == ' ' || *end == '\t')
    end++;
  if (*end != '/')
    return -1;
  end++;
  while (*end == ' ' || *end == '\t')
    end++;
  str = end;
  out_val->denominator = parse_float(str, &end);
  if (end == str || out_val->denominator == 0.0f)
    return -1;
  return 0;
}

int cmp_val_flex_parse(const char *str, cmp_val_flex_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str || end[0] != 'f' || end[1] != 'r')
    return -1;
  return 0;
}

int cmp_val_string_parse(const char *str, cmp_val_string_t *out_val) {
  size_t len;
  if (!str || !out_val)
    return -1;
  len = strlen(str);
  if (len < 2)
    return -1;
  if ((str[0] == '"' && str[len - 1] == '"') ||
      (str[0] == '\'' && str[len - 1] == '\'')) {
    out_val->value = str + 1;
    out_val->length = len - 2;
    return 0;
  }
  return -1;
}

int cmp_val_custom_ident_parse(const char *str,
                               cmp_val_custom_ident_t *out_val) {
  size_t len;
  if (!str || !out_val)
    return -1;
  len = strlen(str);
  if (len == 0)
    return -1;
  /* Basic validation: start with --, -, _ or alpha */
  if (!(str[0] == '-' || str[0] == '_' || (str[0] >= 'a' && str[0] <= 'z') ||
        (str[0] >= 'A' && str[0] <= 'Z'))) {
    return -1;
  }
  /* For simplicity in this parser, we just store it. A full lexer would be
   * stricter. */
  out_val->value = str;
  out_val->length = len;
  return 0;
}

int cmp_val_url_parse(const char *str, cmp_val_url_t *out_val) {
  size_t len;
  const char *start;
  const char *end;
  if (!str || !out_val)
    return -1;
  if (strncmp(str, "url(", 4) != 0)
    return -1;
  len = strlen(str);
  if (str[len - 1] != ')')
    return -1;

  start = str + 4;
  end = str + len - 1;

  while (start < end && (*start == ' ' || *start == '\t'))
    start++;
  while (end > start && (*(end - 1) == ' ' || *(end - 1) == '\t'))
    end--;

  if (start < end && ((*start == '"' && *(end - 1) == '"') ||
                      (*start == '\'' && *(end - 1) == '\''))) {
    start++;
    end--;
  }

  out_val->url = start;
  out_val->length = end - start;
  return 0;
}

int cmp_length_phys_parse(const char *str, cmp_length_phys_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1;

  if (strncmp(end, "px", 2) == 0)
    out_val->unit = CMP_LENGTH_PX;
  else if (strncmp(end, "in", 2) == 0)
    out_val->unit = CMP_LENGTH_IN;
  else if (strncmp(end, "cm", 2) == 0)
    out_val->unit = CMP_LENGTH_CM;
  else if (strncmp(end, "mm", 2) == 0)
    out_val->unit = CMP_LENGTH_MM;
  else if (strncmp(end, "pt", 2) == 0)
    out_val->unit = CMP_LENGTH_PT;
  else if (strncmp(end, "pc", 2) == 0)
    out_val->unit = CMP_LENGTH_PC;
  else
    return -1;

  return 0;
}

int cmp_length_rel_parse(const char *str, cmp_length_rel_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1;

  if (strncmp(end, "em", 2) == 0)
    out_val->unit = CMP_LENGTH_EM;
  else if (strncmp(end, "rem", 3) == 0)
    out_val->unit = CMP_LENGTH_REM;
  else if (strncmp(end, "ex", 2) == 0)
    out_val->unit = CMP_LENGTH_EX;
  else if (strncmp(end, "ch", 2) == 0)
    out_val->unit = CMP_LENGTH_CH;
  else
    return -1;

  return 0;
}

int cmp_length_rel_l4_parse(const char *str, cmp_length_rel_l4_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1;

  if (strncmp(end, "cap", 3) == 0)
    out_val->unit = CMP_LENGTH_CAP;
  else if (strncmp(end, "rcap", 4) == 0)
    out_val->unit = CMP_LENGTH_RCAP;
  else if (strncmp(end, "ic", 2) == 0)
    out_val->unit = CMP_LENGTH_IC;
  else if (strncmp(end, "ric", 3) == 0)
    out_val->unit = CMP_LENGTH_RIC;
  else if (strncmp(end, "lh", 2) == 0)
    out_val->unit = CMP_LENGTH_LH;
  else if (strncmp(end, "rlh", 3) == 0)
    out_val->unit = CMP_LENGTH_RLH;
  else if (strncmp(end, "rex", 3) == 0)
    out_val->unit = CMP_LENGTH_REX;
  else if (strncmp(end, "rch", 3) == 0)
    out_val->unit = CMP_LENGTH_RCH;
  else
    return -1;

  return 0;
}

int cmp_length_vw_parse(const char *str, cmp_length_vw_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1;

  if (strncmp(end, "vmin", 4) == 0)
    out_val->unit = CMP_LENGTH_VMIN;
  else if (strncmp(end, "vmax", 4) == 0)
    out_val->unit = CMP_LENGTH_VMAX;
  else if (strncmp(end, "vw", 2) == 0)
    out_val->unit = CMP_LENGTH_VW;
  else if (strncmp(end, "vh", 2) == 0)
    out_val->unit = CMP_LENGTH_VH;
  else
    return -1;

  return 0;
}

int cmp_length_viewport_l4_parse(const char *str,
                                 cmp_length_viewport_l4_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1;

  if (strncmp(end, "dvh", 3) == 0)
    out_val->unit = CMP_LENGTH_DVH;
  else if (strncmp(end, "dvw", 3) == 0)
    out_val->unit = CMP_LENGTH_DVW;
  else if (strncmp(end, "lvh", 3) == 0)
    out_val->unit = CMP_LENGTH_LVH;
  else if (strncmp(end, "lvw", 3) == 0)
    out_val->unit = CMP_LENGTH_LVW;
  else if (strncmp(end, "svh", 3) == 0)
    out_val->unit = CMP_LENGTH_SVH;
  else if (strncmp(end, "svw", 3) == 0)
    out_val->unit = CMP_LENGTH_SVW;
  else if (strncmp(end, "dvi", 3) == 0)
    out_val->unit = CMP_LENGTH_DVI;
  else if (strncmp(end, "dvb", 3) == 0)
    out_val->unit = CMP_LENGTH_DVB;
  else if (strncmp(end, "lvi", 3) == 0)
    out_val->unit = CMP_LENGTH_LVI;
  else if (strncmp(end, "lvb", 3) == 0)
    out_val->unit = CMP_LENGTH_LVB;
  else if (strncmp(end, "svi", 3) == 0)
    out_val->unit = CMP_LENGTH_SVI;
  else if (strncmp(end, "svb", 3) == 0)
    out_val->unit = CMP_LENGTH_SVB;
  else if (strncmp(end, "vi", 2) == 0)
    out_val->unit = CMP_LENGTH_VI;
  else if (strncmp(end, "vb", 2) == 0)
    out_val->unit = CMP_LENGTH_VB;
  else
    return -1;

  return 0;
}

int cmp_length_cq_parse(const char *str, cmp_length_cq_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1;

  if (strncmp(end, "cqmin", 5) == 0)
    out_val->unit = CMP_LENGTH_CQMIN;
  else if (strncmp(end, "cqmax", 5) == 0)
    out_val->unit = CMP_LENGTH_CQMAX;
  else if (strncmp(end, "cqw", 3) == 0)
    out_val->unit = CMP_LENGTH_CQW;
  else if (strncmp(end, "cqh", 3) == 0)
    out_val->unit = CMP_LENGTH_CQH;
  else if (strncmp(end, "cqi", 3) == 0)
    out_val->unit = CMP_LENGTH_CQI;
  else if (strncmp(end, "cqb", 3) == 0)
    out_val->unit = CMP_LENGTH_CQB;
  else
    return -1;

  return 0;
}

int cmp_angle_parse(const char *str, cmp_angle_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1;

  if (strncmp(end, "deg", 3) == 0)
    out_val->unit = CMP_ANGLE_DEG;
  else if (strncmp(end, "grad", 4) == 0)
    out_val->unit = CMP_ANGLE_GRAD;
  else if (strncmp(end, "rad", 3) == 0)
    out_val->unit = CMP_ANGLE_RAD;
  else if (strncmp(end, "turn", 4) == 0)
    out_val->unit = CMP_ANGLE_TURN;
  else
    return -1;

  return 0;
}

int cmp_time_parse(const char *str, cmp_time_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1;

  if (strncmp(end, "ms", 2) == 0)
    out_val->unit = CMP_TIME_MS;
  else if (strncmp(end, "s", 1) == 0)
    out_val->unit = CMP_TIME_S;
  else
    return -1;

  return 0;
}

int cmp_resolution_parse(const char *str, cmp_resolution_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1;

  if (strncmp(end, "dpi", 3) == 0)
    out_val->unit = CMP_RESOLUTION_DPI;
  else if (strncmp(end, "dpcm", 4) == 0)
    out_val->unit = CMP_RESOLUTION_DPCM;
  else if (strncmp(end, "dppx", 4) == 0)
    out_val->unit = CMP_RESOLUTION_DPPX;
  else if (strncmp(end, "x", 1) == 0)
    out_val->unit = CMP_RESOLUTION_X;
  else
    return -1;

  return 0;
}

int cmp_frequency_parse(const char *str, cmp_frequency_t *out_val) {
  const char *end;
  if (!str || !out_val)
    return -1;
  out_val->value = parse_float(str, &end);
  if (end == str)
    return -1;

  if (strncmp(end, "kHz", 3) == 0)
    out_val->unit = CMP_FREQUENCY_KHZ;
  else if (strncmp(end, "Hz", 2) == 0)
    out_val->unit = CMP_FREQUENCY_HZ;
  else
    return -1;

  return 0;
}