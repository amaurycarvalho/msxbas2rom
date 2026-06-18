## Context

Foundational refactoring of the entire project to adopt SDD, TDD, Clean Architecture, and SOLID principles.

## Goals / Non-Goals

**Goals:**
- Restructure code into Clean Architecture layers
- Establish TDD infrastructure
- Create initial SDD specifications

**Non-Goals:**
- Feature changes — refactoring only

## Decisions

### Decision 1: Four-layer architecture
Code organized as `src/cli`, `src/application`, `src/domain`, `src/infrastructure`.

### Decision 2: SDD as specification methodology
Requirements captured as user stories with BDD scenarios in `specs/`.

## Risks / Trade-offs

- Major restructuring requires careful file mapping to avoid breaking includes.
