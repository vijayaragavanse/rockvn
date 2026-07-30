# Project Philosophy

This repository is a long-running engineering project whose real deliverable
is demonstrated judgment. Working code is table stakes; the artifact under
review is the reasoning — why each piece exists, what was rejected, and what
each choice cost.

## The three questions

Nothing enters this repository without answering:

1. **What problem are we solving?** The problem must exist *today*. "We will
   need it later" is a reason to write it down, not a reason to build it.
2. **How can we prove the solution works?** Measurement, tests, or
   demonstration — chosen before implementation, not after.
3. **Could we explain this design choice to a senior interviewer in under
   two minutes?** If the explanation needs ten minutes, the design is not
   understood well enough to be committed.

The rule is enforced by process, not memory: the
[PR template](../.github/PULL_REQUEST_TEMPLATE.md),
[ADR template](adr/template.md), and
[milestone review template](reviews/template.md) all embed these questions.

## Technology must earn its place

Every dependency, pattern, and infrastructure component is introduced at the
moment it solves a measurable problem — never earlier. This is why the
repository starts with no message broker, no cache, and no metrics stack,
despite all three appearing in the target architecture. The
[decision tree](decision-tree.md) is the standard path into the repo; ideas
that fail it are recorded in [failed-ideas](failed-ideas/README.md).

## Walking skeleton over parallel scaffolds

One service is built end to end — HTTP, configuration, logging, tests,
container, CI — before a second service exists. Eight half-finished skeletons
demonstrate copy-paste; one deep vertical slice demonstrates engineering.
Later services inherit conventions that were proven, not guessed.

## Measure before optimizing

No optimization lands without a baseline showing the problem and a
measurement showing the improvement. Performance baselines are a dedicated
milestone (M8) that gates the caching milestone (M10) by design.

## Simplicity beats enterprise patterns

When a simpler solution solves the problem, the simpler solution wins, and
the enterprise pattern is named in the documentation with the reason it was
not needed. Complexity is only accepted in exchange for a demonstrated
benefit. Patterns adopted to appear sophisticated fail question 1.

## Documentation is a deliverable

Docs describe the system as it is, not as it is planned to be. Planned work
is labeled planned; deferred work names the milestone that owns it; rejected
ideas keep their reasoning in `failed-ideas/`. A document that flatters the
project is worse than no document.
