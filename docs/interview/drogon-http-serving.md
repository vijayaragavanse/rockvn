# Drogon & HTTP Serving

## Why it is here

The transport layer of every service — chosen in
[ADR-0005](../adr/0005-http-framework-drogon.md) after a documented
reversal from Boost.Beast on opportunity-cost grounds. The reversal story
itself is interview material: framework-vs-library is a question about
where your project's learning budget should go, not about which is "better".

## Concepts that matter

**Event-loop model.** Drogon runs N event loops (configurable I/O threads);
handlers execute on loop threads and must not block — a blocked handler
stalls every connection on that loop. This service runs single-threaded
until M8 produces a measured reason not to; the blocking-work policy gets
decided when the first blocking dependency (the database, M3) arrives.

**Runtime registration vs static macros.** Drogon's documented path is
`HttpController` with registration macros — static global state. This
codebase prohibits it (ADR-0005 constraint 1): handlers are plain
constructor-injected objects, registered in the composition root via
`registerHandler` with thin lambdas. Cost: no macro conveniences. Benefit:
handlers unit-test without the framework running, and `main.cpp` shows the
entire route table in one place.

**Framework confinement.** Only `api/` and `main.cpp` may include Drogon
headers; the core library target does not link Drogon, so a leak is a
compile/link error. The request-ID and access-log middleware live in `api/`
as pre-routing/post-handling observers — the one legitimate home for
framework hooks.

## Likely questions

**"Why does your service return problem+json?"** RFC 9457 gives error
responses a stable, machine-readable contract — `type`, `title`, `status`,
`detail`, `instance` — instead of ad-hoc `{"error": "..."}` shapes that
drift per endpoint. Fixed in M1 so every later endpoint inherits it.

**"How do you test an HTTP service without mocking the framework?"** Three
layers: the handler's core logic is framework-free (`status()` returns a
DTO — pure unit test with a fake clock); the wiring is one shared
`configure_app` used by both production and tests; integration tests run
the real server on loopback and assert over real HTTP — including that
every request emits one *parseable* log line.

**"What happens when a handler throws?"** The framework's catch-all
converts it to a 500 — treated strictly as a safety net for bugs, never as
the error mechanism; recoverable failures travel as values
([ADR-0007](../adr/0007-tl-expected-error-model.md)).

## Common mistakes

- Blocking calls (database, sleeps) on event-loop threads.
- Letting framework request/response types seep into business logic until
  the framework is unremovable.
- Registering state via static-initialization macros, then wondering why
  tests can't isolate anything.
- Returning HTML-shaped default error pages from a JSON API (this repo
  replaces Drogon's default 404 with problem+json).
