#include "te_parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <iostream>

namespace TE {

// ===== 유틸리티 함수 구현 =====

namespace Utils {

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool is_valid_label(const std::string& label) {
    if (label.empty()) return false;
    if (!isalpha(label[0]) && label[0] != '_') return false;
    for (char c : label) {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

// x86-64 유효 니모닉 목록
static const std::vector<std::string> VALID_MNEMONICS = {
    // 데이터 이동
    "mov", "lea", "push", "pop", "xchg",
    // 산술 연산
    "add", "sub", "mul", "div", "inc", "dec",
    // 논리 연산
    "and", "or", "xor", "not", "shl", "shr", "sar", "sal",
    // 제어 흐름
    "jmp", "je", "jne", "jz", "jnz", "jl", "jg", "jle", "jge",
    "call", "ret", "syscall", "sysenter",
    // 비교 & 테스트
    "cmp", "test",
    // 기타
    "nop", "hlt", "int",
    // x86-64 특화
    "movq", "movl", "movw", "movb",
    "addq", "subl", "imul", "idiv"
};

bool is_valid_mnemonic(const std::string& mnemonic) {
    std::string lower = mnemonic;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    return std::find(VALID_MNEMONICS.begin(), VALID_MNEMONICS.end(), lower) 
           != VALID_MNEMONICS.end();
}

}  // namespace Utils

// ===== TEExecutable 검증 =====

bool TEExecutable::is_valid() const {
    return !metadata.name.empty() && 
           !metadata.entry_point.empty() &&
           !code.functions.empty();
}

std::string TEExecutable::validate() const {
    if (metadata.name.empty()) {
        return "Error: metadata.name is required";
    }
    if (metadata.entry_point.empty()) {
        return "Error: metadata.entry_point is required";
    }
    if (code.functions.empty()) {
        return "Error: code section is empty";
    }
    
    // entry_point이 존재하는지 확인
    auto it = code.functions.find(metadata.entry_point);
    if (it == code.functions.end()) {
        return "Error: entry_point '" + metadata.entry_point + "' not found in code section";
    }
    
    return "";  // 모든 검증 통과
}

// ===== TEParser 구현 =====

std::optional<TEExecutable> TEParser::parse_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        error_message = "Cannot open file: " + filename;
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return parse_string(buffer.str());
}

std::optional<TEExecutable> TEParser::parse_string(const std::string& yaml_content) {
    TEExecutable te;
    te.format = "text-executable";
    te.version = FORMAT_VERSION;

    // YAML을 섹션으로 분할 (간단한 파싱)
    std::vector<std::string> lines;
    std::stringstream ss(yaml_content);
    std::string line;

    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    std::string metadata_section, imports_section, data_section, code_section;
    std::string current_section;

    for (const auto& l : lines) {
        std::string trimmed = Utils::trim(l);

        // 주석과 빈 줄 무시
        if (trimmed.empty() || trimmed[0] == '#') continue;

        // 섹션 구분
        if (trimmed == "metadata:" || trimmed.find("metadata:") != std::string::npos) {
            current_section = "metadata";
            continue;
        } else if (trimmed.find("imports:") != std::string::npos) {
            current_section = "imports";
            continue;
        } else if (trimmed.find("data:") != std::string::npos) {
            current_section = "data";
            continue;
        } else if (trimmed.find("code:") != std::string::npos) {
            current_section = "code";
            continue;
        }

        // 섹션별로 내용 추가
        if (current_section == "metadata") {
            metadata_section += l + "\n";
        } else if (current_section == "imports") {
            imports_section += l + "\n";
        } else if (current_section == "data") {
            data_section += l + "\n";
        } else if (current_section == "code") {
            code_section += l + "\n";
        }
    }

    // 각 섹션 파싱
    if (!parse_metadata_section(metadata_section, te.metadata)) {
        error_message = "Failed to parse metadata section";
        return std::nullopt;
    }

    if (!parse_imports_section(imports_section, te.imports)) {
        error_message = "Failed to parse imports section";
        return std::nullopt;
    }

    if (!parse_data_section(data_section, te.data)) {
        error_message = "Failed to parse data section";
        return std::nullopt;
    }

    if (!parse_code_section(code_section, te.code)) {
        error_message = "Failed to parse code section";
        return std::nullopt;
    }

    // 최종 검증
    std::string validation_error = te.validate();
    if (!validation_error.empty()) {
        error_message = validation_error;
        return std::nullopt;
    }

    return te;
}

bool TEParser::parse_metadata_section(const std::string& section, Metadata& metadata) {
    std::stringstream ss(section);
    std::string line;

    while (std::getline(ss, line)) {
        std::string trimmed = Utils::trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;

        // "key: value" 형식 파싱
        size_t colon_pos = trimmed.find(':');
        if (colon_pos == std::string::npos) continue;

        std::string key = Utils::trim(trimmed.substr(0, colon_pos));
        std::string value = Utils::trim(trimmed.substr(colon_pos + 1));

        if (key == "name") {
            metadata.name = value;
        } else if (key == "version") {
            metadata.version = value;
        } else if (key == "target_os") {
            // 배열 파싱 예: [linux, windows]
            if (value[0] == '[' && value[value.length()-1] == ']') {
                std::string array_content = value.substr(1, value.length() - 2);
                auto items = Utils::split(array_content, ',');
                for (auto& item : items) {
                    metadata.target_os.push_back(Utils::trim(item));
                }
            }
        } else if (key == "architecture") {
            if (value[0] == '[' && value[value.length()-1] == ']') {
                std::string array_content = value.substr(1, value.length() - 2);
                auto items = Utils::split(array_content, ',');
                for (auto& item : items) {
                    metadata.architecture.push_back(Utils::trim(item));
                }
            }
        } else if (key == "entry_point") {
            metadata.entry_point = value;
        } else if (key == "timestamp") {
            metadata.timestamp = value;
        } else if (key == "author") {
            metadata.author = value;
        } else if (key == "description") {
            metadata.description = value;
        }
    }

    return !metadata.name.empty() && !metadata.entry_point.empty();
}

bool TEParser::parse_imports_section(const std::string& section, ImportInfo& imports) {
    std::stringstream ss(section);
    std::string line;
    std::string current_subsection;

    while (std::getline(ss, line)) {
        std::string trimmed = Utils::trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;

        if (trimmed.find("syscalls:") != std::string::npos) {
            current_subsection = "syscalls";
            continue;
        } else if (trimmed.find("libraries:") != std::string::npos) {
            current_subsection = "libraries";
            continue;
        }

        // "- item" 형식의 배열 파싱
        if (trimmed[0] == '-') {
            std::string item = Utils::trim(trimmed.substr(1));
            if (current_subsection == "syscalls") {
                imports.syscalls.push_back(item);
            } else if (current_subsection == "libraries") {
                imports.libraries.push_back(item);
            }
        }
    }

    return true;
}

bool TEParser::parse_data_section(const std::string& section, DataSection& data) {
    std::stringstream ss(section);
    std::string line;
    std::string current_subsection;

    while (std::getline(ss, line)) {
        std::string trimmed = Utils::trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;

        if (trimmed.find("strings:") != std::string::npos) {
            current_subsection = "strings";
            continue;
        } else if (trimmed.find("constants:") != std::string::npos) {
            current_subsection = "constants";
            continue;
        }

        // "key: value" 파싱
        size_t colon_pos = trimmed.find(':');
        if (colon_pos == std::string::npos) continue;

        std::string key = Utils::trim(trimmed.substr(0, colon_pos));
        std::string value = Utils::trim(trimmed.substr(colon_pos + 1));

        if (current_subsection == "strings") {
            // "string_name: \"value\""
            if (value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }
            data.strings[key] = value;
        } else if (current_subsection == "constants") {
            try {
                data.constants[key] = std::stoll(value);
            } catch (...) {
                // 파싱 실패
            }
        }
    }

    return true;
}

bool TEParser::parse_code_section(const std::string& section, CodeSection& code) {
    std::stringstream ss(section);
    std::string line;
    std::string current_function;
    int line_num = 0;

    while (std::getline(ss, line)) {
        line_num++;
        std::string trimmed = Utils::trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;

        // 함수 레이블 감지
        if (trimmed.back() == ':' && trimmed.find("  ") == std::string::npos) {
            current_function = trimmed.substr(0, trimmed.length() - 1);
            code.functions[current_function] = {};
            continue;
        }

        // 어셈블리 명령어 파싱
        if (!current_function.empty()) {
            auto instr = parse_asm_line(trimmed, line_num);
            if (instr) {
                code.functions[current_function].push_back(instr.value());
            }
        }
    }

    return !code.functions.empty();
}

std::optional<AsmInstruction> TEParser::parse_asm_line(const std::string& line, int line_num) {
    std::string trimmed = Utils::trim(line);
    if (trimmed.empty()) return std::nullopt;

    AsmInstruction instr;
    instr.line_number = line_num;

    // 레이블 처리 (끝에 :)
    if (trimmed.back() == ':') {
        instr.label = trimmed.substr(0, trimmed.length() - 1);
        return instr;
    }

    // 니모닉과 피연산자 분리
    size_t space_pos = trimmed.find(' ');
    if (space_pos == std::string::npos) {
        // 피연산자 없는 명령어
        instr.mnemonic = trimmed;
    } else {
        instr.mnemonic = Utils::trim(trimmed.substr(0, space_pos));
        instr.operands = Utils::trim(trimmed.substr(space_pos + 1));
    }

    // 유효성 검사
    if (!Utils::is_valid_mnemonic(instr.mnemonic)) {
        error_message = "Invalid mnemonic: " + instr.mnemonic + " at line " + std::to_string(line_num);
        return std::nullopt;
    }

    return instr;
}

bool TEParser::validate_asm_instruction(const std::string& mnemonic, const std::string& operands) {
    return Utils::is_valid_mnemonic(mnemonic);
}

// ===== TEGenerator 구현 =====

std::string TEGenerator::generate_yaml(const TEExecutable& te) const {
    std::stringstream ss;

    // 헤더
    ss << "# ===== TE (Text Executable) Format =====\n";
    ss << "te_version: " << te.version << "\n";
    ss << "format: \"" << te.format << "\"\n";
    ss << "file_extension: \"" << FILE_EXTENSION << "\"\n\n";

    // 메타데이터
    ss << generate_metadata_section(te.metadata);
    ss << "\n";

    // Import
    ss << generate_imports_section(te.imports);
    ss << "\n";

    // 데이터
    ss << generate_data_section(te.data);
    ss << "\n";

    // 코드
    ss << generate_code_section(te.code);

    return ss.str();
}

std::string TEGenerator::generate_metadata_section(const Metadata& metadata) const {
    std::stringstream ss;
    ss << "# ===== Metadata =====\n";
    ss << "metadata:\n";
    ss << "  name: \"" << metadata.name << "\"\n";
    ss << "  version: \"" << metadata.version << "\"\n";
    ss << "  target_os: [";
    for (size_t i = 0; i < metadata.target_os.size(); ++i) {
        ss << "\"" << metadata.target_os[i] << "\"";
        if (i < metadata.target_os.size() - 1) ss << ", ";
    }
    ss << "]\n";
    ss << "  architecture: [";
    for (size_t i = 0; i < metadata.architecture.size(); ++i) {
        ss << "\"" << metadata.architecture[i] << "\"";
        if (i < metadata.architecture.size() - 1) ss << ", ";
    }
    ss << "]\n";
    ss << "  entry_point: \"" << metadata.entry_point << "\"\n";
    return ss.str();
}

std::string TEGenerator::generate_imports_section(const ImportInfo& imports) const {
    std::stringstream ss;
    ss << "# ===== Imports =====\n";
    ss << "imports:\n";
    ss << "  syscalls:\n";
    for (const auto& syscall : imports.syscalls) {
        ss << "    - " << syscall << "\n";
    }
    ss << "  libraries:\n";
    for (const auto& lib : imports.libraries) {
        ss << "    - " << lib << "\n";
    }
    return ss.str();
}

std::string TEGenerator::generate_data_section(const DataSection& data) const {
    std::stringstream ss;
    ss << "# ===== Data Section =====\n";
    ss << "data:\n";
    ss << "  strings:\n";
    for (const auto& [key, value] : data.strings) {
        ss << "    " << key << ": \"" << value << "\"\n";
    }
    ss << "  constants:\n";
    for (const auto& [key, value] : data.constants) {
        ss << "    " << key << ": " << value << "\n";
    }
    return ss.str();
}

std::string TEGenerator::generate_code_section(const CodeSection& code) const {
    std::stringstream ss;
    ss << "# ===== Code Section (Assembly) =====\n";
    ss << "code:\n";
    for (const auto& [func_name, instructions] : code.functions) {
        ss << "  " << func_name << ":\n";
        for (const auto& instr : instructions) {
            if (!instr.label.empty()) {
                ss << "    " << instr.label << ":\n";
            }
            if (!instr.mnemonic.empty()) {
                ss << "      " << instr.mnemonic;
                if (!instr.operands.empty()) {
                    ss << " " << instr.operands;
                }
                ss << "\n";
            }
        }
    }
    return ss.str();
}

bool TEGenerator::save_to_file(const TEExecutable& te, const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << generate_yaml(te);
    file.close();
    return true;
}

}  // namespace TE
