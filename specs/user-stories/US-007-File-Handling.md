# US-007 - File Handling Support for MSXBAS2ROM

## Story

As an MSX-BASIC developer, I want to use OPEN, READ#, PRINT#, INPUT#, LINE INPUT#, MAXFILES and CLOSE commands - and also EOF(), LOC(), LOF(), FPOS(), ERR() and DSKF() functions - in my programs so that I can persist and retrieve data from files across different MSX devices (disk, cassette, memory, etc.).

## Acceptance Criteria

- The compiler supports the OPEN instruction with device, path, filename, direction, file number, and optional record length.
- Sequential file access modes (INPUT, OUTPUT, APPEND) are correctly mapped to BDOS routines.
- READ#, INPUT# and LINE INPUT# operations correctly retrieve data from an opened file.
- CLOSE properly releases the file handle and flushes buffers when needed.
- Multiple files (up to MAXFILES limit) can be opened and handled independently and when MAXFILES was set it adjust the correct memory allocation for disk operation.
- EOF() conditions are correctly detected and handled.
- ERR() returns the error code when file handling errors occurs.
- LOC(), LOF(), and FPOS() return file state information correctly.
- DSKF() function returns if the MSX DISK is functional.
- Always checks if MSX DISK is functional before every file access statement to avoid disk calls errors.
- MSXBAS2ROM programs that use file operations will have access to less free RAM memory (limited to FILTAB pointed address) than programs that do not use file operations (limited to HEAPEND address).

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

### Scenario 3b: Read full lines with LINE INPUT#

Given a file is opened for INPUT as #1
When I read data using LINE INPUT#
Then each read returns the full line content
And only CR/LF is treated as delimiter

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

### Code reference

Z80 assembly code:

- `/infrastructure/kernel/asm/src/`;
  - `20_runtime.asm`;
  - `30_basic_helpers.asm`;
  - `34_file_handling.asm`.

C++ code:

- `/application/compiler/functions/strategies/io/`;
- `/application/compiler/statements/strategies/io/`;

### Disk vs Non-Disk Modes

Concept:

- **Non-Disk Mode (Default)** — Maximum RAM available, no file I/O support.
- **Disk Mode (MSX-DOS / Disk BASIC)** — Enables file operations (`OPEN`, `INPUT#`, `PRINT#`, `CLOSE`).

The default mode is **Non-Disk Mode** when the MSX-BASIC program dont needs file access.

Implemented in `clear_basic_environment` Z80 assembly routine at `20_runtime.asm`.

### Checking Disk BASIC support

Implemented in `cmd_preflight_disk` (`34_file_handling.asm`):

- Input:
  - `A = drive number` (`0=A:`, `1=B:`, ...).
- Output:
  - `A = 0` when drive entry is valid (disk support available for that drive);
  - `A != 0` when drive entry is null (disk unavailable).
- Flow:
  - computes `DRVTBL + (drive * 2)` and reads a 16-bit entry (slot pointer/descriptor for that drive).

### MAXFILES statement

Implemented in `cmd_fmaxfiles` (`34_file_handling.asm`):

- Input:
  - `A = number of user I/O channels`.
- Flow:
  - Guard: if `A >= 16`, routine returns immediately (no changes).
  - On valid value:
    - Stores `A` into `MAXFIL`.
    - Uses `FCBBASE` as the top reference and relocates memory control variables for file support:
      - updates `HIMEM`,
      - computes/updates `FILTAB`,
      - updates `MEMSIZ`,
      - recalculates `HEAPSIZ = MEMSIZ - HEAPSTR`.
    - Builds FILTAB pointer table and initializes channel buffers:
      - each channel allocates metadata + 256-byte data area (`-(256+9+2)` and companion stride logic in routine),
      - marks each channel as closed (`0` in channel status byte),
      - updates `NULBUF` to point to channel 0 buffer payload (`start + 9`).

### DSKF function

