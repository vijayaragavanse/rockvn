# 0001 — Scaffold all eight services up front

- **Date:** 2026-07-30 (project kickoff)
- **Status:** Rejected

## Context

The target architecture names eight services. The obvious way to start is to
create all eight directories with matching skeletons — HTTP server, health
endpoint, Dockerfile — and then fill them in.

## The idea

Generate every service skeleton in the first milestone so the repository
immediately looks like the target architecture diagram.

## Why it was rejected

Eight skeletons demonstrate one skill (copy-paste) eight times. The
conventions being replicated would be guesses, since none would have been
proven by a service that actually works end to end — and a wrong guess gets
multiplied by eight and then corrected eight times. Reviewers of portfolio
repositories see this failure mode constantly: wide, uniform, shallow, and
abandoned at the hard parts (persistence, messaging, failure handling).

## What we did instead

A walking skeleton: one service built deep — HTTP, configuration, logging,
tests, container, CI — across M1–M3. Later services inherit conventions that
survived contact with reality, and each new service is added only when it
introduces a new engineering problem (see [docs/vision.md](../vision.md),
non-goals).

## Revisit when

Never as stated. The related-but-different question "should we extract a
service template once three services share structure" becomes legitimate
around M9 and should be argued through the [decision tree](../decision-tree.md).
