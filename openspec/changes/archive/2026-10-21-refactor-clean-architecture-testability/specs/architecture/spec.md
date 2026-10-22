## MODIFIED Requirements

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

## ADDED Requirements

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
