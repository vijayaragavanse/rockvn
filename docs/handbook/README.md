# Engineering Handbook

The handbook a newcomer could onboard from. Chapters are written when the
milestone that earns them lands — a chapter written before the experience
behind it would be a tutorial summary, which this repository does not do.
Status below is honest: *written* links to real content, *planned* names the
milestone that will produce it.

Interview-focused treatments of each technology live separately in
[docs/interview/](../interview/README.md); the handbook explains how things
work here, the interview notes explain how to talk about them.

## Chapters

| Chapter | Status |
|---|---|
| Git workflow | Written — [standards/git-workflow.md](../standards/git-workflow.md) |
| Repository & machine setup | Written — [standards/repository-standards.md](../standards/repository-standards.md) |
| Coding guidelines | Written — [standards/coding-guidelines.md](../standards/coding-guidelines.md) |
| Decision making & ADRs | Written — [decision-tree.md](../decision-tree.md), [adr/](../adr/template.md) |
| Build system & dependencies | Written — [interview/cmake.md](../interview/cmake.md), [interview/vcpkg.md](../interview/vcpkg.md) |
| HTTP server fundamentals | Written — [M1 design](../architecture/m1-server-foundation.md), [interview/drogon-http-serving.md](../interview/drogon-http-serving.md), [interview/structured-logging.md](../interview/structured-logging.md) |
| Layered architecture & DI | Planned — M2 |
| Docker | Planned — M1/M3 |
| PostgreSQL: schema, indexing, transactions | Planned — M3 |
| Authentication: JWT, OAuth concepts | Planned — M4 |
| Networking: TCP, sockets, HTTP on the wire | Planned — M5 |
| REST vs gRPC, serialization | Planned — M6 |
| Reverse proxies, gateways, rate limiting | Planned — M7 |
| Performance measurement & load testing | Planned — M8 |
| Message queues & event-driven design | Planned — M9 |
| Caching strategies | Planned — M10 |
| Monitoring & distributed logging | Planned — M11 |
| Resilience: retries, circuit breakers, CAP | Planned — M12 |
| Production readiness & deployment | Planned — M14 |
| Debugging & incident write-ups | Ongoing — entries added as incidents happen |
