# Git Workflow

Trunk-based development with short-lived branches, squash merges, and
conventional commits. The workflow is optimized for one engineer on
multiple machines producing a history a reviewer can actually read.

## Branches

- `main` is always green: it builds and passes tests on both platforms.
  Nothing lands on it directly except through a PR.
- Work happens on short-lived branches named `<type>/<milestone>-<topic>`:
  `feat/m1-http-server`, `fix/m3-connection-leak`, `docs/m0-handbook`,
  `ci/m0-cache-key`.
- One exception, recorded here for honesty: M0's very first commit
  (license, hygiene files) landed directly on `main`, because the PR
  machinery this document describes did not exist until that commit did.

## Commits

[Conventional Commits](https://www.conventionalcommits.org): `feat:`,
`fix:`, `docs:`, `test:`, `build:`, `ci:`, `chore:`, `refactor:`, `perf:`.
Subject in imperative mood, ≤ 72 characters, no trailing period. The body
explains *why* when the subject alone cannot.

```text
feat: add health endpoint to user service

Liveness must be observable before the service can join docker-compose
health checks in M3. Returns build metadata alongside status so a
misdeployed binary is identifiable from the endpoint alone.
```

## Pull requests and squash merge

Every change lands via PR using the
[template](../../.github/PULL_REQUEST_TEMPLATE.md) — the PR description is
where the three questions get answered in writing, and that record is part
of the portfolio. PRs are **squash-merged**: one merged commit per justified
change keeps `main` linear and reviewable milestone-by-milestone. The
trade-off — losing intra-branch WIP history — costs little when branches
live days, not weeks. The squash commit title must itself be a valid
conventional commit.

## Milestones and tags

Each completed milestone gets an annotated tag (`m0`, `m1`, …) pointing at
the squash commit that closed it, so any two milestones can be diffed
directly. The milestone's exit review in `docs/reviews/` references the tag.

## Multi-machine discipline

Machines synchronize through GitHub only — never through anything local.

1. **Before leaving a machine:** commit and push, even work-in-progress
   (`git push -u origin <branch>`). Unpushed work is work that does not
   exist.
2. **Before starting on a machine:** `git fetch --all --prune`, then rebase
   your branch on `origin/main` if it moved.
3. Never let two machines hold different unpushed states of the same
   branch; the branch's truth is always `origin`.
