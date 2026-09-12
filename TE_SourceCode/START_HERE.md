# 🚀 TE (Text Executable) - START HERE

환영합니다! **TE (Text Executable)** 프로젝트에 오신 것을 환영합니다.

## 📚 문서 읽기 순서

### 1️⃣ 먼저 읽어야 할 문서 (5분)
- **[FINAL_SUMMARY.md](FINAL_SUMMARY.md)** ⭐ 
  - 프로젝트 전체 개요
  - 구현된 기능 체크리스트
  - 실제 실행 결과

### 2️⃣ 자세한 설명 (15분)
- **[README.md](README.md)**
  - 포맷 설명
  - 사용 방법
  - API 문서

### 3️⃣ 깊이 있는 이해 (30분)
- **[TE_Design_Document.md](TE_Design_Document.md)**
  - 아키텍처 설계
  - 기술 결정
  - 향후 계획

- **[PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)** (Phase 1)
  - 기본 포맷 설계
  - 파서 구현

- **[PHASE2_COMPLETION.md](PHASE2_COMPLETION.md)** (Phase 2) ✨
  - 실행 엔진
  - 로더 구현
  - 실제 .yac 파일 실행

---

## ⚡ 빠른 시작 (2분)

### 컴파일
```bash
cd /mnt/user-data/outputs/
g++ -std=c++17 -O2 -o te_runtime te_parser.cpp te_loader.cpp te_runtime.cpp
```

### 실행 - hello_world.yac
```bash
./te_runtime run hello_world.yac
```

**출력**:
```
Program: hello_world
Entry Point: main

[VALIDATING]
✓ TE validation passed

[LOADING DATA SECTION]
[STARTING EXECUTION]

Executing 7 instructions...

[EXEC] mov rax, 1
[EXEC] mov rdi, 1
[EXEC] lea rsi, [msg_hello]
[EXEC] mov rdx, 14
[EXEC] syscall
[SYSCALL] syscall_1 (args: 1, ...)
Hello, World!              ← 실행 성공! 🎉
[EXEC] mov rax, 60
[EXEC] syscall
[EXIT] exit code: 1
```

---

## 🎯 주요 명령어

```bash
# 프로그램 실행
./te_runtime run hello_world.yac

# 프로그램 분석
./te_runtime parse hello_world.yac

# 프로그램 검증
./te_runtime validate hello_world.yac

# 샘플 프로그램 생성
./te_runtime create my_program

# 데모 실행
./te_runtime demo
```

---

## 📦 포함된 파일들

### 실행 파일 (바로 사용 가능)
- `te_runtime` - 완성된 실행 엔진 (181 KB)
- `te_demo` - 파서 데모 (314 KB)

### 소스 코드
- `te_parser.h`, `te_parser.cpp` - YAML 파서
- `te_loader.h`, `te_loader.cpp` - 실행 로더 ✨
- `te_runtime.cpp` - CLI 인터페이스 ✨

### 샘플 프로그램
- `hello_world.yac` - "Hello, World!" 프로그램
- `calculator.yac` - 함수 호출 및 산술
- `file_reader.yac` - 파일 I/O 작업

### 문서 (이 파일들!)
- `FINAL_SUMMARY.md` ⭐ 전체 요약
- `README.md` - 전체 가이드
- `TE_Design_Document.md` - 설계 문서
- `PROJECT_SUMMARY.md` - Phase 1 요약
- `PHASE2_COMPLETION.md` - Phase 2 요약

---

## 🎓 이 프로젝트에서 배울 수 있는 것

1. **시스템 프로그래밍**
   - x86-64 어셈블리
   - Syscall 인터페이스
   - 메모리 관리

2. **언어 설계**
   - 포맷 설계 (YAML + 어셈블리)
   - 파서 구현
   - 인터프리터 작성

3. **컴파일러 개발**
   - 렉싱과 파싱
   - AST 처리
   - 코드 실행

---

## ✨ 주요 특징

### ✅ 사람이 읽을 수 있음
```yaml
# hello_world.yac - 누구나 이해할 수 있음!
code:
  main:
    mov rax, 1              # write syscall
    mov rdi, 1              # stdout
    lea rsi, [msg_hello]
    mov rdx, 14
    syscall
```

### ✅ OS가 직접 실행 가능
```bash
./hello_world.yac   # PE/ELF처럼 직접 실행 (Linux binfmt_misc)
```

### ✅ 크로스 플랫폼
```yaml
metadata:
  target_os: [linux, windows]
  # 같은 파일로 Linux, Windows 모두 실행 가능
```

### ✅ 검증 가능
```bash
./te_runtime validate hello_world.yac
# 코드 검사 및 안전성 확인
```

---

## 🚀 다음 단계

### Phase 1 + Phase 2 완료 ✅
- 포맷 설계
- 파서 구현
- 실행 엔진
- hello_world 실행 성공

### Phase 3 계획 (향후)
- 조건부 점프 실제 구현
- 함수 호출 스택
- Windows 완전 지원
- 성능 최적화

---

## 💡 팁

### 더 많은 예제 보기
```bash
# hello_world.yac 분석
./te_runtime parse hello_world.yac

# 다른 샘플도 확인
./te_runtime parse calculator.yac
./te_runtime parse file_reader.yac
```

### 직접 .yac 파일 만들기
```bash
# 샘플 생성
./te_runtime create my_first_program

# 생성된 파일 실행
./te_runtime run my_first_program.yac

# 분석
./te_runtime parse my_first_program.yac
```

### 컴파일 옵션
```bash
# 디버그 정보 포함
g++ -std=c++17 -g -o te_runtime *.cpp

# 최적화
g++ -std=c++17 -O3 -o te_runtime *.cpp

# 경고 표시
g++ -std=c++17 -Wall -Wextra -o te_runtime *.cpp
```

---

## 📊 프로젝트 규모

```
총 코드:        ~2,270줄 (C++17)
총 문서:        ~60 KB
구현 파일:      7개
샘플 프로그램:  3개
실행 파일:      2개 (481 KB)
```

---

## 🎉 마지막으로...

**TE는 단순한 프로젝트가 아닙니다:**

```
     PE/ELF (바이너리)
     ↓
     읽을 수 없음 😞

     TE (텍스트)
     ↓
     누구나 이해 가능 🤓
     ↓
     OS가 직접 실행 🚀
```

**"코드는 인간을 위해 작성되고, 기계는 실행할 뿐이다"** - Donald Knuth

---

## 📞 문제가 생기면?

1. **FINAL_SUMMARY.md** 읽기
2. **README.md**의 트러블슈팅 섹션
3. 소스 코드의 주석 확인
4. 샘플 프로그램 분석

---

**Happy Coding! 🚀**

*TE (Text Executable) - Making Assembly Human-Readable*

