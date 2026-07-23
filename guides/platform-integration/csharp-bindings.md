# C# Bindings (.NET)

The framework provides comprehensive C# bindings located in `bindings/csharp/`, leveraging .NET's `DllImport` (P/Invoke) capabilities. This allows developers to build high-performance, cross-platform UI applications using C# 10+ and the .NET ecosystem (including async/await, LINQ, and NuGet packages).

## Getting Started

Reference the UI framework project or NuGet package in your `.csproj`:

```xml
<ItemGroup>
  <ProjectReference Include="..\..\bindings\csharp\UIFramework.Core\UIFramework.Core.csproj" />
</ItemGroup>
```

Ensure the native C shared library (`.dll`, `.so`, or `.dylib`) is copied to your output directory.

## App Lifecycle & Async/Await

The C# wrapper abstracts the C event loop and smoothly integrates with .NET's `Task` and `async/await` infrastructure. Background threads can seamlessly marshal UI updates back to the main rendering thread via `Engine.Invoke`.

```csharp
using UIFramework;
using UIFramework.Components;

public class Program
{
    public static void Main()
    {
        using var engine = new Engine();
        var arena = engine.FrameArena;

        var btn = new Button(arena, "Fetch Data");

        btn.OnClick += async (sender, e) =>
        {
            btn.Text = "Loading...";
            btn.Disable();

            // Do non-blocking network I/O on a background thread
            var data = await FetchDataFromApiAsync();

            // Update the UI safely on the main thread
            engine.Invoke(() => {
                btn.Text = data;
                btn.Enable();
            });
        };

        engine.Mount(btn);
        engine.Run(); // Blocks until the app exits
    }
}
```

## Memory Management

Because C# uses a Garbage Collector and the native framework uses Arena allocation, the bindings carefully manage the boundary between the two.

*   **Managed to Native:** When you pass a C# delegate (like an event handler) to the native UI, the C# binding allocates a `GCHandle` to prevent the Garbage Collector from freeing the delegate while the C UI is still holding a function pointer to it.
*   **Native to Managed:** UI components are created within an `Arena`. When the C# wrapper goes out of scope, it *does not* free the C component; the native engine reclaims memory when the arena is destroyed.

## Data Binding (MVVM)

The C# bindings are fully compatible with standard .NET data binding interfaces like `INotifyPropertyChanged`. The framework provides a `BindingContext` utility to automatically wire C# view models to native UI properties.

```csharp
// Example MVVM Binding
var vm = new UserViewModel { Name = "Alice" };
var label = new Label(arena);

// Bind the C# ViewModel 'Name' property to the native Label's text
label.Bind(Label.TextProperty, vm, nameof(vm.Name));
```
