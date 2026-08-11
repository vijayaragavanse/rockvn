# Milestone 1 Exit Review — Server Foundation (User Service)

- **Date:** 2026-08-11
- **Tag / commit range:** `m1` (`m0..m1`)

## What shipped

The first service process, production-shaped and empty of business logic:
Drogon wired at a composition root under ADR-0005's constraints, immutable
env-only configuration with aggregate-error fail-fast, structured JSON
logging with request IDs, `/health` with embedded build identity,
problem+json as the error wire contract, a non-root multi-stage Dockerfile,
and 20 new tests (15 unit, 5 integration over real HTTP). Dependencies
added: drogon, spdlog, tl-expected (ADR-0006, ADR-0007).

## The three questions, answered for the milestone

1. **Problem solved:** every later milestone needs a service process with
   configuration, logging, and health semantics already decided; deciding
   them per-service later means deciding them eight times, inconsistently.
2. **Proof:** 23/23 tests green locally (MSVC, `/W4 /WX` clean on first
   build). Verified live: cold start to first healthy response **154 ms**
   (debug); `/health` returns the documented contract with the real git
   SHA; fail-fast exits 1 reporting *both* of two injected config
   violations; the response's `X-Request-Id` matches the logged line's
   `request_id`, and the log line parses as JSON (asserted by test, not
   eyeballed).
3. **Two-minute explanation:** the milestone's core decision is what the
   framework is *not* allowed to touch — registration macros, storage, and
   everything outside `api/`; the target structure makes violations
   link errors rather than review comments.

## Architecture review

The layer boundaries exist and are load-bearing: `user_service_core`
(config, logging) cannot link Drogon, and M2's domain target extends the
same mechanism. The composition root proved out — Drogon's runtime
registration API supports constructor-injected handlers cleanly, so
ADR-0005's reversal condition did **not** trigger.

## Code review

Honest flags a reviewer would raise: the integration fixture manages
wiring lifetime manually (raw `new`/static members) — contained to test
scaffolding, tidied in M2's shared fixture; `flush_on(trace)` per log
record is an observability-over-throughput trade, revisited with M8
numbers; text-format log rendering is minimal (values JSON-encoded inline)
— adequate for dev, not a contract.

## Failure-mode analysis

- Invalid config → exit 1 before the listener opens, all violations named
  (tested). Missing `VCPKG_ROOT` → configure fails per M0 troubleshooting.
- Handler exception → Drogon's catch-all 500; safety net only, and M2's
  error mapper is the real path.
- Port already bound → Drogon logs and the process exits nonzero; health
  checks catch the dead container.
- Log stream interleaving under concurrency → synchronized sink; the
  parse-the-line test would catch corruption.

## Security review

No secrets exist; config is env-only. Container runs UID 10001 with a
minimal runtime image. problem+json 500s withhold internals from clients
(details go to logs). Dependency supply chain unchanged: everything enters
via the pinned baseline.

## Performance notes (baselines for M8)

Measured on the Windows dev machine, Debug unless noted: cold start to
first healthy `/health` **154 ms**; request handling `duration_ms` ≈ 0.2
for `/health`; Release binary **0.1 MB** + **9.5 MB** runtime DLLs
(Windows dynamic triplet; the Linux container uses vcpkg's static triplet
and carries no DLL tree). First full dependency compile ≈ 35 min
(OpenSSL-dominated), warm configure 17 s — the binary cache is what makes
multi-machine work viable.

## Scalability review

`io_threads` defaults to 1 by design until M8 measures; the request path
holds no shared mutable state, so raising it is a config change, not a
refactor. The known cliff: blocking work must never reach the event loop —
M3's design already handles this with a worker queue before the first
blocking dependency (PostgreSQL) arrives.

## Trade-offs accepted

Framework catch-all as 500 safety net; per-record log flushing; Windows
container criteria verified in CI only (no local Docker on this machine);
integration tests share one process-global server (Drogon singleton —
accepted, worked around with a shared fixture from M2).

## Future improvements (documented debt)

| Item | Trigger |
|---|---|
| Async log sink / relaxed flushing | M8 measurement showing log cost |
| Readiness vs liveness split | First real dependency (M3) |
| Graceful-shutdown hardening (drain, timeout) | M14 |
| clang-tidy into CI | M2 (as planned in M0) |

## Interview questions this milestone generated

1. Why values-not-exceptions for expected failures in an async framework? →
   [ADR-0007](../adr/0007-tl-expected-error-model.md)
2. Walk through your request-ID design — why echo *and* generate? →
   [interview/structured-logging.md](../interview/structured-logging.md)
3. Why does your config loader report all violations at once? →
   [m1-server-foundation.md](../architecture/m1-server-foundation.md)
4. How is the framework kept out of your business logic — mechanically? →
   [interview/drogon-http-serving.md](../interview/drogon-http-serving.md)
5. What does your `/health` prove and deliberately not prove? →
   [services/user/README.md](../../services/user/README.md)
