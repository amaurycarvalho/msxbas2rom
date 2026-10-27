# Spec Delta

## ADDED Requirements

### Requirement: MTF window parameters are validated without restricting map coordinates

When `CMD MTF` uses operation `2`, a literal `width` or `height` SHALL be greater than or equal to zero, and a literal `screen_x` or `screen_y` SHALL be greater than or equal to zero. Destination coordinates above the screen bounds remain valid and are clipped by the kernel. Negative `map_x` and `map_y` SHALL remain valid for operations 1 and 2 and SHALL use the documented modulo wrapping. The operation literal SHALL be `0`, `1`, or `2`.

#### Scenario: Negative map coordinates are accepted
- **WHEN** `CMD MTF 2,1,-10,-5` or `CMD MTF 2,2,-10,-5,8,8,0,0` is compiled
- **THEN** compilation succeeds

#### Scenario: Negative destination coordinate is rejected
- **WHEN** `CMD MTF 2,2,0,0,8,8,-1,0` is compiled
- **THEN** compilation fails with a destination-coordinate error

#### Scenario: Destination beyond the screen remains valid
- **WHEN** `CMD MTF 2,2,0,0,10,6,32,0` is compiled
- **THEN** compilation succeeds and runtime clipping yields no draw

#### Scenario: Negative window size is rejected
- **WHEN** `CMD MTF 2,2,0,0,-1,6,0,0` is compiled
- **THEN** compilation fails with a window-size error

#### Scenario: Invalid operation is rejected
- **WHEN** `CMD MTF 2,3,0,0` is compiled
- **THEN** compilation fails with an operation error
