# Project Vision

## What this is

A production-inspired microservices backend built from scratch in C++20,
developed milestone by milestone over months, across multiple machines, with
the discipline of a real engineering team. The domain is a deliberately
conventional commerce system (users, products, orders, inventory) — the
domain is not the point; the engineering around it is.

## What done looks like

The project is done when a staff-level reviewer can:

- Clone the repo on Windows or Linux and build green with two commands
  (`cmake --preset`, `cmake --build --preset`).
- Run the system locally via Docker Compose and trace one request from the
  API gateway through a service, its database, and an asynchronous consumer.
- Open any dependency in `vcpkg.json` and find the ADR that admitted it.
- Read a measured performance story: baseline (M8), change, result (M10).
- Find, for every technology in the stack, an interview-grade explanation in
  `docs/interview/` and a design review in `docs/reviews/`.
- Ask "why not X?" for the obvious alternatives and find the answer written
  down — in an ADR's alternatives section or in `docs/failed-ideas/`.

## Long-term goals

1. Every milestone in the [roadmap](../README.md#roadmap) closed with its
   exit review.
2. A resilience story that is tested, not asserted: failure-scenario tests
   for retries, circuit breaking, and broker outages (M12).
3. A production-readiness milestone (M14) covering graceful shutdown,
   secrets handling, API versioning, and readiness/liveness separation —
   the difference between "runs" and "operable".
4. An engineering handbook a newcomer could genuinely onboard from.

## Non-goals

- **Not a product.** No real users, no feature completeness; features exist
  to create engineering problems worth solving.
- **No Kubernetes** unless the deployment milestone demonstrates a problem
  Docker Compose cannot solve at this scale.
- **No microservices theater.** Services are split where the split teaches
  or solves something; a service that exists only to raise the service count
  fails the three questions.
- **Not a framework showcase.** Libraries are admitted for problems, not
  résumés — the [decision tree](decision-tree.md) governs.

## Review cadence

Every milestone ends with a written exit review
([template](reviews/template.md)) covering architecture, code, failure
modes, security, scalability, trade-offs accepted, and the interview
questions the milestone generated. Reviews are honest: known weaknesses are
listed as documented debt, not hidden.
