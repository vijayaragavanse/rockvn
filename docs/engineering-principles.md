# Engineering Principles

Concrete rules that govern code in this repository. The
[philosophy](philosophy.md) explains how decisions are made; this document
fixes how code is written once a decision is made. The
[coding guidelines](standards/coding-guidelines.md) cover style mechanics.

## 1. The simplest design that solves the problem

Abstractions are added when the second concrete case appears, not when it is
imagined. An interface with one implementation and no test double is a smell.
If a function suffices, no class; if a class suffices, no hierarchy.

## 2. Ownership is explicit — RAII everywhere

Every resource (memory, socket, file handle, connection) is owned by an
object whose destructor releases it. `std::unique_ptr` is the default owning
type; `std::shared_ptr` is a design decision requiring justification, not a
convenience. Raw pointers and references never own; they observe. Manual
`new`/`delete` does not appear in first-party code.

## 3. No global mutable state

Dependencies are passed in through constructors, as interfaces at
architectural seams (repository, clock, transport) and as concrete types
elsewhere. No service locators, no singletons, no dependency-injection
framework — constructor wiring in `main()` is the composition root and is
enough at this scale.

## 4. Errors are part of the interface

No error is silently swallowed; every failure path either handles the error
meaningfully or propagates it. Exceptions are never used for control flow.
The concrete error-handling policy (exceptions vs. result types at layer
boundaries) is deliberately deferred to M2, where the first real service
layer gives the decision something to bind to — choosing it now would be
designing in the abstract.

## 5. Tests are the executable specification

Unit tests are fast, deterministic, and independent; integration tests
exercise real boundaries (database, network) and are clearly separated.
Failure paths are tested with the same seriousness as success paths — an
untested error branch is untested code. Test code is held to production
standards.

## 6. Observable by default

Every service explains itself at runtime: structured logs with request
durations and failure reasons, health endpoints from the first milestone,
metrics endpoints once a scraper exists to consume them (M11). Log messages
are written for the engineer debugging at 3 a.m.

## 7. Configuration comes from the environment

No hardcoded hosts, ports, paths, or credentials — machines change, IPs
change, secrets rotate. Invalid configuration fails at startup with a
message naming the offending variable, never at first use.

## 8. Security is not a milestone

No secrets in version control, ever. Inputs are validated at trust
boundaries. Dependencies are version-pinned (vcpkg baseline) so builds are
reproducible and auditable. Auth arrives in M4, but these habits start at M0.

## 9. Performance work requires numbers

A baseline before, a measurement after, both recorded. Speculative
optimization that complicates code without a motivating measurement is
rejected in review (see M8 gating M10).

## 10. Boring and pinned beats novel and floating

Dependencies are chosen for maturity and pinned to exact versions. Chasing
newest releases is not a goal; reproducing last month's build exactly is.
