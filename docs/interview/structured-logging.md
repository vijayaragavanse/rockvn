# Structured Logging & Request IDs

## Why it is here

A service that cannot explain itself at runtime is undebuggable in
production. From M1, every request produces exactly one machine-parseable
JSON line and every response carries a request ID — the observability
foundation the later stack (M11) consumes without re-plumbing. Decision
record: [ADR-0006](../adr/0006-spdlog-structured-logging.md).

## Concepts that matter

**Structured vs formatted logs.** `"GET /health took 0.4ms"` needs a regex
per message shape; `{"event":"http_request","path":"/health",
"duration_ms":0.4}` is a queryable record. The rule with teeth: fields are
composed by a JSON library (escaping guaranteed), and tests *parse* the
line rather than grepping it — parseability is asserted, not hoped.

**stdout as the only sink (12-factor).** The process writes lines; the
runtime (docker, compose, later an aggregator) owns routing, rotation, and
shipping. File sinks and in-process rotation are complexity the platform
already provides.

**Request IDs / correlation.** Incoming `X-Request-Id` is honored (a
gateway or client can trace across hops), generated otherwise, echoed on
the response, stamped on every request-scoped line. This is distributed
tracing's minimum viable seed — when real tracing arrives, the header
discipline is already in place.

**Log at boundaries, not in depths.** The access-log middleware and
composition root log; domain code returns rich errors instead of logging.
This kills double-logging (the same failure logged at four layers) and
keeps domain code pure.

## Likely questions

**"What fields does every log line carry, and why those?"** `ts` (ISO-8601
UTC — sortable, timezone-unambiguous), `level`, `service` (multi-service
aggregation needs the source), `event` (machine-readable kind), then
event-specific fields. Request lines add method, path, status,
`duration_ms`, `request_id` — enough to answer "what was slow and for
whom" without more instrumentation.

**"Why not log inside business logic?"** Because the boundary already
logs the outcome; inner logging duplicates it with less context. Domain
code returns errors carrying what the boundary needs to log once, well.

**"Text or JSON?"** Both, config-selected: humans get readable text in
local dev; containers set JSON. The *content* is identical — format is
presentation, decided per environment.

## Common mistakes

- Printf-ing user-controlled strings into "JSON" logs until one embedded
  quote breaks the pipeline (solved structurally here: jsoncpp composes
  every line).
- Logging the same error at every layer it passes through.
- Global logger singletons that make log output untestable (here the
  logger is constructor-injected; tests attach a capture sink).
- Leaving request correlation "for later" — retrofitting IDs across
  services costs 100× what carrying the header from day one does.
