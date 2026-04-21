# Theme Data Flow Diagram

The following Mermaid sequence diagram illustrates the lifecycle of a system theme change down to individual node repainting.

```mermaid
sequenceDiagram
    participant OS as Operating System
    participant Core as cmp_modality_t (Main Loop)
    participant Bus as Event Bus
    participant Theme as cmp_theme_t
    participant UI as cmp_ui_node_t (Tree)
    participant Render as cmp_gpu_t

    OS->>Core: WM_SETTINGCHANGE (Windows) / KVO (macOS)
    Core->>Bus: Dispatch CMP_EVENT_THEME_CHANGED
    Bus->>Theme: Recompute HCT Palette & Scheme
    Theme-->>Bus: Cache Updated (New Dictionary)
    
    Bus->>UI: Traverse Tree
    loop For each Node
        alt Has bg_color_bind or text_color_bind
            UI->>UI: Set DIRTY_PAINT flag
            UI->>Theme: Fetch new mapped color
            UI->>UI: Push new color to Transition Engine
        end
    end
    
    Core->>Render: VSync Tick
    loop Animation Over 300ms
        Render->>UI: Evaluate interpolated color
        UI-->>Render: Issue Draw Command
    end
```

## Description
1. The OS signals a system preference change.
2. The core detects this without blocking and pushes a generic `CMP_EVENT_THEME_CHANGED` event to the lock-free ring buffer.
3. The event bus orchestrates the invalidation of the current active color dictionary and rebuilds the HCT tonal schemes.
4. The framework traverses the existing DOM/UI tree. Any nodes using declarative variable bindings are flagged as dirty and queue a cross-fade transition.
5. The compositor updates the GPU vertex buffers incrementally over the next 300ms, creating a smooth transition to dark mode.
