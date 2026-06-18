## ADDED Requirements

### Requirement: Support file I/O operations for MSX-BASIC programs
As an MSX-BASIC developer, the system SHALL support OPEN, READ#, PRINT#, INPUT#, LINE INPUT#, MAXFILES, and CLOSE commands, plus EOF(), LOC(), LOF(), FPOS(), ERR(), and DSKF() functions so that programs can persist and retrieve data from files across MSX devices (disk, cassette, memory, etc.).

The compiler SHALL support the OPEN instruction with device, path, filename, direction, file number, and optional record length. Sequential file access modes (INPUT, OUTPUT, APPEND) SHALL be correctly mapped to BDOS routines. READ#, INPUT#, and LINE INPUT# operations SHALL correctly retrieve data from an opened file. CLOSE SHALL properly release the file handle and flush buffers when needed. Multiple files (up to MAXFILES limit) SHALL be opened and handled independently; when MAXFILES is set, it SHALL adjust the correct memory allocation for disk operation. EOF() conditions SHALL be correctly detected and handled. ERR() SHALL return the error code when file handling errors occur. LOC(), LOF(), and FPOS() SHALL return file state information correctly. DSKF() SHALL return whether the MSX DISK is functional. The system SHALL always check if MSX DISK is functional before every file access statement. Programs using file operations SHALL have access to less free RAM (limited to FILTAB pointed address) than non-file programs (limited to HEAPEND address).

#### Scenario: Open a file for writing and save data
- **WHEN** a program opens `"A:TEST.TXT" FOR OUTPUT AS #1`, writes data using PRINT #1, and closes using CLOSE
- **THEN** the file TEST.TXT is created on disk with the written content persisted correctly

#### Scenario: Open a file for reading and retrieve data
- **WHEN** a file TEST.TXT exists and a program opens it `FOR INPUT AS #1` and reads using INPUT#
- **THEN** the content is correctly loaded into variables until EOF is reached

#### Scenario: Detect end-of-file during reading
- **WHEN** a file is opened for INPUT as #1 and data is repeatedly read
- **THEN** the program detects EOF correctly without crashing or reading invalid data

#### Scenario: Read full lines with LINE INPUT#
- **WHEN** a file is opened for INPUT as #1 and data is read using LINE INPUT#
- **THEN** each read returns the full line content with only CR/LF as delimiter

#### Scenario: Append data to an existing file
- **WHEN** an existing file is opened `FOR APPEND AS #1` and additional content is written
- **THEN** the new content is appended to the end without altering existing content

#### Scenario: Handle multiple open files independently
- **WHEN** two files are opened as #1 and #2 and operations are performed on both
- **THEN** each file maintains its own independent state without interference

#### Scenario: Close a file and release resources
- **WHEN** a file opened as #1 is closed via CLOSE #1
- **THEN** the file is properly closed and the handle becomes available for reuse

### Technical Specification — File I/O

**Code reference:**
- Z80: `src/infrastructure/kernel/asm/src/34_file_handling.asm`, `20_runtime.asm`, `30_basic_helpers.asm`
- C++: `src/application/compiler/functions/strategies/io/`, `src/application/compiler/statements/strategies/io/`

**Disk vs Non-Disk Modes:**
- Non-Disk Mode (Default): Maximum RAM, no file I/O support. HIMEM=0xF380.
- Disk Mode: Enables file operations. HIMEM=0xF1C9, MEMSIZ adjusted, DSKDIS enabled.
- Implemented in `clear_basic_environment` at `20_runtime.asm`.

**Checking Disk BASIC support (`cmd_preflight_disk`):**
- Input: A = drive number (0=A:, 1=B:, ...)
- Output: A = 0 when drive entry valid, A != 0 when unavailable
- Flow: computes `DRVTBL + (drive * 2)` and reads 16-bit slot pointer

**MAXFILES statement (`cmd_fmaxfiles`):**
- Input: A = number of user I/O channels
- Guard: if A >= 16, return immediately
- Stores A into MAXFIL, relocates memory: updates HIMEM, FILTAB, MEMSIZ, recalculates HEAPSIZ
- Builds FILTAB pointer table, initializes channel buffers (256+9+2 bytes each), updates NULBUF

**DSKF function (`cmd_fdskf`):**
- Input: A = disk number (0=default, 1=A:, 2=B:, ...)
- Output: HL = free clusters; negative value means error
- Rejects A >= 9, calls `cmd_preflight_disk`, then BDOS GetLoginVector (0x18) and GetAllocationInfo (0x1B)

**OPEN statement (`cmd_fopen`):**
```
OPEN "<Device>:<Path><Filename>" FOR <Direction> AS #<FileNumber> LEN=<RecordLength>
```

