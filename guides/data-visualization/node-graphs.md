# Node Graphs

For applications that require visual programming, data flow mapping, or complex state machine editors, the `ui_node_graph_base.h` provides an infinite canvas and primitives for drawing draggable nodes and connecting them with edges (noodles).

## Setting Up the Canvas

The Node Graph component handles panning (middle mouse/space+drag) and zooming (scroll wheel) out of the box.

```c
#include "ui_node_graph_base.h"

// Initialize the graph canvas
ui_component_t* graph_canvas = ui_node_graph_create(arena);

// Add to your UI hierarchy
ui_dom_append_child(app_root, graph_canvas);
```

## Adding Nodes

Nodes are floating containers within the graph canvas. You can append standard UI components (inputs, labels, images) inside them.

```c
// Create a Node at X: 100, Y: 200
ui_graph_node_t* math_node = ui_graph_node_create(graph_canvas, "Multiply", 100.0f, 200.0f);

// Add a standard input field to the node's body
ui_component_t* multiplier_input = ui_input_create(arena);
ui_dom_append_child((ui_component_t*)math_node, multiplier_input);
```

## Pins and Edges (Noodles)

Nodes connect to each other via "Pins" (input/output ports).

```c
// 1. Define Input Pins on the math node
ui_graph_pin_t* pin_a = ui_graph_node_add_pin(math_node, "A", UI_PIN_INPUT);
ui_graph_pin_t* pin_b = ui_graph_node_add_pin(math_node, "B", UI_PIN_INPUT);

// 2. Define an Output Pin
ui_graph_pin_t* pin_out = ui_graph_node_add_pin(math_node, "Result", UI_PIN_OUTPUT);

// ... create another node (e.g., 'Display Result') ...
ui_graph_pin_t* display_in = ui_graph_node_add_pin(display_node, "Value", UI_PIN_INPUT);

// 3. Connect them programmatically
ui_graph_edge_t* connection = ui_graph_connect(graph_canvas, pin_out, display_in);
```

## Interaction and Callbacks

The framework handles drawing the bezier curves for the edges and the drag-and-drop logic for connecting pins. You listen to events to update your application's underlying data model when the user mutates the graph.

```c
void on_nodes_connected(ui_component_t* graph, ui_graph_connection_event_t event) {
    printf("User connected Pin %s to Pin %s\n",
           event.source_pin->label,
           event.target_pin->label);

    // Update backend logic model...
}

ui_node_graph_on_connect(graph_canvas, on_nodes_connected);
```

## Custom Edge Rendering

If the default bezier curve "noodles" don't fit your aesthetic, you can override the edge rendering function using the Canvas drawing API (`ui_canvas_t`) to draw straight lines, stepped lines, or custom paths.
