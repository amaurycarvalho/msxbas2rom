# Spec Delta

## ADDED Requirements

### Requirement: Literal resource numbers resolve in the resource map

When a command or directive accepts a resource number and the argument is a compile-time literal, the compiler SHALL reject the program if the number is outside the compiled resource map. This applies to resource-consuming commands such as `CMD RESTORE`, `CMD MTF`, `CMD WRTFNT`, `CMD WRTVRAM`, `CMD WRTSPR`, `SCREEN LOAD`, `SPRITE LOAD`, and the Arkos commands. Non-literal expressions remain runtime values.

#### Scenario: Existing resource literal is accepted
- **WHEN** a program declares `FILE "a.bin"` and compiles `CMD RESTORE 0`
- **THEN** compilation succeeds

#### Scenario: Missing resource literal is rejected
- **WHEN** a program declares one resource and compiles `CMD MTF 1`
- **THEN** compilation fails with a resource-number error

#### Scenario: Variable resource number is not compile-time rejected
- **WHEN** a program compiles `CMD MTF N%` where `N%` is a variable
- **THEN** compilation succeeds and validation is deferred to runtime

### Requirement: CMD RUNASM literal resources are executable

For a compile-time literal resource, `CMD RUNASM` SHALL be rejected when the resource is larger than 255 bytes or when it is stored in a compressed format that cannot be executed directly.

#### Scenario: Small uncompressed resource is accepted
- **WHEN** `CMD RUNASM 0` refers to an uncompressed resource of at most 255 bytes
- **THEN** compilation succeeds

#### Scenario: Oversized resource is rejected
- **WHEN** `CMD RUNASM 0` refers to a 256-byte resource
- **THEN** compilation fails with a size-limit error

#### Scenario: Compressed resource is rejected
- **WHEN** `CMD RUNASM 0` refers to a compressed resource
- **THEN** compilation fails with an executable-format error
