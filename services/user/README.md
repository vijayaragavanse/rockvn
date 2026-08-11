# User Service

The walking skeleton of the system. In M1 it is a production-shaped process
with no business endpoints: it configures from the environment, logs
structured lines, answers `/health`, and runs in a container as a non-root
user. M2 adds the layered architecture (domain, repository) and the first
real endpoints inside this shell.

Design: [M1 server foundation](../../docs/architecture/m1-server-foundation.md) ·
Framework decision: [ADR-0005](../../docs/adr/0005-http-framework-drogon.md)

## Configuration

The process reads only real environment variables (never `.env` files —
those are for compose and dev scripts). Precedence: environment > defaults.
Invalid configuration exits non-zero before the listener opens, reporting
**every** violation at once.

| Variable | Default | Rule |
|---|---|---|
| `USER_SERVICE_HTTP_HOST` | `0.0.0.0` | non-empty |
| `USER_SERVICE_HTTP_PORT` | `8080` | integer in [1, 65535] |
| `USER_SERVICE_LOG_LEVEL` | `info` | trace \| debug \| info \| warn \| error |
| `USER_SERVICE_LOG_FORMAT` | `text` | `text` or `json` (the container image sets `json`) |
| `USER_SERVICE_IO_THREADS` | `1` | integer in [1, 64] — single-threaded until M8 measures a need |

## API

| Route | Response |
|---|---|
| `GET /health` | `200` — status, service, version+git SHA, uptime, timestamp (liveness; readiness split arrives with the first real dependency) |
| anything else | RFC 9457 `application/problem+json` |

Every response carries `X-Request-Id` (echoed if supplied, generated
otherwise); every request produces exactly one structured log line with
method, path, status, `duration_ms`, and `request_id`.

## Build, test, run

From the repository root (see
[machine setup](../../docs/standards/repository-standards.md)):

```bash
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
```

On Windows: `--preset windows`, then `windows-debug`. Run locally with
`./build/<preset>/services/user/user_service`. Container:

```bash
docker build -f services/user/Dockerfile -t rockvn/user-service .
docker run --rm -p 8080:8080 rockvn/user-service
```

## Structure

Per the [services contract](../README.md): `src/api/` is the only place
framework types exist (enforced by target structure — `user_service_core`
cannot link Drogon); `src/config/` is framework-free configuration and
logging; `main.cpp` is the composition root, wiring only. `domain/` and
`repository/` arrive in M2 with their first content.
