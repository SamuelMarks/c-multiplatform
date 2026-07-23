# Go Bindings

The framework provides idiomatic Go (Golang) bindings located in `bindings/go/`. By utilizing `cgo`, you can write your application's business logic, networking, and concurrency in Go, while the C framework handles the high-performance UI rendering.

## Getting Started

To use the bindings in your Go module, import the package:

```go
import "github.com/your-org/ui-framework/bindings/go/ui"
```

*Note: Ensure you have a C compiler installed and that `CGO_ENABLED=1` when building.*

## The Event Loop & Goroutines

One of the biggest advantages of using Go is its powerful concurrency model. However, standard UI frameworks require all DOM mutations to happen on the **Main Thread**.

The bindings automatically handle this synchronization for you using a thread-safe command queue.

```go
package main

import (
	"fmt"
	"time"
	"github.com/your-org/ui-framework/bindings/go/ui"
)

func main() {
	// Initialize the Engine on the main thread
	engine := ui.NewEngine()
	arena := engine.FrameArena()

	button := ui.NewButton(arena, "Start Background Task")
	engine.Mount(button)

	button.OnClick(func(event ui.Event) {
		// Launch a Goroutine for work
		go func() {
			fmt.Println("Doing heavy work...")
			time.Sleep(2 * time.Second)

			// Safely update the UI from the background Goroutine!
			// RunOnMain automatically queues the closure for the next C render frame.
			engine.RunOnMain(func() {
				button.SetText("Work Complete!")
				button.SetColor(ui.ColorSuccess)
			})
		}()
	})

	// Start the blocking C event loop
	engine.Run()
}
```

## Memory Management

Unlike the Rust bindings which use compile-time lifetimes, Go is garbage-collected. To bridge Go's GC with the C framework's Arena allocator, the Go wrappers hold C pointers.

When building a UI tree, you typically pass an `ui.Arena` to the component constructors. When that view is destroyed, the C framework reclaims the entire arena block instantly, without triggering Go GC pauses.

```go
func createCard(arena *ui.Arena, title string) *ui.Container {
    card := ui.NewContainer(arena)
    card.AddClass("card-style")

    titleLabel := ui.NewLabel(arena, title)
    card.AppendChild(titleLabel)

    return card
}
```

## Distributing the App

When building your Go application for release, `cgo` will statically link the C framework into your final binary. This means you ship a single, standalone executable without needing to distribute a separate `.dll` or `.so` file!

```bash
# Build a standalone native executable
go build -ldflags="-s -w" -o my_app main.go
```
