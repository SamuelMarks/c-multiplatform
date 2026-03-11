Future work
===========

## Ecosystem Integration Roadmap

LibCMPC is part of a larger ecosystem of C libraries being developed to provide a comprehensive cross-platform application toolkit. The following integrations are planned:

- **HTTP/HTTPS Client Integration**: Full integration with `c-abstract-http` for robust cross-platform networking, powering internal asynchronous asset fetching and API communication directly within C multiplatform.
- **Room-Style Database Interface**: Building upon `c-orm`, we will expose a type-safe, declarative local database abstraction (similar to Android's Room database) allowing C applications to easily persist data via SQLite.
- **C Client GUI Generation**: Using `cdd-c` in conjunction with `c-abstract-http` and `c-orm`, we will add support to automatically generate LibCMPC client GUIs and data models directly from OpenAPI specifications. This will drastically reduce boilerplate when building API-driven applications.
- **Native File System Management**: Replacing simplistic native I/O with `c-fs` to provide a robust `std::filesystem` equivalent C89 port for seamless cross-platform path management and directory operations.

## Other language examples

There are only 5 languages in popular usage for frontends. These are:
- C++
- TypeScript/JavaScript
- Dart (basically just because of Flutter)
- Kotlin (esp. in Android world, but now also iOS, desktop, web thanks to Kotlin Multiplatform)
- Java
- Swift (specifically for iOS and macOS)

Go and Python also have some users, as does C, but that's about it.

Through FFI, C can be called into from 40+ programming languages; basically any of them. Rather than starting with all 40+ of them, building out these 8 seems like a better first step.

Start with creating really nice examples in C. Then port those C examples to these 7 languages.
