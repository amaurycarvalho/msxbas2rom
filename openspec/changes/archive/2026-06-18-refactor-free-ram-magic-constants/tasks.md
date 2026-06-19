## 1. Add Constants to `compiler_hooks.h`

- [x] 1.1 Add `def_VAR_RAM_NON_DISK` (10534) with documentation explaining it represents free variable RAM without file I/O
- [x] 1.2 Add `def_VAR_RAM_DISK` (5931) with documentation explaining it represents free variable RAM with file I/O
- [x] 1.3 Add `def_KERNEL_RAM_OVERHEAD` (0x538) with documentation explaining it covers kernel internal structures above HEAPEND

## 2. Refactor `compiler.cpp`

- [x] 2.1 Replace inline magic constants in `compiler.cpp:314-319` with the named constants from `compiler_hooks.h`
- [x] 2.2 Remove the `//! @todo needs refactor the code below to eliminate its magic constants` comment since the refactor is complete
- [x] 2.3 Verify the build compiles cleanly with `make`
- [x] 2.4 Run the test suite to confirm zero behavioral change
