# Failed Ideas

Rejected architectural decisions, kept on purpose.

A rejected idea with recorded reasoning is some of the strongest material in
this repository: it prevents relitigating settled questions months later,
and "we considered X and rejected it because Y" is precisely the kind of
answer senior interviews probe for. Deleting this history would delete the
judgment it demonstrates.

An idea earns an entry here when it was seriously considered and rejected
for reasons worth remembering — not for every passing thought. Rejections
that are fully explained where the winning design is documented (an ADR's
alternatives section, an engineering principle) do not need a duplicate
entry.

## Entry format

Each entry answers five things: **Context** (when and why it came up),
**The idea**, **Why it was rejected**, **What we did instead**, and
**Revisit when** — the concrete condition that would reopen the question.
An empty "revisit when" is allowed and means "probably never".

## Index

| Entry | Idea | Rejected because |
|---|---|---|
| [0001](0001-parallel-service-skeletons.md) | Scaffold all eight services up front | Breadth over depth produces uniform shallow scaffolds |
| [0002](0002-kafka-from-day-one.md) | Adopt Kafka at project start | No event producer exists; violates the decision tree's first gate |
