# Milestone 0 Exit Review — Repository, Standards & Toolchain Bootstrap

- **Date:** 2026-07-30
- **Tag / commit range:** `m0` (root commit → M0 squash merge; tag applied at merge)

## What shipped

A repository that enforces its own standards: CMake presets as the
cross-machine build contract, a vcpkg manifest with a pinned baseline,
warnings-as-errors and sanitizer policies as interface targets, a two-OS CI
matrix with a format gate, the decision-making framework (three questions,
decision tree, ADRs, failed-ideas record), four founding ADRs, standards
documents, and the seeds of the handbook and interview knowledge base. One
smoke test proves the toolchain contract. No services — deliberately.

## The three questions, answered for the milestone

1. **Problem solved:** development across several Windows and Linux
   machines fails without reproducible builds, enforced standards, and CI
   as referee; standards retrofitted after code exists never stick.
2. **Proof it works:** verified on Windows — vcpkg resolved GoogleTest
   1.17.0 from the pinned baseline, MSVC 19.44 built the smoke test under
   `/W4 /WX`, all 3 CTest cases pass, clang-format check exits 0. The Linux
   leg runs in CI on first push; until that run is green, Linux support is
   *designed*, not *proven* — the distinction matters and is tracked below.
3. **Two-minute explanation:** before writing features, the repo was made
   self-enforcing — presets make every machine build identically, the
   manifest baseline makes dependencies a reviewable diff, CI re-runs the
   same presets as referee, and the PR/ADR/review templates force every
   future change to justify itself.

## Architecture review

M0's architecture is process architecture. The boundaries that will bear
load later: `services/` contract (independent build/container per service),
the earned-shared-code rule, and env-only configuration. The riskiest
structural bet is the deliberate *absence* of infrastructure; the roadmap
depends on M1–M3 proving conventions worth replicating.

## Code review

Honest assessment: the C++ surface is one intentionally trivial test file —
its value is what it exercises (compiler baseline, dependency resolution,
CTest wiring), not what it computes. The real "code" of M0 is the build
system. Strong: interface-target policies, preset conditions preventing
wrong-OS use, no machine paths anywhere. A sharp reviewer would flag the
generator asymmetry (VS multi-config on Windows, Ninja single-config on
Linux) as cognitive overhead — accepted so Windows needs no
developer-prompt setup and VS debugging works out of the box; the presets
absorb the asymmetry.

## Failure-mode analysis

- **`VCPKG_ROOT` unset** → configure fails with an unhelpful toolchain-file
  error. Mitigated by troubleshooting docs; a friendlier preflight check is
  possible if it bites repeatedly.
- **Baseline duplication** → the vcpkg commit appears in `vcpkg.json` and
  again as `VCPKG_COMMIT` in `ci.yml`. If they drift, CI builds a different
  snapshot than developers. Accepted for now (two files, one grep); trigger
  for fixing: the first drift incident adds a CI step that fails on
  mismatch.
- **clang-format version drift** → local VS ships 19.1.5, ubuntu-latest
  currently ships 18.x; a future version disagreement could pass locally
  and fail CI. Policy: CI is authoritative. Trigger for pinning an exact
  LLVM version in CI: first false-positive failure.
- **Runner image drift** → `ubuntu-latest`/`windows-latest` move under us
  (GCC major bumps). Accepted: catching toolchain drift early is a goal,
  and CI failures attribute cleanly to image changes.
- **Linux leg unverified locally** — the top open risk of this milestone,
  closed by the first green CI run.

## Security review

No secrets exist or are needed; `.gitignore` blocks `.env` from birth.
Dependencies are pinned by baseline commit (supply-chain reproducibility).
CI uses only official `actions/*` pinned by major tag — SHA-pinning is
deferred with a hard trigger: before any secret or token enters the
workflow. MIT license applied.

## Performance notes

No runtime to measure. Recorded for reference: first configure on this
machine took ~6.7 min (5.7 min of that is vcpkg compiling GoogleTest);
warm configures take seconds. CI cold runs will pay the same cost once per
baseline change, then hit the binary cache.

## Scalability review

CI builds everything on every PR — correct at this size; the monorepo ADR
names path-filtered jobs as the fix when measured pain arrives. The docs
framework scales by construction (per-milestone additions), but the
handbook's "planned" table is a promise that must be paid every milestone
or it becomes the dishonest documentation the philosophy forbids.

## Trade-offs accepted

Bootstrapping before any product code (violates "problem must exist today"
in the small, justified by multi-machine development being a day-one
problem); generator asymmetry for Windows convenience; baseline duplicated
in CI; actions pinned by tag not SHA; M0's first commit landed directly on
`main` (recorded as the documented exception in the git workflow).

## Future improvements (documented debt)

| Item | Trigger |
|---|---|
| CI step verifying `VCPKG_COMMIT` == manifest baseline | First drift incident |
| Pin exact clang-format version in CI | First version-skew false failure |
| SHA-pin GitHub Actions | Before any secret enters the workflow |
| CI status badge in README | Repo pushed to GitHub |
| clang-tidy in CI | M2, with the first layered architecture |
| Path-filtered CI jobs | Measured CI latency pain |

## Interview questions this milestone generated

1. Why C++ for microservices, knowing Go exists? →
   [ADR-0001](../adr/0001-cpp20-for-a-microservices-backend.md)
2. How do you make C++ builds reproducible across OSes and months? →
   [ADR-0003](../adr/0003-vcpkg-manifest-mode.md), [interview/vcpkg.md](../interview/vcpkg.md)
3. Monorepo vs polyrepo — argue both sides. →
   [ADR-0002](../adr/0002-monorepo.md)
4. What does target-based CMake buy over flags-in-variables? →
   [interview/cmake.md](../interview/cmake.md), [cmake/CompilerWarnings.cmake](../../cmake/CompilerWarnings.cmake)
5. What does a green CI build prove — and not prove? →
   [interview/github-actions-ci.md](../interview/github-actions-ci.md)
6. Why defer the message broker everyone puts in the diagram? →
   [failed-ideas/0002](../failed-ideas/0002-kafka-from-day-one.md)
