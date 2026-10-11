# Contributing

Thanks for your interest in Pentomino. This page covers where code goes and
the style every change must follow. How to build and run the project is in
the [README](README.md#build).

## Before you send a change

| Check | How |
| --- | --- |
| Builds without warnings | GCC 13 and Clang 18 with libc++ ([build commands](README.md#build)) |
| Tests cover the change | Add or update [GoogleTest](https://google.github.io/googletest/) tests for any change in behavior |
| Style passes | `scripts/format.sh --check --names` exits 0 |

## Commit messages

```
core: Add Status, the result type of every core call that can fail

What changed and why, wrapped at 72 columns.
```

| Rule | Detail |
| --- | --- |
| Subject | `<component>: <Summary>`. Capital letter after the colon, imperative ("Add", not "Added"), no period |
| Component | The part the commit touches, from the table below |
| Body | What changed and why, wrapped at 72 columns |

| Component | Covers |
| --- | --- |
| `core` | `core/` |
| `generator` | `generator/`, when several stages change |
| `loader`, `optimizer`, `emitter` | One generator stage |
| `cli`, `ui`, `examples`, `scripts` | That directory |
| `tests` | Tests only |
| `build` | CMake files |
| `doc` | `README.md`, `CONTRIBUTING.md`, `doc/` |
| `style` | `.clang-format`, `.clang-tidy`, `.editorconfig` |

## Where code goes

When you add code:

| Rule | Example |
| --- | --- |
| `core` and `generator` never use each other. Only the generated C++ links `core` | `emitter` writes calls to `core` as text and does not include it |
| The generator stages never use each other and share only `model` | `emitter` does not include `optimizer/` |
| Every header goes in `include/<dir>/`, its source in `src/<dir>/`, same subpath | `loader/JsonLoader.hpp` and `loader/JsonLoader.cpp` |
| Include a header by its path below `include/` | `#include "model/Graph.hpp"` |
| A new kind of thing gets its own directory instead of joining a crowded one | |
| Tests mirror the code they test | `tests/generator/loader/` for `generator/src/loader/` |

## Code style

### Running the format script

```sh
scripts/format.sh                  # fix the layout in place
scripts/format.sh --check --names  # check everything, change nothing
```

The rules below are configured in [`.clang-format`](.clang-format) and
[`.clang-tidy`](.clang-tidy). Whatever the script cannot fix by itself, it
reports, and you fix it by hand. `--names` needs the build directory to be
configured first (`cmake -S . -B build`), no compiling needed. The tools need
version 18 or newer. On Debian or Ubuntu:
`sudo apt install clang-format clang-tidy`.

### Layout

| Rule | Setting |
| --- | --- |
| Line length | **80 columns**, no exceptions. Split long strings by hand |
| Indentation | **4 spaces**, never tabs (also in CMake and shell scripts) |
| Braces | Allman: `{` and `}` on their own lines, also before `else` and `catch` |
| Control statements | One space before `(`: `if (`, `else if (`, `for (`, `while (`, `switch (`, `catch (` |
| Calls and call-like keywords | No space before `(`: `Run(value)`, `sizeof(T)`, `decltype(value)`, `static_assert(...)` |
| Pointers and references | Attached to the name: `int *frame`, `const T &value` |
| Short bodies | Only lambdas may stay on one line. Functions, `if`, loops and `case` never do |
| Access specifiers | 2 spaces in from `class`, members 4 spaces in. See the example below |
| Namespaces | No extra indentation. The closing brace gets `// namespace Name` |
| Includes | Sorted and grouped by the format script |
| Blank lines | At most one in a row |

### Naming

| Kind | Style | Example |
| --- | --- | --- |
| Namespace, class, struct, enum, enum value, alias, template parameter | UpperCamelCase | `Pento`, `FrameCounter`, `MemoryDomain::DmaBuf` |
| Function, method | UpperCamelCase | `LoadGraph()`, `Count()` |
| Parameter, local variable | lowerCamelCase | `droppedFrames`, `graphPath` |
| Class data member, static or not | `m_` + lowerCamelCase | `m_frameCount` |
| Public field of a plain-data struct | lowerCamelCase | `port.frameRate` |
| `constexpr` variable, namespace-scope `const`, `static const` member | UPPER_SNAKE_CASE | `MAX_RETRY_COUNT` |
| `const` local variable or parameter | lowerCamelCase | `const int clampedWidth = ...;` |
| Boolean, yes/no function | `is`, `has`, `can` or `should` + name | `isRequired`, `HasErrors()` |
| Macro (`#define`) | `PENTO_` + UPPER_SNAKE_CASE | `PENTO_SOURCE_DIR` |
| C++ file | UpperCamelCase | `JsonLoader.hpp`, `JsonLoader.cpp` |
| Directory | lowercase | `loader/`, `emitter/cpp/` |

`main` is the only exception, because C++ requires that name. A function
that does work and reports success with a `bool`, such as `ParseCaps()`, is
named for the work, not as a question.

### Other rules

| Rule | Avoid | Prefer |
| --- | --- | --- |
| Names say what they are, at least 3 characters (loop counters `i`, `j`, `k` and `id` excepted) | `int t;` | `int timeoutMs;` |
| No hardcoded numbers except `0` and `1` | `if (retries > 3)` | `if (retries > MAX_RETRY_COUNT)` |
| Every class declares `SPtr` and `UPtr` aliases | | `using SPtr = std::shared_ptr<FrameCounter>;` |
| Overrides say `override`, not `virtual` | `virtual void Run();` | `void Run() override;` |
| Null pointers are `nullptr` | `int *frame = 0;` | `int *frame = nullptr;` |
| One-argument constructors are `explicit` | `FrameCounter(int limit);` | `explicit FrameCounter(int limit);` |
| No `using namespace` in headers | `using namespace std;` | `std::string` |
| Headers have an include guard: `PENTO_`, the part, then the path below `include/` | `#pragma once` | `#ifndef PENTO_GENERATOR_MODEL_EDGE_HPP` in `generator/include/model/Edge.hpp` |

### Example

```cpp
namespace Pento
{

constexpr int MAX_RETRY_COUNT = 3;

class FrameCounter
{
  public:
    using SPtr = std::shared_ptr<FrameCounter>;
    using UPtr = std::unique_ptr<FrameCounter>;

    int Count() const
    {
        return m_frameCount;
    }

    void Record(int droppedFrames)
    {
        for (int attempt = 0; attempt < MAX_RETRY_COUNT; ++attempt)
        {
            if (droppedFrames > 0)
            {
                m_frameCount += droppedFrames;
            }
            else
            {
                ++m_frameCount;
            }
        }
    }

  private:
    int m_frameCount = 0;
};

}  // namespace Pento
```

## Comments

Comments explain why, not what. Headers carry a short "DESIGN NOTES" block
saying what the thing is for and what it deliberately does not do. Keep that
habit when you add a new component. Longer background belongs in
[`doc/`](doc/README.md).

## License headers

Every source file starts with:

```cpp
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SoonTheanSiew
```

(`#` instead of `//` in CMake, shell and YAML files.) By contributing you agree
that your contribution is licensed under the project's
[MIT license](LICENSE).
