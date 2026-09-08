cmake_minimum_required(VERSION 3.20)
project(JE VERSION 1.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(je_core
    src/core/je.c
    src/core/je_log.c
    src/core/je_error.c
    src/core/je_text.c
    src/pe/pe_parser.c
    src/pe/pe_writer.c
    src/pe/pe_sections.c
    src/pe/pe_imports.c
    src/pe/pe_exports.c
    src/pe/pe_reloc.c
    src/memory/je_memory.c
    src/memory/je_alloc.c
    src/io/je_file.c
    src/io/je_stream.c
    src/io/je_textio.c
    src/win/je_winapi.c
    src/win/je_process.c
    src/loader/je_loader.c
    src/loader/je_dispatch.c
    src/loader/je_exec.c
    src/loader/je_runtime.c
    src/convert/pe_to_je.c
    src/convert/je_to_pe.c
    src/convert/je_roundtrip.c
)

target_include_directories(je_core PUBLIC include)

if (WIN32)
    if (NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "JE Windows builds require MinGW-w64 GCC. Use the MinGW Makefiles generator or a MinGW-w64 toolchain.")
    endif()

    if (NOT MINGW)
        message(FATAL_ERROR "JE Windows builds require MinGW-w64 (MINGW=TRUE was not detected).")
    endif()

    target_compile_definitions(je_core PRIVATE
        JE_PLATFORM_WINDOWS=1
        JE_TOOLCHAIN_MINGW=1
    )

endif()

add_executable(je src/cli/main.c src/cli/commands.c)
target_link_libraries(je PRIVATE je_core)
target_include_directories(je PRIVATE include)

add_executable(je_smoke tests/smoke.c)
target_link_libraries(je_smoke PRIVATE je_core)
target_include_directories(je_smoke PRIVATE include)

if (WIN32)
    target_link_options(je PRIVATE -static-libgcc)
    target_link_options(je_smoke PRIVATE -static-libgcc)
endif()

enable_testing()
add_test(NAME je_smoke COMMAND je_smoke)
