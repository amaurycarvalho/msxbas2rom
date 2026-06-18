## Context

The project has accumulated specifications in an old SDD (Spec Driven Development) format under `specs/`. These include user stories, ADRs, glossary, constitution, roadmap, insights, and tasks. The OpenSpec workflow is now the standard for specification management, but these legacy artifacts were never migrated.

All content exists and is well-understood — this is a format migration, not a discovery effort. The mapping is straightforward:
- Each user story → a capability spec
- ADR-001 → architecture spec
- constitution → governance spec
- glossary → glossary spec
- insights → promote stable entries into architecture spec ADRs
- plan.md sprints → OpenSpec changes (completed = archived, pending = active)

## Goals / Non-Goals

**Goals:**
- Migrate all 8 user stories into capability specs under `openspec/specs/`
- Migrate ADR-001, constitution, and glossary into appropriate specs
- Promote stable insights into architecture spec ADR entries
- Create OpenSpec changes for all 7 sprints from plan.md
- Update CHANGELOG.md and README.md to reference OpenSpec
- Remove the old `specs/` directory

**Non-Goals:**
- No source code changes
- No change to specification content — format migration only
- No restructuring of the project beyond the spec directories

## Decisions

### Decision 1: One capability spec per user story
Each user story maps directly to a capability spec. This preserves the established boundaries and makes it easy to trace requirements to implementation.

### Decision 2: Dedicated specs for architecture, governance, glossary
Three cross-cutting concerns get their own specs rather than being embedded into capability specs:
- **Architecture** captures foundations (Clean Architecture, DDD, C++11, testing strategy) and ADRs
- **Governance** captures principles and mandatory guidelines (from constitution.md)
- **Glossary** captures shared terminology

### Decision 3: Sprints become changes (historical archived, pending active)
Each sprint in plan.md becomes an OpenSpec change. Completed sprints (1-3) are created with minimal artifacts and archived. Pending sprints (4-7) are created with full artifacts and kept active for future implementation.

### Decision 4: Insights promoted to ADRs in architecture spec
Stable, recurring insights from insights.md are promoted to ADR entries within the architecture spec. Transient or observational insights are dropped.

### Decision 5: Three-verification safeguard before removing specs/
The old `specs/` directory SHALL NOT be removed until three complete verification rounds have been executed, each checking that no content was lost during migration. After the third round, the maintainer must give explicit formal confirmation before deletion proceeds. This prevents accidental data loss from an automated migration.

## Risks / Trade-offs

- **Content drift during migration**: Manually transcribing user stories into the new format could introduce errors. Mitigation: use the original files as direct source material and verify each spec against its source.
- **Orphaned references**: External docs or tools may reference `specs/` paths. Mitigation: update CHANGELOG and README, and add a brief grace period note before removal.
- **Premature deletion of specs/**: Removing the source before verifying completeness could lose hard-to-recover content. Mitigation: require 3 verification rounds + maintainer confirmation before deletion.
- **Historical change fidelity**: Completed sprints recreated as changes may not capture all original decisions. Mitigation: keep them minimal (proposal + tasks) since they're archival, not actionable.
