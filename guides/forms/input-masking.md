# Input Masking

Input masks guide users in entering data correctly by forcing input to conform to a specific structure (like phone numbers, dates, or credit card numbers) in real-time. The framework handles this via `ui_input_mask.h`.

## How Masking Works

When an input has a mask, the user only types the raw characters (e.g., numbers). The framework automatically inserts the decorative formatting characters (spaces, dashes, slashes) as the user types, and manages cursor placement so the user can backspace seamlessly.

## Setting a Mask

The framework provides several common mask presets.

```c
#include "ui_input_base.h"
#include "ui_input_mask.h"

ui_component_t* phone_input = ui_input_create(arena);

// Apply a US Phone Number mask: (XXX) XXX-XXXX
ui_input_set_mask(phone_input, ui_mask_preset_us_phone());

ui_component_t* date_input = ui_input_create(arena);

// Apply a Date mask: MM/DD/YYYY
ui_input_set_mask(date_input, ui_mask_preset_date_us());
```

## Creating Custom Masks

If a preset isn't sufficient, you can define a custom structural mask string.

The mask string uses specific token characters:
*   `0`: Requires a digit (0-9).
*   `A`: Requires an alphabetical character (a-z, A-Z).
*   `*`: Requires any alphanumeric character.
*   Any other character (like `-`, ` `, `(`, `)`) is considered a static decorative token and will be inserted automatically.

```c
ui_component_t* license_key_input = ui_input_create(arena);

// Create a custom mask for a software license key: 4 letters/numbers per block
ui_input_mask_t* custom_mask = ui_mask_create_from_string("****-****-****-****");

ui_input_set_mask(license_key_input, custom_mask);
```

## Extracting the Value

When the user submits the form, you often need the raw data without the decorative formatting (e.g., saving `1234567890` to the database instead of `(123) 456-7890`).

```c
// Returns the fully formatted string shown on screen
const char* display_value = ui_input_get_text(phone_input);

// Returns only the user-entered tokens based on the mask
const char* raw_value = ui_input_get_unmasked_text(phone_input);
```

## Dynamic Masks (Credit Cards)

Some inputs change their mask dynamically based on the first few digits entered (e.g., an Amex credit card has 15 digits grouped differently than a Visa 16-digit card). You can provide a mask resolution callback:

```c
ui_input_mask_t* resolve_cc_mask(const char* current_input) {
    if (strncmp(current_input, "34", 2) == 0 || strncmp(current_input, "37", 2) == 0) {
        return ui_mask_preset_credit_card_amex(); // XXXX XXXXXX XXXXX
    }
    return ui_mask_preset_credit_card_default();  // XXXX XXXX XXXX XXXX
}

ui_input_set_dynamic_mask(cc_input, resolve_cc_mask);
```
