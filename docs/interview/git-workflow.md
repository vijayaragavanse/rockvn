# Git Workflow

## Why it is here

The workflow ([full standard](../standards/git-workflow.md)) is optimized
for a specific reality: one engineer, several machines, GitHub as the only
synchronization channel, and a history that doubles as portfolio evidence.

## Concepts that matter

**Trunk-based development.** Short-lived branches merged into an
always-green `main`. The alternative, GitFlow, adds `develop`/`release`
branch machinery that solves release-train coordination for teams shipping
versioned artifacts — pure overhead for continuous solo development. Naming
the problem GitFlow solves, then showing you don't have it, is the
interview-grade justification.

**Squash merging.** One merged commit per justified change; `main` reads as
a sequence of decisions, each tied to a PR that answered the three
questions. Cost: intra-branch history is discarded. That cost is real for
long-lived collaborative branches and negligible for day-scale solo
branches — trade-off, weighed, chosen.

**Conventional commits.** `type: subject` isn't bureaucracy; it makes
history greppable (`git log --oneline --grep '^feat'`) and machine-readable
for changelog tooling later, at the cost of five characters of discipline.

**Multi-machine hygiene.** The rule that prevents real pain: a branch's
truth is always `origin`. Push before leaving a machine, fetch before
starting on one, never hold divergent unpushed state on two machines —
recovering from that is manual archaeology.

## Likely questions

**"Squash, merge commit, or rebase-merge — and why?"** There is no
universally right answer; there is a right answer per context. Merge
commits preserve full topology (valuable for large teams doing archaeology),
rebase-merge keeps every commit but demands each be atomic and green,
squash trades granularity for a linear, review-sized history. For a solo
portfolio repo, squash wins: the unit of review is the PR, so the unit of
history should be too.

**"How do you keep a feature branch fresh?"** Rebase onto `origin/main`
rather than back-merging — linear history, conflicts resolved once at the
point they arise. Safe here because branches are private to one person;
rewriting shared branches is where rebase becomes a footgun.

**"Why does every solo change still go through a PR?"** The PR is where
the three questions are answered in writing, CI gates the merge, and the
record of *why* survives. Discipline that only exists when someone is
watching isn't discipline.

## Common mistakes

- Long-lived branches drifting from `main` until merging becomes a
  project of its own.
- Commit subjects describing *what changed* mechanically ("update file")
  instead of intent.
- Force-pushing shared branches (fine on private branches, destructive on
  shared ones — knowing the difference is the point).
- Treating `.gitattributes` as optional on cross-platform repos, then
  debugging phantom whole-file diffs from line-ending churn.
