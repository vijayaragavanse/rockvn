# Coding Guidelines

Google C++ Style is the base. Formatting mechanics are enforced by
[.clang-format](../../.clang-format) and CI — style is never argued in
review comments. This document records what clang-format cannot enforce:
naming, deviations from the base style, and the rules behind them.
Higher-level design rules live in
[engineering-principles.md](../engineering-principles.md).

## Deviations from Google style

Each deviation from a well-known base style is a conscious, recorded
decision — that is the point of using a known base.

| Area | Google style | This repository | Why |
|---|---|---|---|
| Line length | 80 columns | 100 columns | Modern C++ (templates, concepts, nested namespaces) hits 80 constantly; 100 removes noise-wrapping while staying reviewable side-by-side |
| Header guards | `#ifndef` guards | `#pragma once` | Supported by every targeted compiler; eliminates guard-name maintenance and copy-paste collision bugs |
| Function naming | `CamelCase()` | `snake_case()` | Server code here composes heavily with the standard library; one calling convention (`std::ranges::sort`, `co_await`, our `sum()`) reads better than two interleaved |
| Exceptions | Banned | Policy deferred to M2 | Google's ban is a constraint of their legacy codebase, not a first-principles rule; ours is decided when the first service layer exists to bind it to |

## Naming

| Entity | Convention | Example |
|---|---|---|
| Types, concepts, enum values | `PascalCase` | `OrderRepository`, `Summable`, `Status::Ready` |
| Functions, methods, variables | `snake_case` | `find_by_id`, `retry_count` |
| Private data members | trailing underscore | `connection_pool_` |
| Compile-time constants | `kPascalCase` | `kMaxRetries` |
| Namespaces | `snake_case`, project root `rockvn` | `rockvn::orders` |
| Files | `snake_case` | `order_repository.cpp` |
| Test suites/cases | `PascalCase` (GoogleTest convention) | `TEST(OrderRepository, RejectsUnknownId)` |

## Rules clang-format cannot enforce

- **Ownership:** `std::unique_ptr` for owning, raw pointer/reference for
  observing, `std::shared_ptr` only with written justification. No `new` /
  `delete` in first-party code.
- **Headers:** every header self-sufficient (compiles alone);
  include-what-you-use; no transitive-include reliance.
- **`auto`** where the type is obvious from the right-hand side or is an
  unutterable type; spelled-out types where the reader would otherwise
  guess.
- **Comments** explain *why*, never *what* — a comment restating the code is
  deleted in review. `TODO` requires a linked issue; an unlinked TODO fails
  review.
- **Warnings** are errors (`rockvn::warnings` target); suppressing a warning
  locally requires a comment naming the reason.
- **Test code is production code:** same style, same review bar, no dead
  test helpers.

## Tooling

- Format: `scripts/format.ps1` / `scripts/format.sh` (`-Check` / `--check`
  to verify). CI's clang-format version is authoritative if versions ever
  disagree.
- Static analysis: [.clang-tidy](../../.clang-tidy) locally and in IDEs
  today; joins CI in M2 alongside the first real architecture
  (rationale in the file header).
