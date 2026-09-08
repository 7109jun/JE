# JE 1.1 — MinGW-w64 Build

JE 1.1 uses MinGW-w64 GCC as the Windows build toolchain.

## Requirements

- Windows 10/11
- CMake 3.20+
- MinGW-w64 GCC
- `gcc` in `PATH`
- `mingw32-make` in `PATH`

## Build

Run:

```bat
build_windows.bat
```

The build uses the CMake `MinGW Makefiles` generator and produces:

```text
build-windows\je.exe
```

The Windows build is intentionally rejected when the C compiler is not MinGW-w64 GCC.

## Runtime ABI

JE runtime imports use the host C runtime through GCC/MinGW-w64:

- `malloc`
- `calloc`
- `realloc`
- `free`
- `memcpy`
- `memset`
- `fopen`
- `fread`
- `fwrite`
- `fclose`
- `fseek`
- `fflush`

Windows native process execution uses the Win32 API from MinGW-w64 headers and libraries.
