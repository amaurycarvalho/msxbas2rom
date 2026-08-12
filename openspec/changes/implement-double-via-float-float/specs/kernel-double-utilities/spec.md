## ADDED Requirements

### Requirement: Kernel provides double-double arithmetic utility routines
The Z80 kernel binary (`header.bin`, ROM page at 0x4000-0x7FFF) SHALL contain size-optimized utility routines implementing the float-float (double-double) arithmetic algorithms: TwoSum, FastTwoSum, TwoProd (Veltkamp split), division, comparison, and 6-byte load/store. These routines SHALL be placed in the existing `BASIC_KUN_START_FILLER` zero-fill area (0x61FA-0x6480, 647 bytes) in `90_support.asm` and SHALL be accessible via the `wrapper_routines_map_table` dispatch at new indices 224-231. The kernel binary SHALL remain exactly 32768 bytes.

#### Scenario: All utility routines fit within filler area
- **WHEN** all double utility routines are assembled
- **THEN** the total code size does not exceed 647 bytes
- **AND** the kernel binary remains 32768 bytes (no page size increase)
- **AND** `BASIC_KUN_START_FILLER` padding shrinks to absorb remaining free space

#### Scenario: Dispatch entries accessible via compiler_hooks.h
- **WHEN** a C++ compiler source includes `compiler_hooks.h`
- **THEN** constants `DISP_doubleFastTwoSum` (224) through `DISP_doubleNeg` (231) are defined
- **AND** `DISP_ENTRIES` is 232 (was 224)
- **AND** `optimizer.addKernelCall(DISP_doubleTwoSum)` resolves to the correct kernel address

### Requirement: doubleFastTwoSum routine
The kernel routine at DISP 224 SHALL compute the Dekker FastTwoSum error-free transform: given two single-precision floats a and b with |a| >= |b|, compute s = a + b and e = b - (s - a), returning (s, e) as a double-double in B:HL (high=s) + C:DE (low=e).

The routine SHALL:
- Accept a in B:HL and b in C:DE
- Use the hardware stack (push/pop) and shadow registers (exx) for intermediate saves to minimize byte count
- Call XBASIC_ADD_FLOATS (DISP 128) and XBASIC_SUBTRACT_FLOATS (DISP 129) via their dispatch entries
- Return s in B:HL and e in C:DE
- Preserve no registers (caller-save convention)

**Estimated size**: ≤ 50 bytes

#### Scenario: FastTwoSum with |a| >= |b|
- **WHEN** a = 3.0 (B:HL) and b = 0.5 (C:DE)
- **THEN** B:HL = 3.5 (sum s)
- **AND** C:DE = error term e
- **AND** s + e ≈ a + b

#### Scenario: FastTwoSum violates precondition silently
- **WHEN** |a| < |b| is passed
- **THEN** the routine may produce incorrect error term
- **AND** callers SHALL ensure |a| >= |b| before calling

### Requirement: doubleTwoSum routine
The kernel routine at DISP 225 SHALL compute the full Dekker TwoSum error-free transform: given two single-precision floats a and b (no ordering constraint), compute the exact sum (s, e) = a + b where s + e = a + b and e is the rounding error.

The routine SHALL:
- Internally call doubleFastTwoSum (DISP 224) as the first step
- Then compute the full error term via 3 additional XBASIC calls (add/sub)
- Accept a in B:HL and b in C:DE
- Return s in B:HL and e in C:DE

**Estimated size**: ≤ 55 bytes (delta from doubleFastTwoSum due to code sharing)

#### Scenario: TwoSum with any ordering
- **WHEN** a = 0.5 (B:HL) and b = 3.0 (C:DE) (|a| < |b|)
- **THEN** B:HL = 3.5 (sum s)
- **AND** C:DE = error term e
- **AND** s + e = a + b exactly

### Requirement: doubleTwoProd routine
The kernel routine at DISP 226 SHALL compute the Dekker TwoProd error-free transform via Veltkamp splitting: given two single-precision floats a and b, compute the exact product (ph, pl) = a * b where ph + pl = a * b.

