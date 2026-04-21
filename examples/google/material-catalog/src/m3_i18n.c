/* clang-format off */
#include "m3_i18n.h"
#include <stddef.h>
#include <string.h>

#if defined(_MSC_VER)
#define STRNCPY_S(dest, destsz, src, count) strncpy_s(dest, destsz, src, count)
#else
#define STRNCPY_S(dest, destsz, src, count) strncpy(dest, src, count)
#endif

typedef struct {
  const char *key;
  const char *en;
  const char *ar;
  const char *he;
} i18n_entry_t;

static const i18n_entry_t dictionary[] = {
  {"title_theme_studio", "Theme Studio & Settings", "استوديو السمات والإعدادات", "סטודיו לערכות נושא והגדרות"},
  {"label_language", "Language", "اللغة", "שפה"},
  {"val_english_us", "English (US)", "الإنجليزية (الولايات المتحدة)", "אנגלית (ארה\"ב)"},
  {"label_high_contrast", "High Contrast Mode", "وضع التباين العالي", "מצב ניגודיות גבוהה"},
  {"title_colors_hct", "Colors & HCT", "الألوان ونظام HCT", "צבעים ו-HCT"},
  {"label_seed_color", "Seed Color (Hex)", "لون البذرة (Hex)", "צבע בסיס (הקס)"},
  {"title_live_preview", "Live Preview", "معاينة حية", "תצוגה מקדימה חיה"},
  {"label_typography", "Typography", "الطباعة", "טיפוגרפיה"},
  {"label_shapes", "Shapes", "الأشكال", "צורות"},
  {"btn_system", "System", "النظام", "מערכת"},
  {"btn_light", "Light", "فاتح", "בהיר"},
  {"btn_dark", "Dark", "داكن", "כהה"},
  {"btn_save", "Save", "حفظ", "שמור"},
  {"btn_cancel", "Cancel", "إلغاء", "ביטול"},
  {"app_title", "Material 3 Catalog", "كتالوج ماتيريال 3", "קטלוג Material 3"},
  {NULL, NULL, NULL, NULL}
};

int m3_i18n_init(material_catalog_state_t *state) {
  if (!state)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  state->is_rtl = 0; /* Default Left-to-Right */
  STRNCPY_S(state->current_locale, sizeof(state->current_locale), "en-US", 5);
  state->current_locale[5] = '\0';
  return MATERIAL_CATALOG_SUCCESS;
}

int m3_i18n_set_locale(material_catalog_state_t *state, const char *locale) {
  int is_rtl;
  if (!state || !locale)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  STRNCPY_S(state->current_locale, sizeof(state->current_locale), locale, sizeof(state->current_locale) - 1);
  state->current_locale[sizeof(state->current_locale) - 1] = '\0';

  is_rtl = (strncmp(locale, "ar", 2) == 0 || strncmp(locale, "he", 2) == 0) ? 1 : 0;
  return m3_i18n_set_rtl(state, is_rtl);
}

const char *m3_i18n_get_string(material_catalog_state_t *state, const char *key) {
  int i = 0;
  int is_ar = 0;
  int is_he = 0;

  if (!state || !key) return key;

  is_ar = (strncmp(state->current_locale, "ar", 2) == 0);
  is_he = (strncmp(state->current_locale, "he", 2) == 0);

  while (dictionary[i].key != NULL) {
    if (strcmp(dictionary[i].key, key) == 0) {
      if (is_ar && dictionary[i].ar) return dictionary[i].ar;
      if (is_he && dictionary[i].he) return dictionary[i].he;
      return dictionary[i].en;
    }
    i++;
  }
  return key; /* Fallback to key if not found */
}

int m3_i18n_set_rtl(material_catalog_state_t *state, int is_rtl) {
  if (!state)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (state->is_rtl != is_rtl) {
    state->is_rtl = is_rtl;
    material_catalog_invalidate_ui(state);
  }
  return MATERIAL_CATALOG_SUCCESS;
}

static void mirror_node_layout(cmp_ui_node_t *node) {
  if (!node || !node->layout)
    return;

  /* Since Flexbox in cmp.h uses flex-direction column vs row natively,
     but doesn't have ROW_REVERSE explicitly, we can either manually
     reverse the children array during drawing, or map padding left/right. */
  
  /* Swap Left and Right Padding (1 = Right, 3 = Left) */
  {
    float temp = node->layout->padding[1];
    node->layout->padding[1] = node->layout->padding[3];
    node->layout->padding[3] = temp;
  }
  
  /* Swap Left and Right Margin */
  {
    float temp = node->layout->margin[1];
    node->layout->margin[1] = node->layout->margin[3];
    node->layout->margin[3] = temp;
  }

  /* Swap Left and Right absolute positioning offsets */
  if (node->layout->position_type == CMP_POSITION_ABSOLUTE) {
    /* Instead of swapping, we should re-calculate relative to parent,
       but for basic mirroring swapping right/left is a fallback */
    float temp = node->layout->position[1];
    node->layout->position[1] = node->layout->position[3];
    node->layout->position[3] = temp;
  }

  /* Note: Reverse children manually here if direction is ROW */
  if (node->layout->direction == CMP_FLEX_ROW) {
    size_t i, j;
    for (i = 0, j = node->child_count > 0 ? node->child_count - 1 : 0; i < j; ++i, --j) {
      cmp_ui_node_t *temp_child = node->children[i];
      node->children[i] = node->children[j];
      node->children[j] = temp_child;
    }
  }

  /* Check if it's an image node with a directional icon */
  if (node->type == 7 && node->properties) {
    const char *img_path = (const char *)node->properties;
    if (m3_i18n_is_directional_icon(img_path)) {
      node->is_rtl_mirrored = 1;
    }
  }

  /* Recurse */
  {
    size_t i;
    for (i = 0; i < node->child_count; ++i) {
      mirror_node_layout(node->children[i]);
    }
  }
}

int m3_i18n_apply_rtl_mirroring(material_catalog_state_t *state, cmp_ui_node_t *root) {
  if (!state || !root)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  if (state->is_rtl) {
    mirror_node_layout(root);
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int m3_i18n_is_directional_icon(const char *svg_path) {
  if (!svg_path) return 0;
  
  if (strstr(svg_path, "arrow_back") != NULL ||
      strstr(svg_path, "arrow_forward") != NULL ||
      strstr(svg_path, "chevron_left") != NULL ||
      strstr(svg_path, "chevron_right") != NULL ||
      strstr(svg_path, "backspace") != NULL ||
      strstr(svg_path, "forward") != NULL ||
      strstr(svg_path, "reply") != NULL) {
    return 1;
  }
  
  return 0;
}
/* clang-format on */
