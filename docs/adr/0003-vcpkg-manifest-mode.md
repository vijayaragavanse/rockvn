# ADR-0003: vcpkg in manifest mode

- **Status:** Accepted
- **Date:** 2026-07-30
- **Milestone:** M0

## Context

C++ has no default package manager, and this project's constraints make the
choice consequential: identical dependency versions across several Windows
and Linux machines, reproducible months from now, with MSVC and GCC both
first-class. Manual dependency management fails all three constraints
immediately.

## Decision

vcpkg in manifest mode: dependencies declared in `vcpkg.json`, versions
pinned by `builtin-baseline`, integrated through the vcpkg CMake toolchain
file located via the `VCPKG_ROOT` environment variable. One simplification
over common setups: the baseline is pinned directly in `vcpkg.json` rather
than a separate `vcpkg-configuration.json` — one file, same pin, less to
explain.

## Alternatives considered

**Conan 2** — the most capable alternative: true version ranges, remotes,
lockfiles, profiles that model cross-compilation cleanly. Rejected because
its power targets problems this project lacks (private artifact servers,
per-team profiles), it adds a Python runtime dependency to every machine,
and its learning curve would tax every machine-switch. If this were a
multi-team product, Conan would likely win.

**CMake FetchContent** — no extra tool at all, which is genuinely
attractive. Rejected because every clean configure recompiles all
dependencies from source (painful on laptops, worse in CI), transitive
dependencies must be hand-managed, and there is no binary caching story.
Fine for one header-only dependency; wrong for a growing service stack.

**System package managers (apt + manual on Windows)** — rejected outright:
versions differ by OS release, Windows has no parity, reproducibility is
zero.

## Consequences

**Positive:** One committed manifest defines dependencies for every machine
and CI. The baseline pin makes builds reproducible and dependency upgrades
explicit, reviewable diffs. Binary caching (GitHub Actions cache in CI)
eliminates repeated dependency compiles.

**Negative / accepted costs:** Every machine needs a vcpkg clone and
`VCPKG_ROOT` set — documented one-time setup in
[repository standards](../standards/repository-standards.md). First
configure on a fresh machine compiles dependencies from source. vcpkg's
curated registry occasionally lags upstream releases; overlay ports are the
escape hatch if it ever matters. Triplets add a concept to learn, though
default triplets suffice so far.

## Interview framing

"C++ dependency management is a real engineering decision, not an install
step. I needed reproducibility across Windows and Linux machines over
months, so I used vcpkg manifests with a pinned baseline — the manifest
declares *what*, the baseline pins *which exact version*, and CMake's
toolchain file wires it in without any hardcoded paths. I'd pick Conan for
a multi-team product with an artifact server; for this project its power
was pure overhead."
