# ADR-0001: C++20 for a microservices backend

- **Status:** Accepted
- **Date:** 2026-07-30
- **Milestone:** M0

## Context

The project needs one implementation language for a multi-service backend
that will be developed for months across Windows and Linux machines. The
language choice shapes every later decision: build system, dependency
management, hiring-signal value of the repository, and how much of the
stack's machinery stays visible versus hidden by a framework.

## Decision

All services are written in C++20, built with CMake, with dependencies
managed by vcpkg ([ADR-0003](0003-vcpkg-manifest-mode.md)).

## Alternatives considered

**Go** — the default choice for this domain, and the strongest alternative.
First-class HTTP, gRPC, and Postgres support; goroutines map naturally onto
request handling; second-fastest path from empty repo to running system.
Rejected because it works against both project goals: it would demonstrate
ecosystem assembly rather than systems engineering (the interesting
machinery — connection pooling, serialization, lifecycle management — comes
prebuilt), and it duplicates what thousands of portfolio repos already show.

**Java / Spring Boot** — the enterprise incumbent with the deepest
ecosystem. Rejected for the same reason amplified: Spring's value *is*
hiding the machinery this project exists to expose.

**C# / ASP.NET Core** — technically excellent, same objection as Java, with
a smaller cross-platform C++-adjacent audience.

## Consequences

**Positive:** Performance and memory behavior stay transparent and
explainable. RAII, ownership, and lifetime discipline are demonstrated in
every file rather than asserted in a résumé. The repository differentiates:
C++ microservices done carefully are rare. Aligns with the author's systems
background, so depth compounds instead of restarting.

**Negative / accepted costs:** Substantially more plumbing is on us — HTTP
serving, JSON handling, and DB access are libraries to integrate, not
built-ins. The library ecosystem is uneven, making dependency choices
genuinely hard (hence the decision-tree gate). Compile times and toolchain
setup cost real iteration speed. Memory-safety bugs are possible in ways
managed languages preclude — mitigated, not eliminated, by sanitizers in
every Linux debug build and warnings-as-errors everywhere.

## Interview framing

"I chose C++20 knowing Go was the pragmatic default — and I can argue Go's
case. But my goal was demonstrating systems engineering, not framework
assembly: in C++ every design decision about ownership, concurrency, and
failure handling is visible in the code and I have to get it right myself.
I paid for that with integration work a Go developer gets free, and I
managed the risk with sanitizers, warnings-as-errors, and a pinned,
minimal dependency set."
