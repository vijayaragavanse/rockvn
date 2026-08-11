# Milestone 2 Exit Review — Layered Architecture (User Service)

- **Date:** 2026-08-11
- **Tag / commit range:** `m2` (`m1..m2`)

## What shipped

The layer contract every later service copies: a framework-free domain
(`User`, `UserService`, repository/clock/ID-generator seams, error
vocabulary), an in-memory repository behind the seam, wire DTOs and the
error mapper in `api/`, full User CRUD over HTTP, and 21 new tests —
mock-based service tests, a reusable typed contract suite for repository
implementations, and integration coverage for every route and error
status including a concurrency race. No new dependencies.

## The three questions, answered for the milestone

1. **Problem solved:** business logic needed a home that cannot rot into
   the framework — enforced by structure, not review vigilance.
2. **Proof:** 44/44 tests green under MSVC `/W4 /WX`. The dependency rule
   is a build fact: `user_service_domain` links only `tl::expected`. The
   duplicate-email race test (two concurrent creates → exactly one 201 and
   one 409) passes against the real server.
3. **Two-minute explanation:** uniqueness lives in the repository, not the
   service — a pre-check is a TOCTOU race; the in-memory implementation
   enforces it under a lock, M3's PostgreSQL via a unique index, and the
   contract suite pins the semantics for both.

## Architecture review

The seam held under its first real test: swapping storage is one line in
the composition root (`main.cpp` says so in a comment M3 will delete).
DTO/domain duplication is deliberate and documented (wire contracts and
entities change for different reasons). Handlers stayed logic-free.

## Code review — including the bug worth remembering

The milestone's real war story: after adding GoogleMock, unit-test
discovery silently found **zero tests** while the build stayed green.
Diagnosis: with vcpkg's dynamic triplet, `gtest_main.dll` and `gmock.dll`
each carry their own copy of the gtest registry — tests registered in one,
`main()` read the other. Confirmed via `dumpbin /dependents` (no
`gtest.dll` import), fixed by linking `GTest::gmock_main` alone, and
recorded as a comment at the link line. Lesson: *silent* test-count
regressions are a CI blind spot — the totals are asserted in this review
precisely because green-with-zero-tests looks identical to green.

Remaining flags: `find_all` materializes and sorts the full set (fine
until pagination, M3); the email validator is syntactic-sanity only, by
documented decision.

## Failure-mode analysis

Malformed/mistyped JSON → 400 at the boundary, domain never sees it
(tested). Concurrent duplicate creates → exactly one winner (tested).
Unknown IDs on get/rename/delete → 404 (tested). Handler exception →
framework 500 safety net, unchanged from M1.

## Security review

Unchanged surface plus: all `/users` endpoints are **unauthenticated until
M4** — stated in the service README rather than discovered by a reviewer.
Input length limits (email ≤ 254, name ≤ 100) bound request-shaped memory
growth; no query language exists yet to inject into.

## Performance notes

No new measurements — M2 added no I/O. The in-memory store means current
latency numbers say nothing about M3; baselines recorded then.

## Scalability review

The mutex-guarded map is a deliberate non-optimization (single service,
`io_threads=1`). The email-uniqueness scan is O(n) — irrelevant at test
scale, deleted in M3 when the unique index takes over.

## Trade-offs accepted

DTO/entity duplication; O(n) uniqueness scan (dies in M3); email
immutability until auth semantics exist; unauthenticated endpoints until
M4.

## Future improvements (documented debt)

| Item | Trigger |
|---|---|
| PostgreSQL behind the same seam | M3 (designed) |
| Pagination for `GET /users` | M3, with a real data store |
| clang-tidy in CI | M2 planned it; carried to M3 with the first Linux-verified CI run after push |

## Interview questions this milestone generated

1. Where should uniqueness be enforced, and why not in the service? →
   [domain/user_repository.h](../../services/user/src/domain/user_repository.h)
2. Contract tests vs mocks — why both? →
   [interview/layered-architecture-di.md](../interview/layered-architecture-di.md)
3. Tell me about a bug where the tooling lied to you. → the gtest
   double-registry story above
4. Why do your DTOs duplicate your entities? →
   [interview/layered-architecture-di.md](../interview/layered-architecture-di.md)
