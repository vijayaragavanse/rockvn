# ADR-0002: Monorepo

- **Status:** Accepted
- **Date:** 2026-07-30
- **Milestone:** M0

## Context

The target architecture has eight services plus shared infrastructure,
developed by one engineer switching between multiple Windows and Linux
machines, synchronized only through GitHub. The repository layout decision
must be made before the first commit.

## Decision

One repository holds everything: all services under `services/`,
infrastructure under `infra/`, shared documentation under `docs/`. Shared
code is *earned*: no common library exists until two services demonstrably
duplicate something, and extracting it requires the duplication as evidence.

## Alternatives considered

**Polyrepo (one repo per service)** — the honest case: it enforces service
independence at the strongest possible boundary, mirrors how large
organizations often operate, and keeps per-repo CI trivially scoped.
Rejected because every claimed benefit solves a coordination problem this
project does not have (multiple teams, independent release cadences), while
every cost lands immediately: cross-cutting changes (a logging convention, a
CI fix) become N pull requests, machine switching means N clones to keep in
sync, and standards drift between repos with no mechanism to stop it.

**Meta-repo with git submodules** — combines polyrepo's coordination costs
with submodules' operational sharp edges (detached HEADs, forgotten
`--recurse`). Rejected without much grief.

## Consequences

**Positive:** Atomic cross-service changes with one commit and one review.
One toolchain, one CI pipeline, one set of standards that cannot drift.
Machine switching is `git pull`. The whole system's history reads as one
narrative — valuable for a portfolio.

**Negative / accepted costs:** CI currently builds everything on every
change; when that gets slow, path-filtered jobs are the known fix and are
deliberately deferred until the pain is measured. Service independence is
now enforced by discipline (each service must remain independently buildable
and containerizable) rather than by repository walls — reviews must watch
for casual cross-service includes.

## Interview framing

"Monorepo versus polyrepo is a question about coordination costs, not code.
Polyrepo pays coordination tax to buy team independence — but I'm one
engineer on multiple machines, so I'd pay the tax and collect no benefit. I
kept the one real risk, service coupling, controlled by rule: services must
stay independently buildable, and shared code must be earned by demonstrated
duplication before it may exist."
