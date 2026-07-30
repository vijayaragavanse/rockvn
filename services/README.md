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
