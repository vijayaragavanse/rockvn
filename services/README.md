# Services

One directory per service. None exist yet — the first (User Service) is
built as a walking skeleton across M1–M2, deliberately alone until its
conventions are proven (see
[failed-ideas/0001](../docs/failed-ideas/0001-parallel-service-skeletons.md)).

## The contract every service signs

Each service directory is independently buildable, testable, and
containerizable, and contains:

- `README.md` — purpose, API summary, how to run and test it alone
- HTTP layer (controllers), business logic, and repository layer as
  separate, separately testable layers
- DTOs at the API boundary — wire types never leak into domain logic
- Configuration exclusively from environment variables, validated at startup
- Structured logging with request durations and failure reasons
- Health endpoint from birth; metrics endpoint from M11
- Unit tests beside the code; integration tests marked and separable
- `Dockerfile`

No service may depend on another service at compile time, reach into
another service's database schema, or hardcode any address. Communication
is via published APIs and, from M9, events.

## Canonical layout

Every service uses this layout from its first commit — uniformity is what
makes the second service cheap and the eighth service boring:

```text
services/<name>/
├── README.md            # purpose, API summary, run/test instructions
├── CMakeLists.txt       # independently configurable and buildable
├── Dockerfile
├── src/
│   ├── main.cpp         # composition root: construct, wire, run — no logic
│   ├── api/             # transport layer: routing, handlers, wire DTOs
│   ├── domain/          # business logic; knows nothing of transport or storage
│   ├── repository/      # storage interfaces and their implementations
│   └── config/          # environment loading and startup validation
└── tests/
    ├── unit/            # fast, deterministic, no I/O
    └── integration/     # real boundaries (network, database); separable in CI
```

The dependency rule matching the layout: `api/` and `repository/` may
depend on `domain/`; `domain/` depends on neither. DTOs live in `api/` and
never cross into `domain/` — wire formats change for reasons domain logic
must not feel. Directories gain subdirectories when growth demands it, not
before. The layout is validated by its first user (M1–M2); changing it
afterward is a PR against this contract, applied to every service in the
same change.
