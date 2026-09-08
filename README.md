# JE — Just Executable

**JE (Just Executable)** is a new executable file format designed to be represented as text.

Unlike conventional binary executable formats, a JE file is **text-based**, and its contents can be executed through the JE runtime.

The standard file extension is `.jxj`.

A JE file can also be recognized without an extension when the beginning of the file contains:

```text
[ JE : O]
```

## Features

* Text-based executable file format
* `.jxj` extension support
* Extensionless JE detection using the JE Magic header
* PE-like executable structure
* x86 / x86-64 PE analysis support
* PE → JE conversion
* JE → PE conversion
* JE code execution engine
* Register-based instructions
* `CALL`, `RET`, and conditional branching
* Windows process execution bridge
* Bounds checking for malformed input
* ASan / UBSan stability testing
* C-based core implementation with a Rust safety layer

## Simple Example

```text
[ JE : O]

AUTHOR = "JE"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
}

CODE {
    main:
        MOV R0, 40
        PUSH R0
        POP R1
        INC R1
        CALL sum_two
        PRINT R0
        EXIT 0

    sum_two:
        ADD R1, 1
        MOV R0, R1
        RET
}
```

Output:

```text
42
```

## Instructions

The JE execution engine provides instructions such as:

```text
MOV
ADD
SUB
XOR
AND
OR
SHL
SHR

INC
DEC

PUSH
POP

CMP

CALL
RET

JMP
JZ
JNZ

PRINT
EXIT
```

Available registers:

```text
R0
R1
R2
R3
R4
R5
R6
R7
```

## CLI

```text
je detect <file>
je validate <file>
je run <file>
je launch <file>

je pe2je <input.exe> <output.jxj>
je je2pe <input.jxj> <output.exe>
```

Example:

```bash
je validate hello.jxj
je run hello.jxj
```

## PE ↔ JE

JE uses an executable structure similar to PE and supports conversion between PE and JE files.

```text
PE → JE

example.exe
     ↓
  pe2je
     ↓
example.jxj
```

And the reverse:

```text
JE → PE

example.jxj
     ↓
  je2pe
     ↓
example.exe
```

The original PE data can be preserved inside the JE representation so that the original PE can be reconstructed after conversion.

## Project Structure

The core implementation is primarily written in C.

```text
include/
src/
    core/
    pe/
    convert/
    loader/
    win/
    io/
    memory/
    cli/
rust/
tests/
tools/
examples/
```

Windows process execution uses Win32 APIs such as:

```text
CreateProcessA
WaitForSingleObject
GetExitCodeProcess
```

## Building

JE uses CMake.

```bash
cmake -S . -B build
cmake --build build
```

Run the tests with:

```bash
ctest --test-dir build --output-on-failure
```

## Current Verification Status

The project currently verifies:

```text
CMake build              PASS
Strict compiler warnings  PASS
ASan                     PASS
UBSan                    PASS
CTest                    PASS
JE parsing               PASS
JE execution             PASS
PE parsing               PASS
PE ↔ JE round-trip        PASS
Windows API bridge       PASS
```

Actual `CreateProcessA()` execution should be verified on a Windows environment.

## Goal

JE is intended to be more than a text file. Its goal is to provide an executable file format that can be:

**"Written as text, executed directly through JE, and converted to and from PE."**

JE aims to preserve familiar concepts from existing executable formats while combining them with a text-based representation and execution environment.

## License

See the license files in this repository for the project's licensing policy.
