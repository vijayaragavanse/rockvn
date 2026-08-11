# ADR-0005: HTTP framework — Drogon

- **Status:** Accepted
- **Date:** 2026-07-30
- **Milestone:** M1
- **Revision history:** v1 proposed Boost.Beast; reversed at the design
  review of 2026-07-30 on opportunity-cost grounds (analysis below). The
  reversal is kept visible on purpose — the review process changing an
  outcome is the process working.

## Context

M1 builds the first HTTP service, and every later service inherits its
transport layer, making this the highest-leverage dependency decision in
the project. Hard requirements: first-class Windows *and* Linux, MSVC and
GCC, vcpkg availability, a path to WebSocket (M13), and no coupling between
transport and persistence (the repository layer is decided in M3 on its own
merits).

The tension: [ADR-0001](0001-cpp20-for-a-microservices-backend.md) chose
C++ to keep machinery visible, which argues for a low-level library; but
the repository's primary objective is demonstrating backend architecture,
distributed systems, observability, testing, and production engineering —
which argues against spending the critical path reimplementing transport
infrastructure that already exists.

## Decision

Drogon, used under three binding constraints that protect our
[engineering principles](../engineering-principles.md) from the framework's
defaults:

1. **No static-registration macros.** Handlers are plain objects built with
   constructor injection and registered explicitly in the composition root
   via Drogon's runtime registration API. Drogon's `HttpController`
   macro/CRTP auto-registration path is global state and is prohibited in
   this codebase.
2. **Drogon's ORM and database clients stay out.** Persistence is M3's
   decision; services depend on our repository interfaces only. No
   Drogon database module is linked, ever, so the temptation cannot
   compile.
3. **Framework types stop at `api/`.** Per the
   [services contract](../../services/README.md), Drogon request/response
   types and JSON values are mapped to our DTOs at the transport boundary
   and never reach `domain/`.

Networking internals remain a learning goal — relocated, not abandoned: M5
(networking fundamentals) hosts a bounded transport lab (raw sockets and/or
Boost.Beast) as a learning artifact off the critical path, where its
schedule risk is zero.

## The deciding analysis: educational value vs opportunity cost

**What building on Beast teaches:** async accept loops, buffer and
session-lifetime management, routing design, graceful shutdown — real
knowledge, concentrated in one milestone. **What it costs:** the knowledge
purchase is one-time, but the ownership is permanent. Every HTTP edge case
— keep-alive timeouts, slow clients, partial writes, malformed requests,
connection limits — becomes our backlog, taxing every milestone from M2 to
M14 across eight services. And the milestones it delays are precisely the
differentiating ones for this repository's thesis: authentication (M4),
transactional persistence (M3), messaging with outbox and idempotent
consumers (M9), observability (M11), resilience (M12), production
readiness (M14).

**The asymmetry that decides it:** Drogon's costs are bounded and
front-loaded; Beast's are unbounded and recurring. Beast's benefit, by
contrast, is *separable* — M5 can deliver the same learning in a bounded
lab whose failure costs nothing. When the benefit is separable and the cost
is not, take the benefit separately. A backend-engineering portfolio is
probed on architecture, data, and operations; transport-layer
implementation is a systems-programming signal — valuable, but not this
repository's thesis, and not worth the critical path.

## Comparison

Maintenance facts verified against upstream 2026-07-30; sources at the end.

| Criterion | Drogon | Boost.Beast | Crow | Pistache | oat++ |
|---|---|---|---|---|---|
| Ecosystem | Own stack (trantor, jsoncpp…), vcpkg | Boost; vcpkg first-class | Header-only + standalone ASIO | Meson, Linux-first | Zero-dep, self-contained world |
| Performance | Top-tier (TechEmpower regular) | Excellent primitive; end perf is your code's | Very good | Good (Linux) | Good |
| Learning value (transport internals) | Low — framework absorbs the machinery | Highest — async model, buffers, lifetimes all yours | Medium — sugar over ASIO | Medium | Low — proprietary idioms |
| Maintainability | Active (1.9.12, Jan 2026) | Boost release train, stable API | Community fork, sporadic releases | Moderate | 1.4.0 pending for years |
| Windows + Linux | Supported, both | First-class, both | Both | Windows recent, unproven; docs contradict | Both |
| PostgreSQL | Built-in async ORM — **excluded by constraint 2** | None | None | None | Own ORM module |
| WebSocket | Yes | First-class | Yes | No — long-standing gap | Yes |
| Testing story | Good, given constraint 1: injected handlers test without the framework | You own every seam | Handlers invocable directly | Adequate | Built-in framework, own idioms |
| Documentation | Good | Extensive, dense | Good, tutorial-flavored | Thin, partly stale | Decent |
| Community | Large, active | Boost-scale | Moderate | Small | Small; bus-factor risk |
| Portfolio story | Architecture and operations on a production framework | Transport-layer implementation depth | Neither depth | Weak | Niche |

