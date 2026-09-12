#pragma once

#include "te_parser.h"
#include <cstring>
#include <map>
#include <memory>
#include <vector>

namespace TE {

// ===== 메모리 관리 =====

/**
 * @brief 메모리 영역을 관리하는 클래스
 */
class MemoryManager {
public:
    MemoryManager(size_t total_size = 1024 * 1024);  // 1MB
    ~MemoryManager();

    /**
     * @brief 메모리 할당
     * @param size 할당할 크기
     * @return 할당된 메모리의 주소
     */
    uintptr_t allocate(size_t size);

    /**
     * @brief 메모리 해제
     */
    void deallocate(uintptr_t addr);

    /**
     * @brief 메모리에 데이터 쓰기
     */
    void write(uintptr_t addr, const void* data, size_t size);

    /**
     * @brief 메모리에서 데이터 읽기
     */
    void read(uintptr_t addr, void* data, size_t size) const;

    /**
     * @brief 메모리 주소의 유효성 검사
     */
    bool is_valid_address(uintptr_t addr) const;

    /**
     * @brief 데이터 섹션 로드
     */
    void load_data_section(const DataSection& data);

    /**
     * @brief 심볼과 메모리 주소 매핑
     */
    std::map<std::string, uintptr_t>& get_symbol_table() {
        return symbol_table;
    }

    const std::map<std::string, uintptr_t>& get_symbol_table() const {
        return symbol_table;
    }

private:
    uint8_t* memory;
    size_t total_size;
    size_t current_offset;
    std::map<std::string, uintptr_t> symbol_table;
};

// ===== Syscall 처리 =====

/**
 * @brief Syscall 핸들러
 */
class SyscallHandler {
public:
    SyscallHandler(MemoryManager& mem) : memory(mem) {}

    /**
     * @brief Syscall 실행
     * @param syscall_num 시스템콜 번호 (rax)
     * @param arg1-arg6 인자들 (rdi, rsi, rdx, r10, r8, r9)
     * @return 반환값
     */
    long handle_syscall(long syscall_num, long arg1, long arg2, long arg3,
                       long arg4, long arg5, long arg6);

private:
    MemoryManager& memory;

    // 개별 syscall 핸들러
    long syscall_write(long fd, const void* buf, long count);
    long syscall_read(long fd, void* buf, long count);
    long syscall_open(const char* filename, long flags, long mode);
    long syscall_close(long fd);
    long syscall_exit(long code);
    long syscall_exit_group(long code);
    long syscall_brk(void* addr);
    long syscall_mmap(void* addr, long length, long prot, long flags,
                     long fd, long offset);
};

// ===== 실행 컨텍스트 =====

/**
 * @brief CPU 레지스터 상태
 */
struct RegisterState {
    uint64_t rax = 0, rbx = 0, rcx = 0, rdx = 0;
    uint64_t rsi = 0, rdi = 0, rbp = 0, rsp = 0;
    uint64_t r8 = 0, r9 = 0, r10 = 0, r11 = 0;
    uint64_t r12 = 0, r13 = 0, r14 = 0, r15 = 0;
    uint64_t rip = 0;  // 명령어 포인터

    void reset() {
        std::memset(this, 0, sizeof(RegisterState));
    }

    void print_state() const;
};

/**
 * @brief 실행 컨텍스트 (CPU 상태 + 메모리)
 */
class ExecutionContext {
public:
    ExecutionContext(MemoryManager& mem) 
        : memory(mem), syscall_handler(mem) {}

    RegisterState& get_registers() { return registers; }
    const RegisterState& get_registers() const { return registers; }

    MemoryManager& get_memory() { return memory; }
    SyscallHandler& get_syscall_handler() { return syscall_handler; }

    int get_exit_code() const { return exit_code; }
    void set_exit_code(int code) { exit_code = code; }

    bool is_running() const { return running; }
    void set_running(bool r) { running = r; }

private:
    RegisterState registers;
    MemoryManager& memory;
    SyscallHandler syscall_handler;
    int exit_code = 0;
    bool running = true;
};

// ===== 어셈블리 실행 엔진 =====

/**
 * @brief x86-64 어셈블리 인터프리터
 */
class AsmInterpreter {
public:
    AsmInterpreter(ExecutionContext& ctx) : context(ctx) {}

    /**
     * @brief 단일 어셈블리 명령어 실행
     */
    bool execute_instruction(const AsmInstruction& instr);

    /**
     * @brief 여러 명령어 실행
     */
    bool execute_instructions(const std::vector<AsmInstruction>& instructions);

private:
    ExecutionContext& context;

    // 명령어 실행 함수들
    bool exec_mov(const std::string& operands);
    bool exec_lea(const std::string& operands);
    bool exec_push(const std::string& operands);
    bool exec_pop(const std::string& operands);
    bool exec_add(const std::string& operands);
    bool exec_sub(const std::string& operands);
    bool exec_mul(const std::string& operands);
    bool exec_div(const std::string& operands);
    bool exec_imul(const std::string& operands);
    bool exec_idiv(const std::string& operands);
    bool exec_and(const std::string& operands);
    bool exec_or(const std::string& operands);
    bool exec_xor(const std::string& operands);
    bool exec_cmp(const std::string& operands);
    bool exec_test(const std::string& operands);
    bool exec_jmp(const std::string& operands);
    bool exec_je(const std::string& operands);
    bool exec_jne(const std::string& operands);
    bool exec_jz(const std::string& operands);
    bool exec_jnz(const std::string& operands);
    bool exec_jl(const std::string& operands);
    bool exec_jg(const std::string& operands);
    bool exec_syscall(const std::string& operands);
    bool exec_call(const std::string& operands);
    bool exec_ret(const std::string& operands);
    bool exec_nop(const std::string& operands);
    bool exec_inc(const std::string& operands);
    bool exec_dec(const std::string& operands);

    // 헬퍼 함수
    uint64_t& get_register(const std::string& reg_name);
    uint64_t get_operand_value(const std::string& operand);
    void set_operand_value(const std::string& operand, uint64_t value);
    bool parse_operands(const std::string& operands, 
                       std::string& op1, std::string& op2);
};

// ===== TE 로더 =====

/**
 * @brief TE 파일을 로드하고 실행하는 메인 로더
 */
class TELoader {
public:
    TELoader() : memory(10 * 1024 * 1024) {}  // 10MB

    /**
     * @brief .yac 파일 로드 및 실행
     */
    int load_and_execute(const std::string& filename);

    /**
     * @brief TE 구조체 직접 실행
     */
    int execute(const TEExecutable& te);

    /**
     * @brief 최종 상태 출력
     */
    void print_final_state() const;

private:
    MemoryManager memory;
    std::unique_ptr<ExecutionContext> context;

    bool load_te(const TEExecutable& te);
    bool setup_stack();
    bool load_code_section(const CodeSection& code);
};

}  // namespace TE
