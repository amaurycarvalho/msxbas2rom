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
