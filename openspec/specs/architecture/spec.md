# Architecture

## Purpose

Defines the project-wide architecture principles: SDD/DDD/Clean Architecture layering, C++11 cross-platform implementation, TDD/BDD, the mandatory user-story format, semantic versioning, release tagging, and architecture decision records.

**Status:** Accepted
**Context:** General definitions applicable to the entire project.

## Requirements

### Requirement: Adopt SDD, DDD, Clean Architecture with layered separation
The system SHALL adopt Spec Driven Development (SDD), Domain-Driven Design
(DDD), Clean Code, SOLID, and Clean Architecture with four layers — CLI,
application, domain, infrastructure.

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

The deliverable SHALL be a CLI tool. Dependencies SHALL flow inward: domain
shall not depend on application, CLI, or infrastructure; application shall
depend on domain and abstractions it owns, not concrete filesystem, generated
kernel, output, or presentation implementations; CLI and infrastructure shall
implement and compose those abstractions. Application state exposed to
strategies SHALL be encapsulated behind narrow collaborators rather than a
public all-purpose context.

#### Scenario: Source code follows layered structure
- **WHEN** the source code is inspected
- **THEN** it SHALL be organized as `src/cli`, `src/application`, `src/domain`,
  and `src/infrastructure`
- **AND** dependencies shall flow inward
- **AND** domain and application behavior shall not directly perform
  filesystem persistence or access the generated kernel representation

#### Scenario: Application behavior is tested through ports
- **WHEN** an application-level lexer, parser, compiler, ROM builder, resource
  reader, or symbol exporter is tested
- **THEN** its external source, kernel, output, and diagnostic collaborators
  can be replaced by in-memory implementations
- **AND** the test shall not require an OS file or CLI process unless it is
  specifically testing an infrastructure or CLI adapter

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

Changelog documentation is automated by the `openspec-changelog` skill (see `openspec/specs/governance/spec.md`). No agent SHALL manually edit CHANGELOG.md, CHANGELOG-ARCHIVE.md, debian/changelog, rpm spec %changelog, or info_history for release history purposes.

#### Scenario: Version is consistent across all files
- **WHEN** `app_version` in `src/cli/appinfo.h` is updated
- **THEN** CHANGELOG.md, debian/changelog, and rpmbuild/SPECS/msxbas2rom.spec SHALL reflect the same version

#### Scenario: Changelog is updated by skill
- **WHEN** a release is created
- **THEN** the `openspec-changelog` skill SHALL handle all changelog-related file updates
- **AND** no agent SHALL edit those files directly

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

### Requirement: Application dependencies are composed explicitly

Application and domain components SHALL receive their external collaborators
through constructors or parameter objects. They SHALL NOT resolve process-wide
default services, use mutable global providers, or create concrete outer-layer
adapters internally. Production composition SHALL be confined to the CLI
composition root.

#### Scenario: Application component receives its ports

- **WHEN** an application component needs a source, kernel, resource, symbol,
  or ROM persistence collaborator
- **THEN** the collaborator SHALL be supplied by the caller as an owned
  abstraction
- **AND** the component SHALL NOT look up a process-wide default
- **AND** the component SHALL NOT construct a concrete infrastructure adapter

#### Scenario: Production composition is centralized

- **WHEN** the CLI application starts
- **THEN** the CLI SHALL construct the concrete file, kernel, symbol, and ROM
  adapters
- **AND** it SHALL pass them to the application workflow explicitly
- **AND** no application entry point SHALL depend on global initialization
  order

#### Scenario: Tests compose their own collaborators

- **WHEN** a unit test exercises an application workflow
- **THEN** the test SHALL provide its own in-memory adapters
- **AND** it SHALL NOT require global adapter setup before the test runs

### Requirement: Automated architecture boundary check with approved exceptions

The build/test pipeline SHALL run an automated architecture-boundary check that
enforces the inward dependency rule and detects direct filesystem,
generated-kernel, and concrete-diagnostics access in `domain` and
`application`. A violation SHALL be permitted only when it matches an entry in
a version-controlled exception list containing the exact path, the forbidden
dependency or pattern, the owning migration task, and the removal criterion.

#### Scenario: A new inward-dependency violation fails the check
- **WHEN** a `domain` source includes an `application`, `cli`, or
  `infrastructure` header, or an `application` source includes a `cli` header
- **THEN** the boundary check SHALL fail
- **AND** the failure SHALL identify the source file and the forbidden include

#### Scenario: Direct I/O or kernel access fails without an exception
- **WHEN** an `application` or `domain` source directly uses
  `fopen`/`ifstream`/`ofstream`/`remove`, `bin_header_bin`, `header.h`, or
  `start.h`
- **THEN** the boundary check SHALL fail unless the exact file and pattern are
  listed in the approved exception file

#### Scenario: Approved exceptions are explicit and temporary
- **WHEN** an exception entry names its owning migration task and removal
  criterion
- **THEN** the boundary check SHALL pass while the violation still exists
- **AND** the entry SHALL be removed when the owning task eliminates the
  violation
- **AND** a stale entry that no longer matches a violation SHALL fail the check

#### Scenario: The check runs in the normal quality gate
- **WHEN** `make test`, `make quality-gate`, or CI runs
- **THEN** the boundary check and its fixture-based self-tests SHALL run
- **AND** a violation outside the approved list SHALL fail the command

## Architecture Decision Records

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
