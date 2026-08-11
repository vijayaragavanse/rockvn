# CI with GitHub Actions

## Why it is here

With development switching between machines, CI is the only referee that
outranks every laptop: `main` is green means green on a matrix nobody's
local setup biases. The workflow: [ci.yml](../../.github/workflows/ci.yml).

## Design decisions in this pipeline

**A real OS/compiler matrix.** Windows/MSVC and Linux/GCC on every PR —
because development genuinely happens on both, and each compiler catches
what the other tolerates (MSVC's `/permissive-` vs GCC's stricter
two-phase lookup, different stdlib implementations). The Linux leg also
runs ASan+UBSan, so every PR is memory-checked without anyone remembering
to do it.

**`fail-fast: false`.** Default matrix behavior cancels remaining jobs on
first failure — which hides whether a break is platform-specific or
universal, the exact signal a cross-platform matrix exists to produce.

**Pinned dependency snapshot.** CI checks out vcpkg at the same commit as
the manifest baseline (`VCPKG_COMMIT` must equal `builtin-baseline`), so CI
builds the dependency set developers build — not whatever the runner image
happens to carry.

**Caching with honest keys.** The vcpkg binary cache is keyed on platform +
`hashFiles('vcpkg.json')`: dependency compiles are reused until the
manifest changes, then rebuilt from scratch. A too-loose cache key serves
stale artifacts; a too-tight one recompiles the world every run.

**A fast, separate format job.** clang-format verification fails in
seconds without occupying a build runner — cheap checks should fail cheap.

## Likely questions

**"What does a green build actually prove?"** Exactly what the pipeline
tests: compiles warning-free on two compilers, tests pass, Linux leg is
sanitizer-clean, formatting conforms. Saying precisely what it does *not*
prove (no load behavior, no integration beyond what tests cover) is the
senior part of the answer.

**"How do you keep CI fast as the project grows?"** Measure first. The
known levers, in order: binary/ccache-style caching (already in place for
dependencies), path-filtered jobs per service (the monorepo ADR names this
as deferred), splitting slow integration suites from the PR gate. Adding
them before the pain is measurable is ceremony.

**"Why build PRs and not just main?"** The PR is the decision point —
red must arrive before merge, not after. Post-merge-only CI turns `main`
into the place where breakage is discovered.

## Common mistakes

- Cancel-on-first-failure hiding platform-specific breaks (`fail-fast`).
- Cache keys that don't include the thing that invalidates them.
- CI environment drifting from developer environments (solved here by
  shared presets + pinned vcpkg).
- Trusting unpinned third-party actions with repo access — this repo uses
  only official `actions/*` by major version; SHA-pinning is listed as a
  hardening step in the M0 review.
