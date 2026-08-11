# 0002 — Adopt Kafka at project start

- **Date:** 2026-07-30 (project kickoff)
- **Status:** Rejected (deferred with conditions)

## Context

The original project outline listed Kafka in the infrastructure stack from
the beginning, alongside PostgreSQL and Redis.

## The idea

Stand up Kafka in Docker Compose during the bootstrap milestone so
event-driven patterns are available whenever services want them.

## Why it was rejected

It fails the first gate of the [decision tree](../decision-tree.md): no
service exists, so nothing produces or consumes events — the problem does
not exist today. Infrastructure without a consumer is pure carrying cost:
compose complexity, RAM pressure on 16 GB development machines, and a broker
choice made before the requirements (ordering, retention, replay, fan-out)
are known. Choosing the broker before the problem also silently forecloses
alternatives — RabbitMQ's routing model or Redpanda's operational footprint
might fit the actual requirements better.

## What we did instead

Messaging is introduced in M9, when Order and Inventory create a real
producer and consumer. The broker decision gets a full ADR at that point,
and Redpanda (Kafka-API-compatible, single binary, no JVM) must appear in
the comparison because development-machine footprint is a real constraint
here.

## Revisit when

M9 begins. The ADR written then supersedes this entry's deferral.
