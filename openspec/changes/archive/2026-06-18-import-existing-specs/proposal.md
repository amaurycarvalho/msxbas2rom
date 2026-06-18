## Why

The project currently uses an old Spec Driven Development (SDD) format in `specs/` that is incompatible with the OpenSpec workflow. All specifications, definitions, and planning artifacts need to be reverse-engineered into the `openspec/` format so that future development can leverage OpenSpec's change management, artifact generation, and archival capabilities.

## What Changes

- Migrate 8 user stories from `specs/user-stories/` into capability specs under `openspec/specs/<capability>/spec.md`
- Migrate ADR-001 (architecture foundations) into `openspec/specs/architecture/spec.md`
- Migrate `constitution.md` into `openspec/specs/governance/spec.md`
- Migrate `glossary.md` into `openspec/specs/glossary/spec.md`
- Promote stable insights from `insights.md` into the architecture spec as ADR entries
- Create archived changes for completed sprints (1-3 from `plan.md`)
- Create active changes for pending sprints (4-7 from `plan.md`)
- Update `CHANGELOG.md` to reference OpenSpec changes
- Update `README.md` to reference `openspec/` as the spec source of truth
- Remove the old `specs/` directory **only after**: (a) 3 verification rounds confirming nothing was left behind, and (b) explicit formal confirmation from the project maintainer

## Capabilities

### New Capabilities
- `lexer`: MSX-BASIC lexical analysis — tokenization of source code into classified lexemes
- `parser`: MSX-BASIC syntax analysis — transforms tokens into action/symbol structures
- `compiler`: MSX-BASIC compilation — converts parsed actions into Z80 machine code
- `builder`: ROM construction — packages compiled code and resources into ROM/MegaROM images
- `cli`: Command-line interface — orchestrates lexer, parser, compiler, and builder stages
- `on-statements`: Multi-branch and event-driven control flow (ON GOTO/GOSUB, ON INTERVAL/SPRITE/STRIG/KEY/STOP/ERROR)
- `file-handling`: File I/O support (OPEN, CLOSE, PRINT#, INPUT#, LINE INPUT#, MAXFILES, EOF, LOC, LOF, FPOS, ERR, DSKF)
- `sprites-handling`: Sprite management (SPRITE ON/OFF/STOP/LOAD, PUT SPRITE, SPRITE$, COLOR SPRITE, SET/GET SPRITE, COLLISION)
- `architecture`: System architecture foundations (Clean Architecture, DDD, C++11, TDD/BDD, ADRs)
- `governance`: Project principles and mandatory guidelines
- `glossary`: Terms and concepts used across the project

### Modified Capabilities
- *(none — all specs are new)*

## Impact

- `openspec/specs/` — 11 new capability spec directories created
- `openspec/changes/` — 7 change directories created (3 archived, 4 active)
- `CHANGELOG.md` — updated format to reference OpenSpec changes
- `README.md` — updated to reference `openspec/` as spec source
- `specs/` — entire directory removed after migration
- No source code changes — this is a documentation/specification migration only