The routine SHALL:
- Perform Veltkamp split on both a and b: split the 23-bit XBASIC MBF mantissa into high(12 bits) and low(11 bits) halves using inline Z80 bit manipulation (masking via AND, shifting via SRL/RRC)
- The MBF format stores sign at bit 23 (bit 7 of H), exponent in B, and 23-bit mantissa in HL. The split must preserve sign and exponent, operating only on the mantissa.
- Compute 4 sub-products (a_hi*b_hi, a_hi*b_lo, a_lo*b_hi, a_lo*b_lo) via calls to XBASIC_MULTIPLY_FLOATS (DISP 130)
- Combine sub-products via XBASIC_ADD_FLOATS (DISP 128) and XBASIC_SUBTRACT_FLOATS (DISP 129)
- Compute: ph = a * b (direct multiply), pl = ((a_hi*b_hi - ph) + a_hi*b_lo + a_lo*b_hi) + a_lo*b_lo
- Return ph in B:HL and pl in C:DE

**Estimated size**: ≤ 160 bytes

#### Scenario: TwoProd returns exact product
- **WHEN** a = 1.5 (B:HL) and b = 2.0 (C:DE)
- **THEN** B:HL = 3.0 (approximate product ph)
- **AND** C:DE = error term pl
- **AND** ph + pl = 1.5 * 2.0 = 3.0 exactly

#### Scenario: Veltkamp split preserves value
- **WHEN** an MBF float is split into a_hi + a_lo
- **THEN** a_hi + a_lo = a to within single-precision rounding
- **AND** |a_lo| ≤ 0.5 ulp(a_hi)

### Requirement: doubleDiv routine
The kernel routine at DISP 227 SHALL compute double-double division: given a double-double numerator (a_hi in B:HL, a_lo in C:DE) and denominator (b_hi in B:HL, b_lo in C:DE — passed on the stack), compute the quotient (q_hi, q_lo).

The routine SHALL:
- Accept numerator in B:HL + C:DE and denominator on the stack (8 bytes)
- Compute approximate quotient: qh = a_hi / b_hi via XBASIC_DIVIDE_FLOATS (DISP 131)
- Compute residual: r = a_hi - qh * b_hi via XBASIC subtract and multiply
- Compute correction: t = r + a_lo - qh * b_lo via XBASIC add/sub/mul
- Compute correction quotient: q = t / b_hi via XBASIC divide
- Renormalize: (q_hi, q_lo) = doubleFastTwoSum(qh, q) via internal call to DISP 224
- Return q_hi in B:HL and q_lo in C:DE

**Estimated size**: ≤ 110 bytes

#### Scenario: Divide two double values
- **WHEN** A# / B# is computed via the kernel
- **THEN** the result is a valid double-double quotient
- **AND** division by zero follows XBASIC behavior (overflow or trap)

### Requirement: doubleCompare routine
The kernel routine at DISP 228 SHALL compare two double-double values and return 0xFFFF (true) or 0x0000 (false) in HL matching MSX BASIC convention.

The routine SHALL:
- Accept the first double in B:HL + C:DE and the second double on the stack (8 bytes)
- Accept a comparison mode indicator in register A (0=EQ, 1=NE, 2=LT, 3=LE, 4=GT, 5=GE)
- First compare the high parts via XBASIC_COMPARE_FLOATS
- If high parts are not equal, return the comparison result immediately (early exit)
- If high parts are equal, compare the low parts with appropriate sign handling
- The sign of each double is determined by bit 7 of the mantissa high byte (bit 7 of H)
- For negative values, low-part ordering SHALL be inverted (larger magnitude = smaller value)

**Estimated size**: ≤ 45 bytes (with 6 comparison modes handled via branching on A)

#### Scenario: Equal doubles
- **WHEN** comparing 3.5# and 3.5# with mode EQ
- **THEN** HL = 0xFFFF

#### Scenario: High parts differ, low parts ignored
- **WHEN** comparing 3.5# and 2.0# with mode GT
- **THEN** HL = 0xFFFF (3.5 > 2.0, decided on high parts alone)

#### Scenario: High parts equal, low part tie-break
- **WHEN** two doubles have equal high parts but different low parts
- **THEN** the comparison accounts for the low part difference

