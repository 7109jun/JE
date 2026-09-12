#include "te_parser.h"
#include "te_loader.h"
#include <iostream>
#include <fstream>

using namespace TE;

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <command> [args]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  run <file.yac>        Run a TE executable file\n";
    std::cout << "  parse <file.yac>      Parse and display a TE file\n";
    std::cout << "  validate <file.yac>   Validate a TE file\n";
    std::cout << "  create <name>         Create a sample TE file\n";
    std::cout << "  demo                  Run demo\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << " run hello_world.yac\n";
    std::cout << "  " << program_name << " parse hello_world.yac\n";
    std::cout << "  " << program_name << " validate hello_world.yac\n";
}

void run_command(const std::string& filename) {
    std::cout << "\n╔═══════════════════════════════════════════╗\n";
    std::cout << "║        TE Executable Runtime (RUN)        ║\n";
    std::cout << "╚═══════════════════════════════════════════╝\n\n";

    TELoader loader;
    int exit_code = loader.load_and_execute(filename);
    
    std::cout << "Program exited with code: " << exit_code << "\n";
}

void parse_command(const std::string& filename) {
    std::cout << "\n╔═══════════════════════════════════════════╗\n";
    std::cout << "║        TE File Parser (PARSE)             ║\n";
    std::cout << "╚═══════════════════════════════════════════╝\n\n";

    TEParser parser;
    auto result = parser.parse_file(filename);

    if (!result) {
        std::cerr << "✗ Parse Error: " << parser.get_error() << "\n";
        return;
    }

    auto te = result.value();

    // Metadata
    std::cout << "═════════════════ METADATA ═════════════════\n";
    std::cout << "Name:         " << te.metadata.name << "\n";
    std::cout << "Version:      " << te.metadata.version << "\n";
    std::cout << "Entry Point:  " << te.metadata.entry_point << "\n";
    std::cout << "Target OS:    ";
    for (const auto& os : te.metadata.target_os) {
        std::cout << os << " ";
    }
    std::cout << "\n";
    std::cout << "Architecture: ";
    for (const auto& arch : te.metadata.architecture) {
        std::cout << arch << " ";
    }
    std::cout << "\n";
    if (!te.metadata.author.empty()) {
        std::cout << "Author:       " << te.metadata.author << "\n";
    }
    if (!te.metadata.description.empty()) {
        std::cout << "Description:  " << te.metadata.description << "\n";
    }

    // Imports
    std::cout << "\n════════════════ IMPORTS ════════════════\n";
    std::cout << "Syscalls:\n";
    for (const auto& sc : te.imports.syscalls) {
        std::cout << "  - " << sc << "\n";
    }
    std::cout << "Libraries:\n";
    for (const auto& lib : te.imports.libraries) {
        std::cout << "  - " << lib << "\n";
    }

    // Data
    std::cout << "\n═════════════════ DATA ═════════════════\n";
    std::cout << "Strings (" << te.data.strings.size() << "):\n";
    for (const auto& [name, value] : te.data.strings) {
        std::cout << "  " << name << " = \"";
        // 이스케이프 처리
        for (char c : value) {
            if (c == '\n') std::cout << "\\n";
            else if (c == '\t') std::cout << "\\t";
            else if (c == '\r') std::cout << "\\r";
            else std::cout << c;
        }
        std::cout << "\"\n";
    }

    std::cout << "Constants (" << te.data.constants.size() << "):\n";
    for (const auto& [name, value] : te.data.constants) {
        std::cout << "  " << name << " = " << value << "\n";
    }

    // Code
    std::cout << "\n════════════════ CODE ════════════════\n";
    std::cout << "Functions (" << te.code.functions.size() << "):\n";
    for (const auto& [func_name, instructions] : te.code.functions) {
        std::cout << "\n  " << func_name << " (" << instructions.size() << " instructions):\n";
        int instr_count = 0;
        for (const auto& instr : instructions) {
            if (!instr.label.empty()) {
                std::cout << "    " << instr.label << ":\n";
            }
            if (!instr.mnemonic.empty()) {
                std::cout << "      " << instr.mnemonic;
                if (!instr.operands.empty()) {
                    std::cout << " " << instr.operands;
                }
                std::cout << "\n";
                instr_count++;
            }
        }
    }

    std::cout << "\n════════════════════════════════════════\n";
}

