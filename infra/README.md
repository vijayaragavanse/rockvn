# Infrastructure

Docker Compose definitions and infrastructure configuration will live here.

The directory is intentionally empty of infrastructure at M0: applying the
[decision tree](../docs/decision-tree.md), no container solves a problem
that exists today — there is no service to give a database to, no event to
give a broker to. The first arrival is PostgreSQL in M3, when the User
Service's persistence makes it necessary; the broker question is decided in
M9; the observability stack in M11.

Standing rules for everything that lands here:

- Configuration through `.env` files (never committed; a committed
  `.env.example` documents every variable).
- No fixed IP addresses — services address each other by compose service
  name; host access via published ports.
- Every infrastructure container gets a health check, resource limits sized
  for 16 GB development machines, and a documented reason for existing.