### Requirement: doubleLoadHL routine
The kernel routine at DISP 229 SHALL load a 6-byte double value from memory address HL into register pairs B:HL (high part) + C:DE (low part).

The routine SHALL:
- Accept source address in HL
- Read 6 consecutive bytes from RAM: (HL), (HL+1), (HL+2), (HL+3), (HL+4), (HL+5)
- Place bytes 0-2 in B:HL (high part, same format as single-precision XBASIC float)
- Place bytes 3-5 in C:DE (low part)
- Clobber A, B, C, D, E, H, L
- Return with HL pointing past the last read byte (HL+6) — caller may ignore this

**Estimated size**: ≤ 20 bytes

#### Scenario: Load double from variable address
- **WHEN** HL points to a 6-byte double variable in RAM
- **THEN** B:HL = high part, C:DE = low part

### Requirement: doubleStoreHL routine
The kernel routine at DISP 230 SHALL store a 6-byte double value from register pairs B:HL (high) + C:DE (low) into memory at address HL.

The routine SHALL:
- Accept destination address in HL and value in B:HL + C:DE
- Write 6 consecutive bytes to RAM: (HL), (HL+1), (HL+2), (HL+3), (HL+4), (HL+5)
- Clobber A
- Return with HL pointing past the last written byte (HL+6) — caller may ignore this

**Estimated size**: ≤ 18 bytes

#### Scenario: Store double to variable address
- **WHEN** HL points to a 6-byte double variable destination in RAM
- **AND** B:HL + C:DE contains a valid double value
- **THEN** 6 bytes are written correctly

### Requirement: doubleNeg routine
The kernel routine at DISP 231 SHALL negate both parts of a double-double value: high = -high and low = -low, using the existing single-precision `floatNeg` routine (DISP 40).

The routine SHALL:
- Accept double in B:HL + C:DE
- Call `floatNeg` (DISP 40) to negate B:HL → B:HL
- Swap C:DE into B:HL, call `floatNeg` to negate → B:HL
- Swap result back to C:DE, move original B:HL back
- Return negated double in B:HL + C:DE

**Estimated size**: ≤ 15 bytes

#### Scenario: Negate a double value
- **WHEN** a double in B:HL + C:DE is negated
- **THEN** both high and low parts are negated
- **AND** the result is a valid double-double

### Requirement: Kernel utility routines use caller-save convention with documented clobber list
All new kernel utility routines SHALL follow the existing kernel convention: routines may clobber AF, BC, DE, HL unless documented otherwise. The header comments for each routine SHALL document:
- Input registers and their meaning
- Output registers and their contents
- Which registers are clobbered
- Which XBASIC routines are called as subroutines

#### Scenario: Compiler code generation respects clobbered registers
- **WHEN** the C++ compiler emits a `call DISP_doubleTwoSum`
- **THEN** it pushes any live values in AF/BC/DE/HL before the call and restores them after
- **AND** this matches the pattern used for all existing kernel calls (e.g., `call DISP_XBASIC_ADD_FLOATS`)

### Requirement: Wrapper dispatch table is updated atomically with kernel routines
The `wrapper_routines_map_table` in `20_runtime.asm` SHALL include 8 new `dw` entries for the double utility routines at the end of the table (before `DEFS 0x4000 - $`). The `compiler_hooks.h` SHALL define corresponding `DISP_double*` constants and increment `DISP_ENTRIES` from 224 to 232. The C++ `getKernelCallAddr` function SHALL resolve addresses for indices 0-231.

#### Scenario: Dispatch table built correctly
- **WHEN** `make` assembles the kernel
- **THEN** `header.symbols.asm` exports labels `doubleFastTwoSum` through `doubleNeg`
- **AND** `header.h` contains the kernel binary with resolved addresses
- **AND** C++ `getKernelCallAddr(DISP_doubleTwoSum)` returns the correct address

#### Scenario: DEFS padding adjusts automatically
- **WHEN** 8 new `dw` entries are added to the dispatch table (16 extra bytes)
- **THEN** the `DEFS 0x4000 - $` zero-fill after the table shrinks by 16 bytes
- **AND** the total binary size remains 32768 bytes
