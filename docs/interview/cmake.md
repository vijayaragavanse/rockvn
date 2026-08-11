# CMake

## Why it is here

The only build system with first-class support across MSVC, GCC, every
major IDE, and vcpkg. In this repo it is also the *machine contract*:
`CMakePresets.json` guarantees that "build the project" means the same
thing on every laptop and in CI.

## Concepts that matter

**Targets and usage requirements.** Modern CMake models the build as a
graph of targets carrying their own requirements. `PRIVATE` requirements
apply to the target itself; `INTERFACE` to consumers; `PUBLIC` to both.
This repo's [`rockvn::warnings`](../../cmake/CompilerWarnings.cmake) is the
idiom in pure form — an `INTERFACE` library carrying only compile options,
so the warning policy is one target link away and third-party code is never
subjected to it.

**Presets.** Command-line flags don't survive multiple machines; presets
commit the configuration matrix to the repo. `cmake --preset linux-debug`
is identical everywhere, and CI runs the same `ci-*` presets a developer
can run locally to reproduce failures.

**Toolchain files.** The injection point for "how to compile", separate
from "what to build". vcpkg integrates as a toolchain file, which is why
dependencies resolve with zero find-module hacks — and it's located via
`$env{VCPKG_ROOT}`, keeping machine-specific paths out of the repo.

**Multi-config vs single-config generators.** Visual Studio's generator
holds Debug and Release in one build tree (configuration chosen at build
time); Ninja bakes one configuration per tree at configure time. This repo
uses VS on Windows (no developer-prompt setup, first-class IDE debugging)
and Ninja on Linux — the presets absorb the asymmetry so users never see it.

## Likely questions

**"Why target-based CMake instead of global flags?"** Global
`CMAKE_CXX_FLAGS` leak to every target including third-party code, and
order-dependent directory commands make builds fragile. Targets make
requirements local, explicit, and composable — my warnings policy applies
to exactly the code I own.

**"How do you keep builds identical across machines?"** Committed presets
for configuration, a pinned vcpkg baseline for dependencies, and CI running
the same presets as the referee. The only machine-specific state is one
environment variable.

**"Why require CMake 3.27?"** Presets v6 and mature vcpkg integration. A
floor that old tools silently miss produces confusing errors; a hard
`cmake_minimum_required` fails loud and early.

## Common mistakes (probed in interviews)

- `file(GLOB)` for sources — new files silently missing from builds after
  pulls; sources are listed explicitly here.
- In-source builds — guarded against with a hard error in the top-level
  [CMakeLists.txt](../../CMakeLists.txt).
- Flags via `CMAKE_CXX_FLAGS` string surgery instead of
  `target_compile_options`.
- Treating `find_package` failures with hand-written find modules instead
  of fixing the toolchain/dependency layer.
