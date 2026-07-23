# Virtualized Pagination

When dealing with massive datasets (e.g., millions of database records or network search results), loading all the data at once will crash the application due to out-of-memory errors. The framework solves this by combining Data Sources (`ui_data_source.h`) with Virtualized Pagination controls (`ui_pagination_base.h`).

## The Pagination Controller

Instead of a simple "Next" / "Previous" button, the pagination controller manages the math for total pages, current offset, and page size, rendering the appropriate UI controls (e.g., `[<] [1] [2] ... [8] [9] [>]`).

```c
#include "ui_pagination_base.h"
#include "ui_data_source.h"

// Initialize the paginator
ui_component_t* paginator = ui_pagination_create(arena);

// Configure the view: 50 items per page
ui_pagination_set_page_size(paginator, 50);

// Suppose we query the backend and learn there are 10,000 total records
ui_pagination_set_total_items(paginator, 10000);
```

## Hooking up the Data Source

When the user clicks a page number, the paginator fires an event. You listen to this event, fetch the new chunk of data from your backend, and replace the contents of your grid's `ui_data_source_t`.

```c
void on_page_changed(ui_component_t* paginator, int new_page_index) {
    int page_size = ui_pagination_get_page_size(paginator);
    int offset = new_page_index * page_size;

    // Fetch the specific chunk from the database
    my_record_t* records = db_query_limit_offset("users", page_size, offset);

    // Update the UI data source
    ui_data_source_clear(global_user_source);
    for (int i = 0; i < page_size; i++) {
        ui_data_source_append(global_user_source, records[i].id, records[i].name);
    }
}

ui_pagination_on_change(paginator, on_page_changed);
```

## Infinite Scrolling vs. Traditional Pagination

The `ui_pagination_base.h` component is specifically for *Traditional Pagination* (explicit page numbers).

If you prefer an *Infinite Scrolling* UX (where new data loads seamlessly as the user hits the bottom of the list), you do not use the paginator component. Instead, you attach an Intersection Observer (`ui_intersection_observer.h`) to a hidden "sentinel" element at the bottom of your virtualized list (`ui_list_base.h`), triggering the data source append when the sentinel becomes visible.
