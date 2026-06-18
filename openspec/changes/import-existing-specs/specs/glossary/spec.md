## ADDED Requirements

### Requirement: Provide a shared glossary of project terms
The system SHALL maintain a glossary of terms and concepts used across the project to ensure consistent communication.

#### Scenario: Glossary contains all major component definitions
- **WHEN** the glossary is consulted
- **THEN** it SHALL contain definitions for Lexer, Parser, Compiler, Rom, SymbolManager, ResourceManager, and IZ80/Z80
- **AND** it SHALL contain definitions for supporting data structures: Lexeme, LexerLine, SymbolNode, CodeNode, FixNode, TagNode, ActionNode, BuildOptions

#### Scenario: Glossary uses consistent format
- **WHEN** a new term is added to the glossary
- **THEN** it SHALL include a concise description of the component or data structure
- **AND** it SHALL explain the component's role in the pipeline