Implemented in `cmd_fdskf` (`34_file_handling.asm`):

- Input:
  - `A = disk number` (`0=default`, `1=A:`, `2=B:`, ...).
- Output:
  - `HL = free clusters`; negative value means error.
- Flow:
  - Rejects invalid disk numbers `A >= 9`.
  - Calls `cmd_preflight_disk`; if unavailable, returns error.
  - Calls BDOS `GetLoginVector` (`C=0x18`, via `ROMBDOS`) and checks `L != 0`; if zero, returns error.
  - Calls BDOS `GetAllocationInfo` (`C=0x1B`) through `cmd_bdos_we` (error-handled wrapper).
  - On success (`A < 0x80`), returns with `HL` from BDOS (free clusters).
  - On failure, fallback error return is `HL = 0xFFxx` (`H=0xFF`, `L=A`), i.e., negative signed result.

BDOS calls actually used in `34_file_handling.asm`:

```
GetLoginVector (0x18, via `ROMBDOS`):
  input:
    C = 18H
  output:
    HL = Login vector
  note:
    can be called directly using ROMBDOS

GetAllocationInfo (0x1B, via `cmd_bdos_we`):
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
  note:
    must be called indirectly using cmd_bdos_we
```

If an error occurs, DSKF(n) should return a negative value as a result.

### OPEN statement

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

Implemented in `cmd_fopen` (`34_file_handling.asm`).

```
cmd_fopen:
  input:
    a = file number
    hl = file name address (pascal string)
    e = direction
    bc = record len
  output:
    a = 0 if successfull, otherwise error
```

BDOS calls actually used in `34_file_handling.asm`:

```
BDOS_FILEVL (0x6A11) - file name parser/device resolver (used before open).

BDOS_OPNFIL (0x6AFA) - open I/O channel with mode (input/output/random/append).

FCB scheme:

Offset  Size  Descrição
------  ----  ----------------------------------------
00h     1     Drive number (0 = default, 1 = A:, 2 = B:)
01h     8     Filename (ASCII, padded with spaces)
09h     3     Extension (ASCII, padded with spaces)
0Ch     1     Current extent (EX)
0Dh     2     Reserved
0Fh     1     Record count (RC)
10h     16    Disk map (allocation blocks)
20h     1     Current record (CR)
21h     2     Random record number (RR - low/high)
23h     2     Random record number (RR - extended)
```

### CLOSE Implementation

Parse the CLOSE command with the following structure:

```
CLOSE [#<FileNumber>[, #<FileNumber> ...]]

When no file number is provided, it means CLOSE ALL.
```

Compiler behavior (`compiler_close_statement_strategy.cpp`):

- `CLOSE #n1, #n2, ...`:
  - each expression is evaluated/cast to numeric;
  - each channel is closed individually by calling `cmd_fclose` in sequence.
- `CLOSE` (no params):
  - emits a call to `cmd_fclose` passing 0xFF as file number.

`cmd_fclose` behavior (`34_file_handling.asm`):

```
cmd_fclose:
  input:
    a = file number (0xFF = close all)
  output:
    a = 0 if successfull, otherwise error
```

BDOS calls actually used in `34_file_handling.asm`:

```
BDOS_CLSFIL (0x6B24) - close one I/O channel (CLOSE #n).

BDOS_CLSALL (0x6BE9) - close all I/O channels (CLOSE sem parametros).
```

### ERR Handling

Get the last error code from `def_ERRFLG` address.

### EOF, LOC, LOF and FPOS Handling

Get the file number passed as parameter to each function and query the active channel state.

Implemented in `cmd_feof`, `cmd_floc`, `cmd_flof`, and `cmd_fpos` (`34_file_handling.asm`).

BDOS calls actually used in `34_file_handling.asm`:

```
BDOS_EOF (0x6D25) - returns EOF status for channel.

BDOS_LOC (0x6D03) - returns bytes read (sequential) or current record (random).

BDOS_LOF (0x6D14) - returns file size in bytes.

BDOS_FPOS (0x6D39) - returns current file pointer position.
```

### INPUT#/LINE INPUT# Implementation

Implemented in `cmd_finput` (`34_file_handling.asm`).

```
cmd_finput:
  input:
    a=file number
    hl=string address (pascal style)
    e=mode (0=INPUT#, 1=LINE INPUT#)
  output:
    hl=string address (pascal style)
```

INPUT strategy behavior (`compiler_input_statement_strategy.cpp`) when a file number (#) is passed as first parameter:

- A temporary string must be passed to `cmd_finput` to be populated before each variable in the INPUT# list. The same temporary string can be used sequentially;
- The `cmd_input` output must be converted from string to the correct data type of the variable being processed in the list, and an assignment must then be performed.
- `INPUT#` calls `cmd_finput` with `e=0` (field mode):
  - delimiters are comma (`0x2C`), TAB (`0x09`), CR and LF;
  - line endings are accepted as `LF` only or `CR+LF`; when `CR+LF` is found, `LF` is consumed as part of the same terminator (no empty field in the next read);
  - if the first character is quote (`0x22`), it opens quoted mode and the value is read until the next quote or CR/LF;
  - opening/closing quotes are not copied to the output string;
  - quote characters in the middle/end of non-quoted values are treated as regular characters and copied to the output string.
- `LINE INPUT#` calls `cmd_finput` with `e=1` (line mode): delimiter is only CR/LF, with `LF` only and `CR+LF` both accepted (no quoted parsing, no TAB/comma field split).

BDOS calls actually used in `34_file_handling.asm`:

```
BDOS_SETFIL (0x6AAA) - selects active I/O channel for operation.

BDOS_INDSKC (0x6C71) - reads next character from selected file/channel.

BDOS_EOF_FLAG (0x1A) - value checked by `cmd_finput` as end-of-file marker.
```

### PRINT# Implementation

Implemented in `cmd_fprint` (`34_file_handling.asm`).

```
cmd_fprint:
  input:
    a=file number
    hl=string address (pascal style)
    e=suffix1 (i.e: 0x09=TAB or 0x0D=CR)
    d=suffix2 (i.e: 0x0A=LF)
```

PRINT strategy behavior (`compiler_print_statement_strategy.cpp`) when a file number (#) is passed as first parameter:

- All variables/constants must be converted to strings before being passed to `cmd_fprint` so that they can be processed by it;
- If one variable/constant is followed by `,`, it must pass `E=0x09` (TAB) and `D=0x00` to `cmd_fprint`;
- If the last variable/constant is not followed by `,` or `;`, it must pass `E=0x0D` and `D=0x0A` (CRLF) to `cmd_fprint`;
- In all other cases, it must pass `E=0x00` and `D=0x00`.

BDOS calls actually used in `34_file_handling.asm`:

```
BDOS_SETFIL (0x6AAA) - selects active I/O channel for operation.

BDOS_FILOUT (0x6C48) - writes one character to selected file/channel.
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
- [LOC function](<https://www.msx.org/wiki/LOC()>);
- [LOF function](<https://www.msx.org/wiki/LOF()>);
- [FPOS function](https://www.msx.org/wiki/FPOS);
- [DSKF function](<https://www.msx.org/wiki/DSKF()>);
- [ROM with disks support](https://www.msx.org/wiki/Develop_a_program_in_cartridge_ROM#Create_a_ROM_with_disks_support);
- [Using MSX DISK calls example](https://www.msx.org/forum/msx-talk/development/asm-basic-files);
- [MSX memory map](https://www.msx.org/wiki/The_Memory);
- [BDOS function calls](http://map.tni.nl/resources/dos2_functioncalls.php);
- [BDOS error handling](https://map.grauw.nl/articles/dos-error-handling.php).
