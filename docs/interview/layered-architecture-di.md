# Layered Architecture & Dependency Injection

## Why it is here

M2 established the layer contract every service copies: framework-confined
transport (`api/`), framework-free business logic (`domain/`), swappable
storage (`repository/`), and a composition root that is the only place the
object graph exists. Design record:
[M2 design](../architecture/m2-layered-architecture.md).

## Concepts that matter

**The dependency rule, enforced by the linker.** `api/` and `repository/`
may depend on `domain/`; `domain/` depends on neither — and this is not a
convention but a build fact: the domain library target links only the
error-value type. A Drogon or jsoncpp include in domain is a build error.
"Where would the compiler stop you?" is the difference between architecture
and aspiration.

**Constructor injection without a framework.** Every dependency is a
constructor parameter; `main.cpp` wires the graph in ~15 lines. A DI
container would add reflection-ish magic to solve a problem this scale
doesn't have — the honest answer to "why no DI framework?" is "my
composition root is shorter than the container's configuration would be."

**Interfaces only at seams that earn them.** Four exist: repository
(storage will change in M3), clock (tests need controlled time), ID
generator (tests need determinism), logger sink (tests capture output).
Handlers and services are concrete — an interface with one implementation
and no test double is indirection, not abstraction.

**Where invariants live.** Validation sits in the domain service (input
shape) but *uniqueness* sits in the repository: checking "email exists?"
before inserting is a TOCTOU race. The in-memory implementation enforces
it under a lock; PostgreSQL will via a unique index; the domain just
propagates `Conflict`. Putting the invariant where it can actually be
enforced atomically is the interview-grade point.

**Contract tests over mock-only tests.** The repository *interface* has a
typed contract suite; every implementation must pass it unchanged. Mocks
verify the service uses the seam correctly; the contract suite verifies
implementations honor the seam's semantics. Only having mocks lets an
implementation drift silently.

## Likely questions

**"Walk me through a request."** Middleware stamps a request ID and start
time → handler parses and shape-validates the wire DTO → domain service
applies business rules and returns `expected<User, Error>` → repository
enforces storage invariants → handler maps the result (or the error, via
one mapping table) to JSON/problem+json → middleware logs one structured
line with status and duration.

**"Why do DTOs and domain entities both exist? They look the same."**
Today they do; they change for different reasons. The wire format is a
public contract (versioned, backward-compatible); the entity is internal
(refactorable freely). Merging them couples every internal refactor to an
API break. The duplication is the cheap side of that trade.

**"How would you swap the storage?"** Implement the repository interface,
pass the contract suite, change one line in the composition root. That
claim is tested, not asserted — M3 does exactly this with PostgreSQL.

## Common mistakes

- Business rules leaking into handlers ("just one if") until the domain is
  a hollow shell.
- Interfaces for everything — service interfaces with one implementation,
  mocked in tests that then only test the mock.
- Uniqueness checked in application code with a read-then-write race.
- Framework types in domain signatures, making the framework unremovable
  and the domain untestable without it.
