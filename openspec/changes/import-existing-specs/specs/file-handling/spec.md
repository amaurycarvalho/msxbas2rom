## ADDED Requirements

### Requirement: Support file I/O operations
The system SHALL support OPEN, READ#, PRINT#, INPUT#, LINE INPUT#, MAXFILES, and CLOSE commands, plus EOF(), LOC(), LOF(), FPOS(), ERR(), and DSKF() functions so that programs can persist and retrieve data from files across MSX devices.

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
