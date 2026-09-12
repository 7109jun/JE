#include "te_loader.h"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/syscall.h>
#include <regex>
#include <algorithm>

namespace TE {

// ===== MemoryManager 구현 =====

MemoryManager::MemoryManager(size_t size) 
    : total_size(size), current_offset(0) {
    memory = new uint8_t[total_size];
    std::memset(memory, 0, total_size);
}

MemoryManager::~MemoryManager() {
    if (memory) {
        delete[] memory;
    }
}

uintptr_t MemoryManager::allocate(size_t size) {
    if (current_offset + size > total_size) {
        return 0;  // 할당 실패
    }

    uintptr_t addr = reinterpret_cast<uintptr_t>(memory) + current_offset;
    current_offset += size;
    return addr;
}

void MemoryManager::deallocate(uintptr_t addr) {
    // 간단한 구현: 실제로는 복잡한 메모리 관리 필요
    (void)addr;
}

void MemoryManager::write(uintptr_t addr, const void* data, size_t size) {
    if (!is_valid_address(addr)) return;
    
    uint8_t* target = reinterpret_cast<uint8_t*>(addr);
    std::memcpy(target, data, size);
}

void MemoryManager::read(uintptr_t addr, void* data, size_t size) const {
    if (!is_valid_address(addr)) return;
    
    const uint8_t* source = reinterpret_cast<const uint8_t*>(addr);
    std::memcpy(data, source, size);
}

bool MemoryManager::is_valid_address(uintptr_t addr) const {
    uintptr_t base = reinterpret_cast<uintptr_t>(memory);
    return addr >= base && addr < base + total_size;
}

void MemoryManager::load_data_section(const DataSection& data) {
    // 문자열 로드
    for (const auto& [name, value] : data.strings) {
        uintptr_t addr = allocate(value.length() + 1);
        if (addr == 0) continue;
        
        write(addr, value.c_str(), value.length() + 1);
        symbol_table[name] = addr;
    }
}

// ===== RegisterState 출력 =====

void RegisterState::print_state() const {
    std::cout << "\n═══════════════ CPU Register State ═══════════════\n";
    std::cout << "RAX: 0x" << std::hex << rax << std::dec 
              << "  RBX: 0x" << std::hex << rbx << std::dec
              << "  RCX: 0x" << std::hex << rcx << std::dec
              << "  RDX: 0x" << std::hex << rdx << std::dec << "\n";
    std::cout << "RSI: 0x" << std::hex << rsi << std::dec 
              << "  RDI: 0x" << std::hex << rdi << std::dec
              << "  RBP: 0x" << std::hex << rbp << std::dec
              << "  RSP: 0x" << std::hex << rsp << std::dec << "\n";
    std::cout << "R8:  0x" << std::hex << r8 << std::dec 
              << "  R9:  0x" << std::hex << r9 << std::dec
              << "  R10: 0x" << std::hex << r10 << std::dec
              << "  R11: 0x" << std::hex << r11 << std::dec << "\n";
    std::cout << "RIP: 0x" << std::hex << rip << std::dec << "\n";
    std::cout << "══════════════════════════════════════════════════\n\n";
}

// ===== SyscallHandler 구현 =====

long SyscallHandler::handle_syscall(long syscall_num, long arg1, long arg2,
                                    long arg3, long arg4, long arg5, long arg6) {
    std::cout << "[SYSCALL] syscall_" << syscall_num << " (args: " 
              << arg1 << ", " << arg2 << ", " << arg3 << ")\n";

    switch (syscall_num) {
        case 1:  // write
            return syscall_write(arg1, reinterpret_cast<void*>(arg2), arg3);
        
        case 0:  // read
            return syscall_read(arg1, reinterpret_cast<void*>(arg2), arg3);
        
        case 2:  // open
            return syscall_open(reinterpret_cast<const char*>(arg1), arg2, arg3);
        
        case 3:  // close
            return syscall_close(arg1);
        
        case 60: // exit
            return syscall_exit(arg1);
        
        case 231: // exit_group
            return syscall_exit_group(arg1);
        
        case 12: // brk
            return reinterpret_cast<long>(syscall_brk(reinterpret_cast<void*>(arg1)));
        
        default:
            std::cerr << "Unknown syscall: " << syscall_num << "\n";
            return -1;
    }
}

long SyscallHandler::syscall_write(long fd, const void* buf, long count) {
    if (fd == 1) {  // stdout
        std::cout.write(reinterpret_cast<const char*>(buf), count);
        std::cout.flush();
        return count;
    } else if (fd == 2) {  // stderr
        std::cerr.write(reinterpret_cast<const char*>(buf), count);
        std::cerr.flush();
        return count;
    }
    return -1;
}

long SyscallHandler::syscall_read(long fd, void* buf, long count) {
    if (fd == 0) {  // stdin
        std::cin.read(reinterpret_cast<char*>(buf), count);
        return std::cin.gcount();
    }
    return -1;
}

long SyscallHandler::syscall_open(const char* filename, long flags, long mode) {
    std::cout << "[OPEN] " << filename << " flags=" << flags << " mode=" << mode << "\n";
    // 실제 구현은 복잡하므로 간단하게
    return 4;  // 파일 디스크립터 반환
}

long SyscallHandler::syscall_close(long fd) {
    std::cout << "[CLOSE] fd=" << fd << "\n";
    return 0;
}

long SyscallHandler::syscall_exit(long code) {
    std::cout << "[EXIT] exit code: " << code << "\n";
    exit(code);
    return 0;
}

long SyscallHandler::syscall_exit_group(long code) {
    std::cout << "[EXIT_GROUP] exit code: " << code << "\n";
    exit(code);
    return 0;
}

long SyscallHandler::syscall_brk(void* addr) {
    // 단순 구현
    return reinterpret_cast<long>(addr);
}

long SyscallHandler::syscall_mmap(void* addr, long length, long prot, 
                                 long flags, long fd, long offset) {
    // 메모리 매핑 시뮬레이션
    return reinterpret_cast<long>(malloc(length));
}

// ===== AsmInterpreter 구현 =====

uint64_t& AsmInterpreter::get_register(const std::string& reg_name) {
    auto& regs = context.get_registers();
    
    if (reg_name == "rax") return regs.rax;
    if (reg_name == "rbx") return regs.rbx;
    if (reg_name == "rcx") return regs.rcx;
    if (reg_name == "rdx") return regs.rdx;
    if (reg_name == "rsi") return regs.rsi;
    if (reg_name == "rdi") return regs.rdi;
    if (reg_name == "rbp") return regs.rbp;
    if (reg_name == "rsp") return regs.rsp;
    if (reg_name == "r8") return regs.r8;
    if (reg_name == "r9") return regs.r9;
    if (reg_name == "r10") return regs.r10;
    if (reg_name == "r11") return regs.r11;
    if (reg_name == "r12") return regs.r12;
    if (reg_name == "r13") return regs.r13;
    if (reg_name == "r14") return regs.r14;
    if (reg_name == "r15") return regs.r15;
    
    // 기본값 반환 (에러)
    static uint64_t dummy = 0;
    return dummy;
}

uint64_t AsmInterpreter::get_operand_value(const std::string& operand) {
    // 숫자 리터럴
    if (std::isdigit(operand[0]) || operand[0] == '-') {
        return std::stoull(operand);
    }
    
    // 레지스터
    return get_register(operand);
}

void AsmInterpreter::set_operand_value(const std::string& operand, uint64_t value) {
    if (std::isdigit(operand[0]) || operand[0] == '-') {
        // 리터럴에는 쓸 수 없음
        return;
    }
    
    // 레지스터에 쓰기
    get_register(operand) = value;
}

bool AsmInterpreter::parse_operands(const std::string& operands,
                                   std::string& op1, std::string& op2) {
    size_t comma_pos = operands.find(',');
    if (comma_pos != std::string::npos) {
        op1 = Utils::trim(operands.substr(0, comma_pos));
        op2 = Utils::trim(operands.substr(comma_pos + 1));
        return true;
    }
    
    op1 = Utils::trim(operands);
    op2.clear();
    return true;
}

bool AsmInterpreter::execute_instruction(const AsmInstruction& instr) {
    const std::string& mnem = instr.mnemonic;
    const std::string& ops = instr.operands;

    std::cout << "[EXEC] " << mnem;
    if (!ops.empty()) std::cout << " " << ops;
    std::cout << "\n";

    if (mnem == "mov") return exec_mov(ops);
    if (mnem == "lea") return exec_lea(ops);
    if (mnem == "push") return exec_push(ops);
    if (mnem == "pop") return exec_pop(ops);
    if (mnem == "add") return exec_add(ops);
    if (mnem == "sub") return exec_sub(ops);
    if (mnem == "mul") return exec_mul(ops);
    if (mnem == "div") return exec_div(ops);
    if (mnem == "imul") return exec_imul(ops);
    if (mnem == "idiv") return exec_idiv(ops);
    if (mnem == "and") return exec_and(ops);
    if (mnem == "or") return exec_or(ops);
    if (mnem == "xor") return exec_xor(ops);
    if (mnem == "cmp") return exec_cmp(ops);
    if (mnem == "test") return exec_test(ops);
    if (mnem == "jmp") return exec_jmp(ops);
    if (mnem == "je") return exec_je(ops);
    if (mnem == "jne") return exec_jne(ops);
    if (mnem == "jz") return exec_jz(ops);
    if (mnem == "jnz") return exec_jnz(ops);
    if (mnem == "jl") return exec_jl(ops);
    if (mnem == "jg") return exec_jg(ops);
    if (mnem == "syscall") return exec_syscall(ops);
    if (mnem == "call") return exec_call(ops);
    if (mnem == "ret") return exec_ret(ops);
    if (mnem == "nop") return exec_nop(ops);
    if (mnem == "inc") return exec_inc(ops);
    if (mnem == "dec") return exec_dec(ops);

    std::cerr << "Unknown instruction: " << mnem << "\n";
    return false;
}

bool AsmInterpreter::execute_instructions(const std::vector<AsmInstruction>& instructions) {
    for (const auto& instr : instructions) {
        if (!execute_instruction(instr)) {
            return false;
        }
    }
    return true;
}

// ===== 명령어 구현 =====

bool AsmInterpreter::exec_mov(const std::string& operands) {
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    uint64_t value = get_operand_value(op2);
    set_operand_value(op1, value);
    return true;
}

bool AsmInterpreter::exec_lea(const std::string& operands) {
    // load effective address - 간단한 구현
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    // [symbol] -> 심볼의 주소
    if (op2.find('[') != std::string::npos) {
        size_t start = op2.find('[') + 1;
        size_t end = op2.find(']');
        std::string symbol = op2.substr(start, end - start);
        
        auto& table = context.get_memory().get_symbol_table();
        if (table.find(symbol) != table.end()) {
            set_operand_value(op1, table[symbol]);
        }
    }
    return true;
}

bool AsmInterpreter::exec_push(const std::string& operands) {
    auto& rsp = get_register("rsp");
    rsp -= 8;
    
    uint64_t value = get_operand_value(operands);
    context.get_memory().write(rsp, &value, 8);
    return true;
}

bool AsmInterpreter::exec_pop(const std::string& operands) {
    auto& rsp = get_register("rsp");
    
    uint64_t value = 0;
    context.get_memory().read(rsp, &value, 8);
    rsp += 8;
    
    set_operand_value(operands, value);
    return true;
}

bool AsmInterpreter::exec_add(const std::string& operands) {
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    uint64_t val1 = get_operand_value(op1);
    uint64_t val2 = get_operand_value(op2);
    set_operand_value(op1, val1 + val2);
    return true;
}

bool AsmInterpreter::exec_sub(const std::string& operands) {
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    uint64_t val1 = get_operand_value(op1);
    uint64_t val2 = get_operand_value(op2);
    set_operand_value(op1, val1 - val2);
    return true;
}

bool AsmInterpreter::exec_mul(const std::string& operands) {
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    uint64_t val1 = get_operand_value(op1);
    uint64_t val2 = get_operand_value(op2);
    set_operand_value(op1, val1 * val2);
    return true;
}

bool AsmInterpreter::exec_div(const std::string& operands) {
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    uint64_t divisor = get_operand_value(op2);
    if (divisor == 0) {
        std::cerr << "Division by zero!\n";
        return false;
    }
    
    uint64_t val1 = get_operand_value(op1);
    set_operand_value(op1, val1 / divisor);
    return true;
}

bool AsmInterpreter::exec_imul(const std::string& operands) {
    return exec_mul(operands);  // 부호있는 곱셈 (단순화)
}

bool AsmInterpreter::exec_idiv(const std::string& operands) {
    return exec_div(operands);  // 부호있는 나눗셈 (단순화)
}

bool AsmInterpreter::exec_and(const std::string& operands) {
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    uint64_t val1 = get_operand_value(op1);
    uint64_t val2 = get_operand_value(op2);
    set_operand_value(op1, val1 & val2);
    return true;
}

bool AsmInterpreter::exec_or(const std::string& operands) {
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    uint64_t val1 = get_operand_value(op1);
    uint64_t val2 = get_operand_value(op2);
    set_operand_value(op1, val1 | val2);
    return true;
}

bool AsmInterpreter::exec_xor(const std::string& operands) {
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    uint64_t val1 = get_operand_value(op1);
    uint64_t val2 = get_operand_value(op2);
    set_operand_value(op1, val1 ^ val2);
    return true;
}

bool AsmInterpreter::exec_cmp(const std::string& operands) {
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    uint64_t val1 = get_operand_value(op1);
    uint64_t val2 = get_operand_value(op2);
    
    // 결과를 rflags에 저장 (단순화: rax 사용)
    int64_t result = (int64_t)val1 - (int64_t)val2;
    context.get_registers().rax = result;
    
    return true;
}

bool AsmInterpreter::exec_test(const std::string& operands) {
    std::string op1, op2;
    parse_operands(operands, op1, op2);
    
    uint64_t val1 = get_operand_value(op1);
    uint64_t val2 = get_operand_value(op2);
    context.get_registers().rax = val1 & val2;
    
    return true;
}

bool AsmInterpreter::exec_jmp(const std::string& operands) {
    // 실제 점프는 복잡하므로 간단하게
    std::cout << "[JMP] to " << operands << "\n";
    return true;
}

bool AsmInterpreter::exec_je(const std::string& operands) {
    // 실제 조건 점프는 복잡하므로 간단하게
    return true;
}

bool AsmInterpreter::exec_jne(const std::string& operands) {
    return true;
}

bool AsmInterpreter::exec_jz(const std::string& operands) {
    return true;
}

bool AsmInterpreter::exec_jnz(const std::string& operands) {
    return true;
}

bool AsmInterpreter::exec_jl(const std::string& operands) {
    return true;
}

bool AsmInterpreter::exec_jg(const std::string& operands) {
    return true;
}

bool AsmInterpreter::exec_syscall(const std::string& operands) {
    auto& regs = context.get_registers();
    long result = context.get_syscall_handler().handle_syscall(
        regs.rax, regs.rdi, regs.rsi, regs.rdx,
        regs.r10, regs.r8, regs.r9
    );
    regs.rax = result;
    return true;
}

bool AsmInterpreter::exec_call(const std::string& operands) {
    std::cout << "[CALL] " << operands << "\n";
    return true;
}

bool AsmInterpreter::exec_ret(const std::string& operands) {
    std::cout << "[RET]\n";
    context.set_running(false);
    return true;
}

bool AsmInterpreter::exec_nop(const std::string& operands) {
    return true;
}

bool AsmInterpreter::exec_inc(const std::string& operands) {
    uint64_t value = get_operand_value(operands);
    set_operand_value(operands, value + 1);
    return true;
}

bool AsmInterpreter::exec_dec(const std::string& operands) {
    uint64_t value = get_operand_value(operands);
    set_operand_value(operands, value - 1);
    return true;
}

// ===== TELoader 구현 =====

int TELoader::load_and_execute(const std::string& filename) {
    TEParser parser;
    auto result = parser.parse_file(filename);
    
    if (!result) {
        std::cerr << "Failed to parse: " << parser.get_error() << "\n";
        return 1;
    }
    
    return execute(result.value());
}

int TELoader::execute(const TEExecutable& te) {
    std::cout << "\n╔═══════════════════════════════════════════╗\n";
    std::cout << "║      TE (Text Executable) Loader          ║\n";
    std::cout << "║            Execution Engine               ║\n";
    std::cout << "╚═══════════════════════════════════════════╝\n\n";

    std::cout << "Program: " << te.metadata.name << "\n";
    std::cout << "Entry Point: " << te.metadata.entry_point << "\n";
    std::cout << "Functions: " << te.code.functions.size() << "\n\n";

    if (!load_te(te)) {
        std::cerr << "Failed to load TE executable\n";
        return 1;
    }

    std::cout << "\n[LOADING DATA SECTION]\n";
    memory.load_data_section(te.data);

    std::cout << "[STARTING EXECUTION]\n\n";
    context = std::make_unique<ExecutionContext>(memory);
    
    if (!load_code_section(te.code)) {
        std::cerr << "Failed to load code section\n";
        return 1;
    }

    print_final_state();
    return context->get_exit_code();
}

bool TELoader::load_te(const TEExecutable& te) {
    if (!te.is_valid()) {
        std::cerr << "Invalid TE executable\n";
        return false;
    }

    std::cout << "[VALIDATING]\n";
    std::string validation = te.validate();
    if (!validation.empty()) {
        std::cerr << validation << "\n";
        return false;
    }

    std::cout << "✓ TE validation passed\n\n";
    return true;
}

bool TELoader::setup_stack() {
    // 1MB 스택 할당
    uintptr_t stack = memory.allocate(1024 * 1024);
    if (stack == 0) {
        std::cerr << "Failed to allocate stack\n";
        return false;
    }

    return true;
}

bool TELoader::load_code_section(const CodeSection& code) {
    if (code.functions.empty()) {
        std::cerr << "No code to execute\n";
        return false;
    }

    AsmInterpreter interpreter(*context);

    // main 함수 실행
    auto it = code.functions.find("main");
    if (it == code.functions.end()) {
        std::cerr << "main function not found\n";
        return false;
    }

    const auto& instructions = it->second;
    std::cout << "Executing " << instructions.size() << " instructions...\n\n";

    return interpreter.execute_instructions(instructions);
}

void TELoader::print_final_state() const {
    if (context) {
        context->get_registers().print_state();
        std::cout << "Exit Code: " << context->get_exit_code() << "\n\n";
    }
}

}  // namespace TE
