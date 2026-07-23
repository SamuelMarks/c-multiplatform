# Custom Form Validators

While the framework provides a suite of built-in validators (e.g., `ui_validator_required`, `ui_validator_email`), complex business logic often requires custom validation rules. The `ui_form_validators.h` API makes it easy to compose and attach custom validation logic to your form controls.

## Anatomy of a Validator

A custom validator is simply a function that takes a `ui_form_control_t*` and returns a `ui_validation_result_t`. If the control is valid, you return a success result. If it's invalid, you return an error object containing a key and an optional error message.

```c
#include "ui_form_validators.h"
#include <string.h>
#include <ctype.h>

// A custom validator that ensures a password has at least one uppercase letter
struct ui_validation_result validate_has_uppercase(struct ui_form_control *control) {
    const char* value = ui_form_control_get_string(control);

    // Empty fields are usually handled by the 'required' validator,
    // so we pass if empty to allow for optional fields.
    if (!value || strlen(value) == 0) {
        return ui_validation_valid();
    }

    for (int i = 0; value[i] != '\0'; i++) {
        if (isupper(value[i])) {
            return ui_validation_valid();
        }
    }

    // Return an error dictionary
    return ui_validation_error("missing_uppercase", "Password must contain an uppercase letter.");
}
```

## Attaching Validators

You attach your custom validator during form construction using the builder API, often composing it with built-in validators.

```c
ui_form_group_t* form = ui_form_builder_group(&fb, 1,
    ui_form_builder_control(&fb, "new_password", "",
        ui_validator_compose(3,
            ui_validator_required(),
            ui_validator_min_length(8),
            validate_has_uppercase // Our custom validator
        )
    )
);
```

## Cross-Field Validation

Sometimes a field's validity depends on another field (e.g., "Confirm Password" must match "Password"). For this, you attach the validator to the *Group* instead of the individual control.

```c
struct ui_validation_result validate_passwords_match(struct ui_form_control *group_control) {
    struct ui_form_group *group = (struct ui_form_group*)group_control;

    const char* pass1 = ui_form_get_string(group, "password");
    const char* pass2 = ui_form_get_string(group, "confirm_password");

    if (strcmp(pass1, pass2) != 0) {
        return ui_validation_error("password_mismatch", "Passwords do not match.");
    }

    return ui_validation_valid();
}

// Attach to the group
ui_form_group_set_validator(my_form_group, validate_passwords_match);
```

## Async Validation

For validations that require a network request (e.g., checking if a username is already taken), the framework supports returning a pending state (`ui_validation_pending()`) and fulfilling it later via a background task callback.
