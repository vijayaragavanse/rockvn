# ADR-0007: tl-expected as the error-value carrier

- **Status:** Accepted
- **Date:** 2026-08-11
- **Milestone:** M1

## Context

The M1 design fixes the error doctrine: expected, recoverable failures
travel as values; exceptions are reserved for broken invariants. That needs
a `Result`-shaped vocabulary type. C++20 — this project's pinned baseline —
does not have `std::expected`; C++23 does, and both toolchains (MSVC 19.4x,
GCC 13) could provide it.

## Decision

`tl::expected` (the reference `std::expected` polyfill by Sy Brand),
header-only via vcpkg. First consumer: configuration validation, which
returns `tl::expected<Config, std::vector<ConfigError>>` so every violation
is reported in one failure. From M2 it carries domain errors across layer
boundaries.

## Alternatives considered

**Bump the project to C++23 for `std::expected`** — tempting and
technically feasible, rejected because the C++20 baseline is a published
contract (ADR-0001, README, CI) and moving a language baseline to acquire
one type is disproportionate. When the baseline is revisited deliberately,
migration is mechanical: `tl::expected` mirrors the `std::expected` API by
design.

**Custom `Result<T, E>`** — a hundred lines that would slowly grow
`and_then`, `map`, comparison operators… i.e., re-deriving `expected`.
Adopt-over-build applies.

**Exceptions for everything** — rejected in the M1 design: recoverable
failures are control flow, and Drogon's async handlers make exception
propagation across callback boundaries the wrong default.

## Consequences

**Positive:** explicit error paths visible in signatures; monadic
composition; a straight migration path to `std::expected`. **Negative /
accepted:** one vocabulary type that a future C++23 bump obsoletes
(cheaply); discipline required so `tl::expected` stays for *expected*
failures and does not creep into invariant checking, where assertions and
exceptions belong.

## Interview framing

"My layers return `expected<T, Error>` for failures a caller can act on and
throw only on broken invariants. I wanted `std::expected`, but my baseline
is C++20 and I wasn't going to move a language standard for one type — so I
took the reference polyfill with an identical API and a mechanical
migration path. First proof it earns its keep: config validation aggregates
every violation into one startup failure instead of failing one variable at
a time."
