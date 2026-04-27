# US-007 - File Handling Support for MSXBAS2ROM

## Story

As an MSX-BASIC developer, I want to use OPEN, READ#, PRINT#, INPUT#, MAXFILES and CLOSE commands - and also EOF() and DSKF() functions - in my programs so that I can persist and retrieve data from files across different MSX devices (disk, cassette, memory, etc.).

## Acceptance Criteria

- The compiler supports the OPEN instruction with device, path, filename, direction, file number, and optional record length.
- Sequential file access modes (INPUT, OUTPUT, APPEND) are correctly mapped to MSX BIOS routines.
- READ# and INPUT# operations correctly retrieve data from an opened file.
- CLOSE properly releases the file handle and flushes buffers when needed.
- Multiple files (up to MAXFILES limit) can be opened and handled independently and when MAXFILES was set it adjust the correct memory allocation for disk operation.
- EOF() conditions are correctly detected and handled.
- DSKF() function returns if the MSX DISK is functional.
- Always checks DSKF for default disk (A) before every file access statement to avoid disk calls errors.
- MSXBAS2ROM programs that use file operations use RAM memory until F1C9h address for stack and variables.
- Existing MSXBAS2ROM programs that do not use file operations remain unaffected and use RAM memory until F380H address for stack and variables.

---

## BDD Scenarios

### Scenario 1: Open a file for writing and save data

Given I have a BASIC program that opens a file using OPEN "A:TEST.TXT"
FOR OUTPUT AS #1
When the program writes data using PRINT #1
And closes the file using CLOSE
Then the file TEST.TXT is created on disk
And the written content is persisted correctly

### Scenario 2: Open a file for reading and retrieve data

Given a file TEST.TXT exists on disk
And my BASIC program opens it using OPEN "A:TEST.TXT" FOR INPUT AS #1
When I read data using INPUT#
Then the content is correctly loaded into variables
And reading continues until EOF is reached

### Scenario 3: Detect end-of-file during reading

Given a file is opened for INPUT as #1
When I repeatedly read data using INPUT#
Then the program detects EOF correctly
And stops reading without crashing or reading invalid data

### Scenario 4: Append data to an existing file

Given a file TEST.TXT already exists
When I open it using OPEN "A:TEST.TXT" FOR APPEND AS #1
And write additional content
Then the new content is appended to the end of the file
And existing content remains unchanged

### Scenario 5: Handle multiple open files

Given I open two files using OPEN ... AS #1 and OPEN ... AS #2
When I write and read from both files
Then each file maintains its own independent state
And operations do not interfere with each other

### Scenario 6: Close a file and release resources

Given a file is opened as #1
When I execute CLOSE #1
Then the file is properly closed
And the file handle becomes available for reuse

---

## Technical Specification

### Parsing and Compilation

Parse the OPEN command with the following structure:

```
OPEN "<Device>:<Path><Filename>" FOR <Direction> AS #<FileNumber> LEN=<RecordLength>

<Path> is used to specify the location in folders of file to load. Each folder name in path are separate by a backslash (\). This parameter is only available in version 2 of Disk BASIC.

<Filename> is the name of the file to be opened. This parameter is not required for text screen, graphic screen and printer.

The format of file name is limited to 6 characters without extension, when using cassette. If another device is used, then the format is 8 characters followed by a point and an extension with 3 characters.

<Direction> must be preceded by FOR to be used. It is one of the 3 types of sequential access:
OUTPUT = sequential write
INPUT = sequential read
APPEND = sequential write to the end of an existing file

This parameter is not required for text screen, graphic screen and printer, but the listing is more easy to read when you use it. To open a file with random access on disk, you need to skip the [FOR <direction>] parameter.

<FileNumber> is a number that you must assign to open the file. It can vary between 1 and 15, but can't exceed the maximum number of files eventually defined with MAXFILES. The # in front can be omitted.

<RecordLength> must be preceded by LEN to be used. It is an optional parameter to use only if you choose to open a file in random access on disk. It must be an integer. The default value is 256. In case FIELD definitions exceed record length "Field overflow" error is generated. Reading/writing is always done whole record at a time.
```

Notes:

- String for filename in MSX-DISK BIOS calls should be null-terminated.
- LEN parameter is only required for RANDOM access (future extension).

### MSX-DISK relevant assembly constants

```
FILEVL          EQU #6A0E
OPNFIL          EQU #6AFA
CLSFIL          EQU #6B24
INDSKC          EQU #6C71 ;(Undocumented)
OUTDO           EQU #18

INPUT_FILE      EQU 1
OUTPUT_FILE     EQU 2
RANDOM_FILE     EQU 4
APPEND_FILE     EQU 8
RECORD_SIZE     EQU #F33D ; Only needed if RANDOM_FILE is used

EOF             EQU #1A
```

