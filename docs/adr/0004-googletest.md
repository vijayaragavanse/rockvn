# ADR-0004: GoogleTest as the testing framework

- **Status:** Accepted
- **Date:** 2026-07-30
- **Milestone:** M0

## Context

M0's proof-of-work is an executable test, so the framework decision cannot
wait. Looking one milestone ahead sharpens the requirements: from M2 onward,
the layered architecture tests service logic against repository *interfaces*,
which requires a mocking story, not just assertions.

## Decision

GoogleTest (with GoogleMock), resolved through vcpkg, integrated with CTest
via `gtest_discover_tests` so IDEs, the command line, and CI all see the
same test list.

## Alternatives considered

**Catch2 v3** — nicer assertion syntax (`REQUIRE(x == y)` with expression
decomposition), BDD-style sections, less macro noise. Rejected on two
grounds: no mocking framework, so M2 would force either a second dependency
(trompeloeil) or hand-rolled fakes everywhere; and weaker recognition value
— GoogleTest is the convention in the industry segments this portfolio
targets.

**doctest** — the fastest-compiling option by far, ideal for TDD loops.
Same mocking gap as Catch2, smaller ecosystem; compile speed is not yet the
bottleneck.

**Boost.Test** — capable but dated ergonomics, and it would pull Boost into
the dependency set before anything else needs Boost. A dependency that large
must be admitted for its own merits, not ride in with the test framework.

## Consequences

**Positive:** Assertions and mocking from one coherent dependency. CTest
integration gives uniform `ctest --preset <name>` on every machine and in
CI. Familiar to essentially every C++ interviewer.

**Negative / accepted costs:** Macro-heavy API and slower test compiles
than doctest. GoogleMock encourages interaction-style tests, which can
overcouple tests to implementations — reviews will push toward
state-based assertions where possible, using mocks at genuine seams only.

## Interview framing

"I picked the test framework by looking one milestone ahead: my layered
architecture tests services against repository interfaces, so I needed
mocking as a first-class citizen — that eliminated Catch2 and doctest,
whose ergonomics I otherwise like. GoogleTest plus CTest also gives me one
uniform test-running story across two OSes, IDEs, and CI, and it's the
framework any C++ interviewer already knows."
