#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <memory>

namespace TE {

// ===== TE 포맷 상수 =====
constexpr const char* MAGIC_HEADER = "YAC/TE";
constexpr float FORMAT_VERSION = 1.0f;
constexpr const char* FILE_EXTENSION = ".yac";

// ===== 구조체 정의 =====

/**
 * @brief 메타데이터 섹션
 */
struct Metadata {
    std::string name;
    std::string version;
    std::vector<std::string> target_os;  // ["linux", "windows"]
    std::vector<std::string> architecture;  // ["x86-64"]
    std::string entry_point;  // "main"
    std::string timestamp;
    std::string author;
    std::string description;
};

/**
 * @brief Import 정보 (외부 함수, 시스템콜)
 */
struct ImportInfo {
    std::vector<std::string> syscalls;
    std::vector<std::string> libraries;
    std::map<std::string, std::string> external_functions;
};

/**
 * @brief 데이터 섹션
 */
struct DataSection {
    std::map<std::string, std::string> strings;
    std::map<std::string, long long> constants;
    std::map<std::string, std::vector<uint8_t>> binary_data;
};

/**
 * @brief 어셈블리 명령어
 */
struct AsmInstruction {
    std::string label;
    std::string mnemonic;
    std::string operands;
    int line_number;
    
    std::string to_string() const {
        std::string result;
        if (!label.empty()) {
            result += label + ":\n";
        }
        result += "    " + mnemonic;
        if (!operands.empty()) {
            result += " " + operands;
        }
        return result;
    }
};

/**
 * @brief 코드 섹션 (어셈블리)
 */
struct CodeSection {
    std::map<std::string, std::vector<AsmInstruction>> functions;
    std::map<std::string, size_t> symbol_table;  // 심볼명 -> 메모리 주소
};

/**
 * @brief 완전한 TE 파일 구조
 */
struct TEExecutable {
    std::string format;
    float version;
    
    Metadata metadata;
    ImportInfo imports;
    DataSection data;
    CodeSection code;
    
    // 유효성 검사
    bool is_valid() const;
    std::string validate() const;  // 에러 메시지 반환
};

// ===== Parser 클래스 =====

class TEParser {
public:
    TEParser() = default;
    ~TEParser() = default;

    /**
     * @brief .yac 파일 파싱
     * @param filename 파일 경로
     * @return 파싱된 TE 구조체, 실패 시 nullptr
     */
    std::optional<TEExecutable> parse_file(const std::string& filename);

    /**
     * @brief YAML 문자열 파싱
     * @param yaml_content YAML 문자열
     * @return 파싱된 TE 구조체
     */
    std::optional<TEExecutable> parse_string(const std::string& yaml_content);

    /**
     * @brief 파싱 에러 메시지
     */
    const std::string& get_error() const { return error_message; }

private:
    std::string error_message;

    // 파싱 헬퍼
    bool parse_metadata_section(const std::string& section, Metadata& metadata);
    bool parse_imports_section(const std::string& section, ImportInfo& imports);
    bool parse_data_section(const std::string& section, DataSection& data);
    bool parse_code_section(const std::string& section, CodeSection& code);
    
    // 어셈블리 파싱
    std::optional<AsmInstruction> parse_asm_line(const std::string& line, int line_num);
    bool validate_asm_instruction(const std::string& mnemonic, const std::string& operands);
};

// ===== Generator 클래스 (TE 파일 생성) =====

class TEGenerator {
public:
    TEGenerator() = default;
    ~TEGenerator() = default;

    /**
     * @brief TE 구조체를 YAML 포맷으로 변환
     */
    std::string generate_yaml(const TEExecutable& te) const;

    /**
     * @brief TE 파일로 저장
     */
    bool save_to_file(const TEExecutable& te, const std::string& filename) const;

private:
    std::string generate_metadata_section(const Metadata& metadata) const;
    std::string generate_imports_section(const ImportInfo& imports) const;
    std::string generate_data_section(const DataSection& data) const;
    std::string generate_code_section(const CodeSection& code) const;
};

// ===== 유틸리티 함수 =====

namespace Utils {
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    bool is_valid_label(const std::string& label);
    bool is_valid_mnemonic(const std::string& mnemonic);
}

}  // namespace TE
