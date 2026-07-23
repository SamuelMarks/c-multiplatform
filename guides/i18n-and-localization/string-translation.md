# String Translation (i18n)

Hardcoding English strings into your UI components makes localization nearly impossible. The `ui_i18n.h` module provides a robust translation dictionary system with support for plurals and variable interpolation.

## Setting up Dictionaries

Translations are typically loaded from JSON or custom binary dictionary files into the `ui_i18n_manager_t`.

```c
#include "ui_i18n.h"

// Initialize the manager and set the active locale
ui_i18n_manager_t* i18n = ui_i18n_manager_create(arena);
ui_i18n_set_locale(i18n, "fr-FR");

// In a real app, you would load this from disk.
// For example: ui_i18n_load_json(i18n, "locales/fr.json");
ui_i18n_add_translation(i18n, "welcome_message", "Bienvenue, {name}!");
ui_i18n_add_translation(i18n, "item_count_zero", "Aucun article");
ui_i18n_add_translation(i18n, "item_count_one", "1 article");
ui_i18n_add_translation(i18n, "item_count_other", "{count} articles");
```

## Resolving Strings in the UI

Instead of passing static strings to components, you use the translation resolution macros/functions. The engine will allocate the translated string in the current frame arena.

```c
// Simple key lookup
ui_component_t* btn = ui_button_create(arena, ui_t("submit_button_key"));

// Interpolation
ui_i18n_args_t args = ui_i18n_args_create();
ui_i18n_args_set_string(&args, "name", "Alice");

const char* greeting = ui_t_args("welcome_message", &args);
ui_component_t* label = ui_label_create(arena, greeting);
```

## Handling Plurals

Different languages have wildly different rules for plurals (e.g., Arabic has six plural forms). The framework utilizes the CLDR (Common Locale Data Repository) rules engine under the hood.

```c
// Pluralization requires passing the numeric count
int cart_size = 3;
const char* items_text = ui_t_plural("item_count", cart_size);

// If cart_size == 0, outputs: "Aucun article"
// If cart_size == 1, outputs: "1 article"
// If cart_size == 3, outputs: "3 articles"
```

## Hot-Swapping Locales

When the user changes their language preference in the app settings, you can trigger an engine-wide invalidation.

```c
void on_language_changed(const char* new_locale) {
    ui_i18n_set_locale(engine->i18n, new_locale);

    // Broadcast an invalidation signal.
    // All active components will re-run their `on_update` hooks, allowing them to re-fetch ui_t() strings.
    ui_engine_broadcast_event(engine, UI_EVENT_LOCALE_CHANGED);
}
```
