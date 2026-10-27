# Spec Delta

## ADDED Requirements

### Requirement: Kernel command handlers share one parameter-guard policy

All kernel command handlers SHALL use a shared guard policy for parameter counts, fixed-table indices, resource opening, resource sizes, and mode values. The policy SHALL define a deterministic outcome for invalid dynamic values: documented no-op, clamp, or error result. Handlers SHALL NOT implement divergent fallback behavior for the same class of parameter.

#### Scenario: Invalid index behavior is consistent
- **WHEN** two handlers receive an out-of-range fixed-table index
- **THEN** both follow the same documented normalization/failure rule

#### Scenario: Invalid resource behavior is consistent
- **WHEN** two resource-consuming commands receive a missing resource
- **THEN** both return without using undefined pointers or sizes

### Requirement: Malformed kernel inputs fail without corruption or hangs

For malformed resources and invalid dynamic parameters, the kernel SHALL terminate the command within a bounded time and SHALL NOT corrupt memory or enter an infinite loop. This contract applies to MTF maps, `.SPR` files, fonts, RUNASM resources, and all fixed-table index parameters.

#### Scenario: Malformed input terminates
- **WHEN** a command receives a truncated or structurally invalid resource
- **THEN** it returns within its normal execution bound
- **AND** canary memory outside the intended destination is unchanged

#### Scenario: Guarded hot path remains within budget
- **WHEN** the guard layer is measured against the kernel size and graphics performance checks
- **THEN** the fixed kernel page and established hot-path budgets remain within limits
