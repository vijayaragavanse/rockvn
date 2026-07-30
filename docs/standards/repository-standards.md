# Repository Standards

What lives where, how a fresh machine becomes productive, and what "done"
means. If setup takes more than 30 minutes on a clean machine, this
document has a bug — file it.

## Layout

| Path | Purpose |
|---|---|
| `services/` | One directory per service; each independently buildable and containerizable |
| `infra/` | Docker Compose and infrastructure configuration (first content arrives in M3) |
| `docs/` | ADRs, standards, handbook, interview notes, reviews, failed ideas |
| `cmake/` | Shared CMake modules (`rockvn::warnings`, `rockvn::sanitizers`) |
| `scripts/` | Developer tooling (formatting); cross-platform in pairs (`.ps1`/`.sh`) |
| `tests/` | Cross-cutting tests; service-specific tests live inside the service |
| `tools/` | First-party developer tools — created when the first tool is earned |
| `build/` | All build trees, one per preset; never committed |

## New machine setup

### Both platforms

1. Install Git and CMake ≥ 3.27.
2. Clone vcpkg anywhere you like and set `VCPKG_ROOT` — the location is the
   machine's business; the repository never hardcodes it:

   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   ```

### Windows

1. Visual Studio 2022 with the "Desktop development with C++" workload.
2. Bootstrap vcpkg and persist the variable:

   ```powershell
   .\vcpkg\bootstrap-vcpkg.bat -disableMetrics
   [Environment]::SetEnvironmentVariable('VCPKG_ROOT', "$PWD\vcpkg", 'User')
   ```

3. clang-format: ships with the VS C++ workload, or install LLVM and add it
   to `PATH`.

### Ubuntu

```bash
sudo apt install build-essential g++ ninja-build clang-format
./vcpkg/bootstrap-vcpkg.sh -disableMetrics
echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.bashrc   # adjust to your clone path
```

GCC must be ≥ 13 (`g++ --version`) — the C++20 baseline depends on it.

## Building and testing

```bash
cmake --preset <name>            # configure; first run builds dependencies
cmake --build --preset <name>
ctest --preset <name>
```

Presets: `windows` (multi-config; build/test presets `windows-debug`,
`windows-release`), `linux-debug` (ASan+UBSan on), `linux-release`. The
`ci-*` presets are what GitHub Actions runs — reproduce CI locally with the
same preset name.

## Troubleshooting

- **"Could not read presets" / toolchain file not found** → `VCPKG_ROOT` is
  unset in this shell. Set it and re-open the terminal.
- **First configure is slow** → expected: vcpkg is compiling dependencies.
  Subsequent configures reuse them.
- **clang-format disagreement between machines** → CI's version is
  authoritative; match it rather than arguing locally.

## Definition of done

A change is done when: it builds and tests green under the relevant presets
on both platforms (locally or via CI); formatting passes; documentation
affected by the change is updated in the same PR; and the PR answers the
three questions. Milestones are additionally done only when their exit
review exists in `docs/reviews/`.
