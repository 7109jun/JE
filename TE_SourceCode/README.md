# TE (Text Executable) - 인간이 읽을 수 있는 실행 파일 포맷

![TE Logo](https://via.placeholder.com/200x100?text=TE+TextExecutable)

## 📋 프로젝트 개요

**TE (Text Executable)**는 PE, ELF와 같은 기존 바이너리 실행파일과 달리 **사람이 읽고 이해할 수 있는 텍스트 형식**의 실행파일입니다. YAML 기반의 메타데이터와 x86-64 어셈블리 코드로 구성되어 있으며, OS 커널 수준에서 직접 실행할 수 있습니다.

**공식 확장자**: `.yac` (Yet Another Code)

---

## 🎯 주요 특징

| 특징 | 설명 |
|------|------|
| **사람이 읽을 수 있음** | YAML + 어셈블리로 이루어진 텍스트 포맷 |
| **OS 직접 실행** | 별도의 인터프리터 불필요 (PE/ELF처럼 커널이 처리) |
| **플랫폼 독립적** | 같은 파일을 Linux, Windows에서 실행 가능 |
| **검증 가능** | 코드 검사 및 검증이 용이 |
| **개발 친화적** | 디버깅과 개발이 편리함 |
| **보안** | 어셈블리 화이트리스트 및 syscall 필터링 가능 |

---

## 🏗️ 파일 포맷

### 전체 구조

```yaml
# Header
te_version: 1.0
format: "text-executable"

# Metadata (메타데이터)
metadata:
  name: "프로그램명"
  version: "1.0"
  target_os: [linux, windows]
  architecture: [x86-64]
  entry_point: "main"

# Imports (외부 함수/시스템콜)
imports:
  syscalls: [write, exit, read]
  libraries: [libc]

# Data (데이터 섹션)
data:
  strings:
    msg: "Hello, World!\n"
  constants:
    stdout: 1

# Code (어셈블리 코드)
code:
  main:
    mov rax, 1
    mov rdi, 1
    lea rsi, [msg]
    mov rdx, 14
    syscall
```

### 세부 설명

#### 1️⃣ Metadata Section
프로그램의 메타정보를 포함합니다.

```yaml
metadata:
  name: "hello_world"              # 프로그램 이름
  version: "1.0"                   # 버전
  target_os: [linux, windows]      # 지원 OS
  architecture: [x86-64]           # 지원 아키텍처
  entry_point: "main"              # 진입점 함수
  author: "TE Developer"           # 작성자
  description: "..."               # 설명
  timestamp: "2024-01-15T..."      # 타임스탬프
```

#### 2️⃣ Imports Section
프로그램이 사용할 외부 함수와 시스템콜을 선언합니다.

```yaml
imports:
  syscalls:
    - write      # 파일/stdout 쓰기
    - read       # 파일/stdin 읽기
    - open       # 파일 열기
    - close      # 파일 닫기
    - exit       # 프로세스 종료
  libraries:
    - libc       # C 표준 라이브러리
    - kernel32   # Windows API (Windows의 경우)
```

#### 3️⃣ Data Section
프로그램이 사용하는 전역 데이터를 정의합니다.

```yaml
data:
  strings:
    msg_hello: "Hello, World!\n"
    msg_error: "Error occurred\n"
  constants:
    stdout: 1        # 표준 출력 파일 디스크립터
    exit_success: 0  # 종료 코드: 성공
    exit_failure: 1  # 종료 코드: 실패
  binary_data:
    magic: [0xFF, 0xD8, 0xFF, 0xE0]  # 바이너리 데이터
```

#### 4️⃣ Code Section
실행 가능한 어셈블리 코드를 포함합니다.

```yaml
code:
  main:                    # 함수명
    mov rax, 1             # x86-64 명령어
    mov rdi, 1
    lea rsi, [msg]
    mov rdx, 14
    syscall               # 시스템콜 호출
    
    mov rax, 60           # exit syscall
    mov rdi, 0
    syscall
```

---

## 🖥️ 지원 아키텍처 및 OS

### 현재 지원 (v1.0)
- ✅ **Architecture**: x86-64 (Intel/AMD 64-bit)
- ✅ **OS**: Linux (binfmt_misc), Windows (custom loader)

### 향후 계획
- 🔜 ARM64 (Apple Silicon, Android)
- 🔜 RISC-V
- 🔜 macOS

---

## 🚀 시작하기

### 1. 컴파일

```bash
# 직접 컴파일 (C++17 필요)
g++ -std=c++17 -o te_demo main.cpp te_parser.cpp

# 또는 CMake 사용
mkdir build && cd build
cmake ..
make
```

### 2. 데모 실행

```bash
./te_demo
```

**출력**:
```
╔════════════════════════════════════════════════════════════╗
║                  TE (Text Executable)                      ║
║            Human-Readable Executable Format                ║
║                    Version 1.0 Demo                        ║
╚════════════════════════════════════════════════════════════╝

✓ All tests completed successfully!
```

### 3. 자신의 TE 파일 만들기

```yaml
# my_program.yac
te_version: 1.0
format: "text-executable"

metadata:
  name: "my_app"
  version: "1.0"
  target_os: [linux]
  architecture: [x86-64]
  entry_point: "main"

imports:
  syscalls: [write, exit]

data:
  strings:
    message: "Hello from TE!\n"

code:
  main:
    mov rax, 1
    mov rdi, 1
    lea rsi, [message]
    mov rdx, 17
    syscall
    
    mov rax, 60
    mov rdi, 0
    syscall
```

### 4. Linux에서 직접 실행 (binfmt_misc)

```bash
# binfmt_misc 등록 (관리자 권한 필요)
echo ':yac:M::YAC/TE::te-loader:' | sudo tee /proc/sys/fs/binfmt_misc/register

# 직접 실행
./my_program.yac
```

---

## 📚 포함된 샘플 프로그램

### 1️⃣ hello_world.yac
기본적인 "Hello, World!" 프로그램

```bash
Features:
- write() syscall
- exit() syscall
- 데이터 섹션 사용
```

### 2️⃣ calculator.yac
간단한 계산기

```bash
Features:
- 함수 정의 및 호출 (add, subtract, multiply, divide)
- 함수 프롤로그/에필로그
- 에러 처리
- 조건부 분기 (cmp, je)
```

### 3️⃣ file_reader.yac
파일 읽기 프로그램

```bash
Features:
- open(), read(), close() syscall
- 버퍼 관리
- 에러 처리
- 스택 할당
```

---

## 🔧 API 문서

### C++ 파서 API

#### Parser 클래스

```cpp
#include "te_parser.h"

TE::TEParser parser;

// 파일에서 파싱
auto result = parser.parse_file("program.yac");
if (result) {
    auto te = result.value();
    // ...
} else {
    std::cerr << "Error: " << parser.get_error() << "\n";
}

// 문자열에서 파싱
auto result2 = parser.parse_string(yaml_content);
```

#### Generator 클래스

```cpp
TE::TEGenerator generator;

// TE 구조체를 YAML로 변환
std::string yaml = generator.generate_yaml(te);

// 파일로 저장
generator.save_to_file(te, "output.yac");
```

#### 데이터 구조

```cpp
TE::TEExecutable te;

// 메타데이터
te.metadata.name = "my_app";
te.metadata.entry_point = "main";

// Imports
te.imports.syscalls = {"write", "exit"};

// 데이터
te.data.strings["msg"] = "Hello\n";
te.data.constants["stdout"] = 1;

// 코드
TE::AsmInstruction instr;
instr.mnemonic = "mov";
instr.operands = "rax, 1";
te.code.functions["main"].push_back(instr);
```

---

## 📋 지원되는 x86-64 명령어 (v1.0)

### 데이터 이동
```
mov, lea, push, pop, xchg
```

### 산술 연산
```
add, sub, mul, div, inc, dec
imul, idiv
```

### 논리 연산
```
and, or, xor, not, shl, shr, sar, sal
```

### 제어 흐름
```
jmp, je, jne, jz, jnz, jl, jg, jle, jge
call, ret, syscall, sysenter
```

### 기타
```
cmp, test, nop, hlt, int
```

---

## 🔐 보안 고려사항

TE는 다음과 같은 보안 메커니즘을 제공/계획하고 있습니다:

### 현재 구현 ✅
- **어셈블리 검증**: 유효한 니모닉만 허용
- **구조적 검증**: 필수 섹션 존재 확인
- **메타데이터 검증**: 진입점 존재 여부 확인

### 계획 중 🔜
- **Syscall 필터링**: 허용된 syscall만 실행
- **메모리 샌드박싱**: 메모리 접근 제한
- **코드 서명**: 디지털 서명으로 무결성 보장
- **감사 로깅**: 실행 추적

---

## 📊 프로젝트 구조

```
TE-Project/
├── README.md                    # 이 파일
├── TE_Design_Document.md        # 상세 설계 문서
├── CMakeLists.txt               # CMake 빌드 설정
├── te_parser.h                  # 파서 헤더
├── te_parser.cpp                # 파서 구현
├── main.cpp                     # 데모 프로그램
├── examples/
│   ├── hello_world.yac          # Hello World 샘플
│   ├── calculator.yac           # 계산기 샘플
│   └── file_reader.yac          # 파일 읽기 샘플
└── build/                       # 빌드 산출물
    └── te_demo                  # 실행 파일
```

---

## 🎓 사용 사례

### 1. 교육
- 어셈블리 학습
- OS 개념 이해
- 시스템 프로그래밍 실습

### 2. 개발
- 크로스 플랫폼 저수준 프로그래밍
- 성능 최적화 필요한 부분만 구현
- 버그 재현 및 분석

### 3. 보안
- 악성 코드 분석
- 코드 검증
- 샌드박싱된 실행

### 4. 연구
- 새로운 아키텍처 지원 연구
- 컴파일러/인터프리터 개발
- 형식 검증 연구

---

## 🐛 알려진 문제

1. **Windows 지원**: 현재는 Linux 중심 (binfmt_misc). Windows 버전은 래퍼 필요
2. **제한된 명령어**: x86-64 서브셋만 지원 (AVX, SSE 등 미지원)
3. **최적화 부족**: 현재는 기능성 중심, 성능 최적화 필요
4. **디버깅 정보**: DWARF 등 디버깅 정보 미지원

---

## 🗺️ 로드맵

### v1.0 (현재) ✅
- [x] YAML 기반 포맷 정의
- [x] C++ 파서/생성기 구현
- [x] 기본 샘플 프로그램
- [x] 문서화

### v1.1 (계획)
- [ ] Windows 로더 개선
- [ ] 더 많은 x86-64 명령어 지원
- [ ] 기본적인 링커 구현

### v1.2 (계획)
- [ ] ARM64 지원
- [ ] 디버깅 정보 (DWARF)
- [ ] 코드 서명

### v2.0 (장기)
- [ ] RISC-V 지원
- [ ] 고급 보안 기능 (샌드박싱)
- [ ] 최적화 패스

---

## 📖 참고 문헌

- Intel x86-64 Manual
- System V AMD64 ABI
- Linux binfmt_misc
- ELF 포맷 스펙
- PE (Windows) 포맷 스펙

---

## 📝 라이선스

MIT License - 자유롭게 사용, 수정, 배포 가능

---

## 👥 기여하기

TE는 오픈소스 프로젝트입니다. 기여를 환영합니다!

```bash
# 1. Fork
git clone https://github.com/yourusername/TE.git

# 2. Feature branch 생성
git checkout -b feature/awesome-feature

# 3. Commit
git commit -m "Add awesome feature"

# 4. Push
git push origin feature/awesome-feature

# 5. Pull Request 생성
```

---

## 🎉 결론

**TE (Text Executable)**는 전통적인 바이너리 포맷의 한계를 뛰어넘어 **"사람과 기계가 모두 이해할 수 있는 실행파일"**을 목표로 합니다.

프로그래밍의 근본을 이해하고, 새로운 가능성을 탐험하는 데 도움이 되기를 바랍니다! 🚀

---

**마지막 업데이트**: 2024-01-15  
**버전**: 1.0.0  
**상태**: Beta 🔧