Parameter rules:
- `<Path>` specifies folder location; each folder name separated by backslash. Only available in version 2 of Disk BASIC.
- `<Filename>`: cassette format limited to 6 characters without extension; disk format uses 8 characters + dot + 3-character extension.
- `<Direction>`: `OUTPUT` (sequential write), `INPUT` (sequential read), `APPEND` (sequential write to end of existing file). Not required for text screen, graphic screen, or printer. To open a file with random access on disk, skip the `[FOR <direction>]` parameter.
- `<FileNumber>`: number between 1 and 15, but cannot exceed MAXFILES. The `#` prefix is optional.
- `<RecordLength>`: preceded by LEN, optional, for random access. Default is 256. If FIELD definitions exceed record length, "Field overflow" error is generated. Reading/writing is always done whole record at a time.

- Input: A = file number, HL = file name (pascal string), E = direction, BC = record len
- Output: A = 0 if successful, otherwise error
- BDOS calls: BDOS_FILEVL (0x6A11), BDOS_OPNFIL (0x6AFA)

FCB scheme:

| Offset | Size | Description |
|--------|------|-------------|
| 0x00   | 1    | Drive number (0=default, 1=A:, 2=B:) |
| 0x01   | 8    | Filename (ASCII, padded with spaces) |
| 0x09   | 3    | Extension (ASCII, padded with spaces) |
| 0x0C   | 1    | Current extent (EX) |
| 0x0D   | 2    | Reserved |
| 0x0F   | 1    | Record count (RC) |
| 0x10   | 16   | Disk map (allocation blocks) |
| 0x20   | 1    | Current record (CR) |
| 0x21   | 2    | Random record number (RR — low/high) |
| 0x23   | 2    | Random record number (RR — extended) |

**CLOSE Implementation:**
- `CLOSE #n1, #n2, ...`: each channel closed individually via `cmd_fclose` calling BDOS_CLSFIL (0x6B24)
- `CLOSE` (no params): emits `cmd_fclose` with 0xFF, calling BDOS_CLSALL (0x6BE9)

**INPUT#/LINE INPUT# (`cmd_finput`):**
- Input: A = file number, HL = string address (pascal), E = mode (0=INPUT#, 1=LINE INPUT#)
- INPUT# (field mode): delimiters are comma, TAB, CR, LF; quoted mode with double quotes
- LINE INPUT# (line mode): delimiter is only CR/LF
- BDOS calls: BDOS_SETFIL (0x6AAA), BDOS_INDSKC (0x6C71), BDOS_EOF_FLAG (0x1A)

**PRINT# (`cmd_fprint`):**
- Input: A = file number, HL = string (pascal), E = suffix1, D = suffix2
- Comma separator → E=0x09 (TAB); last item without `,` or `;` → E=0x0D, D=0x0A (CRLF)
- BDOS calls: BDOS_SETFIL (0x6AAA), BDOS_FILOUT (0x6C48)

**EOF, LOC, LOF, FPOS:**
- `cmd_feof`: BDOS_EOF (0x6D25)
- `cmd_floc`: BDOS_LOC (0x6D03)
- `cmd_flof`: BDOS_LOF (0x6D14)
- `cmd_fpos`: BDOS_FPOS (0x6D39)

**ERR:**
- Get last error code from `def_ERRFLG` address.

### References
- [OPEN statement](https://www.msx.org/wiki/OPEN)
- [CLOSE statement](https://www.msx.org/wiki/CLOSE)
- [READ statement](https://www.msx.org/wiki/READ)
- [PRINT statement](https://www.msx.org/wiki/PRINT)
- [INPUT statement](https://www.msx.org/wiki/INPUT)
- [MAXFILES statement](https://www.msx.org/wiki/MAXFILES)
- [EOF function](<https://www.msx.org/wiki/EOF()>)
- [LOC function](<https://www.msx.org/wiki/LOC()>)
- [LOF function](<https://www.msx.org/wiki/LOF()>)
- [FPOS function](https://www.msx.org/wiki/FPOS)
- [DSKF function](<https://www.msx.org/wiki/DSKF()>)
- [ROM with disks support](https://www.msx.org/wiki/Develop_a_program_in_cartridge_ROM#Create_a_ROM_with_disks_support)
- [Using MSX DISK calls example](https://www.msx.org/forum/msx-talk/development/asm-basic-files)
- [MSX memory map](https://www.msx.org/wiki/The_Memory)
- [BDOS function calls](http://map.tni.nl/resources/dos2_functioncalls.php)
- [BDOS error handling](https://map.grauw.nl/articles/dos-error-handling.php)
