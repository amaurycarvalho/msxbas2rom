# Spec Delta

## ADDED Requirements

### Requirement: PRINT and STRING scenarios validate the 40-column text layout

The regression workflow SHALL include the split `PRINT/test5` and `STRING/test1` programs with explicit 40-column text extraction, expected text for separator and concatenation forms, and VRAM byte probes for rows whose spacing or content is significant. Probes SHALL be positioned on rows that no later statement overwrites.

#### Scenario: PRINT separator layout is validated

- **WHEN** `PRINT/test5` is run through the regression workflow
- **THEN** the captured text contains the expected rows for trailing `;`, trailing `,`, repeated commas, and leading separators
- **AND** byte probes confirm the exact tab/space layout for those rows
- **AND** each `PRINT` without a trailing separator advances to its own row

#### Scenario: Concatenated STRING$ result is validated

- **WHEN** `STRING/test1` is run through the regression workflow
- **THEN** the captured name table contains `AAAAABBBBBB` for `STRING$(5,65)+STRING$(6,66)`
- **AND** it contains `AAAAABBBBBB!!!` for the literal concatenation with a suffix
- **AND** it contains `AAAAABBBBB` for each variable-based concatenation
- **AND** a VRAM probe at the first character of each result matches the expected bytes

#### Scenario: PRINT line layout is validated

- **WHEN** either program reaches its terminal capture point
- **THEN** the captured rows begin at column 0 of the configured 40-column name table
- **AND** a regression using the pre-fix 32-column layout fails the configured text or probe checks

### Requirement: All PRINT and STRING$ regression expectations are reviewed

The change SHALL audit every regression program whose source uses `PRINT`, `?`, or `STRING$`, and SHALL review or regenerate its baseline and manifest checks after the default screen layout changes.

#### Scenario: PRINT and STRING baselines are refreshed

- **WHEN** the startup layout changes to 40 columns
- **THEN** every existing `PRINT/*` and `STRING/*` baseline is regenerated and reviewed
- **AND** the new `PRINT/test5` and `STRING/test1` baselines are added

#### Scenario: Programs with explicit SCREEN statements are checked

- **WHEN** a regression program using `PRINT`, `?`, or `STRING$` sets its own `SCREEN`
- **THEN** its captured mode is verified against that statement
- **AND** its text or VRAM probes are updated if `STRING$` results changed

#### Scenario: Manifest text checks use the fixed layout

- **WHEN** a manifest entry configures `vram.text` for a program that does not set `SCREEN`
- **THEN** its `name_table` and `width` match the fixed 40-column `SCREEN 0` layout
- **AND** the baseline records the corrected screen mode