# ADR-001 – MSXBAS2ROM architecture foundations

---

## Status

Accepted

## Context

General definitions applicable to the entire project.

---

## Decision

Adopt as a foundation:

1. Spec Driven Development (SDD);
2. Domain-Driven Design (DDD);
3. Clean Code, implementing Dependency Inversion Principle (DIP) and Separation of Concerns (SoC);
4. Clean Architecture;
5. C++ as the main stack;
6. TDD + BDD for user stories;
7. Deliverable as CLI.

---

## Reference Structure

```
MSXBAS2ROM
|
+--cli
   |
   +--lexer
   |
   +--parser
   |
   +--compiler
   |
   +--builder
```

---

## Mandatory Use Case Pattern

All use cases must contain:

1. User Story in the format: "As [actor], I need [functionality] so that [benefit/value]".
2. Explicit Acceptance Criteria.
3. Behavioral scenarios in BDD (`Given / When / Then`).

---

### Technological Restrictions

- Do not use poorly understood, unprofessionally unvalidated, or unmaintained third-party frameworks, SDKs, libraries, components, or services.
- Do not use the `friend` keyword in any C++ code (classes, functions, or anything else).

--

### Security and Quality

- Security by design.
- Alert if there are known vulnerabilities (OWASP, CISA) and propose mitigation.
- Propose removal of unused code.

--

## Testing Strategy

- Use of TDD.
- Minimum coverage of 100% for medium/high criticality code.
- Minimum coverage of 90% for low criticality code.

--

## Application versioning

- Uses semantic versioning style;
- App version it's mantained as a constant named `app_version` at `src/cli/appinfo.h` and replicated at `CHANGELOG.md`, `debian/changelog` and `rpmbuild/SPECS/msxbas2rom.spec` files.

--

## Application releases

New releases needs to be tagged and registered as a new branch on repository:

```
git tag -a v0.0.0.0 -m "Release 0.0.0.0" && git push origin --tags
git checkout -b release/v0.0.0.0
git push origin release/v0.0.0.0
git switch master
```

See `CONTRIBUTING.md` for more information.