### Memory mapping relevant addresses

```
HIMEM (FC4AH): Address of the highest byte available to BASIC (below the system work area).
MEMSIZ (F672H): address of the top of the usable RAM for the BASIC interpreter (set by CHKRAM boot routine).
STKTOP (F674h): top location to be used for the stack.
BOTTOM (FC48H): Address of the start of RAM available to BASIC.
NLONLY (F87CH): loading basic program flags (bit 0=not close i/o buffer 0, bit 7=not close user i/o buffer)

F380H - FD99H: System Work Area containing variables for the BIOS and Disk system.
FD9AH - FFCAH: RAM Hook Area, used by the disk interface and RS-232 to intercept system calls.
F30FH (approx): In machines with a disk drive, the system variables often start lower (around F30Fh) to accommodate the extra Disk BIOS variables.

F1C9H TO F380H = fixed area for disks communication

F1E2H (6 bytes) – Routine to abort the program in case of error.
F273H (3 bytes) – Disk access error handling routine.
F302H (2 bytes) – Pointer to the abort routine handler for MSXDOS.
F304H (2 bytes) – Stores the value of the SP (Stack Pointer) register.
F323H (2 bytes) – Address of the disk error handler (pointer to pointer).
F325H (2 bytes) – Address of the abort error handler (pointer to pointer).
F37DH (3 bytes) – ROMBDOS Jump to the BDOS command handler.
```

### OPEN Implementation

1. Load filename (ASCIIZ) into HL
2. Call FILEVL
3. Set:

- A = file number (channel)
- E = mode (INPUT_FILE, OUTPUT_FILE, etc.)

4. Call OPNFIL

Example:

```asm
LD HL,NAME
CALL FILEVL

LD A,1              ; File number
LD E,OUTPUT_FILE    ; Mode
CALL OPNFIL
```

### READ/INPUT Implementation

