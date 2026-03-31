# Usage

**Purpose:** This guide demonstrates how to build a basic LibCMPC application, initialize the modality engine, and render a UI tree.
**Current State:** The framework allows for the composition of unstyled, primitive UI nodes (`cmp_ui_box`, `cmp_ui_button`, etc.) that are routed through a modality-agnostic event loop and flexbox layout engine.
**Upcoming Features:** While current usage examples focus on primitive widgets, upcoming releases will introduce native theming capabilities, allowing developers to seamlessly apply **Material 3, Cupertino / HIG, or Fluent 2** styling profiles to their applications without changing business logic.

This guide demonstrates how to build a basic LibCMPC application, initialize the modality engine, and render a UI tree.

## Basic Application Loop

```c
#include "cmp.h"
#include <stdio.h>

int main(int argc, char **argv) {
    cmp_window_t *window = NULL;
    cmp_window_config_t config = {0};
    cmp_modality_t modality;
    cmp_ui_node_t *root_box = NULL;
    cmp_ui_node_t *button = NULL;

    /* 1. Initialize the Modality Engine (Single-threaded) */
    if (cmp_modality_single_init(&modality) != CMP_SUCCESS) {
        return 1;
    }

    /* 2. Initialize the Window System */
    cmp_window_system_init();

    config.title = "LibCMPC Hello World";
    config.width = 800;
    config.height = 600;

    if (cmp_window_create(&config, &window) != CMP_SUCCESS) {
        return 1;
    }

    /* 3. Build the UI Tree */
    cmp_ui_box_create(&root_box);
    cmp_ui_button_create(&button, "Click Me!");
    cmp_ui_node_add_child(root_box, button);

    /* Bind the UI tree to the window */
    cmp_window_set_ui_tree(window, root_box);
    cmp_window_show(window);

    /* 4. Main Event Loop */
    while (!cmp_window_should_close(window)) {
        cmp_window_poll_events(window);
        cmp_modality_run(&modality); /* Process async tasks, VFS, HTTP */
    }

    /* 5. Cleanup */
    cmp_window_destroy(window);
    cmp_window_system_shutdown();
    cmp_modality_destroy(&modality);
    cmp_ui_node_destroy(root_box);

    /* Optional: Print memory leak report */
    cmp_mem_check_leaks();

    return 0;
}
```

## Adding Networking (c-abstract-http)

LibCMPC embeds `c-abstract-http`. You can utilize it within your modality loop:

```c
struct HttpClient *client = NULL;
cmp_http_client_create(&modality, &client);

/* Issue an asynchronous GET request that resolves via the modality loop */
cmp_http_client_get(client, "https://api.github.com/zen", my_callback_fn, user_data);
```

## Integrating the Database (c-orm)

Data binding allows UI nodes to automatically react to database changes:

```c
cmp_orm_observable_t *username_obs = NULL;
/* Initialize observable from your ORM model... */

cmp_ui_node_t *text_label;
cmp_ui_text_create(&text_label, "Default User");

/* The label will now automatically update when the observable changes */
cmp_ui_node_bind(text_label, username_obs, CMP_BIND_TEXT_CONTENT);
```

For more comprehensive examples, see the `examples/` directory in the repository.
