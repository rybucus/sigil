sigil: compile-time byte patterns with an AVX2 scanner
======================================================

**IDA-style signatures parsed at compile time, matched 32 bytes at a time, one header.**
Header-only C++23 library for Windows and Linux.

![C++23](https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus&logoColor=white)
![AVX2](https://img.shields.io/badge/SIMD-AVX2-0071C5?logo=intel&logoColor=white)
![Windows](https://img.shields.io/badge/platform-Windows-0078D6?logo=windows&logoColor=white)
![Linux](https://img.shields.io/badge/platform-Linux-FCC624?logo=linux&logoColor=black)
![MSVC | GCC | Clang](https://img.shields.io/badge/compiler-MSVC%20%7C%20GCC%20%7C%20Clang-5C2D91)
![License: MIT](https://img.shields.io/badge/license-MIT-yellow.svg)

`header-only` `C++23` `avx2` `pattern-scanning` `signature-scanning` `constexpr`

sigil turns a pattern string into a type: the string is parsed by the compiler, a malformed pattern is a compile error, and every pattern gets comparison code generated for exactly its bytes. At run time the first and the last fully known bytes are compared 32 positions at a time with AVX2, and the candidates are verified with 1, 2, 4 or 8 byte wide comparisons.

## Scanning

```cpp
#include <sigil/sigil.hxx>

std::uint8_t* const match = sigil::byte_scanner<"48 8B 05 ?? ?? ?? ?? 4? ?F">.search( begin, size );
```

| Token | Meaning |
|---|---|
| `8B` | exact byte, case-insensitive |
| `??` or `?` | any byte |
| `4?` | high nibble is `4`, low nibble is anything |
| `?F` | low nibble is `F`, high nibble is anything |

Tokens are separated by a single space. `byte_scanner<pattern, offset = 0>` is a stateless constant; `offset` is added to every reported address.

| Call | Result |
|---|---|
| `search( begin, size )` | first match or `nullptr` |
| `search( begin, end )` | same, for a pointer range |
| `search( container )` | same, for anything with `data( )` and `size( )` over `char` or `std::uint8_t` |
| `search( ..., actions... )` | returns `void`, calls every action with each match |
| `search_unbounded( begin )` | no bounds check, the pattern must exist |

Bounded searches never read outside `[ begin, begin + size )`. The pointee may be `const`.

Include `<sigil/sigil.hxx>` only. It undefines the internal `SIGIL_*` macros after use, so the other headers are not meant to be included on their own afterwards.

## Build and integration

Requirements: CMake 3.28+, Ninja and Visual Studio 2022+ on Windows, or GCC 14+ / Clang 18+ on Linux, and a CPU with AVX2. GoogleTest 1.17.0 is fetched with `FetchContent` for the tests only. There are no submodules and no package manager.

```bat
tools\build.cmd cmake --preset debug
tools\build.cmd cmake --build --preset debug
tools\build.cmd ctest --preset debug
```

```sh
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

`tools\build.cmd` opens the Visual Studio developer environment and runs its arguments in the repository root. Presets: `debug` and `release` (Ninja + MSVC), `windows-vs2022` / `windows-vs2026` (solutions), `linux-gcc-*` and `linux-clang-*`. `tools\format.cmd` runs clang-format over `include` and `tests`; `tools\format.cmd --check` only reports differences.

Using sigil from another CMake project:

```cmake
include(FetchContent)

FetchContent_Declare(sigil
  GIT_REPOSITORY https://github.com/<owner>/sigil.git
  GIT_TAG <commit>)
FetchContent_MakeAvailable(sigil)

target_link_libraries(my_target PRIVATE sigil::sigil)
```

`add_subdirectory` works the same way. `sigil::sigil` is an `INTERFACE` target: it carries the include directory, the C++23 requirement and, on GCC and Clang, `-mavx2`. Tests are built only when sigil is the top-level project.

| Option | Default | Meaning |
|---|---|---|
| `SIGIL_BUILD_TESTS` | `ON` when top-level | Test executable `sigil_tests`. |
| `SIGIL_STATIC_RUNTIME` | `ON` | Static MSVC runtime (`/MT`, `/MTd`) for the tests; only applied when top-level. |
| `SIGIL_WARNINGS_AS_ERRORS` | `OFF` | `/WX` or `-Werror` for the tests. |
| `SIGIL_SANITIZE` | empty | `address`, `undefined` or `address,undefined` for the tests (MSVC: `address` only). |

## Tests

`sigil_tests` covers the pattern parser (bytes, wildcards, nibble masks, comparison grouping), `optional_t`, and the scanner: exact and masked matches, result offsets, matches at both buffer ends, buffers shorter than a SIMD block or than the pattern, pointer-range and container overloads, const data and multi-match actions.

## License

sigil is distributed under the [MIT License](LICENSE).
