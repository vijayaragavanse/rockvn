# ADR-0006: spdlog for structured logging

- **Status:** Accepted
- **Date:** 2026-08-11
- **Milestone:** M1

## Context

M1 requires structured, machine-parseable log lines from the first request
([design](../architecture/m1-server-foundation.md)), and the logging choice
must be framework-agnostic — from M2, `domain/` code cannot depend on
anything Drogon-shaped (ADR-0005 constraint 3), which rules out logging
through the framework.

## Decision

spdlog, wrapped in a small `StructuredLogger` owned by the composition root
and passed by reference (no spdlog global registry, keeping the no-global-
state principle intact). Lines are composed as JSON through jsoncpp —
already in the dependency set via Drogon — so field values are escaped by a
JSON library, never by string surgery.

## Alternatives considered

**Drogon/trantor's built-in logging** — zero new dependencies, but it is
the framework leaking into every layer, exactly what constraint 3 exists to
prevent; and its formatting is line-oriented, not structured.

**Boost.Log** — capable but heavy, and would admit Boost for a problem
spdlog solves in a fraction of the footprint.

**Hand-rolled logger** — a logger looks trivial until multi-threaded
flushing, level filtering, and sink management arrive; building one
violates the adopt-over-build principle for critical-path infrastructure.

## Consequences

**Positive:** mature, fast, ubiquitous (recognition value); sink
abstraction gives tests a capture sink through the constructor — log
*content* is unit-testable. **Negative / accepted:** one more dependency;
per-record flushing is enabled for observability and test determinism,
which costs throughput — revisit with M8 numbers if it shows up.

## Interview framing

"I log structured JSON lines to stdout — the container runtime owns
routing, the aggregation stack added later consumes them without
re-plumbing. Two details I'd defend: log lines are built by a JSON library
so a hostile path can't break parseability — my tests assert lines *parse*,
not just match a regex; and the logger is constructor-injected rather than
global, so tests capture and assert log output like any other dependency."
