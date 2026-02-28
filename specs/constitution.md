# Constitution

This document defines the immutable principles of the system.

---

## General Principles

1. The role of this file is to describe common mistakes and confusion points that agents might encounter as they work in this project;
2. If you ever encounter something in the project that surprises you, please alert the developer working with you and propose a change in this file to help prevent future agents from having the same issue.

These principles are not optional.

## Mandatory Technical Guidelines

- Consult the `specs/adr` folder for current architectural decisions.

## Project Requirements

- Consult the `specs/user-stories` folder for project main requirements.

## Project Roadmap

- Consult the `specs/plan.md` for project main roadmap and `specs/tasks.md` for the current sprint backlog.

## Insights

- Consult and maintain the `specs/insights.md` using it as a memory bank of your discoveries about this project.

## Legacy Test Assets

- `tests/deprecated/` is an archival and manual-only test asset area.
- Its content is out of CI/TDD acceptance flow and has no mandatory coverage target.
- Do not add new automated test cases there; prefer `tests/unit/` or `tests/integration/`.
- Legacy scenarios from `tests/deprecated/` should be migrated to active suites when still relevant.
