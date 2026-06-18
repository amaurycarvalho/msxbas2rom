## ADDED Requirements

### Requirement: Document common mistakes and confusion points
The system SHALL maintain a document describing common mistakes and confusion points that agents might encounter, and alert the developer when surprising behavior is discovered.

#### Scenario: Surprising behavior is documented
- **WHEN** an agent encounters unexpected behavior in the project
- **THEN** the agent SHALL alert the developer
- **AND** propose a change to the constitution/governance document to prevent future issues

### Requirement: Prohibit poorly understood third-party dependencies
The system SHALL NOT use poorly understood, unprofessionally unvalidated, or unmaintained third-party frameworks, SDKs, libraries, components, or services.

#### Scenario: Dependency is evaluated before use
- **WHEN** a new third-party dependency is proposed
- **THEN** its maintenance status, professional validation, and community trust SHALL be evaluated before adoption

### Requirement: Prohibit the `friend` keyword
The system SHALL NOT use the `friend` keyword in any C++ code.

#### Scenario: Code review rejects friend keyword
- **WHEN** C++ code is reviewed
- **THEN** no occurrence of the `friend` keyword SHALL be present

### Requirement: Security by design
The system SHALL follow security by design principles, alerting on known vulnerabilities (OWASP, CISA) and proposing mitigation. Unused code SHALL be proposed for removal.

#### Scenario: Security vulnerabilities are flagged
- **WHEN** known vulnerabilities are identified in dependencies or code patterns
- **THEN** the system SHALL alert and propose mitigation

### Requirement: Consult ADRs and user stories as current references
The system SHALL consult the ADR directory for current architectural decisions and the user stories directory for project requirements.

#### Scenario: ADRs are consulted for architectural context
- **WHEN** making architectural decisions
- **THEN** existing ADRs SHALL be consulted for context and precedent
