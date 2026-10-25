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

### Requirement: ADR: Kernel modularization uses build-time packing and runtime dispatch

The kernel SHALL be organized as fixed-resident core code plus relocatable modules. Module placement and relocation SHALL be decided at build time; runtime SHALL only select the segment recorded in the central table. Plain ROM SHALL place overflow in the user page, and MegaROM SHALL place overflow in segments.

#### Scenario: Allocation is build-time
- **WHEN** a program is compiled
- **THEN** the used-module closure and module placement SHALL be computed by the builder
- **AND** the runtime SHALL NOT allocate or relocate kernel modules

#### Scenario: Plain ROM overflow shares the user page
- **WHEN** Plain ROM cannot keep all used flex modules in the kernel page
- **THEN** overflow SHALL be placed in `0x8000-0xBFFF`
- **AND** the build SHALL fail if the combined page budget is exceeded

#### Scenario: MegaROM overflow uses segments
- **WHEN** MegaROM cannot keep all used flex modules in the kernel page
- **THEN** overflow SHALL be placed in segments after user code
- **AND** mapper-patched segment switching SHALL be used at runtime

### Requirement: ADR: Player and Arkos remain fixed-resident

The Arkos player body and timer interrupt handler SHALL remain in the fixed kernel page at their designated addresses. They SHALL NOT be paged, relocated, or recompiled during BASIC compilation because playback runs from the interrupt while the song resource occupies the `0x8000` window.

#### Scenario: Interrupt context stays page-safe
- **WHEN** the timer interrupt executes the player
- **THEN** the player code SHALL already be mapped in the fixed page
- **AND** no page switch SHALL be required to execute it

#### Scenario: Song data window is preserved
- **WHEN** the player reads song data
- **THEN** the song resource SHALL remain mapped in `0x8000-0xBFFF`
- **AND** the player code SHALL remain in `0x4000-0x7FFF`

### Requirement: ADR: Relocatable modules are isolated and self-describing

A module SHALL know only its own routines, fixed kernel entry points, and build-time import placeholders. It SHALL carry embedded relocation metadata for its internal absolute references, and cross-module calls SHALL use the central table or a module export table resolved after placement. Address-of references to movable routines SHALL use pointer fixups.

#### Scenario: Module does not embed another module's address
- **WHEN** a module needs a routine from another module
- **THEN** it SHALL call a central-table entry resolved at build time
- **AND** it SHALL NOT contain that routine's absolute assembly address

#### Scenario: Indirect pointer table is fixed up
- **WHEN** a module initializes a pointer table for movable routines
- **THEN** the builder SHALL patch the pointer with the final resolved address
- **AND** the module SHALL NOT rely on the assembly-time address

#### Scenario: Module relocation is self-describing
- **WHEN** the builder places a module
- **THEN** it SHALL use the module's embedded relocation records
- **AND** it SHALL NOT require an external linker or module file

### Requirement: ADR: Resource-owning routines remain in the fixed kernel page

A module that maps resource data into `0x8000` and continues executing in the module SHALL remain in the fixed kernel page. This includes modules using `resource.open_and_get_address`/`resource.address`, indirect BLOAD/SCREEN LOAD paths, and decompression called with compressed data mapped at `0x8000`.

#### Scenario: Resource data must not replace the executing module
- **WHEN** a module executes at `0x8000` and maps a resource into the same window
- **THEN** the module SHALL be classified fixed-resident
- **AND** the packer SHALL reject forced overflow for it

#### Scenario: Fixed resource routines cover DATA/RESTORE/READ and BLOAD
- **WHEN** DATA/RESTORE/READ, BLOAD, MTF, USR, SCREEN LOAD, or resource decompression runs
- **THEN** the relevant routines SHALL execute from the fixed kernel page
- **AND** `70_pletter` SHALL remain fixed because callers map compressed data at `0x8000`

### Requirement: ADR: BASIC compilation remains self-contained

The `msxbas2rom` binary SHALL contain the module library, manifests, relocation records, and mapper patch metadata needed to compile a BASIC program. BASIC compilation SHALL NOT depend on an external assembler, linker, or data file.

#### Scenario: Self-contained BASIC build
- **WHEN** a user compiles a BASIC program
- **THEN** all kernel module data and metadata SHALL come from the `msxbas2rom` binary
- **AND** no external tool or file SHALL be required

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
