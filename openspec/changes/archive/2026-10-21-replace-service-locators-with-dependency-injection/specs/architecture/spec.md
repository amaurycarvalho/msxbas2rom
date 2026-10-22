## ADDED Requirements

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
