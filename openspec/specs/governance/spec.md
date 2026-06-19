## ADDED Requirements

### Requirement: Document common mistakes and confusion points
The role of this file is to describe common mistakes and confusion points that agents might encounter as they work in this project. If any agent encounters something in the project that surprises them, they SHALL alert the developer working with them and propose a change to this file to help prevent future agents from having the same issue. These principles are not optional.

#### Scenario: Surprising behavior is documented
- **WHEN** an agent encounters unexpected behavior in the project
- **THEN** the agent SHALL alert the developer
- **AND** propose a change to the governance document to prevent future issues

### Requirement: Prohibit poorly understood third-party dependencies
The system SHALL NOT use poorly understood, unprofessionally unvalidated, or unmaintained third-party frameworks, SDKs, libraries, components, or services.

#### Scenario: Dependency is evaluated before use
- **WHEN** a new third-party dependency is proposed
- **THEN** its maintenance status, professional validation, and community trust SHALL be evaluated before adoption

### Requirement: Prohibit the `friend` keyword
The system SHALL NOT use the `friend` keyword in any C++ code (classes, functions, or anything else).

#### Scenario: Code review rejects friend keyword
- **WHEN** C++ code is reviewed
- **THEN** no occurrence of the `friend` keyword SHALL be present

### Requirement: Security by design with vulnerability alerts
The system SHALL follow security by design principles, alerting on known vulnerabilities (OWASP, CISA) and proposing mitigation. Unused code SHALL be proposed for removal.

#### Scenario: Security vulnerabilities are flagged
- **WHEN** known vulnerabilities are identified in dependencies or code patterns
- **THEN** the system SHALL alert and propose mitigation

### Requirement: Consult ADRs for current architectural decisions
The system SHALL consult `openspec/specs/architecture/spec.md` for current architectural decisions.

#### Scenario: ADRs are consulted for architectural context
- **WHEN** making architectural decisions
- **THEN** the architecture spec SHALL be consulted for context and precedent

### Requirement: Consult capability specs for project requirements
The system SHALL consult `openspec/specs/` for capability specifications and project requirements.

#### Scenario: Capability specs are consulted
- **WHEN** implementing a new feature
- **THEN** the relevant capability spec in `openspec/specs/` SHALL be consulted for requirements

### Requirement: Maintain project discoveries
Agents SHALL consult and maintain project discoveries, using the insights captured in this project's change history as a memory bank of discoveries about this project.

#### Scenario: Discoveries are recorded
- **WHEN** an agent discovers unexpected behavior or non-obvious patterns
- **THEN** the agent SHALL record the discovery in the relevant change's artifacts or propose a new insight for the governance/architecture specs

### Requirement: Consult changes for roadmap and tasks
The system SHALL consult `openspec/changes/` for the project roadmap and current/archived changes.

#### Scenario: Changes are consulted for planning context
- **WHEN** planning new work
- **THEN** active changes in `openspec/changes/` SHALL be consulted for current sprint items
- **AND** archived changes in `openspec/changes/archive/` SHALL be consulted for completed work

### Requirement: Every change must be associated with a release and registered in CHANGELOG
Every change SHALL be associated with a target release version (e.g., `1.0.0.0`). The release MUST be captured during the explore or propose phase — if the user does not specify it, the agent SHALL ask. At archive time, the agent SHALL update `CHANGELOG.md` with the change's entries under the correct release section, categorized as Added, Changed, Fixed, Deprecated, Removed, or Security per the Keep a Changelog format. If the release affects the `info_history` string in `src/cli/appinfo.h`, that SHALL also be updated to reflect the current release.

#### Scenario: Release is captured during planning
- **WHEN** exploring or proposing a new change
- **THEN** the agent SHALL confirm the target release version with the user
- **AND** if the user does not provide it, the agent SHALL ask before proceeding

#### Scenario: CHANGELOG is updated at archive time
- **WHEN** archiving a completed change
- **THEN** the agent SHALL add the change's entries to `CHANGELOG.md` under the appropriate release section
- **AND** SHALL use the Keep a Changelog categories (Added, Changed, Fixed, Deprecated, Removed, Security)

#### Scenario: info_history is synced from CHANGELOG at release time
- **WHEN** preparing a release
- **THEN** `src/cli/appinfo.h` `info_history` SHALL be synced to mirror the current release entry from `CHANGELOG.md`, a summary of the last 2 releases, and the release URL
- **AND** `CHANGELOG.md` is the authoritative source — do NOT write to `info_history` first
- **AND** the architecture spec (`openspec/specs/architecture/spec.md`) SHALL be consulted for the full syncing rule

### Requirement: Run builds and tests asynchronously
The project has **278 `.cpp` source files** and a full build (`make release` or `make debug`) can take **several minutes**. To avoid timeouts and blocking the agent, all compilation and test commands SHALL be run asynchronously via the Task tool or in background processes. Direct synchronous compilation of the full tree SHALL be avoided.

#### Scenario: Large compilation tasks are delegated
- **WHEN** a full build or test run is needed (e.g., `make release`, `make test-unit`)
- **THEN** the agent SHALL delegate the command to a background task or sub-agent
- **AND** SHALL NOT wait synchronously for the result
- **AND** SHALL verify compilation of only the changed files synchronously when possible (using `g++ -c` with proper include paths)

#### Scenario: Changed files are verified individually
- **WHEN** only a subset of files have been modified
- **THEN** the agent SHALL prefer compiling only the changed `.cpp` files with `g++ -c <file> -o <output>` using the same flags as the Makefile
- **AND** SHALL use the full project include tree (`$(shell find src -type f \( -name "*.h" -o -name "*.hpp" \) -exec dirname {} + | sort -u | sed 's/^/-I /')`)
- **AND** only fall back to `make release` or `make test-unit` as a final integration verification, run asynchronously
