# Spec Delta

## ADDED Requirements

### Requirement: PLYSOUND parameters have fixed defaults

The compiler SHALL accept one to three arguments for `CMD PLYSOUND`, default the channel to `2`, default the inverted volume to `0`, and pass the third argument from the correct action position. More than three arguments SHALL be rejected.

#### Scenario: PLYSOUND third argument is preserved
- **WHEN** `CMD PLYSOUND 1,2,3` is compiled
- **THEN** the effect is `1`, the channel is `2`, and the inverted volume is `3`
- **AND** the emitted volume is not a copy of the channel

#### Scenario: PLYSOUND defaults
- **WHEN** `CMD PLYSOUND 1` is compiled
- **THEN** the effect is `1`, the channel is `2`, and the inverted volume is `0`
- **WHEN** `CMD PLYSOUND 1,0` is compiled
- **THEN** the channel is `0` and the inverted volume is `0`

#### Scenario: PLYSOUND excess arguments are rejected
- **WHEN** `CMD PLYSOUND 1,2,3,4` is compiled
- **THEN** compilation fails with a syntax error

### Requirement: PLYLOAD and PLYSONG optional parameters have defaults

The compiler SHALL accept one or two arguments for `CMD PLYLOAD` and, when the second is omitted, emit the first resource number into the second parameter. It SHALL accept zero or one argument for `CMD PLYSONG` and default the subsong to `0`. Excess arguments SHALL be rejected.

#### Scenario: PLYLOAD second default
- **WHEN** `CMD PLYLOAD 3` is compiled
- **THEN** the first resource number is `3` and the second resource number is also `3`

#### Scenario: PLYLOAD excess arguments are rejected
- **WHEN** `CMD PLYLOAD 1,2,3` is compiled
- **THEN** compilation fails with a syntax error

#### Scenario: PLYSONG optional subsong
- **WHEN** `CMD PLYSONG` is compiled
- **THEN** the subsong number is `0`
- **WHEN** `CMD PLYSONG 2` is compiled
- **THEN** the subsong number is `2`

#### Scenario: PLYSONG excess arguments are rejected
- **WHEN** `CMD PLYSONG 1,2` is compiled
- **THEN** compilation fails with a syntax error

### Requirement: PLYLOOP requires a valid mode

`CMD PLYLOOP` SHALL require exactly one argument and accept literal `0` or `1`.

#### Scenario: Valid PLYLOOP compiles
- **WHEN** `CMD PLYLOOP 0` or `CMD PLYLOOP 1` is compiled
- **THEN** compilation succeeds

#### Scenario: Invalid PLYLOOP is rejected
- **WHEN** `CMD PLYLOOP 2` or `CMD PLYLOOP` is compiled
- **THEN** compilation fails

### Requirement: Function calls have bounded arity

The compiler SHALL reject a function call with more than four arguments before evaluating or storing argument subtypes, and SHALL report a syntax error instead of writing past its fixed result storage. The compiler SHALL require exactly one argument for `FRE`.

#### Scenario: Too many function arguments are rejected
- **WHEN** a function call with five or more arguments is compiled
- **THEN** compilation fails without invoking undefined behavior

#### Scenario: FRE requires one argument
- **WHEN** `A=FRE(0)` is compiled
- **THEN** compilation succeeds
- **WHEN** `A=FRE()` or `A=FRE(0,1)` is compiled
- **THEN** compilation fails with a syntax error

### Requirement: Literal parameter ranges are validated

The compiler SHALL validate supported literal values for commands whose contracts define a closed range: `CMD TURBO` `0..1`, `CMD CLIP` `0..1`, `CMD PLAY` channel `0..1`, `CMD PAGE` mode `0..1` and delays `0..15`, `SET FONT` style `0..3` and bank `0..2` or omitted, `CMD PLYLOOP` `0..1`, and tile banks of `0` or greater. Literal values outside those ranges SHALL fail compilation with a command-specific syntax error.

#### Scenario: Invalid TURBO literal is rejected
- **WHEN** `CMD TURBO 2` is compiled
- **THEN** compilation fails

#### Scenario: Invalid PAGE literal is rejected
- **WHEN** `CMD PAGE 1,16` or `CMD PAGE 2,0` is compiled
- **THEN** compilation fails

#### Scenario: Invalid font style is rejected
- **WHEN** `SET FONT 4` or `SET FONT 1,3` is compiled
- **THEN** compilation fails

#### Scenario: Negative tile bank is rejected
- **WHEN** `SET TILE PATTERN 1,(1,2,3,4,5,6,7,8),-1` is compiled
- **THEN** compilation fails

### Requirement: RANDOMIZE only accepts the parameterless form

The compiler SHALL accept `RANDOMIZE` without parameters and SHALL reject `RANDOMIZE` with an argument, because the documented contract only defines the parameterless form.

#### Scenario: Parameterless RANDOMIZE is accepted
- **WHEN** `RANDOMIZE` is compiled
- **THEN** compilation succeeds

#### Scenario: RANDOMIZE with an argument is rejected
- **WHEN** `RANDOMIZE 42` is compiled
- **THEN** compilation fails with a syntax error

### Requirement: CLOSE without parameters closes all files

The compiler SHALL emit the close-all file number `0xFF` when `CLOSE` has no parameters, matching the documented behavior implemented by the kernel close routine.

#### Scenario: Naked CLOSE emits close-all
- **WHEN** `CLOSE` is compiled
- **THEN** the emitted code loads `0xFF` into `A` before calling the close routine
