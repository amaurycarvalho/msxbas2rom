# US-003 - Compiler

## Story
As a ROM builder user, I need the compiler to convert parsed BASIC actions into Z80 machine code and symbol/resource metadata so that the ROM image can be generated reliably.

## Acceptance Criteria
- Compilation starts only when parser output contains tags/actions.
- Compiler emits start/end program stubs and generated code for each tag.
- Duplicate or invalid symbols/flow states (e.g., FOR without NEXT) are reported as compile errors.
- DATA/IDATA parser flags produce corresponding compiler-managed resources.
- Compiler tracks generated code/RAM usage and exposes metrics used by the CLI.

## BDD Scenarios
### Scenario 1: Compile valid parser output
Given a parser result with valid tags and actions
When semantic analysis is executed
Then compilation succeeds
And code size is greater than zero
And RAM usage metrics are available

### Scenario 2: Fail on duplicated line symbol mapping
Given parser tags that map to an already declared line symbol
When semantic analysis is executed
Then compilation fails
And error message indicates line number already declared

### Scenario 3: Register DATA and IDATA resources
Given parser output with `has_data` and `has_idata` enabled
When semantic analysis is executed
Then the compiler adds data and idata resources to the resource manager
