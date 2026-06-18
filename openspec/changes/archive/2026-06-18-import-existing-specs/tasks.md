## 1. Create Main Capability Specs

- [x] 1.1 Sync lexer delta spec to openspec/specs/lexer/spec.md (from spec/delta US-001)
- [x] 1.2 Sync parser delta spec to openspec/specs/parser/spec.md (from US-002)
- [x] 1.3 Sync compiler delta spec to openspec/specs/compiler/spec.md (from US-003)
- [x] 1.4 Sync builder delta spec to openspec/specs/builder/spec.md (from US-004)
- [x] 1.5 Sync CLI delta spec to openspec/specs/cli/spec.md (from US-005)
- [x] 1.6 Sync ON statements delta spec to openspec/specs/on-statements/spec.md (from US-006)
- [x] 1.7 Sync file handling delta spec to openspec/specs/file-handling/spec.md (from US-007)
- [x] 1.8 Sync sprites handling delta spec to openspec/specs/sprites-handling/spec.md (from US-008)
- [x] 1.9 Sync architecture delta spec to openspec/specs/architecture/spec.md (from ADR-001 + insights)
- [x] 1.10 Sync governance delta spec to openspec/specs/governance/spec.md (from constitution.md)
- [x] 1.11 Sync glossary delta spec to openspec/specs/glossary/spec.md (from glossary.md)

## 2. Create Archived Changes for Completed Sprints

- [x] 2.1 Create change for sprint 1 (SDD/TDD/Clean Architecture refactoring, release 0.3.3.0) with minimal proposal+tasks and archive
- [x] 2.2 Create change for sprint 2 (ON statement refactoring, US-006, release 0.3.4.0) with minimal proposal+tasks and archive
- [x] 2.3 Create change for sprint 3 (File handling support, US-007, release 1.0.0.0) with minimal proposal+tasks and archive

## 3. Create Active Changes for Pending Sprints

- [x] 3.1 Create change for sprint 4 (SET/GET TILE PATTERN improved syntax, release 1.1.0.0) with full artifacts
- [x] 3.2 Create change for sprint 5 (SET TILE FLIP/ROTATE, release 1.2.0.0) with full artifacts
- [x] 3.3 Create change for sprint 6 (SET/GET TILE COLOR/PATTERN, release 1.3.0.0) with full artifacts
- [x] 3.4 Create change for sprint 7 (MSX Tile Forge support enhancements, release 1.4.0.0) with full artifacts

## 4. Update Project Documentation

- [x] 4.1 Update CHANGELOG.md format to reference OpenSpec changes
- [x] 4.2 Update README.md to reference openspec/ as spec source of truth
- [x] 4.3 Conditionally remove old specs/ directory — see section 5 for the verification protocol

## 5. Verification (3-Round Protocol)

**Round 1 — Automated completeness check:**
- [x] 5.1 Run automated comparison between specs/ and openspec/ cataloging every section, heading, scenario, technical detail, and reference
- [x] 5.2 File all gaps found as issues to be fixed before proceeding

**Round 2 — Content parity check:**
- [x] 5.3 After all gaps from round 1 are resolved, manually verify that every requirement and scenario from specs/ has a corresponding entry in openspec/
- [x] 5.4 Verify CHANGELOG.md and README.md are updated correctly

**Round 3 — Final integrity check:**
- [x] 5.5 Re-run the automated comparison from round 1 to confirm zero gaps remain

**Pending maintainer confirmation:**
- [x] 5.6 Present the final diff report to the maintainer and request formal confirmation to delete specs/
  - Round 1: 11 gaps found → all fixed (story sentences, acceptance criteria, tech specs, references, glossary, architecture, governance)
  - Round 2: 6 gaps found → all fixed (wishlist, FCB table, OPEN docs, COLLISION examples, governance wording, git commands)
  - Round 3: 5 gaps found → all fixed (SPRITE LOAD example, user story template, ADR status/context, insights maintenance, badge link)
  - Zero gaps remaining after 3 verification rounds

**Deletion (only after maintainer confirms):**
- [x] 5.7 Remove old specs/ directory after maintainer's formal confirmation
