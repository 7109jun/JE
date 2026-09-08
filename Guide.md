# JE — Just Executable Guide

JE (Just Executable) is a text-based executable file format.

Its standard extension is `.jxj`, and a file can also be recognized as a JE file without an extension when the JE header is present at the very beginning of the file.

```text
[ JE : O]
```

JE uses an executable-file structure similar to PE and supports concepts such as headers, sections, entry points, architectures, subsystems, imports, exports, and relocations.

---

# 1. Getting Started with JE

## 1.1 Your First JE Program

Create a file named `hello.jxj`.

```text
[ JE : O]

AUTHOR = "YourName"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

SECTION .text {
    CODE {
        main:
            MOV R0, 42
            PRINT R0
            EXIT 0
    }
}
```

Run it:

```bash
je run hello.jxj
```

Output:

```text
42
```

---

# 2. JE File Structure

A typical JE file has the following structure:

```text
[ JE : O]

AUTHOR = "YourName"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

SECTION .text {
    CODE {
        main:
            MOV R0, 42
            PRINT R0
            EXIT 0
    }
}
```

Components:

| Component | Description                       |
| --------- | --------------------------------- |
| JE Magic  | JE file identifier                |
| AUTHOR    | Author information                |
| FORMAT    | File format                       |
| VERSION   | JE format version                 |
| ARCH      | Target CPU architecture           |
| SYSTEM    | System-related execution settings |
| SECTION   | Section                           |
| CODE      | Executable code                   |

---

# 3. JE Magic

A JE file can begin with the following string:

```text
[ JE : O]
```

It must appear exactly at the beginning of the file.

Example:

```text
[ JE : O]

AUTHOR = "Example"
```

The JE system uses this Magic value to identify JE files.

Therefore, a JE file can be recognized even without a file extension.

For example:

```text
program
```

may contain:

```text
[ JE : O]

AUTHOR = "Example"
...
```

However, using the `.jxj` extension is recommended for normal JE files.

---

# 4. Header

The JE Header defines the basic information about the JE file.

## AUTHOR

Specifies the author.

```text
AUTHOR = "7109jun"
```

## FORMAT

Specifies the file format.

```text
FORMAT = "JE"
```

## VERSION

Specifies the JE format version.

```text
VERSION = 1
```

## ARCH

Specifies the target architecture.

```text
ARCH = X86_64
```

Common architectures include:

```text
X86
X86_64
ARM
ARM64
```

---

# 5. SYSTEM

The `SYSTEM` block specifies system-related executable information.

```text
SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}
```

## ENTRY

Specifies the program entry point.

```text
ENTRY = main
```

Here, `main` is a label defined inside the CODE block.

```text
main:
    MOV R0, 42
```

## SUBSYSTEM

Specifies the type of program.

```text
SUBSYSTEM = CONSOLE
```

---

# 6. SECTION

JE uses sections.

Example:

```text
SECTION .text {
    ...
}
```

Sections can follow concepts similar to those used by traditional executable formats.

Examples:

```text
.text
.rdata
.data
.bss
```

Example:

```text
SECTION .text {
    CODE {
        main:
            MOV R0, 42
            EXIT 0
    }
}
```

---

# 7. CODE

Executable JE instructions are written inside a `CODE` block.

```text
CODE {
    main:
        MOV R0, 42
        PRINT R0
        EXIT 0
}
```

It is recommended to write one instruction per line.

---

# 8. Registers

The JE execution engine provides the following registers:

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

Each register can store an integer value.

Example:

```text
MOV R0, 10
MOV R1, 20
ADD R0, R1
PRINT R0
```

Output:

```text
30
```

---

# 9. Data Movement

## MOV

Moves a value into a register.

```text
MOV R0, 10
```

Values can also be moved between registers.

```text
MOV R1, R0
```

---

# 10. Arithmetic Operations

## ADD

```text
ADD R0, R1
```

`R0 = R0 + R1`

## SUB

```text
SUB R0, R1
```

`R0 = R0 - R1`

## INC

```text
INC R0
```

`R0 = R0 + 1`

## DEC

```text
DEC R0
```

`R0 = R0 - 1`

---

# 11. Bitwise Operations

## XOR

```text
XOR R0, R1
```

## AND

```text
AND R0, R1
```

## OR

```text
OR R0, R1
```

## SHL

```text
SHL R0, 1
```

## SHR

```text
SHR R0, 1
```

---

# 12. Comparison

## CMP

Compares two values.

```text
CMP R0, R1
```

The comparison result is used by conditional branches.

Example:

```text
CMP R0, R1
JZ equal
```

---

# 13. Conditional Branching

## JMP

Performs an unconditional jump.

```text
JMP loop
```

## JZ

Jumps when the result of a comparison or operation is zero.

```text
JZ equal
```

## JNZ

Jumps when the result is not zero.

```text
JNZ loop
```

---

# 14. Labels

A label gives a name to a position in the code.

```text
main:
    MOV R0, 10
```

The label can be referenced from another location.

```text
JMP main
```

Labels can also be used for function calls.

```text
CALL function
```

---

# 15. Stack

JE uses a value stack and a call stack.

## PUSH

```text
PUSH R0
```

## POP

```text
POP R1
```

Example:

```text
MOV R0, 123
PUSH R0
POP R1
PRINT R1
```

---

# 16. Function Calls

## CALL

```text
CALL add
```

## RET

```text
RET
```

Example:

```text
main:
    MOV R0, 10
    CALL add
    PRINT R0
    EXIT 0

add:
    INC R0
    RET
```