1. Use INDSKC to read byte-by-byte
2. Compare with EOF (#1A)
3. Store into buffer until EOF

Example:

```
CALL INDSKC
CP EOF
RET Z
LD (HL),A
INC HL
JP READ
```

### WRITE Implementation

1. Load byte from buffer
2. Output using RST OUTDO

Example:

```
LD A,(HL)
INC HL
AND A
RET Z
RST OUTDO
JP WRITE
```

### CLOSE Implementation

1. Set file number in A
2. Call CLSFIL

Example:

```
LD A,1
CALL CLSFIL
```

### EOF Handling

1. EOF is detected when INDSKC returns #1A
2. Compiler must generate loop guards for INPUT# constructs

### Checking MSX DISK presence and DSKF function

First, DSKF(n) function must check DRVTBL (drive table) located at address 0xFB21 that lists the initialized drives.

- Start Address: 0xFB21
- Size: 2 bytes per entry (16-bit pointers).
- Total Size: Typically 16 bytes (8 drives x 2 bytes).
- DSKF n parameter is 0 for default drive (A), 1 for drive B, 2 for drive C...

Each 2-byte entry at DRVTBL + (drive_number \* 2) does not usually point to raw data, but rather to a Device Driver Structure in the ROM of the disk interface.

If the entry is 0: No drive is assigned/detected for that drive number. If the entry is non-zero: It is an address (pointer) in a ROM slot that tells the BIOS how to communicate with that drive.

Next, if disk is available, DSKF(n) must call BDOS 0x1B function (GetAllocationInfo) to return the number of free clusters on the disk inserted in the specified drive.

```
GetAllocationInfo (0x1B):
  input:
    C = 0x1B
    E = drive number
  output:
    A = Sectors per cluster (255 if error)
    BC = sector size (bytes)
    DE = total clusters on disk
    HL = free clusters on disk
    IX = DPB address
    IY = FAT address

BDOS (0xF37D)
```

If an error occurs, DSKF(n) should return -1 as a result.

Example:

```
ld e, n
ld c, GetAllocationInfo
CALL BDOS
```

### Runtime Memory Configuration (Disk vs Non-Disk Modes)

This section defines the required changes to the MSXBAS2ROM runtime initialization code (`src/infrastructure/kernel/asm/header.asm`) in order to properly support two execution modes:

- **Non-Disk Mode (Default)** — Maximum RAM available, no file I/O support.
- **Disk Mode (MSX-DOS / Disk BASIC)** — Enables file operations (`OPEN`, `INPUT#`, `PRINT#`, `CLOSE`).

The current implementation forces **Non-Disk Mode**, which prevents file handling and assumes a higher memory limit (`0xF380`). This specification introduces a dual-mode configuration with correct memory boundaries and BASIC environment setup, which must be take in consideration when the compiled MSX-BASIC program needs file access.

Impacted variables:

- Disk availability (DSKDIS)
- Memory limits (HIMEM)
- File channels (MAXFIL)
- Stack positioning (STKTOP)

Required Changes:

1. Disk Enable/Disable Control

```
Non-Disk Mode:
  ld a, 0xFF
  ld (DSKDIS), a

Disk Mode:
  xor a
  ld (DSKDIS), a
```

IMPORTANT: Do not overwrite DSKDIS in Disk Mode after DOS initialization.

2. HIMEM / MEMSIZ Adjustment

```
Non-Disk Mode:
  ld hl, 0xF380
  ld (HIMEM), hl
  ld (MEMSIZ), hl

Disk Mode:
  ld hl, 0xF1C9
  ld (HIMEM), hl
  ld (MEMSIZ), hl
```

3. MAXFILES Configuration

```
Non-Disk Mode:
  xor a
  ld (MAXFIL), a

Disk Mode:
  ld a, 1            ; default for MAXFILES
  ld (MAXFIL), a
```

4. Stack Top (STKTOP) Adjustment

```
Current implementation:
  ld bc, 200
  sbc hl, bc
  ld (STKTOP), hl

Required Change:
  Compute stack based on HIMEM.
    Safe Formula:
    STKTOP = HIMEM - StackMargin

Example Implementation:
  ld hl, (HIMEM)
  ld bc, 256         ; safer margin than 200
  or a               ; clear carry
  sbc hl, bc
  ld (STKTOP), hl
```

Disk Mode requires a larger safety margin due to DOS activity.

5. Variable Area Protection (VARTAB / BASMEM)

```
Current:
  ld hl, BASMEM
  ld (VARTAB), hl

Requirement:
  Ensure: BASMEM < HIMEM
```

Recommendation:

- No code change required if BASMEM remains in page 3 (>= C000h), but must be validated by compiler.

6. Avoid DOS Memory Region Overlap

```
Reserved Region (Disk Mode)
  F1C9h → F380h
```

Must NOT be used for:

- Variables
- Stack
- Buffers
- Custom work areas

7. File System Readiness

```
To enable BASIC file operations:
  DSKDIS = 0
  MAXFIL > 0
  BASIC environment must not be in "I/O disabled" state
```

8. NLONLY (at address F87CH).

It is a MSX Disk BASIC system work area variable used by the disk-operating system to manage how directory listings are displayed, specifically regarding newline characters and formatting. This system flag is used for controlling newline handling during file operations, often used to toggle ASCII text mode features.

Bit 0 typically manages the auto-insertion of LF (Line Feed) after CR (Carriage Return), while Bit 7 often indicates if a file is being treated specifically as text/sequential.

- Bit 0 for Line Feed Insertion flag: When set, this bit usually instructs Disk BASIC to add a LF automatically after every CR during disk output, ensuring compatibility with standard text files (CRLF).
- Bit 7 for Text/Sequential Mode Toggle: Setting this bit often forces the file access to act in a "newline-only" mode, commonly used to ensure file I/O operations specifically treat ASCII data and do not add unwanted binary formatting, useful for reading/writing text files.

```
ld (NLONLY), 0     ; reset i/o buffers
```

---

## References

- [OPEN statement](https://www.msx.org/wiki/OPEN);
- [CLOSE statement](https://www.msx.org/wiki/CLOSE);
- [READ statement](https://www.msx.org/wiki/READ);
- [PRINT statement](https://www.msx.org/wiki/PRINT);
- [INPUT statement](https://www.msx.org/wiki/INPUT);
- [MAXFILES statement](https://www.msx.org/wiki/MAXFILES);
- [EOF function](<https://www.msx.org/wiki/EOF()>);
- [DSKF function](<https://www.msx.org/wiki/DSKF()>);
- [ROM with disks support](https://www.msx.org/wiki/Develop_a_program_in_cartridge_ROM#Create_a_ROM_with_disks_support);
- [Using MSX DISK calls example](https://www.msx.org/forum/msx-talk/development/asm-basic-files);
- [MSX memory map](https://www.msx.org/wiki/The_Memory);
- [BDOS function calls](http://map.tni.nl/resources/dos2_functioncalls.php);
- [BDOS error handling](https://map.grauw.nl/articles/dos-error-handling.php).
