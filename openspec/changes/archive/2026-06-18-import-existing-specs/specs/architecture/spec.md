## ADDED Requirements

**Status:** Accepted
**Context:** General definitions applicable to the entire project.

### Requirement: Adopt SDD, DDD, Clean Architecture with layered separation
The system SHALL adopt Spec Driven Development (SDD), Domain-Driven Design (DDD), Clean Code, SOLID, and Clean Architecture with four layers — CLI, application, domain, infrastructure.

Architecture reference structure:
```
MSXBAS2ROM
├── cli
├── application
│   ├── lexer
│   ├── parser
│   └── compiler
├── domain
└── infrastructure
    └── kernel
```

The deliverable SHALL be a CLI tool.

#### Scenario: Source code follows layered structure
- **WHEN** the source code is inspected
- **THEN** it SHALL be organized as `src/cli`, `src/application`, `src/domain`, and `src/infrastructure`
- **AND** dependencies flow inward (outer layers depend on inner layers)

### Requirement: Implement in C++11 with cross-platform support
The system SHALL be implemented in C++11 and be compatible with Linux, Windows, and macOS.

#### Scenario: Build on all supported platforms
- **WHEN** the project is built on Linux, Windows, or macOS
- **THEN** compilation SHALL succeed without platform-specific errors

### Requirement: Use TDD and BDD for development
The system SHALL use Test-Driven Development with minimum coverage of 100% for medium/high criticality code and 90% for low criticality code. BDD scenarios SHALL be used for user stories.

#### Scenario: Tests cover critical code paths
- **WHEN** unit tests are executed
- **THEN** medium/high criticality code SHALL have 100% coverage
- **AND** low criticality code SHALL have at least 90% coverage

### Requirement: Follow mandatory user story pattern
All user stories MUST contain:
1. User Story in format: "As [actor], I need [functionality] so that [benefit/value]"
2. Explicit Acceptance Criteria
3. Behavioral scenarios in BDD (Given / When / Then)
4. Technical specification (if needed)
5. References (if needed)

Template:
```markdown
# US-[nnn] — [summary]

## Story

As [actor], I need [functionality] so that [benefit/value].

## Acceptance Criteria

- [detail]

---

## BDD Scenarios

### Scenario [n]: [summary]

Given / When / Then

---

## Technical Specification

### [summary]

[detail]

---

## References

- [summary](url)
```

#### Scenario: User story follows required template
- **WHEN** a new user story is created
- **THEN** it SHALL include all five required sections: story, acceptance criteria, BDD scenarios, technical specification, and references

### Requirement: Follow semantic versioning
The system SHALL use semantic versioning (MAJOR.MINOR.PATCH.BUILD) with the version constant in `src/cli/appinfo.h` replicated in CHANGELOG.md, debian/changelog, and rpmbuild/SPECS/msxbas2rom.spec.

#### Scenario: Version is consistent across all files
- **WHEN** `app_version` in `src/cli/appinfo.h` is updated
- **THEN** CHANGELOG.md, debian/changelog, and rpmbuild/SPECS/msxbas2rom.spec SHALL reflect the same version

### Requirement: Tag releases with git branches
New releases SHALL be tagged and registered as a new branch on the repository following the procedure below and in CONTRIBUTING.md.

Release procedure:
```
git tag -a v0.0.0.0 -m "Release 0.0.0.0" && git push origin --tags
git checkout -b release/v0.0.0.0
git push origin release/v0.0.0.0
git switch master
```

#### Scenario: Release is properly tagged
- **WHEN** a new release is created
- **THEN** a git tag `vMAJOR.MINOR.PATCH.BUILD` is created and pushed
- **AND** a branch `release/vMAJOR.MINOR.PATCH.BUILD` is created
- **AND** the tag follows semantic versioning rules
- **AND** `CONTRIBUTING.md` is consulted for the full release procedure

### ADR: Behavioral polymorphism via Strategy/State patterns
Behavioral polymorphism SHALL be centralized in the application layer via factories, keeping orchestration classes slim. The lexer SHALL use State + Factory for line state management, and symbol export SHALL use Strategy + Factory for format dispatch.

#### Scenario: Lexer uses State pattern
- **WHEN** the lexer processes source lines
- **THEN** it SHALL delegate line state logic to `src/application/lexer/states/` via a state factory

#### Scenario: Symbol export uses Strategy pattern
- **WHEN** symbols are exported to different formats
- **THEN** the system SHALL use `src/application/symbols/strategies/` dispatched via a strategy factory

### ADR: Infrastructure/kernel isolated under infrastructure
Low-level platform assets (Z80 assembly kernel) SHALL be isolated under `src/infrastructure/kernel/asm/`, reinforcing separation between business logic and platform concerns. `header.asm` SHALL act as an orchestrator with module includes under `header/`, keeping assembly order unchanged while separating by domain (Single Responsibility Principle).

#### Scenario: Assembly kernel is under infrastructure
- **WHEN** inspecting the kernel assembly files
- **THEN** they SHALL reside under `src/infrastructure/kernel/asm/src/`
- **AND** `header.asm` SHALL act as an orchestrator with domain-separated includes under `header/`

### ADR: Two-phase refactoring pattern
Refactors SHALL be executed in two phases: first "one file per class" with no logic change, then pattern introduction (Strategy/State) with minimum code change.

#### Scenario: Refactor follows two-phase pattern
- **WHEN** a refactor is performed on a module
- **THEN** the first phase SHALL extract files without changing behavior
- **AND** the second phase SHALL introduce design patterns with minimal logic changes

### ADR: Build and test discovery is path-driven
Build and test discovery SHALL be path-driven (recursive `find` in Makefiles) to reduce manual source list maintenance after file moves. Unit tests compile almost the full production tree (all `src/**/*.cpp` except `src/cli/main.cpp`), which quickly reveals broken includes after refactors.

#### Scenario: Tests discover sources recursively
- **WHEN** new source files are added
- **THEN** they SHALL be automatically discovered by the test build system via recursive path scanning
- **AND** broken includes SHALL be revealed by unit test compilation

### ADR: Compiler statement dispatch uses Strategy + Factory
Compiler statement dispatch SHALL use Strategy + Factory (`src/application/compiler/strategies/*`). Strategy header names SHALL use compiler-specific prefixes to avoid collisions with parser strategy headers under global include discovery.

#### Scenario: Strategy headers avoid naming collisions
- **WHEN** compiler strategy headers are created
- **THEN** they SHALL use compiler-specific prefixes to avoid collisions with parser strategy headers