void validate_command(const std::string& filename) {
    std::cout << "\n╔═══════════════════════════════════════════╗\n";
    std::cout << "║      TE File Validator (VALIDATE)         ║\n";
    std::cout << "╚═══════════════════════════════════════════╝\n\n";

    TEParser parser;
    auto result = parser.parse_file(filename);

    if (!result) {
        std::cerr << "✗ Parse Error: " << parser.get_error() << "\n";
        return;
    }

    auto te = result.value();
    std::string validation = te.validate();

    if (validation.empty()) {
        std::cout << "✓ TE file is VALID\n";
        std::cout << "  • Metadata: OK\n";
        std::cout << "  • Imports: OK (" << te.imports.syscalls.size() << " syscalls)\n";
        std::cout << "  • Data: OK (" << te.data.strings.size() << " strings, " 
                  << te.data.constants.size() << " constants)\n";
        std::cout << "  • Code: OK (" << te.code.functions.size() << " functions)\n";
    } else {
        std::cerr << "✗ Validation Error: " << validation << "\n";
    }
}

void create_command(const std::string& name) {
    std::cout << "\n╔═══════════════════════════════════════════╗\n";
    std::cout << "║     TE File Creator (CREATE)              ║\n";
    std::cout << "╚═══════════════════════════════════════════╝\n\n";

    TEExecutable te;
    te.format = "text-executable";
    te.version = 1.0f;

    te.metadata.name = name;
    te.metadata.version = "1.0";
    te.metadata.target_os = {"linux", "windows"};
    te.metadata.architecture = {"x86-64"};
    te.metadata.entry_point = "main";
    te.metadata.author = "TE Creator";
    te.metadata.description = "Auto-generated TE program";

    te.imports.syscalls = {"write", "exit"};
    te.imports.libraries = {"libc"};

    te.data.strings["msg"] = "Hello from " + name + "!\n";
    te.data.constants["stdout"] = 1;
    te.data.constants["exit_code"] = 0;

    // Simple assembly
    AsmInstruction instr1;
    instr1.mnemonic = "mov";
    instr1.operands = "rax, 1";

    AsmInstruction instr2;
    instr2.mnemonic = "mov";
    instr2.operands = "rdi, 1";

    AsmInstruction instr3;
    instr3.mnemonic = "lea";
    instr3.operands = "rsi, [msg]";

    AsmInstruction instr4;
    instr4.mnemonic = "mov";
    instr4.operands = "rdx, 20";

    AsmInstruction instr5;
    instr5.mnemonic = "syscall";

    AsmInstruction instr6;
    instr6.mnemonic = "mov";
    instr6.operands = "rax, 60";

    AsmInstruction instr7;
    instr7.mnemonic = "mov";
    instr7.operands = "rdi, 0";

    AsmInstruction instr8;
    instr8.mnemonic = "syscall";

    te.code.functions["main"] = {instr1, instr2, instr3, instr4, instr5, instr6, instr7, instr8};

    // Save
    TEGenerator generator;
    std::string filename = name + ".yac";
    if (generator.save_to_file(te, filename)) {
        std::cout << "✓ Created: " << filename << "\n";
    } else {
        std::cerr << "✗ Failed to create file\n";
    }
}

void demo_command() {
    std::cout << "\n╔═══════════════════════════════════════════╗\n";
    std::cout << "║           TE Demo & Test Suite            ║\n";
    std::cout << "╚═══════════════════════════════════════════╝\n\n";

    std::cout << "Available sample programs:\n";
    std::cout << "  1. hello_world.yac   - Basic output program\n";
    std::cout << "  2. calculator.yac    - Function calls and arithmetic\n";
    std::cout << "  3. file_reader.yac   - File I/O operations\n\n";

    std::cout << "Run with: te_runtime run <filename>\n\n";

    // Test parser
    std::cout << "═════════════════ PARSER TEST ═════════════════\n";

    const char* sample = R"(
te_version: 1.0
format: "text-executable"

metadata:
  name: "test_program"
  version: "1.0"
  entry_point: "main"

imports:
  syscalls: [write, exit]

data:
  strings:
    hello: "Test!\n"

code:
  main:
    mov rax, 1
    mov rdi, 1
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
)";

    TEParser parser;
    auto result = parser.parse_string(sample);

    if (result) {
        auto te = result.value();
        std::cout << "✓ Parser works correctly\n";
        std::cout << "  Program: " << te.metadata.name << "\n";
        std::cout << "  Functions: " << te.code.functions.size() << "\n";
    } else {
        std::cerr << "✗ Parser test failed\n";
    }
}

int main(int argc, char* argv[]) {
    std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║        TE (Text Executable) Runtime v1.0              ║\n";
    std::cout << "║      Human-Readable Executable Format                ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    try {
        if (command == "run" && argc >= 3) {
            run_command(argv[2]);
        } else if (command == "parse" && argc >= 3) {
            parse_command(argv[2]);
        } else if (command == "validate" && argc >= 3) {
            validate_command(argv[2]);
        } else if (command == "create" && argc >= 3) {
            create_command(argv[2]);
        } else if (command == "demo") {
            demo_command();
        } else {
            std::cerr << "Unknown command or missing arguments\n";
            print_usage(argv[0]);
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
