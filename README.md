# Pentomino

Pentomino turns a media inference pipeline, a graph of nodes such as
*decode → scale → classify → encode*, into an optimized execution plan and
C++ code. For every connection it decides how the data should move between
the two nodes, and it explains each decision.

The target machine is described in a plain JSON file (a *capability
profile*), so the same graph can be planned for a CPU-only box, a GPU with
dmabuf sharing, or another vendor's hardware.

> **Status: early.** Pentomino is not usable yet.

## How it works

```
graph.json ──┐
             ├─► Loader ──► Optimizer ──► Emitter ──► C++ ──calls──► Core
profile.json ┘
```

| Part | Job |
| --- | --- |
| **Loader** | Reads the graph and profile files |
| **Optimizer** | Chooses an implementation for every node and a transfer strategy for every edge |
| **Emitter** | Writes the plan as C++ |
| **Core** | The C++ runtime the generated code calls. It runs the nodes, for example an FFmpeg decoder |

## Build

Requirements: CMake 3.24+, and GCC 13+ or Clang 18+. nlohmann_json and
GoogleTest are downloaded if they are not installed.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Background knowledge and design notes
live in [`doc/`](doc/README.md).

## License

MIT. See [LICENSE](LICENSE).
