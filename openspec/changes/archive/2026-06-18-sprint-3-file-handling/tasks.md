## 1. Kernel Assembly

- [x] 1.1 Implement cmd_preflight_disk for drive availability check
- [x] 1.2 Implement cmd_fopen with BDOS_OPNFIL
- [x] 1.3 Implement cmd_fclose with BDOS_CLSFIL/BDOS_CLSALL
- [x] 1.4 Implement cmd_finput with BDOS_INDSKC
- [x] 1.5 Implement cmd_fprint with BDOS_FILOUT
- [x] 1.6 Implement cmd_feof, cmd_floc, cmd_flof, cmd_fpos
- [x] 1.7 Implement cmd_fdskf with BDOS GetAllocationInfo
- [x] 1.8 Implement cmd_fmaxfiles with memory relocation

## 2. Compiler Strategies

- [x] 2.1 Implement OPEN statement strategy
- [x] 2.2 Implement CLOSE statement strategy
- [x] 2.3 Implement PRINT# statement strategy
- [x] 2.4 Implement INPUT#/LINE INPUT# statement strategy
- [x] 2.5 Implement EOF, LOC, LOF, FPOS, ERR, DSKF function strategies

## 3. Memory Management

- [x] 3.1 Implement disk/non-disk memory profile switching
- [x] 3.2 Implement MAXFILES buffer allocation
