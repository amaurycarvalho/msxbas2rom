# US-007 - File Handling Support for MSXBAS2ROM

## Story

As an MSX-BASIC developer, I want to use OPEN, READ#, PRINT#, INPUT#, MAXFILES and CLOSE commands - and also EOF() and DSKF() functions - in my programs so that I can persist and retrieve data from files across different MSX devices (disk, cassette, memory, etc.).

## Acceptance Criteria

- The compiler supports the OPEN instruction with device, path, filename, direction, file number, and optional record length.
- Sequential file access modes (INPUT, OUTPUT, APPEND) are correctly mapped to BDOS routines.
- READ# and INPUT# operations correctly retrieve data from an opened file.
- CLOSE properly releases the file handle and flushes buffers when needed.
- Multiple files (up to MAXFILES limit) can be opened and handled independently and when MAXFILES was set it adjust the correct memory allocation for disk operation.
- EOF() conditions are correctly detected and handled.
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

BDOS (0xF37D) routines associated:

```
GetLoginVector (0x18):
  input:
    C = 18H
  output:
    HL = Login vector
  note:
    can be called directly using ROMBDOS

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

BDOS (0xF37D) routines associated:

```
FILTAB (0xF860) - address of the pointer table for the I/O buffer FCBs
FCBBASE (0xF353) - File Control Block base

OpenFile (0x0F):
  input:
    C = 0FH
    DE = Pointer to unopened FCB
  output:
    L=A = 0FFH if file not found
        =   0  if file is found
  note:
    must be called indirectly using cmd_bdos_we

CreateFile (0x16):
  input:
    C = 16H
    DE = Pointer to unopened FCB
  output:
    L=A = 0FFH if unsuccessful
        =   0  if successful

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
  - emits a call to `cmd_fclose` for each opened FCB in the FCB list.

`cmd_fclose` behavior (`34_file_handling.asm`):

```
cmd_fclose:
  input:
    a = file number
  output:
    a = 0 if successfull, otherwise error
```

BDOS (0xF37D) routines associated:

```
CloseFile (0x10)
  input:
    C = 10H
    DE = Pointer to opened FCB
  output:
    L=A = 0FFH if not successful
        =   0  if successful
```

### EOF Handling

Get the file number passed as parameter to the EOF(n) function and checks the corresponding FCB if exists data to read in the file or not.

BDOS (0xF37D) routines associated:

```
HEOF (0xFEA3)
  input:
    hl = FCB address
  output:
    DAC+2 = true or false (integer)
```

### READ#/INPUT# Implementation

BDOS (0xF37D) routines associated:

```
SequentialReadFile (0x14)
  input:
    C = 14H
    DE = Pointer to opened FCB
  output:
    L=A = 01H if error (end of file)
        =  0  if read was successful

RandomReadFile (0x21)
  input:
    C = 21H
    DE = Pointer to opened FCB
  output:
    L=A = 01H if error (end of file)
        =  0  if read was successful

SetDiskTransferAddress (0x1A)
  input:
    C = 1AH
    DE = Required Disk Transfer Address
  output:
    None

SetRandomRecordFile (0x24)
  input:
    C = 24H
    DE = Pointer to opened FCB
  output:
    None
```

### PRINT# Implementation

BDOS (0xF37D) routines associated:

```
SequentialWriteFile (0x15)
  input:
    C = 15H
    DE = Pointer to opened FCB
  output:
    L=A = 01H if error (disk full)
        =  0  if write was successful

RandomWriteFile (0x22)
  input:
    C = 22H
    DE = Pointer to opened FCB
  output:
    L=A = 01H if error (disk full)
        =  0  if no error
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