## Alternatives considered

**Boost.Beast — v1's recommendation, and the strongest alternative.** A
protocol library, not a framework: maximum control, maximum learning,
first-class WebSocket, Boost-grade maintenance. Rejected by the
opportunity-cost analysis above. Beast remains the documented fallback if
Drogon's constraints prove unworkable in practice (specifically: if
runtime handler registration cannot cleanly support our composition-root
wiring, discovered in M1).

**Crow — the middle path that buys the wrong mixture.** Flask-style
routing over ASIO, pleasant and header-only, but it owns the loop and
routing (transport learning skipped) while providing far less than Drogon
(velocity not maximized either). Community-fork maintenance with sporadic
releases is workable but not a foundation for eight services.

**Pistache — knocked out on requirements.** Windows support is recent and
visibly immature (official docs simultaneously claim Windows support and
recommend WSL), and there is no WebSocket path. Betting every service's
transport on the newest port of a mid-sized project fails basic risk
management.

**oat++ — knocked out on sustainability and fit.** Capable and
zero-dependency, but 1.4.0 has been pending for years, activity
concentrates in one organization, and its DTO-codegen macros would replace
our layered design with the framework's.

**Not shortlisted:** cpp-httplib — admirable simplicity, but a blocking
model caps concurrency below what a gateway-fronted service stack needs.

## Consequences

**Positive:** M1 reaches a production-shaped service (config, logging,
health, container, tests) in a fraction of the time, and every subsequent
milestone starts sooner. HTTP parsing, connection management, TLS, and
WebSocket arrive mature and maintained by an active upstream. The
constraints turn the framework risk into reviewable rules rather than
hopes.

**Negative / accepted costs:** Drogon owns the event loop; our lifecycle
management works within its model (graceful-shutdown design in M14 must
work with the framework's quit mechanism, not against it). The constraints
demand ongoing review discipline — the macro path and the ORM are the easy
path and will stay tempting. Transport-layer depth is deferred to the M5
lab, and the repository's C++ story leans on architecture rather than
protocol implementation — accepted deliberately, matching the project's
stated priorities. Drogon and its dependencies (trantor, jsoncpp) enter
the supply chain — pinned by the vcpkg baseline like everything else.

## Interview framing

"My first draft chose Boost.Beast — I wanted to own the accept loop and
session lifetimes. The design review reversed it, and the reasoning is the
part I'd defend: the ownership cost of a hand-rolled transport layer is
permanent and spread across every service, while its learning benefit is
one-time and separable — so I moved the learning into a bounded networking
lab and put the critical path on a maintained framework. I kept my
architecture honest inside Drogon with three rules: no static-registration
macros — handlers are constructor-injected and registered at the
composition root; no framework ORM — my repository layer is a separate,
deliberate decision; framework types stop at the transport boundary. The
reversal is recorded in the ADR because a review that can't change
outcomes is theater."

## Sources

- [Drogon releases](https://github.com/drogonframework/drogon/releases) — 1.9.12, 2026-01-26
- [Pistache README](https://github.com/pistacheio/pistache) vs [Pistache docs](https://pistacheio.github.io/pistache/docs/) — contradictory Windows claims
- [oat++ 1.4.0 changelog (unreleased)](https://github.com/oatpp/oatpp/blob/master/changelog/1.4.0.md)
- [CrowCpp](https://crowcpp.org/) — community fork, 1.2.x/1.3.0
- [Boost.Beast](https://www.boost.org/doc/libs/latest/libs/beast/doc/html/index.html)