---

# 17. Output

## PRINT

Prints the value of a register.

```text
MOV R0, 42
PRINT R0
```

Output:

```text
42
```

---

# 18. Program Exit

## EXIT

Terminates the program.

```text
EXIT 0
```

Other exit codes can also be used.

```text
EXIT 1
```

---

# 19. Creating a Loop

A loop can be created using labels and conditional branches.

```text
main:
    MOV R0, 0

loop:
    PRINT R0
    INC R0
    CMP R0, 5
    JNZ loop

    EXIT 0
```

Conceptually:

```text
R0 = 0
↓
Print
↓
Increment R0
↓
Compare with 5
↓
Jump to loop if different
```

---

# 20. Running JE

## run

Executes JE code.

```bash
je run hello.jxj
```

This command executes the `CODE` block of the JE file using the JE execution engine.

---

## launch

Passes a file through the execution dispatch path.

```bash
je launch hello.jxj
```

PE files can also be used as execution targets.

```bash
je launch program.exe
```

JE detects the file type and selects the appropriate execution path.

---

## detect

Detects the file type.

```bash
je detect hello.jxj
```

---

## validate

Checks whether the file is recognized as JE.

```bash
je validate hello.jxj
```

For a valid JE-detectable file:

```text
valid JE
```

is displayed.

---

# 21. PE and JE

JE uses executable-file concepts similar to PE.

Major correspondences include:

| PE              | JE                          |
| --------------- | --------------------------- |
| DOS Header      | JE Header                   |
| PE Header       | JE Metadata                 |
| Optional Header | SYSTEM / execution metadata |
| Section         | SECTION                     |
| `.text`         | `.text`                     |
| Entry Point     | SYSTEM ENTRY                |
| Import          | Import metadata             |
| Export          | Export metadata             |
| Relocation      | Relocation metadata         |

JE also supports conversion between PE and JE.

---

# 22. PE → JE

Converts a PE executable into JE format.

```bash
je pe2je program.exe program.jxj
```

The converted JE file can contain data used to preserve the original PE.

For example:

```text
PE_RAW_HEX {
    ...
}
```

This allows the original PE data to be preserved in textual form.

---

# 23. JE → PE

Restores a PE file from a JE file.

```bash
je je2pe program.jxj program.exe
```

If the PE data was preserved inside the JE file, the original PE bytes can be restored.

---

# 24. Using JE on Windows

On Windows, JE is normally used as:

```text
je.exe
```

From a terminal:

```powershell
je run hello.jxj
```

or:

```powershell
je hello.jxj
```

To execute `.jxj` files directly by double-clicking them in Windows Explorer, a file association between `.jxj` and `je.exe` must be configured.

---

# 25. `.jxj` File Association

The JE project provides an association script:

```text
install_jxj_assoc.bat
```

After configuring the file association, `.jxj` files can be set up to launch directly from Windows Explorer.

---

# 26. Comments

JE code supports `#` comments.

```text
MOV R0, 42 # answer
PRINT R0
```

Comments do not affect code execution.

---

# 27. Complete Example

The following is a simple JE program.

```text
[ JE : O]

AUTHOR = "Example"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

SECTION .text {
    CODE {
        main:
            MOV R0, 1
            MOV R1, 2
            ADD R0, R1
            PRINT R0
            EXIT 0
    }
}
```

Run it:

```bash
je run example.jxj
```

Output:

```text
3
```

---

# 28. Recommended Coding Practices

The following structure is recommended for JE programs:

```text
[ JE : O]

AUTHOR = "Author"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

SECTION .text {
    CODE {
        main:
            ...
            EXIT 0
    }
}
```

Recommended practices:

* Clearly define the entry label.
* Write instructions one per line.
* Use meaningful label names.
* Clearly distinguish numbers from registers.
* Use `EXIT` at the end of the program.
* Use the `.jxj` extension for normal JE files.

---

# 29. Troubleshooting

## `valid JE` is not displayed

First, check the JE Magic:

```text
[ JE : O]
```

It must appear at the very beginning of the file.

Also make sure the file uses the `.jxj` extension when appropriate.

---

## `unknown command` is displayed

Make sure the instruction is supported.

Main supported instructions:

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
CMP
PUSH
POP
CALL
RET
JMP
JZ
JNZ
PRINT
EXIT
```

---

## The entry point cannot be found

Make sure the `ENTRY` value in `SYSTEM` matches an actual label in the CODE block.

Example:

```text
SYSTEM {
    ENTRY = main
}
```

The CODE block must contain:

```text
main:
```

---

# 30. Core JE Concept

JE is based on three core concepts:

```text
Text
+
Executable-file structure
+
Executability
```

Unlike traditional executable files that primarily contain binary data, JE represents an executable file in a human-readable text form.

Example:

```text
[ JE : O]

AUTHOR = "Example"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

SECTION .text {
    CODE {
        main:
            MOV R0, 42
            PRINT R0
            EXIT 0
    }
}
```

This file is not merely documentation. It is an executable JE file that can be interpreted by the JE execution system.

---

# 31. Summary

The simplest JE program is:

```jxj
[ JE : O]

AUTHOR = "Example"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

SECTION .text {
    CODE {
        main:
            MOV R0, 42
            PRINT R0
            EXIT 0
    }
}
```

Run it with:

```bash
je run hello.jxj
```

Output:

```text
42
```

JE uses `.jxj` as its standard executable extension, while the `[ JE : O]` Magic allows JE files to be recognized even without an extension.
