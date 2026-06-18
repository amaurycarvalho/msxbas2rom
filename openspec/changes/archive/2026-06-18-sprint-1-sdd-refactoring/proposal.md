## Why

Refactor the entire project to align with Spec Driven Development (SDD), Test-Driven Development (TDD), Clean Architecture, and SOLID principles. This foundational work enables better maintainability, testability, and long-term project evolution.

## What Changes

- Restructure source code into Clean Architecture layers: `src/cli`, `src/application`, `src/domain`, `src/infrastructure`
- Apply SOLID principles across the codebase (especially Single Responsibility Principle)
- Introduce unit testing infrastructure with TDD
- Establish SDD as the specification methodology
- Create the initial `specs/` directory with user stories, ADRs, and supporting documents

## Capabilities

### New Capabilities
- *(all core capabilities were introduced in this sprint — see openspec/specs/ for current state)*

### Modified Capabilities
- *(none — first sprint, no prior specs)*

## Impact

- Complete project restructuring under `src/`
- New `specs/` directory with SDD content
- Initial makefile and build infrastructure for TDD
- Release 0.3.3.0
