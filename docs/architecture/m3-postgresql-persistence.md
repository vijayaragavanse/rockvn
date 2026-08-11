# M3 Design — PostgreSQL Persistence (User Service)

- **Status:** Designed 2026-08-11; implementation follows M2 completion
- **Depends on:** [M2 layered architecture](m2-layered-architecture.md)

M3 swaps the in-memory repository for PostgreSQL behind the unchanged
domain seam — the layered architecture's claim ("storage is swappable"),
paid in full. It also brings the first infrastructure container and the
compose story.

## Scope

**In:** PostgreSQL 16 via `infra/docker-compose.yml`; a libpqxx repository
implementation passing the M2 contract suite unchanged; schema migrations;
blocking-work policy for Drogon's event loops; storage selection by
configuration; CI integration tests against a real PostgreSQL.
**Out:** pagination (arrives here or is explicitly re-deferred at exit
review), connection-pool tuning (M8 measures first), replication/HA
(documented as non-goals at this scale).

## Decisions

**Client library: libpqxx (ADR-0008 at implementation).** The official C++
client, active, vcpkg-first-class. Alternatives at the ADR: raw libpq (C
API, manual resource management — what RAII exists to prevent), Drogon's
ORM (banned by ADR-0005 constraint 2), SOCI/sqlpp11 (abstraction layers
whose value appears with multi-database targets this project doesn't have).

**Blocking work leaves the event loop.** libpqxx is synchronous; a blocked
IO loop stalls every connection on it. The repository stays synchronous
(the domain seam is unchanged and unit-testable); `api/` posts
repository-touching work to a small worker queue and completes the
response via Drogon's callback from the worker thread. This preserves the
layer contract: concurrency is a transport-layer concern, invisible to
domain.

**Connections: a bounded pool, built.** libpqxx connections are not
thread-safe and per-request connections pay TCP+auth every call. No
mature standalone C++ pool exists outside frameworks, so a bounded
blocking pool (~100 lines, RAII handles) is a justified build — the
adopt-over-build bar is met by the absence of anything to adopt. Pool
size defaults to the worker-queue size (no point holding more).

**Migrations: numbered SQL, applied by the service at startup.** Files in
`services/user/migrations/NNNN_name.sql`; a `schema_migrations` table
records applied versions; a PostgreSQL advisory lock serializes competing
starters. Adopting Flyway (JVM) or golang-migrate (Go toolchain) for one
service's migrations fails the dependency gate on footprint; the runner is
~80 lines against libpqxx. Revisit trigger: the third service with its own
schema.

**Storage selection by config:** `USER_SERVICE_STORAGE=memory|postgres`
(default `memory` — clone-and-run stays dependency-free);
`USER_SERVICE_DATABASE_URL` (libpqxx connection string, required and
validated when storage is postgres — never a hardcoded host, per standing
constraints); `USER_SERVICE_DB_POOL_SIZE` (default 4, [1, 32]). Compose
sets postgres.

## Schema (0001_create_users.sql)

`users`: `id text primary key`, `email text not null unique`, `name text
not null`, `created_at timestamptz not null`. The unique index *is* the
Conflict semantics the contract suite demands — the invariant moves from a
mutex to the database, which is the point of the milestone. Index notes
(why the unique index also serves lookup, when a `created_at` index would
be added) go to the handbook's PostgreSQL chapter.

## Compose (`infra/docker-compose.yml`)

`postgres:16-alpine`, healthcheck (`pg_isready`), named volume, resource
limits sized for 16 GB machines, credentials from `.env` (committed
`.env.example` documents every variable; real `.env` never committed).
The user service joins compose here too — first end-to-end containerized
vertical slice.

## Testing

- The M2 contract suite runs against `PgUserRepository` — zero new
  semantics tests, which is the proof the seam held.
- Gating: suites read `USER_SERVICE_TEST_DATABASE_URL`; unset → skipped
  with a visible skip message (never silently green). Linux CI provides
  PostgreSQL via GitHub Actions `services:`; Windows CI runs unit tests
  only (documented, not hidden).
- Migration runner tests: fresh database → all applied; re-run → no-op;
  concurrent starters → serialized by the advisory lock.

## Acceptance criteria

- [ ] Contract suite green against PostgreSQL in CI (Linux) and locally via compose.
- [ ] Full CRUD integration flow green with `USER_SERVICE_STORAGE=postgres`.
- [ ] Duplicate-email concurrency test green against PostgreSQL (unique index, not mutex).
- [ ] Migration idempotence and advisory-lock tests green.
- [ ] `docker compose up` from a clean clone + `.env` yields a healthy service answering `/health` and CRUD.
- [ ] Docs: ADR-0008, compose README, PostgreSQL handbook/interview entries, M3 exit review with baselines (request latency vs in-memory, recorded for M8).
