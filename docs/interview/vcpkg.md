# vcpkg & C++ Dependency Management

## Why it is here

C++ has no default package manager, and this project needs identical
dependency versions on several Windows and Linux machines, reproducible
months apart. Full decision record: [ADR-0003](../adr/0003-vcpkg-manifest-mode.md).

## Concepts that matter

**Manifest mode vs classic mode.** Classic mode installs packages globally
per machine — machine state, invisible to the repo, unreproducible. Manifest
mode declares dependencies in a committed [`vcpkg.json`](../../vcpkg.json);
configure resolves them into the build tree. The repo, not the machine, is
the source of truth.

**The baseline is the lockfile.** `builtin-baseline` pins a commit of the
vcpkg registry; every dependency resolves to the version that registry
snapshot defines. Same manifest + same baseline = same versions, on any
machine, at any date. Upgrading dependencies = moving the baseline = a
reviewable one-line diff. (This repo pins the baseline directly in
`vcpkg.json` rather than a separate `vcpkg-configuration.json` — same pin,
one fewer file.)

**Triplets** name the target ABI (`x64-windows`, `x64-linux`) — the reason
one manifest serves MSVC and GCC builds without conditional logic.

**Binary caching.** vcpkg caches built packages keyed by package + triplet +
compiler (the ABI hash). CI wires this to the Actions cache in
[ci.yml](../../.github/workflows/ci.yml), so dependencies compile once per
baseline change, not once per CI run.

## Likely questions

**"How is this different from `pip install` or `npm install`?"** No
central binary registry serves every compiler/stdlib/flag combination —
C++ has no stable ABI, so packages generally build from source against
*your* toolchain. The baseline pins sources; binary caching recovers
install-speed afterward. Understanding *why* (the ABI problem) matters more
than the tool.

**"Why not Conan?"** More powerful — version ranges, remotes, profiles —
but its power targets multi-team problems I don't have, and it adds a
Python dependency to every machine. I chose the tool whose complexity
matches my problem; at an org with an artifact server, I'd expect Conan to
win. (ADR-0003 has the full comparison.)

**"What breaks if you delete the baseline?"** Versions float to whatever
the machine's vcpkg clone has — two machines resolve differently and
"works on my machine" returns. The baseline is the reproducibility
mechanism, not a formality.

## Common mistakes

- Unpinned baselines (floating versions across machines and time).
- Committing `vcpkg_installed/` — build output, not source
  (ignored [here](../../.gitignore)).
- Classic-mode global installs creeping back in via tutorials.
- Hardcoding the vcpkg path instead of `VCPKG_ROOT`, breaking every other
  machine.
