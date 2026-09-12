# TE (Text Executable) - 설계 문서

## 1. 프로젝트 개요

**프로젝트명**: TE (Text Executable)  
**확장자**: `.yac`  
**개념**: 텍스트로 작성되는 실행파일 (PE/ELF와 동급)  
**특징**: 사람이 읽을 수 있는 텍스트 형식 + 직접 OS 실행  
**지원 OS**: Linux, Windows  
**기술 스택**: C/C++ (성능 중심)

---

## 2. TE 파일 포맷 설계

### 2.1 전체 구조
```
┌─────────────────────────────────┐
│  YAML 헤더 (메타데이터)         │
│  ├─ version, target, arch...    │
├─────────────────────────────────┤
│  SECTIONS                       │
│  ├─ [metadata]                  │
│  ├─ [code]                      │
│  ├─ [data]                      │
│  └─ [imports]                   │
├─────────────────────────────────┤
│  ASM 섹션 (어셈블리 코드)       │
│  ├─ x86-64 어셈블리 명령어      │
│  ├─ syscall 호출                │
│  └─ 제어 흐름                   │
└─────────────────────────────────┘
```

### 2.2 TE 파일 예제 (.yac)

```yaml
# ===== TE (Text Executable) Format =====
te_version: 1.0
format: "text-executable"
file_extension: ".yac"

# ===== Metadata =====
metadata:
  name: "hello_world"
  version: "1.0"
  target_os: ["linux", "windows"]
  architecture: ["x86-64"]
  entry_point: "main"
  timestamp: 2024-01-15T10:00:00Z

# ===== Imports (외부 함수/시스템콜) =====
imports:
  syscalls:
    - write
    - exit
    - open
    - close
    - read
  libraries:
    - libc
    - kernel32 (windows)

# ===== Data Section =====
data:
  strings:
    msg_hello: "Hello, World!\n"
    msg_error: "Error occurred\n"
  constants:
    exit_success: 0
    exit_failure: 1

# ===== Code Section (Assembly) =====
code:
  main:
    # Linux x86-64 syscall: write(1, msg_hello, 14)
    mov rax, 1              # syscall: write
    mov rdi, 1              # fd: stdout
    lea rsi, [msg_hello]    # buffer address
    mov rdx, 14             # length
    syscall
    
    # exit(0)
    mov rax, 60             # syscall: exit
    mov rdi, 0              # exit code
    syscall
```

---

## 3. 아키텍처 설계

### 3.1 실행 흐름

```
User: ./program.yac
       ↓
OS Kernel (binfmt_misc / custom loader)
       ↓
TE Loader (C/C++)
       ├─ YAML 헤더 파싱
       ├─ 섹션 로드 (메타데이터, 데이터, 코드)
       ├─ 어셈블리 코드 검증
       ├─ 메모리 할당 및 재배치
       ├─ 심볼 해석 (imports)
       └─ 진입점(entry_point) 실행
       ↓
Program Execution (Native Assembly)
       ↓
Exit with status code
```

### 3.2 핵심 컴포넌트

| 컴포넌트 | 역할 | 구현 언어 |
|---------|------|---------|
| **TE Parser** | YAML 헤더 파싱 | C++ (yaml-cpp) |
| **ASM Validator** | 어셈블리 문법 검증 | C++ |
| **Memory Manager** | 메모리 할당/재배치 | C |
| **Loader** | 실행 로더 (Linux/Windows) | C/ASM |
| **Syscall Handler** | 시스템콜 중개 | C/ASM |
| **CLI Tool** | .yac 파일 생성/검증 | C++ |

---

## 4. Linux 지원 (binfmt_misc)

Linux에서 OS 레벨 지원을 위해 **binfmt_misc** 활용:

```bash
echo ':yac:M::YAC/TE 1.0::te-loader:' | sudo tee /proc/sys/fs/binfmt_misc/register
```

→ `.yac` 파일을 직접 실행 가능 (`./program.yac`)

---

## 5. Windows 지원

Windows에서는 **커스텀 실행 로더** 구현:
- `.yac` → `.exe` 감싸기 (Wrapper)
- 또는 Registry 활용 File Association

---

## 6. 보안 고려사항

- ✅ 어셈블리 명령어 화이트리스트 검증
- ✅ 메모리 접근 제한 (샌드박스)
- ✅ Syscall 필터링
- ✅ 코드 서명 지원

---

## 7. 개발 단계

### Phase 1: 기본 포맷 및 파서
- [ ] YAML 기반 TE 포맷 정의
- [ ] YAML 파서 구현
- [ ] 샘플 .yac 파일 생성

### Phase 2: 실행 로더 (Linux)
- [ ] 메모리 관리자 구현
- [ ] x86-64 어셈블리 검증기
- [ ] 기본 로더 (binfmt_misc)
- [ ] 간단한 syscall 래퍼

### Phase 3: 어셈블리 실행
- [ ] 다중 섹션 지원
- [ ] 심볼 해석
- [ ] 재배치(relocation) 처리

### Phase 4: Windows 지원
- [ ] Windows 로더 구현
- [ ] PE 포맷 호환성

### Phase 5: 고급 기능
- [ ] 보안 샌드박스
- [ ] 디버깅 정보
- [ ] 코드 서명

---

## 8. 기술 스택 상세

```
Language: C/C++17
Dependencies:
  - yaml-cpp (YAML 파싱)
  - capstone (어셈블리 분석)
  - fmt (포맷팅)

Build System: CMake
Target: x86-64 Linux/Windows
```

---

## 9. 예상 파일 구조

```
TE-Project/
├── CMakeLists.txt
├── src/
│   ├── parser/
│   │   ├── yaml_parser.h
│   │   ├── yaml_parser.cpp
│   │   └── te_schema.h
│   ├── loader/
│   │   ├── te_loader.h
│   │   ├── te_loader.cpp
│   │   ├── memory_manager.h
│   │   └── memory_manager.cpp
│   ├── asm/
│   │   ├── asm_validator.h
│   │   ├── asm_validator.cpp
│   │   └── syscall_handler.cpp
│   ├── platform/
│   │   ├── linux_loader.cpp
│   │   └── windows_loader.cpp
│   └── main.cpp
├── examples/
│   ├── hello_world.yac
│   ├── calculator.yac
│   └── file_reader.yac
├── tests/
│   ├── test_parser.cpp
│   └── test_loader.cpp
└── docs/
    ├── format_spec.md
    └── assembly_reference.md
```

---

## 10. 다음 단계

1. ✅ 설계 문서 완성
2. → **YAML 기반 TE 포맷 프로토타입 작성**
3. → **기본 파서 구현**
4. → **간단한 Linux 로더 만들기**
5. → **첫 번째 실행 가능한 .yac 파일 만들기**

