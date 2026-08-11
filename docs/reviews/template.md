# Milestone N Exit Review — <name>

- **Date:** YYYY-MM-DD
- **Tag / commit range:** `mN` (`<prev-tag>..mN`)

Every milestone closes with this review, written honestly: known weaknesses
are documented debt, not omissions. A review that flatters the milestone is
a defect in the review.

## What shipped

The deliverables, in a paragraph. Link the PRs.

## The three questions, answered for the milestone

1. **Problem solved:**
2. **Proof it works:** (link tests, measurements, CI runs, demonstrations)
3. **Two-minute explanation:** (the milestone's core design choice, argued)

## Architecture review

Boundaries introduced or changed; how they will bear the next milestones;
anything now harder to change than it was.

## Code review

Honest assessment of the code itself: what is strong, what is weak, what a
sharp reviewer would flag. Include what was *not* built and why.

## Failure-mode analysis

What can break, how we would notice, and the blast radius. Include the
boring ones (misconfiguration, missing environment) — they are the common
ones.

## Security review

Secrets handling, input trust boundaries, supply chain, permissions —
whatever the milestone touched.

## Performance notes

Measurements if any were taken; if none, say so and say why that is
acceptable at this milestone.

## Scalability review

What in this milestone will not survive growth (of load, of code, of
services), and at what point it must be revisited.

## Trade-offs accepted

The costs knowingly taken, in plain language. Mirror of the ADRs'
"negative consequences", aggregated at milestone level.

## Future improvements (documented debt)

Each item: what, why deferred, and the trigger that promotes it to real
work. Undated "someday" items are not allowed — a trigger is required.

## Interview questions this milestone generated

The questions this milestone equips you to answer, with pointers to the
artifacts (files, ADRs, diffs) that back each answer.
