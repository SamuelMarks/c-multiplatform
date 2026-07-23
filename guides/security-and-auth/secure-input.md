# Secure Input & Memory Safety

Handling sensitive information like passwords, PIN codes, and API keys requires strict UI patterns and memory management to prevent data leakage. The framework provides specialized components like `ui_pin_input_base.h` and secure input modifiers to assist with this.

## The Secure Input Field

A standard text field keeps the user's input in plain text memory for rendering and accessibility. A secure input field masks the visual output and offers features to zero-out memory.

```c
#include "ui_input_base.h"

ui_component_t* password_field = ui_input_create(arena);

// Set the input type to password.
// This automatically masks characters (e.g., '•') and disables clipboard copying.
ui_input_set_type(password_field, UI_INPUT_TYPE_PASSWORD);
```

## PIN Pads & Multi-Digit Inputs

For authentication flows requiring multi-digit codes (like 2FA SMS codes or lock screens), the framework provides a dedicated PIN input component. It automatically handles focus advancement between individual digit boxes and prevents invalid character entry.

```c
#include "ui_pin_input_base.h"

// Create a 6-digit PIN input
ui_component_t* pin_pad = ui_pin_input_create(arena, 6);

// Allow only alphanumeric characters (e.g., for some 2FA codes)
ui_pin_input_set_mask(pin_pad, UI_INPUT_MASK_ALPHANUMERIC);
```

## Memory Zeroing

In C, simply freeing a string holding a password isn't always enough, as the plaintext might linger in freed memory and be exposed via a heap dump or use-after-free vulnerability.

When retrieving sensitive data from a UI component, you should use the framework's secure string extraction, which guarantees the component's internal buffer is zeroed out when the component is destroyed.

```c
// Retrieve the password for authentication
const char* temp_password = ui_input_get_text(password_field);

authenticate_user(username, temp_password);

// IMPORTANT: If you copied this string to a local buffer, you MUST zero it.
// The framework provides a secure memory wipe utility.
ui_secure_wipe((void*)temp_password, strlen(temp_password));
```

## Disabling OS Features

Secure inputs automatically hint to the operating system to disable predictive text, spell checking, and clipboard history logging to prevent the OS from inadvertently storing the user's password.
