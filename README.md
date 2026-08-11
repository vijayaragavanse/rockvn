# rockvn

A production-inspired microservices backend, built from scratch in C++20.

This is a long-running engineering project, not a tutorial. Every technology
in it was introduced to solve a problem that existed at the moment it was
added; every architectural decision is recorded — including the
[rejected ones](docs/failed-ideas/README.md) — and every milestone closes
with a written [design review](docs/reviews/template.md). The deliverable is
demonstrated engineering judgment, with the repository as the evidence.

## The three questions

Nothing enters this repository without answering:

1. **What problem are we solving?**
2. **How can we prove the solution works** — measurement, tests, or demonstration?
3. **Could we explain this design choice to a senior interviewer in under two minutes?**

The rule is enforced by process, not memory: the
[PR template](.github/PULL_REQUEST_TEMPLATE.md),
[ADR template](docs/adr/template.md), and
[milestone review template](docs/reviews/template.md) all embed it.

## Status

**M0 complete** — toolchain, standards, CI, and the decision-making
framework are in place. No services exist yet; that is deliberate
([why](docs/failed-ideas/0001-parallel-service-skeletons.md)). The
[target architecture](docs/architecture/system-overview.md) shows the
destination; the roadmap below shows how it gets earned.

## Roadmap

| # | Milestone | Status |
|---|---|---|
| M0 | Repository, standards, toolchain, CI | ✅ Complete |
| M1 | Server foundation — first HTTP service process | ⏭ Next |
| M2 | Layered architecture inside the service | Planned |
| M3 | Persistence with PostgreSQL | Planned |
| M4 | Authentication service and JWT | Planned |
| M5 | Networking fundamentals (TCP, sockets, HTTP on the wire) | Planned |
| M6 | Product service and inter-service communication (REST vs gRPC) | Planned |
| M7 | API gateway | Planned |
| M8 | Performance baselines and load testing | Planned |
| M9 | Asynchronous messaging — Order and Inventory | Planned |
| M10 | Redis caching, justified by M8 baselines | Planned |
| M11 | Observability — Prometheus and Grafana | Planned |
| M12 | Resilience — retries, circuit breakers, failure testing | Planned |
| M13 | Notification service (+ Audit if justified) | Planned |
| M14 | Production readiness | Planned |

## Building

Prerequisites and one-time machine setup:
[docs/standards/repository-standards.md](docs/standards/repository-standards.md).
Short version — CMake ≥ 3.27, a vcpkg clone with `VCPKG_ROOT` set, and
VS 2022 (Windows) or GCC ≥ 13 (Linux). Then:

```bash
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
```

On Windows use `--preset windows`, then `windows-debug` for build and test.
The `ci-*` presets reproduce exactly what GitHub Actions runs.

## Repository map

| Path | Contents |
|---|---|
| [docs/philosophy.md](docs/philosophy.md) | How this project makes decisions |
| [docs/engineering-principles.md](docs/engineering-principles.md) | The rules the code follows |
| [docs/decision-tree.md](docs/decision-tree.md) | The gate every addition passes |
| [docs/vision.md](docs/vision.md) | Long-term goals and non-goals |
| [docs/adr/](docs/adr/template.md) | Architecture decision records |
| [docs/failed-ideas/](docs/failed-ideas/README.md) | Rejected decisions, kept on purpose |
| [docs/architecture/](docs/architecture/system-overview.md) | Target architecture and how it gets earned |
| [docs/handbook/](docs/handbook/README.md) | Engineering handbook (grows per milestone) |
| [docs/interview/](docs/interview/README.md) | Interview knowledge base per technology |
| [docs/reviews/](docs/reviews/template.md) | Milestone exit reviews |
| [docs/standards/](docs/standards/repository-standards.md) | Coding, git, and repository standards |
| [services/](services/README.md) | The services (first arrives in M1) |
| [infra/](infra/README.md) | Docker Compose and infrastructure (first arrives in M3) |

## License

[MIT](LICENSE)
