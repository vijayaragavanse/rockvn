# M2 Design — Layered Architecture (User Service)

- **Status:** Approved under delegated authority (2026-08-11); design review
  applied at milestone exit
- **Date:** 2026-08-11
- **Depends on:** [M1 server foundation](m1-server-foundation.md),
  [services contract](../../services/README.md)

M2 fills the M1 shell with the layered architecture every service will
copy: a framework-free domain, a repository seam, DTO mapping at the
boundary, and the first business endpoints. The persistence is an in-memory
repository — deliberately, so the layer contract is proven before a
database (M3) complicates the picture.

## Scope

**In:** `domain/` and `repository/` layers; User CRUD over HTTP; DTO
mapping; the error mapper exercised end to end (400/404/409); GoogleMock
tests against the repository interface; mechanical framework confinement.
**Out:** real persistence (M3), pagination (M3, where a data store makes it
meaningful), auth (M4 — all endpoints are open until then, stated plainly).

## The layer contract, made mechanical

Three targets with one-way dependencies, enforced by the linker:

- `user_service_domain` — entities, services, repository *interfaces*,
  domain errors. Links: `tl-expected` only. **No Drogon, no jsoncpp, no
  spdlog** — a framework type reaching domain is now a link error.
- `user_service_core` — config + logging (unchanged from M1).
- `user_service_api` — handlers, DTO mapping, error mapper, wiring. Links
  domain + core + Drogon. The *only* JSON-aware layer.

The M1 `Clock` seam moves to `domain/` (entities need time; the interface
is framework-free by construction). The in-memory repository implementation
lives in `repository/`, as will the PostgreSQL implementation in M3 —
implementations are swappable behind the same interface, which is the whole
point of the seam.

## Domain model

`User`: `id` (UUID string, assigned at creation), `email` (immutable after
creation — identity semantics arrive with auth in M4; changing it before
then would be designing blind), `name` (mutable, 1–100 chars), `created_at`
(UTC). Validation lives in the domain service, not in handlers: email must
be non-empty, ≤ 254 chars, contain exactly one `@` with non-empty sides —
deliberately *not* a full RFC 5322 regex, which is a well-known tar pit;
the honest validation story is "syntactic sanity here, deliverability is
the mail system's problem" and is documented as such.

`UserService` operations, all returning `tl::expected`:
create (validates, generates ID via seam, timestamps, inserts),
get-by-id, list, rename, remove. Domain errors: `Validation` (field,
message), `NotFound`, `Conflict` (duplicate email).

**ID generation is a seam** (`IdGenerator` interface in domain): the
production adapter in `api/` wraps Drogon's UUID utility — reusing the
existing dependency without letting it into domain — and tests inject a
deterministic fake. Same pattern as `Clock`.

## Repository contract

Interface in `domain/`: `insert` (Conflict on duplicate email), `find_by_id`,
`find_all`, `update` (NotFound on missing id), `remove` (NotFound on
missing id). The in-memory implementation (`repository/`) uses a
mutex-guarded map — correct under any `io_threads` setting; a lock-free
design would be optimization without a measurement (M8 exists for that).
A contract test suite runs against the *interface* so M3's PostgreSQL
implementation inherits the same suite unchanged — the suite, not the
mock, is what keeps implementations honest.

## HTTP API

| Route | Success | Errors |
|---|---|---|
| `POST /users` `{email, name}` | `201` + `Location` + body | `400` validation, `409` duplicate email |
| `GET /users/{id}` | `200` | `404` |
| `GET /users` | `200` `{"users":[...]}` | — |
| `PUT /users/{id}` `{name}` | `200` | `400`, `404` |
| `DELETE /users/{id}` | `204` | `404` |

Wire DTOs live in `api/`; the user response DTO is
`{id, email, name, created_at}`. All errors use the M1 problem+json
envelope through one error mapper: `Validation→400`, `NotFound→404`,
`Conflict→409` — the M1 design's table, now fully exercised. Malformed
JSON bodies are a `400` problem+json produced at the boundary; the domain
never sees unparsed input.

## Testing strategy

- **Domain unit tests (GoogleMock):** `UserService` against a mock
  repository — the reason ADR-0004 chose GoogleTest. Interaction checks
  only at the seam; assertions are state-based where possible.
- **Repository contract tests:** one parameterized suite, run against the
  in-memory implementation now, PostgreSQL in M3.
- **Handler tests:** DTO mapping and error mapping, framework request
  objects constructed directly.
- **Integration:** full CRUD lifecycle over real HTTP (create → get →
  list → rename → delete → 404), plus the 400/409 paths and problem+json
  shape assertions.

## Acceptance criteria

- [ ] CI matrix green; ASan/UBSan clean through the full CRUD integration flow.
- [ ] `user_service_domain` links no framework library (visible in its link line).
- [ ] Every route in the table above covered by integration tests, including each error status.
- [ ] Duplicate-email race: two concurrent creates with the same email yield exactly one `201` and one `409` (test with two threads against the real server).
- [ ] Contract test suite passes against the in-memory repository and is written to be reused by M3.
- [ ] Docs: service README API table updated; M2 exit review; interview entry on layered architecture & DI.
