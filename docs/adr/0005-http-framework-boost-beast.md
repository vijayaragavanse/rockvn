# ADR-0005: HTTP framework — Boost.Beast

- **Status:** Proposed (decision point: M1 design review)
- **Date:** 2026-07-30
- **Milestone:** M1

## Context

M1 builds the first HTTP service, and every later service inherits its
transport layer, so this is the highest-leverage dependency decision in the
project. The choice must serve conflicting masters: development velocity
(a framework maximizes it), learning depth (a framework minimizes it — and
[ADR-0001](0001-cpp20-for-a-microservices-backend.md) chose C++ precisely
to keep the machinery visible), and our engineering principles (no global
state, constructor injection, layers we design ourselves).

Hard requirements: first-class Windows *and* Linux, MSVC and GCC, vcpkg
availability, a path to WebSocket (Notification service, M13), and no
coupling between transport and persistence (our repository layer is decided
in M3 on its own merits).

## Decision

Boost.Beast: an HTTP/WebSocket *protocol library* on Boost.Asio, not a
framework. We write the thin server layer ourselves — accept loop, session
lifetime, routing, graceful shutdown — once, in M1, and every service
reuses it. Facts verified 2026-07-30; sources at the end of this document.

**Reversal condition, stated up front:** if the M1 exit review finds the
hand-built server layer exceeded its milestone budget or cannot meet the
quality bar (sanitizer-clean, tested shutdown), this ADR is reopened with
Drogon as the named fallback. The layered service layout confines the blast
radius of such a switch to the `api/` layer.

## Comparison

| Criterion | Boost.Beast | Drogon | Crow | Pistache | oat++ |
|---|---|---|---|---|---|
| Ecosystem | Boost; vcpkg first-class | Own stack (trantor, jsoncpp…) | Header-only + standalone ASIO | Meson, Linux-first | Zero-dep, self-contained world |
| Performance | Excellent primitive; end perf is our code's | Top-tier (TechEmpower regular) | Very good | Good (Linux) | Good |
| Learning value | **Highest** — async model, buffers, lifetimes all ours | Low — framework absorbs the machinery | Medium — sugar over ASIO | Medium | Low — proprietary idioms |
| Maintainability | Boost release train, stable API | Active (1.9.12, Jan 2026) | Community fork, sporadic releases | Moderate | 1.4.0 pending for years |
| Windows + Linux | First-class, both | Supported, both | Both | Windows recent, unproven; docs contradict | Both |
| PostgreSQL | None — by design, ours in M3 | Built-in async ORM (coupling risk for us) | None | None | Own ORM module |
| WebSocket | First-class | Yes | Yes | No — long-standing gap | Yes |
| Testing story | We own the seams; protocol objects test in isolation | `drogon::test`; global controller registration hurts isolation | Handlers invocable directly | Adequate | Built-in framework, own idioms |
| Documentation | Extensive, dense; strong conference material | Good | Good, tutorial-flavored | Thin, partly stale | Decent |
| Community | Boost-scale | Large, active | Moderate | Small | Small; bus-factor risk |
| Portfolio suitability | **Highest** — demonstrates transport-layer depth | "Used a framework well" | "Flask in C++" | Weak | Niche |

## Alternatives considered

**Drogon — the serious rival, and the right answer under different goals.**
Actively maintained, genuinely fast, batteries included: controllers,
middleware, WebSocket, and an async ORM. If the goal were shipping product
features fastest, Drogon wins and this ADR would say so. It loses on this
project's axes: it re-hides exactly the machinery ADR-0001 chose C++ to
expose; its macro-based controller registration is global state of the kind
our [engineering principles](../engineering-principles.md) prohibit, fighting
constructor injection instead of enabling it; and its bundled ORM creates
standing pressure to couple transport to persistence before M3 makes that
decision properly. Named fallback if the reversal condition triggers.

**Crow — the tempting middle path.** Flask-style routing over ASIO,
header-only, pleasant. Rejected because the middle path buys the wrong
mixture: it still owns the event loop and routing (so the deep learning is
skipped) while providing far less than Drogon does (so velocity isn't
maximized either). Community-fork maintenance with sporadic releases is
workable but not a strength to build eight services on.

**Pistache — knocked out on requirements.** Linux-native REST toolkit whose
Windows support is recent and visibly immature — official docs
simultaneously claim Windows support and recommend WSL. Betting the
transport layer of every service on the newest port of a mid-sized project
fails basic risk management; no WebSocket path compounds it.

**oat++ — knocked out on sustainability and fit.** Zero-dependency and
capable, but version 1.4.0 has been in the pipe for years, activity
concentrates in one organization (bus factor), and its DTO-codegen macros
and self-contained idioms would replace our layered design with the
framework's — the opposite of demonstrating our own architecture.

**Not shortlisted:** cpp-httplib — admirable simplicity, but a blocking
model caps concurrency below what a gateway-fronted service stack needs;
ruled out before detailed comparison.

## Consequences

**Positive:** The transport layer becomes portfolio material instead of a
dependency line: accept loop, session lifetime, routing, and graceful
shutdown are ours to design, test, and explain. Asio competence transfers
directly to M5 (networking fundamentals) and to systems interviews.
WebSocket needs no new dependency in M13. Boost enters the project once,
on its own merits — exactly the standard
[ADR-0004](0004-googletest.md) set when it refused to let Boost ride in
with the test framework.

**Negative / accepted costs:** M1 is slower — realistically several hundred
lines of server scaffolding before the first endpoint, and async lifetime
management is the classic C++ failure mode; mitigated by sanitizers in
every Linux debug build, warnings-as-errors, deliberately minimal M1 scope,
and the stated reversal condition. Boost is a heavyweight dependency to
compile (mitigated by vcpkg binary caching). We forgo framework
conveniences (content negotiation, sessions) and must resist rebuilding
them speculatively — the decision tree applies to each.

## Interview framing

"I evaluated five options and the honest fight was Drogon versus Beast —
Drogon is what I'd pick to ship a product fast. I picked Beast because my
project's stated goal is demonstrating transport-layer engineering: with
Beast, the accept loop, session lifetimes, routing, and graceful shutdown
are my code — I can whiteboard them, I tested them, and my sanitizer runs
cover them. I capped the risk two ways: the HTTP layer is isolated behind
my service layout so a framework swap stays contained, and I wrote the
reversal condition into the ADR before writing any code."

## Sources

- [Drogon releases](https://github.com/drogonframework/drogon/releases) — 1.9.12, 2026-01-26
- [Pistache README](https://github.com/pistacheio/pistache) vs [Pistache docs](https://pistacheio.github.io/pistache/docs/) — contradictory Windows claims
- [oat++ 1.4.0 changelog (unreleased)](https://github.com/oatpp/oatpp/blob/master/changelog/1.4.0.md)
- [CrowCpp](https://crowcpp.org/) — community fork, 1.2.x/1.3.0
- [Boost.Beast](https://www.boost.org/doc/libs/latest/libs/beast/doc/html/index.html)
