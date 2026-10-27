# Spec Delta

## ADDED Requirements

### Requirement: Tile bank literals are non-negative

For `SET TILE PATTERN`, `SET TILE COLOR`, `SET TILE FLIP`, `GET TILE PATTERN`, and `GET TILE COLOR`, a compile-time literal bank parameter SHALL be greater than or equal to zero. Values above two retain their existing meanings: `SET` treats them as all banks and `GET` treats them as the default bank.

#### Scenario: Zero and two are accepted
- **WHEN** `SET TILE PATTERN 1,(1,2,3,4,5,6,7,8),2` is compiled
- **THEN** compilation succeeds

#### Scenario: Three means all banks for SET
- **WHEN** `SET TILE COLOR 1,1,15,3` is compiled
- **THEN** compilation succeeds

#### Scenario: Negative literal bank is rejected
- **WHEN** `SET TILE PATTERN 1,(1,2,3,4,5,6,7,8),-1` is compiled
- **THEN** compilation fails with a tile-bank error
