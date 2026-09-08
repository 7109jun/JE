# JE 1.1 — 공식 문법 · 스펙 · 개발 가이드

> **JE (Just Executable)**
> 텍스트 기반 실행 파일 포맷 및 실행기
> 현재 대상: Windows x64
> 주 빌드 도구체인: MinGW-w64 GCC
> 빌드 시스템: CMake
> 표준 확장자: `.jxj`
> JE 매직: `[ JE : O]`

---

# 목차

1. JE란 무엇인가
2. 현재 구현 상태
3. 파일 기본 구조
4. JE 매직의 정확한 문법
5. 공백과 들여쓰기 규칙
6. 줄바꿈 규칙
7. 주석 규칙
8. 문자열 규칙
9. 숫자 규칙
10. 식별자 규칙
11. 레지스터 규칙
12. 헤더
13. AUTHOR
14. FORMAT
15. VERSION
16. ARCH
17. SYSTEM 블록
18. ENTRY
19. SUBSYSTEM
20. SECTION
21. CODE
22. 레이블
23. 명령어 기본 규칙
24. NOP
25. MOV
26. ADD
27. SUB
28. XOR
29. AND
30. OR
31. SHL
32. SHR
33. CMP
34. INC
35. DEC
36. PUSH
37. POP
38. CALL
39. RET
40. JMP
41. JZ
42. JNZ
43. EXIT
44. STR
45. PRINT
46. PRINTS
47. 문자열 이스케이프
48. 런타임 ABI
49. malloc
50. calloc
51. realloc
52. free
53. memcpy
54. memset
55. fopen
56. fread
57. fwrite
58. fclose
59. fseek
60. fflush
61. 메모리 소유권
62. 파일 핸들 소유권
63. 포인터 검증
64. 문자열 포인터
65. ZERO 상태
66. CALL 동작
67. 실행 순서
68. ENTRY에 대한 주의
69. CODE 블록 탐색
70. 중괄호 규칙
71. 잘못된 JE 예제
72. 올바른 JE 예제
73. 최소 실행 파일
74. 메모리 예제
75. 파일 I/O 예제
76. 함수 호출 예제
77. 루프 예제
78. 스택 예제
79. 주석 사용 예제
80. 명령어 표
81. 런타임 함수 표
82. CLI
83. detect
84. run
85. launch
86. validate
87. pe2je
88. je2pe
89. 직접 실행
90. Windows 파일 연결
91. MinGW-w64 요구 사항
92. CMake 빌드
93. CMakePresets
94. Windows 배치 빌드
95. GitHub Actions
96. 프로젝트 구조
97. PE와 JE의 관계
98. PE_RAW_HEX
99. 변환 시 생성되는 구조
100. 검증 방법
101. 제한 사항
102. 현재 구현에서 주의할 점
103. 권장 코딩 스타일
104. 완전한 JE 예제
105. 최종 문법 요약

---

# 1. JE란 무엇인가

JE는 **Just Executable**의 약자로, Windows PE와 비슷한 실행 파일 구조를 텍스트 형태로 표현하고 실행할 수 있도록 만든 실행 파일 포맷이다.

JE 파일의 일반적인 확장자는:

```text
.jxj
```

이다.

예:

```text
hello.jxj
```

단, JE 실행 파일을 인식할 때 확장자만 사용하는 것은 아니다.

파일의 가장 처음이 정확하게:

```text
[ JE : O]
```

이면 JE 매직으로 인식할 수 있다.

---

# 2. 현재 구현 상태

현재 구현 버전은:

```text
JE 1.1
```

이다.

C 코드에서 버전 매크로는:

```c
#define JE_VERSION_MAJOR 1
#define JE_VERSION_MINOR 1
```

이다.

그러나 **현재 텍스트 JE 파일에서 사용하는 `VERSION` 필드는 숫자 필드이며, 저장소의 실제 예제와 변환기는 다음과 같이 작성한다.**

```text
VERSION = 1
```

즉 다음을 사용해야 한다.

```text
VERSION = 1
```

다음은 현재 헤더 파서가 사용하는 형식과 맞지 않는다.

```text
VERSION = 1.1
```

따라서 이 문서에서는 실제 구현에 맞춰:

```text
VERSION = 1
```

을 정식 예로 사용한다.

---

# 3. 파일 기본 구조

현재 JE 파일의 일반적인 구조는 다음과 같다.

```text
[ JE : O]

AUTHOR = "..."
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

SECTION .text {
    CODE {
        main:
            ...
    }
}
```

가장 중요한 실행 부분은 `CODE`이다.

현재 JE 실행기는 실제 실행할 `CODE { ... }` 블록을 파일에서 찾고 그 내부를 JE 명령어로 실행한다.

---

# 4. JE 매직의 정확한 문법

JE 매직은 다음과 **문자 단위로 정확히 일치해야 한다.**

```text
[ JE : O]
```

정확한 문자 순서는 다음과 같다.

```text
[
공백
J
E
공백
:
공백
O
]
```

즉:

```text
[ JE : O]
```

는 맞는다.

다음은 다르기 때문에 JE 매직과 일치하지 않는다.

```text
[JE : O]
[ JE: O]
[ JE :O]
[ JE :  O]
[ JE : O ]
{ JE : O}
[ je : O]
```

특히 대괄호 내부의 공백을 임의로 추가하거나 삭제하면 안 된다.

**정확한 JE 매직은 한 가지다.**

```text
[ JE : O]
```

파일의 처음부터 바로 시작해야 한다.

잘못된 예:

```text

[ JE : O]
```

처음에 빈 줄이 들어가면 매직이 파일 시작 위치에 있지 않다.

또한:

```text
    [ JE : O]
```

처럼 들여쓰기한 것도 안 된다.

---

# 5. 공백과 들여쓰기 규칙

JE에는 중요한 차이가 있다.

## 5.1 매직은 공백이 엄격하다

매직:

```text
[ JE : O]
```

은 정확히 일치해야 한다.

## 5.2 일반 문법은 공백에 비교적 관대하다

예를 들어 헤더 파서는 다음처럼 토큰을 구분한다.

```text
AUTHOR = "JE"
```

다음도 토큰 수준에서는 처리할 수 있다.

```text
AUTHOR="JE"
```

또한:

```text
AUTHOR   =   "JE"
```

도 처리할 수 있다.

하지만 **권장 형식은 항상 다음이다.**

```text
AUTHOR = "JE"
```

즉:

```text
KEY = VALUE
```

형태를 사용한다.

## 5.3 들여쓰기는 실행기의 핵심 문법이 아니다

현재 실행기는 JE 명령어 앞의 공백과 탭을 제거한 뒤 해석한다.

따라서 다음은 같은 효과를 낸다.

```text
main:
    MOV R0, 1
```

```text
main:
MOV R0, 1
```

```text
main:
        MOV R0, 1
```

```text
main:
	MOV R0, 1
```

현재 구현에서 **4칸 들여쓰기 자체가 필수는 아니다.**

그러나 가독성을 위해 다음을 표준 스타일로 사용한다.

```text
CODE {
    main:
        MOV R0, 1
        EXIT 0
}
```

## 5.4 공백 몇 칸이 틀리는가

일반적으로 명령어 앞의 공백 수 자체는 오류가 아니다.

다음은 모두 실행기가 앞 공백을 제거할 수 있다.

```text
MOV R0, 1
 MOV R0, 1
  MOV R0, 1
    MOV R0, 1
        MOV R0, 1
```

하지만 매직은 다르다.

다음은 서로 다른 문자열이다.

```text
[ JE : O]
[JE : O]
[ JE: O]
[ JE :O]
```

따라서:

> **JE는 일반 코드의 들여쓰기에는 엄격하지 않지만, 매직 문자열에는 엄격하다.**

---

# 6. 줄바꿈 규칙

JE 텍스트 파일은 줄 단위로 처리된다.

Windows에서는 일반적으로:

```text
CRLF
```

Linux에서는:

```text
LF
```

를 사용할 수 있다.

실행기는 줄 끝의 `\r`도 제거하여 처리한다.

권장 형식은 한 명령어를 한 줄에 하나씩 작성하는 것이다.

올바른 예:

```text
MOV R0, 10
ADD R0, 5
PRINT R0
EXIT 0
```

비권장:

```text
MOV R0, 10 ADD R0, 5 PRINT R0
```

현재 명령어 파서는 명령어별 한 줄 구조를 기준으로 동작한다.

---

# 7. 주석 규칙

JE의 주석은:

```text
#
```

으로 시작한다.

예:

```text
# 전체 줄 주석
MOV R0, 10 # 오른쪽도 주석
```

현재 코드 실행기는 문자열 내부의 `#`는 주석으로 처리하지 않도록 되어 있다.

예:

```text
PRINT "hello # world"
```

이 경우 `#`가 문자열 안에 있으므로 문자열의 일부다.

다음은 주석이다.

```text
PRINT "hello"
# 여기부터 주석
```

주석은 줄 끝까지 이어진다.

---

# 8. 문자열 규칙

문자열은 큰따옴표:

```text
"
```

로 감싼다.

예:

```text
"Hello"
```

잘못된 예:

```text
'Hello'
```

문자열에는 지원되는 이스케이프 시퀀스를 사용할 수 있다.

```text
"\n"
"\r"
"\t"
"\""
"\\"
```

문자열의 끝에는 닫는 `"`가 반드시 필요하다.

잘못된 예:

```text
PRINT "Hello
```

---

# 9. 숫자 규칙

현재 JE 숫자 파서는 **10진수**와 **0x로 시작하는 16진수**를 지원한다.

10진수:

```text
10
100
255
4096
```

16진수:

```text
0x10
0xFF
0x1000
```

대문자 `X`도 인식한다.

```text
0XFF
```

음수 리터럴은 현재 직접 지원하지 않는다.

따라서 다음은 사용할 수 없다.

```text
MOV R0, -1
```

대신 레지스터의 64비트 값에서 감산하는 방식 등을 사용해야 한다.

예:

```text
MOV R0, 0
SUB R0, 1
```

이 경우 unsigned 64-bit 레지스터 특성에 따라 값이 래핑된다.

---

# 10. 식별자 규칙

현재 명령어 실행기의 레이블 식별자는 다음 문자를 사용할 수 있다.

```text
A-Z
a-z
0-9
_
.
```

예:

```text
main
sum
loop
loop_1
function.test
```

권장 레이블:

```text
main:
loop:
sum_two:
```

실행기의 레이블 검사에서는 `-`가 일반 명령어 식별자에서 허용되지 않는다.

따라서 권장하지 않는다.

```text
my-label:
```

대신:

```text
my_label:
```

을 사용한다.

---

# 11. 레지스터 규칙

JE 실행 컨텍스트는 8개의 64비트 레지스터를 가진다.

```text
R0
R1
R2
R3
R4
R5
R6
R7
```

대소문자는 레지스터 파서에서 둘 다 허용된다.

즉:

```text
R0
r0
```

는 같은 레지스터로 인식된다.

그러나 문서와 예제에서는 대문자를 권장한다.

```text
R0
R1
R2
R3
```

---

# 12. 헤더

일반적인 JE 헤더는 다음 형식으로 작성한다.

```text
[ JE : O]

AUTHOR = "JE"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64
```

헤더 파서는 다음 필드를 인식한다.

```text
AUTHOR
FORMAT
VERSION
ARCH
```

---

# 13. AUTHOR

형식:

```text
AUTHOR = "문자열"
```

예:

```text
AUTHOR = "JE"
```

문자열이어야 한다.

잘못된 예:

```text
AUTHOR = JE
```

권장:

```text
AUTHOR = "My Program"
```

---

# 14. FORMAT

형식:

```text
FORMAT = "문자열"
```

일반적으로:

```text
FORMAT = "JE"
```

를 사용한다.

현재 헤더 파서에서는 문자열로 읽는다.

---

# 15. VERSION

현재 실제 JE 텍스트 예제와 변환기가 사용하는 형식:

```text
VERSION = 1
```

숫자여야 한다.

현재 파서는 32비트 unsigned 범위를 넘는 숫자를 거부한다.

현재 구현에서:

```text
VERSION = 1
```

은 정상이다.

반면:

```text
VERSION = 1.1
```

은 숫자 토큰 규칙과 맞지 않는다.

---

# 16. ARCH

형식:

```text
ARCH = X86_64
```

현재 헤더 파서는 다음 값을 인식한다.

```text
X86_64
X86
ARM64
ARM
```

Windows x64용 JE에서는 다음을 사용한다.

```text
ARCH = X86_64
```

알 수 없는 값은 파싱 오류가 된다.

예:

```text
ARCH = MIPS
```

는 현재 파서에서 허용되지 않는다.

---

# 17. SYSTEM 블록

일반적인 형식:

```text
SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}
```

현재 저장소의 예제는 이 형식을 사용한다.

중요한 점:

> 현재 실행기에서 `SYSTEM` 내부의 모든 키가 실제 실행 동작을 제어하는 것은 아니다.

현재 텍스트 헤더 파서는 `SYSTEM`을 만나면 헤더 파싱 종료 지점으로 사용한다.

즉 `SYSTEM`은 현재 포맷 구조상 중요한 메타데이터 블록이지만, 모든 항목이 현재 실행 엔진에서 사용되는 것은 아니다.

---

# 18. ENTRY

일반 표기:

```text
ENTRY = main
```

예:

```text
SYSTEM {
    ENTRY = main
}
```

그러나 **현재 JE 1.1 실행기의 중요한 구현 특성**이 있다.

현재 `je_load_and_run_code()`는 `ENTRY = main`을 읽어 실제 실행 위치를 설정하지 않는다.

실행기에서는 `CODE { ... }` 내부를 추출한 뒤:

```text
je_exec_text(...)
```

를 호출하고, `je_exec_text()`는 프로그램의 첫 번째 실행 가능한 줄에서 시작한다.

따라서 현재 구현에서는 다음처럼 `main:`을 CODE의 첫 실행 코드 위치에 두는 것이 안전하다.

```text
CODE {
    main:
        MOV R0, 1
        EXIT 0
}
```

예를 들어 다음은 현재 구현과 맞지 않는 실행 순서가 될 수 있다.

```text
CODE {
    helper:
        PRINT 123

    main:
        PRINT 456
        EXIT 0
}
```

현재 실행기는 `ENTRY = main`을 기반으로 자동 점프하지 않기 때문에 `helper` 위치가 먼저 실행될 수 있다.

따라서 **현재 JE 1.1에서는 엔트리 레이블을 CODE의 첫 실행 위치에 배치하는 것을 권장한다.**

---

# 19. SUBSYSTEM

일반 표기:

```text
SUBSYSTEM = CONSOLE
```

예:

```text
SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}
```

현재 저장소 예제에서는 콘솔 프로그램임을 나타내는 메타데이터로 사용한다.

---

# 20. SECTION

PE와 유사한 구역 구조를 표현하기 위해 사용할 수 있다.

예:

```text
SECTION .text {
    ...
}
```

또는 변환기에서 생성된 메타데이터에는 섹션 정보가 포함될 수 있다.

중요:

> 현재 텍스트 실행기는 `.text`, `.rdata`, `.data`, `.bss` 등의 모든 PE 섹션을 실제 CPU 메모리 섹션처럼 로드하여 실행하는 단계까지 구현된 것은 아니다.

현재 JE 명령어 실행은 `CODE` 블록 기반이다.

---

# 21. CODE

현재 JE 실행에서 가장 중요한 블록이다.

일반 형식:

```text
CODE {
    ...
}
```

예:

```text
CODE {
    main:
        MOV R0, 10
        PRINT R0
        EXIT 0
}
```

현재 로더는 파일에서 `CODE`라는 이름을 찾고, 그 뒤의 `{`를 찾은 다음 해당 중괄호 쌍의 내용을 실행 코드로 사용한다.

---

# 22. 레이블

형식:

```text
label:
```

예:

```text
main:
```

여러 레이블을 만들 수 있다.

```text
main:
    CALL add_one
    EXIT 0

add_one:
    ADD R0, 1
    RET
```

같은 레이블 이름을 두 번 정의할 수 없다.

잘못된 예:

```text
main:
    ...

main:
    ...
```

중복 레이블은 오류가 된다.

현재 실행기는 최대:

```text
512
```

개의 레이블을 저장할 수 있다.

---

# 23. 명령어 기본 규칙

명령어는 한 줄에 하나 작성한다.

기본 형태:

```text
OPCODE ARGUMENT...
```

예:

```text
MOV R0, 10
```

두 피연산자를 사용하는 명령어는 일반적으로 쉼표로 구분한다.

```text
MOV R0, 10
ADD R0, 5
CMP R0, R1
```

현재 파서는 쉼표 주변의 공백을 제거하므로:

```text
MOV R0,10
```

도 토큰상 처리할 수 있다.

하지만 권장 스타일은:

```text
MOV R0, 10
```

이다.

---

# 24. NOP

문법:

```text
NOP
```

아무 동작도 하지 않는다.

예:

```text
NOP
EXIT 0
```

---

# 25. MOV

문법:

```text
MOV destination, source
```

destination은 반드시 레지스터다.

source는:

* 레지스터
* 숫자

중 하나다.

예:

```text
MOV R0, 10
MOV R1, R0
MOV R2, 0xFF
```

잘못된 예:

```text
MOV 10, R0
```

---

# 26. ADD

문법:

```text
ADD destination, value
```

예:

```text
ADD R0, 5
ADD R0, R1
```

동작:

```text
R0 = R0 + value
```

대상 레지스터의 값이 64비트 unsigned 정수처럼 동작한다.

연산 후 값이 0이면 `zero` 상태가 참이 된다.

---

# 27. SUB

문법:

```text
SUB destination, value
```

예:

```text
SUB R0, 5
SUB R0, R1
```

동작:

```text
R0 = R0 - value
```

64비트 레지스터의 산술 규칙에 따른다.

---

# 28. XOR

문법:

```text
XOR destination, value
```

예:

```text
XOR R0, R1
XOR R0, 0xFF
```

---

# 29. AND

문법:

```text
AND destination, value
```

예:

```text
AND R0, 0xFF
AND R0, R1
```

---

# 30. OR

문법:

```text
OR destination, value
```

예:

```text
OR R0, 1
OR R0, R1
```

---

# 31. SHL

문법:

```text
SHL destination, amount
```

예:

```text
SHL R0, 1
SHL R0, R1
```

시프트 값은 내부적으로:

```text
amount & 63
```

으로 제한된다.

즉 64비트 레지스터에서 0~63 범위의 시프트가 사용된다.

---

# 32. SHR

문법:

```text
SHR destination, amount
```

예:

```text
SHR R0, 1
SHR R0, R1
```

시프트 값은 역시:

```text
amount & 63
```

으로 처리된다.

---

# 33. CMP

문법:

```text
CMP R0, R1
```

또는:

```text
CMP R0, 10
```

현재 구현은:

```text
last_cmp = R0 - value
```

와:

```text
zero = (R0 == value)
```

를 설정한다.

중요:

> 현재 JE 1.1에는 `JL`, `JG`, `JE`, `JGE`, `JLE` 같은 비교 분기 명령어가 없다.

따라서 실제 분기에는 현재 제공되는:

```text
JZ
JNZ
```

를 사용한다.

---

# 34. INC

문법:

```text
INC R0
```

동작:

```text
R0 = R0 + 1
```

연산 후 `zero` 상태가 갱신된다.

---

# 35. DEC

문법:

```text
DEC R0
```

동작:

```text
R0 = R0 - 1
```

연산 후 `zero` 상태가 갱신된다.

---

# 36. PUSH

문법:

```text
PUSH value
```

value는:

* 레지스터
* 숫자

가 될 수 있다.

예:

```text
PUSH R0
PUSH 123
```

내부 실행 스택에 64비트 값을 저장한다.

현재 스택 용량은:

```text
256
```

항목이다.

---

# 37. POP

문법:

```text
POP R0
```

스택의 마지막 값을 꺼내 레지스터에 저장한다.

예:

```text
MOV R0, 40
PUSH R0
POP R1
```

비어 있는 스택에서 POP하면 오류가 된다.

---

# 38. CALL

CALL에는 두 가지 형태가 있다.

## 38.1 JE 레이블 호출

```text
CALL function_name
```

예:

```text
CALL add_one
```

현재 실행 위치를 호출 스택에 저장한 뒤 레이블로 이동한다.

호출 스택의 최대 깊이는:

```text
256
```

이다.

## 38.2 런타임 호출

```text
CALL IMPORT.malloc
```

형식으로 호스트 런타임 함수를 호출할 수 있다.

---

# 39. RET

문법:

```text
RET
```

CALL로 들어온 함수에서 돌아온다.

예:

```text
main:
    MOV R0, 10
    CALL add
    EXIT 0

add:
    ADD R0, 5
    RET
```

CALL 스택이 비어 있는데 `RET`이 실행되면 현재 구현은 해당 실행을 종료시키는 방향으로 동작한다.

---

# 40. JMP

문법:

```text
JMP label
```

예:

```text
JMP loop
```

무조건 해당 레이블로 이동한다.

---

# 41. JZ

문법:

```text
JZ label
```

현재:

```text
zero != 0
```

상태인 경우 label로 이동한다.

주로 다음과 함께 사용한다.

```text
CMP R0, 0
JZ done
```

또는:

```text
DEC R0
JZ done
```

---

# 42. JNZ

문법:

```text
JNZ label
```

`zero` 상태가 거짓이면 label로 이동한다.

예:

```text
loop:
    DEC R0
    JNZ loop
```

---

# 43. EXIT

문법:

```text
EXIT 0
```

또는:

```text
EXIT 1
```

종료 코드는 현재 내부에서 8비트 값으로 제한된다.

즉 실질적으로:

```text
exit_code = value & 0xFF
```

로 처리된다.

실행을 종료시키고 프로세스 호출자에게 종료 코드를 반환한다.

---

# 44. STR

문법:

```text
STR R0, "문자열"
```

또는 대상 레지스터를 생략하여:

```text
STR "문자열"
```

사용할 수 있다.

대상 생략 시 기본 목적지는:

```text
R0
```

이다.

예:

```text
STR R0, "Hello"
```

문자열은 JE 런타임에서 관리되는 메모리에 복사된다.

`R0`에는 해당 메모리의 포인터가 들어간다.

---

# 45. PRINT

문법:

```text
PRINT R0
```

이면 레지스터의 숫자 값을 출력한다.

예:

```text
MOV R0, 123
PRINT R0
```

출력:

```text
123
```

문자열을 직접 출력할 수도 있다.

```text
PRINT "Hello"
```

출력 후 줄바꿈이 추가된다.

---

# 46. PRINTS

문법:

```text
PRINTS R0
```

R0가 가리키는 JE 소유 문자열을 출력한다.

일반적인 사용:

```text
STR R0, "Hello"
PRINTS R0
```

현재 구현은 R0가 유효한 JE 소유 allocation을 가리키는지 검사한다.

임의의 포인터를 넣고 `PRINTS`하는 것은 허용되지 않는다.

---

# 47. 문자열 이스케이프

현재 `STR` 문자열 파서에서 지원되는 이스케이프:

```text
\n
\r
\t
\"
\\
```

예:

```text
STR R0, "Hello\nWorld"
```

또는:

```text
STR R0, "A\tB"
```

예:

```text
STR R0, "quote: \"hello\""
```

알 수 없는 이스케이프는 오류가 된다.

예:

```text
STR R0, "\q"
```

현재 구현에서는 허용되지 않는다.

---

# 48. 런타임 ABI

JE 1.1에는 호스트 C 런타임 기반의 제한된 IMPORT ABI가 있다.

인자 전달은:

```text
R0
R1
R2
R3
```

를 사용한다.

일반적으로:

```text
R0 = 첫 번째 인자
R1 = 두 번째 인자
R2 = 세 번째 인자
R3 = 네 번째 인자
```

반환값은 일반적으로:

```text
R0
```

에 놓인다.

예:

```text
MOV R0, 64
CALL IMPORT.malloc
```

호출 후:

```text
R0 = 할당된 메모리 포인터
```

이다.

---

# 49. malloc

문법:

```text
CALL IMPORT.malloc
```

인자:

```text
R0 = size
```

반환:

```text
R0 = pointer
```

예:

```text
MOV R0, 64
CALL IMPORT.malloc
MOV R4, R0
```

JE 런타임은 해당 allocation을 추적한다.

---

# 50. calloc

문법:

```text
CALL IMPORT.calloc
```

인자:

```text
R0 = count
R1 = size
```

반환:

```text
R0 = pointer
```

예:

```text
MOV R0, 16
MOV R1, 8
CALL IMPORT.calloc
```

총 크기는 내부적으로:

```text
count * size
```

계산된다.

곱셈 오버플로가 발생하면 오류다.

---

# 51. realloc

문법:

```text
CALL IMPORT.realloc
```

인자:

```text
R0 = 기존 포인터
R1 = 새 크기
```

반환:

```text
R0 = 새 포인터
```

기존 포인터가 JE가 추적하고 있는 allocation이어야 한다.

예:

```text
MOV R0, 64
CALL IMPORT.malloc
MOV R4, R0

MOV R0, R4
MOV R1, 128
CALL IMPORT.realloc
MOV R4, R0
```

---

# 52. free

문법:

```text
CALL IMPORT.free
```

인자:

```text
R0 = pointer
```

예:

```text
MOV R0, R4
CALL IMPORT.free
```

JE 런타임이 해당 포인터를 추적하고 있어야 한다.

해제 후:

```text
R0 = 0
```

이 된다.

---

# 53. memcpy

문법:

```text
CALL IMPORT.memcpy
```

인자:

```text
R0 = destination
R1 = source
R2 = number_of_bytes
```

예:

```text
MOV R0, R4
MOV R1, R5
MOV R2, 16
CALL IMPORT.memcpy
```

destination과 source 모두 JE가 소유한 유효 allocation 범위 안이어야 한다.

---

# 54. memset

문법:

```text
CALL IMPORT.memset
```

인자:

```text
R0 = destination
R1 = byte_value
R2 = number_of_bytes
```

예:

```text
MOV R0, R4
MOV R1, 0
MOV R2, 64
CALL IMPORT.memset
```

R1의 하위 8비트만 사용된다.

즉:

```text
R1 & 0xFF
```

값으로 채운다.

---

# 55. fopen

문법:

```text
CALL IMPORT.fopen
```

인자:

```text
R0 = filename pointer
R1 = mode pointer
```

두 포인터는 모두 JE가 소유한 NUL 종료 문자열이어야 한다.

예:

```text
STR R0, "test.bin"
MOV R4, R0

STR R0, "wb"
MOV R5, R0

MOV R0, R4
MOV R1, R5
CALL IMPORT.fopen

MOV R6, R0
```

반환:

```text
R0 = FILE*
```

이다.

JE 런타임이 해당 파일 핸들을 추적한다.

---

# 56. fread

문법:

```text
CALL IMPORT.fread
```

인자:

```text
R0 = buffer
R1 = element_size
R2 = element_count
R3 = file
```

예:

```text
MOV R0, R4
MOV R1, 1
MOV R2, 64
MOV R3, R6
CALL IMPORT.fread
```

반환:

```text
R0 = 실제 읽은 element 수
```

---

# 57. fwrite

문법:

```text
CALL IMPORT.fwrite
```

인자:

```text
R0 = buffer
R1 = element_size
R2 = element_count
R3 = file
```

예:

```text
MOV R0, R4
MOV R1, 1
MOV R2, 18
MOV R3, R6
CALL IMPORT.fwrite
```

반환:

```text
R0 = 실제 기록된 element 수
```

---

# 58. fclose

문법:

```text
CALL IMPORT.fclose
```

인자:

```text
R0 = FILE*
```

예:

```text
MOV R0, R6
CALL IMPORT.fclose
```

성공하면:

```text
R0 = 0
```

이 된다.

---

# 59. fseek

문법:

```text
CALL IMPORT.fseek
```

인자:

```text
R0 = FILE*
R1 = offset
R2 = origin
```

origin은 다음 세 값만 허용한다.

```c
SEEK_SET
SEEK_CUR
SEEK_END
```

즉 값 자체는 현재 C 런타임의 상수값을 사용한다.

Windows에서는 JE 1.1 런타임이 `_fseeki64()`를 사용한다.

따라서 Windows 대용량 파일에 대한 파일 위치 이동에서 32비트 `long` 문제를 피한다.

주의:

현재 JE 레지스터는 unsigned 64-bit 값으로 저장되므로 음수 offset을 직접 숫자 리터럴로 쓰는 문법은 없다.

---

# 60. fflush

문법:

```text
CALL IMPORT.fflush
```

인자:

```text
R0 = FILE*
```

성공하면:

```text
R0 = 0
```

실패하면:

```text
R0 = 1
```

이 된다.

---

# 61. 메모리 소유권

JE 1.1 런타임에서 다음으로 생성된 메모리는 JE가 추적한다.

```text
IMPORT.malloc
IMPORT.calloc
IMPORT.realloc
STR
```

런타임은 최대:

```text
128
```

개의 allocation을 추적한다.

프로그램이 끝날 때 아직 남아 있는 allocation은 자동으로 정리된다.

---

# 62. 파일 핸들 소유권

JE 런타임은 최대:

```text
64
```

개의 파일 핸들을 추적한다.

프로그램 종료 시 아직 열린 파일은 자동으로 `fclose()` 처리된다.

따라서:

```text
fopen
```

만 하고 `fclose`하지 않아도 런타임 종료 정리가 존재한다.

그러나 정상적인 프로그램은 직접:

```text
CALL IMPORT.fclose
```

를 호출하는 것을 권장한다.

---

# 63. 포인터 검증

JE 런타임은 임의 포인터를 그대로 사용하지 않는다.

예를 들어 `free`는:

```text
R0
```

가 JE runtime이 추적하는 allocation인지 확인한다.

`memcpy`, `memset`, `fread`, `fwrite`도 지정된 버퍼가 JE allocation 범위 안에 있는지 검사한다.

따라서 다음과 같은 임의 숫자를 포인터처럼 사용하는 것은 정상적인 사용 방법이 아니다.

```text
MOV R0, 1234
CALL IMPORT.free
```

---

# 64. 문자열 포인터

`fopen`은 두 개의 문자열을 필요로 한다.

```text
filename
mode
```

현재 구현은 두 문자열 모두:

1. JE가 추적하는 allocation이어야 하고
2. NUL 종료 문자가 존재해야 한다.

따라서 다음 방식이 안전하다.

```text
STR R0, "file.bin"
MOV R4, R0

STR R0, "rb"
MOV R5, R0

MOV R0, R4
MOV R1, R5
CALL IMPORT.fopen
```

---

# 65. ZERO 상태

JE 실행 컨텍스트에는 `zero` 상태가 있다.

현재 다음 명령어들이 이를 갱신한다.

```text
ADD
SUB
XOR
AND
OR
SHL
SHR
CMP
INC
DEC
STR
```

예:

```text
MOV R0, 1
DEC R0
JZ done
```

첫 번째 감소 후:

```text
R0 == 0
```

이므로:

```text
JZ done
```

이 실행된다.

`CALL IMPORT.*`가 항상 `zero`를 같은 방식으로 설정한다고 가정하면 안 된다.

런타임 함수마다 명시적으로 상태를 갱신하는 정도가 다르므로 분기 전에 필요하면 별도의 비교를 수행하는 것이 안전하다.

---

# 66. CALL 동작

지역 함수 호출:

```text
CALL function
```

이면:

```text
현재 IP + 1
```

을 call stack에 저장한 뒤 function label로 이동한다.

런타임 호출:

```text
CALL IMPORT.malloc
```

이면 로컬 레이블 검색이 아니라 runtime dispatcher를 호출한다.

또한 현재 구현에는 다음과 같은 alias 처리가 있다.

```text
IMPORT.CRT.malloc
IMPORT.CRT.free
...
```

지원되는 런타임 함수라면 일반적으로:

```text
IMPORT.malloc
```

형식을 사용하는 것을 권장한다.

---

# 67. 실행 순서

현재 실행 과정은 대략 다음과 같다.

```text
JE 파일 읽기
    ↓
JE 매직/헤더 파싱
    ↓
CODE 블록 탐색
    ↓
CODE 내부 문자열 추출
    ↓
JE 프로그램 빌드
    ↓
레이블 등록
    ↓
첫 실행 줄부터 실행
    ↓
명령어 처리
    ↓
HALT/EXIT/오류
    ↓
런타임 리소스 정리
```

현재 명령어 단계에는 최대 실행 스텝 제한이 있다.

기본값:

```text
1,000,000
```

이다.

무한 루프를 방지하기 위한 안전장치다.

---

# 68. ENTRY에 대한 주의

현재 JE 1.1 구현에서 아주 중요한 사항이다.

다음:

```text
SYSTEM {
    ENTRY = main
}
```

은 JE 문서 구조상 엔트리를 표현하지만, 현재 실행기는 이 값을 읽어 코드의 실행 위치를 재배치하지 않는다.

따라서 권장 구조:

```text
CODE {
    main:
        ...
}
```

즉:

> **현재 JE 1.1에서는 `main:`을 CODE 블록 안의 첫 실행 코드 위치로 두어라.**

향후 native loader 또는 완전한 entry-point resolver를 추가하면 이 부분이 바뀔 수 있다.

---

# 69. CODE 블록 탐색

현재 로더는 파일 전체에서:

```text
CODE
```

라는 토큰을 찾는다.

그 뒤 공백을 건너뛴 후:

```text
{
```

가 있으면 CODE 블록으로 인정한다.

따라서 다음은 탐색 단계에서 사용할 수 있다.

```text
CODE {
```

```text
CODE{
```

```text
CODE     {
```

권장 스타일은:

```text
CODE {
```

이다.

---

# 70. 중괄호 규칙

CODE 블록은 중괄호 깊이를 계산하여 닫는 위치를 찾는다.

기본:

```text
CODE {
    ...
}
```

현재 로더는 중첩된 `{`와 `}`도 깊이로 계산한다.

문자열 내부의 `{`와 `}`는 실제 블록 깊이로 계산하지 않는다.

예:

```text
PRINT "{ test }"
```

문자열 안의 중괄호는 문자열 데이터다.

---

# 71. 잘못된 JE 예제

다음은 매직 오류다.

```text
[JE : O]
```

올바른:

```text
[ JE : O]
```

---

다음은 버전 형식 오류다.

```text
VERSION = 1.1
```

현재 형식:

```text
VERSION = 1
```

---

다음은 ARCH 오류다.

```text
ARCH = AMD64
```

현재 파서가 허용하는 이름:

```text
ARCH = X86_64
```

---

다음은 레지스터 오류다.

```text
MOV R8, 10
```

현재 레지스터는:

```text
R0 ~ R7
```

까지다.

---

다음은 지원되지 않는 명령어다.

```text
JLE done
```

현재 구현에는 `JLE`가 없다.

---

다음은 존재하지 않는 레이블 호출이다.

```text
CALL unknown_function
```

---

# 72. 올바른 JE 예제

```text
[ JE : O]

AUTHOR = "JE"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

CODE {
    main:
        MOV R0, 10
        ADD R0, 5
        PRINT R0
        EXIT 0
}
```

---

# 73. 최소 실행 파일

최소한의 실용적인 실행 구조:

```text
[ JE : O]

VERSION = 1
ARCH = X86_64

CODE {
    main:
        EXIT 0
}
```

실제 CLI와 파일 포맷 일관성을 위해서는 다음과 같은 확장 헤더를 권장한다.

```text
[ JE : O]

AUTHOR = "JE"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
}

CODE {
    main:
        EXIT 0
}
```

---

# 74. 메모리 예제

```text
[ JE : O]

AUTHOR = "JE"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

CODE {
    main:
        MOV R0, 64
        CALL IMPORT.malloc
        MOV R4, R0

        MOV R0, R4
        MOV R1, 0
        MOV R2, 64
        CALL IMPORT.memset

        STR R0, "JE malloc/free works"
        MOV R5, R0

        MOV R0, R5
        PRINTS R0

        MOV R0, R5
        CALL IMPORT.free

        MOV R0, R4
        CALL IMPORT.free

        EXIT 0
}
```

---

# 75. 파일 I/O 예제

```text
[ JE : O]

AUTHOR = "JE"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

SECTION .text {
    CODE {
        main:
            STR R0, "test.tmp"
            MOV R4, R0

            STR R0, "wb"
            MOV R5, R0

            MOV R0, R4
            MOV R1, R5
            CALL IMPORT.fopen
            MOV R6, R0

            STR R0, "Hello JE!\n"
            MOV R7, R0

            MOV R0, R7
            MOV R1, 1
            MOV R2, 11
            MOV R3, R6
            CALL IMPORT.fwrite

            MOV R0, R6
            CALL IMPORT.fclose

            EXIT 0
    }
}
```

---

# 76. 함수 호출 예제

```text
[ JE : O]

VERSION = 1
ARCH = X86_64

CODE {
    main:
        MOV R0, 40
        CALL add_one
        PRINT R0
        EXIT 0

    add_one:
        ADD R0, 1
        RET
}
```

실행:

```text
41
```

---

# 77. 루프 예제

```text
[ JE : O]

VERSION = 1
ARCH = X86_64

CODE {
    main:
        MOV R0, 5

    loop:
        PRINT R0
        DEC R0
        JNZ loop

        EXIT 0
}
```

---

# 78. 스택 예제

```text
[ JE : O]

VERSION = 1
ARCH = X86_64

CODE {
    main:
        MOV R0, 40
        PUSH R0

        MOV R0, 100
        PUSH R0

        POP R1
        POP R2

        PRINT R1
        PRINT R2

        EXIT 0
}
```

스택에서는 마지막으로 PUSH한 값이 먼저 POP된다.

---

# 79. 주석 사용 예제

```text
[ JE : O]

# 프로그램 메타데이터
VERSION = 1
ARCH = X86_64

CODE {
    main:
        # 숫자 초기화
        MOV R0, 10

        # 값 증가
        ADD R0, 5

        # 출력
        PRINT R0

        EXIT 0
}
```

문자열 안의 `#`는 주석으로 처리되지 않는다.

```text
PRINT "hello # not comment"
```

---

# 80. 명령어 표

| 명령어      | 문법            | 설명              |
| -------- | ------------- | --------------- |
| `NOP`    | `NOP`         | 아무 동작 없음        |
| `MOV`    | `MOV R0, R1`  | 값 복사            |
| `ADD`    | `ADD R0, 1`   | 더하기             |
| `SUB`    | `SUB R0, 1`   | 빼기              |
| `XOR`    | `XOR R0, R1`  | XOR             |
| `AND`    | `AND R0, R1`  | AND             |
| `OR`     | `OR R0, R1`   | OR              |
| `SHL`    | `SHL R0, 1`   | 왼쪽 시프트          |
| `SHR`    | `SHR R0, 1`   | 오른쪽 시프트         |
| `CMP`    | `CMP R0, R1`  | 비교 및 zero 설정    |
| `INC`    | `INC R0`      | 1 증가            |
| `DEC`    | `DEC R0`      | 1 감소            |
| `PUSH`   | `PUSH R0`     | 스택 push         |
| `POP`    | `POP R0`      | 스택 pop          |
| `CALL`   | `CALL name`   | 함수 호출           |
| `RET`    | `RET`         | 함수 복귀           |
| `JMP`    | `JMP label`   | 무조건 점프          |
| `JZ`     | `JZ label`    | zero면 점프        |
| `JNZ`    | `JNZ label`   | zero가 아니면 점프    |
| `EXIT`   | `EXIT 0`      | 종료              |
| `STR`    | `STR R0, "x"` | JE 문자열 생성       |
| `PRINT`  | `PRINT R0`    | 숫자 또는 직접 문자열 출력 |
| `PRINTS` | `PRINTS R0`   | JE 문자열 출력       |

---

# 81. 런타임 함수 표

| 함수               | R0          | R1       | R2     | R3   | 반환      |
| ---------------- | ----------- | -------- | ------ | ---- | ------- |
| `IMPORT.malloc`  | size        | -        | -      | -    | pointer |
| `IMPORT.calloc`  | count       | size     | -      | -    | pointer |
| `IMPORT.realloc` | old pointer | new size | -      | -    | pointer |
| `IMPORT.free`    | pointer     | -        | -      | -    | R0=0    |
| `IMPORT.memcpy`  | dst         | src      | length | -    | dst     |
| `IMPORT.memset`  | dst         | byte     | length | -    | dst     |
| `IMPORT.fopen`   | path        | mode     | -      | -    | `FILE*` |
| `IMPORT.fread`   | buffer      | size     | count  | file | count   |
| `IMPORT.fwrite`  | buffer      | size     | count  | file | count   |
| `IMPORT.fclose`  | file        | -        | -      | -    | 0/실패    |
| `IMPORT.fseek`   | file        | offset   | origin | -    | 0/실패    |
| `IMPORT.fflush`  | file        | -        | -      | -    | 0/실패    |

---

# 82. CLI

JE 실행 프로그램 이름:

```text
je
```

사용 가능한 명령은 현재 다음과 같다.

```text
je <file.jxj>
je detect <file>
je run <file.jxj>
je launch <file>
je pe2je <in.exe> <out.jxj>
je je2pe <in.jxj> <out.exe>
je validate <file>
```

---

# 83. detect

명령:

```text
je detect file
```

파일이 JE인지 PE인지 감지하는 용도다.

JE 파일의 경우 현재 구현은 매직과 유효한 헤더를 확인한다.

표시 결과는 내부 enum 값이다.

---

# 84. run

명령:

```text
je run program.jxj
```

JE 텍스트 코드를 읽고 실행한다.

예:

```text
je run examples\hello.jxj
```

---

# 85. launch

명령:

```text
je launch file
```

JE 또는 지원되는 PE 실행 파일을 dispatch하여 실행한다.

즉 개념적으로:

```text
JE → JE 실행기
PE → Windows 프로세스 실행
```

경로로 나뉠 수 있다.

---

# 86. validate

명령:

```text
je validate program.jxj
```

현재 validate는 다음 단계의 검사를 수행한다.

```text
파일 읽기
↓
JE 헤더 파싱
↓
CODE 블록 위치 확인
↓
CODE 블록 종료 확인
↓
JE CODE 줄 구조 검사
↓
레이블 중복/형식 검사
```

성공하면:

```text
valid JE
```

를 출력한다.

중요:

> 현재 `validate`는 실제 프로그램을 실행하여 모든 명령어의 런타임 의미를 검사하는 것이 아니다.

즉:

```text
validate 성공
```

과:

```text
실행 시 반드시 논리적으로 원하는 결과가 나옴
```

은 같은 의미가 아니다.

---

# 87. pe2je

명령:

```text
je pe2je input.exe output.jxj
```

PE 실행 파일을 JE 텍스트 표현으로 변환한다.

변환기에서는 PE 메타데이터와 원본 바이트를 표현하기 위해 다음과 같은 블록을 생성할 수 있다.

```text
SYSTEM {
    ENTRY_RVA = ...
    IMAGE_BASE = ...
    SUBSYSTEM = ...
    MACHINE = ...
    SECTION_COUNT = ...
}
```

그리고:

```text
PE_METADATA {
    ...
}
```

및:

```text
SECTIONS {
    ...
}
```

및:

```text
PE_RAW_HEX {
    ...
}
```

등을 생성한다.

---

# 88. je2pe

명령:

```text
je je2pe input.jxj output.exe
```

JE 파일 내부의:

```text
PE_RAW_HEX
```

영역을 읽어 원본 PE 바이트를 복원한다.

현재 변환기는 PE_RAW_HEX가 존재하고 올바른 hex 데이터인지 확인한 뒤 PE 파서를 통해 검증한다.

---

# 89. 직접 실행

CLI는 다음 형식도 지원한다.

```text
je program.jxj
```

이 방식은 Windows 파일 연결과 함께 사용할 수 있도록 추가된 진입 방식이다.

---

# 90. Windows 파일 연결

Windows Explorer에서 `.jxj`를 두 번 클릭하여 실행하려면 파일 연결이 필요하다.

일반적인 연결 대상:

```text
je.exe "%1"
```

저장소에는:

```text
install_jxj_assoc.bat
```

가 포함되어 있다.

주의:

Windows가 `.jxj`를 어느 프로그램으로 열지 등록하지 않으면 두 번 클릭만으로 JE가 실행되지는 않는다.

---

# 91. MinGW-w64 요구 사항

Windows용 JE 1.1의 주 도구체인은:

```text
MinGW-w64
GCC
```

이다.

필수 도구:

```text
cmake
gcc
mingw32-make
```

확인:

```bat
cmake --version
gcc --version
mingw32-make --version
```

---

# 92. CMake 빌드

JE 소스 디렉터리에서:

```bat
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc
```

빌드:

```bat
cmake --build build --config Release --parallel
```

실행 파일:

```text
build\je.exe
```

---

# 93. CMakePresets

현재 프로젝트에는 MinGW Release preset을 둘 수 있다.

예:

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "mingw-release",
            "displayName": "JE Release (MinGW-w64)",
            "generator": "MinGW Makefiles",
            "binaryDir": "${sourceDir}/build-mingw",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_C_COMPILER": "gcc"
            }
        }
    ]
}
```

사용:

```bat
cmake --preset mingw-release
cmake --build build-mingw --parallel
```

---

# 94. Windows 배치 빌드

프로젝트에는:

```text
build_windows.bat
```

가 있다.

이 배치 파일은 다음 항목을 검사하도록 설계되어 있다.

```text
cmake
gcc
mingw32-make
```

그리고 MinGW Makefiles generator를 사용한다.

---

# 95. GitHub Actions

Windows GitHub Actions에서는:

```yaml
runs-on: windows-latest
```

를 사용한다.

MinGW-w64 환경을 설치한 뒤:

```text
-G "MinGW Makefiles"
-DCMAKE_C_COMPILER=gcc
```

를 사용해 CMake를 구성한다.

빌드 후:

```text
ctest
```

를 실행하는 것이 권장된다.

빌드 결과:

```text
je.exe
```

---

# 96. 프로젝트 구조

현재 프로젝트의 중요한 파일 구조는 대략 다음과 같다.

```text
JE-1.1-MinGW-fixed/
│
├── CMakeLists.txt
├── CMakePresets.json
├── README-MINGW.md
├── build.sh
├── build_windows.bat
├── install_jxj_assoc.bat
│
├── include/
│   └── je/
│       ├── je.h
│       ├── je_cli.h
│       ├── je_convert.h
│       ├── je_error.h
│       ├── je_exec.h
│       ├── je_file.h
│       ├── je_loader.h
│       ├── je_log.h
│       ├── je_memory.h
│       ├── je_pe.h
│       ├── je_process.h
│       ├── je_runtime.h
│       └── je_text.h
│
├── src/
│   ├── cli/
│   ├── convert/
│   ├── core/
│   ├── io/
│   ├── loader/
│   ├── memory/
│   ├── pe/
│   └── win/
│
├── tests/
│
├── tools/
│
├── examples/
│   ├── hello.jxj
│   ├── runtime_memory.jxj
│   └── runtime_file_io.jxj
│
└── rust/
    └── jesafe/
```

---

# 97. PE와 JE의 관계

JE는 PE를 완전히 무시하는 포맷이 아니다.

현재 설계는 PE와 친숙한 개념을 유지한다.

예:

```text
.text
.rdata
.data
.bss
imports
exports
relocations
entry point
architecture
subsystem
sections
```

즉 JE는:

```text
PE처럼 구조화된 실행 파일
+
텍스트 표현
+
JE 실행기
```

라는 방향이다.

---

# 98. PE_RAW_HEX

PE → JE 변환에서 가장 중요한 보존 영역 중 하나가:

```text
PE_RAW_HEX {
    ...
}
```

이다.

여기에는 원본 PE 바이트가 16진수 형태로 저장될 수 있다.

예:

```text
PE_RAW_HEX {
    4D 5A 90 00 ...
}
```

JE → PE 변환기는 이 데이터를 다시 바이트로 복원한다.

따라서 JE가 직접 의미를 해석하지 못하는 PE 부분도 원시 바이트 형태로 보존할 수 있다.

---

# 99. 변환 시 생성되는 구조

PE를 JE로 변환하면 다음과 비슷한 구조가 나올 수 있다.

```text
[ JE : O]

AUTHOR = "JE converter"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY_RVA = 0x...
    IMAGE_BASE = 0x...
    SUBSYSTEM = ...
    MACHINE = ...
    SECTION_COUNT = ...
}

PE_METADATA {
    SIZE_OF_IMAGE = ...
    SIZE_OF_HEADERS = ...
    SECTION_ALIGNMENT = ...
    FILE_ALIGNMENT = ...
}

SECTIONS {
    SECTION 0 ".text" {
        RVA = ...
        VIRTUAL_SIZE = ...
        RAW_OFFSET = ...
        RAW_SIZE = ...
        CHARACTERISTICS = ...
    }
}

PE_RAW_HEX {
    ...
}
```

이것은 **PE 구조를 JE 텍스트로 설명하면서 원본 PE 바이트를 보존하는 방식**이다.

---

# 100. 검증 방법

Windows에서 가장 기본적인 검증 순서:

```bat
gcc --version
mingw32-make --version
cmake --version
```

그 다음:

```bat
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc
```

빌드:

```bat
cmake --build build --config Release --parallel
```

테스트:

```bat
ctest --test-dir build --output-on-failure
```

실행:

```bat
build\je.exe run examples\hello.jxj
```

메모리 테스트:

```bat
build\je.exe run examples\runtime_memory.jxj
```

파일 I/O 테스트:

```bat
build\je.exe run examples\runtime_file_io.jxj
```

검증:

```bat
build\je.exe validate examples\hello.jxj
```

---

# 101. 제한 사항

현재 JE 1.1의 중요한 제한은 다음과 같다.

## 101.1 임의 DLL 함수를 직접 호출하지 않는다

다음과 같은 형태가 자동으로 지원되는 것은 아니다.

```text
CALL IMPORT.MyRandomDllFunction
```

현재 IMPORT는 허용된 runtime 함수 집합을 통해 처리된다.

---

## 101.2 완전한 native ABI가 아니다

현재 JE runtime의 IMPORT는 C runtime 기반 host-linked ABI다.

즉:

```text
JE
 ↓
JE runtime dispatcher
 ↓
C runtime / Win32
```

형태다.

JE 코드가 임의의 native 함수 시그니처를 자동 해석하여 호출하는 일반적인 DLL linker는 아니다.

---

## 101.3 현재 엔트리 포인트 구현

`ENTRY = main` 메타데이터는 존재하지만 현재 `je_exec_text()`의 실제 시작 위치를 그 값으로 점프시키지는 않는다.

따라서:

```text
main:
```

을 CODE의 첫 실행 위치에 두는 것이 안전하다.

---

## 101.4 validate의 범위

`validate`는 실제 프로그램을 실행하는 것이 아니다.

따라서 모든 실행 논리 오류를 검출하는 정적 분석기는 아니다.

---

## 101.5 분기 명령어가 적다

현재:

```text
JMP
JZ
JNZ
```

를 지원한다.

다음은 현재 명령어 집합에 없다.

```text
JG
JL
JGE
JLE
JE
JNE
```

---

## 101.6 음수 리터럴

현재 숫자 리터럴 파서에서:

```text
-1
-20
```

을 직접 사용할 수 없다.

---

## 101.7 레지스터 수

현재:

```text
R0 ~ R7
```

까지만 있다.

---

## 101.8 스택 크기

data stack:

```text
256
```

call stack:

```text
256
```

이다.

---

## 101.9 레이블 수

최대:

```text
512
```

레이블이다.

---

## 101.10 코드 줄 수

실행 프로그램의 최대 코드 라인은:

```text
4096
```

이다.

---

## 101.11 한 줄 길이

실행기에서 최대 줄 길이는 약:

```text
1023
```

문자까지를 안전한 입력 범위로 사용하는 것이 좋다.

구현에서는:

```text
JE_EXEC_LINE_MAX = 1024
```

이며 길이 1024 이상은 오류 처리된다.

---

# 102. 현재 구현에서 주의할 점

다음은 특히 중요하다.

### 1. 매직은 정확히 써야 한다

```text
[ JE : O]
```

### 2. VERSION은 현재 숫자 1을 사용한다

```text
VERSION = 1
```

### 3. ARCH는 Windows x64라면 다음을 사용한다

```text
ARCH = X86_64
```

### 4. `main:`은 CODE의 첫 실행 코드로 배치한다

```text
CODE {
    main:
        ...
}
```

### 5. 레지스터는 R0~R7이다

```text
R0
R1
R2
R3
R4
R5
R6
R7
```

### 6. 런타임 문자열은 `STR`로 만드는 것이 안전하다

```text
STR R0, "file.txt"
```

### 7. 파일은 직접 닫는다

```text
CALL IMPORT.fclose
```

자동 cleanup이 존재하지만 명시적인 `fclose`가 권장된다.

---

# 103. 권장 코딩 스타일

JE 실행 코드는 다음 스타일을 권장한다.

```text
[ JE : O]

AUTHOR = "My Program"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

SECTION .text {
    CODE {
        main:
            MOV R0, 10
            ADD R0, 5
            PRINT R0
            EXIT 0
    }
}
```

규칙:

```text
대괄호 매직은 수정하지 않는다.
헤더 키 주위에는 =를 사용한다.
CODE 안에서는 4칸 들여쓰기를 사용한다.
명령어 하나당 한 줄을 사용한다.
두 피연산자 사이에는 ", "를 사용한다.
레이블 뒤에는 ":"를 붙인다.
문자열에는 큰따옴표를 사용한다.
주석은 "#"을 사용한다.
```

이 들여쓰기는 현재 실행기상 필수는 아니지만 **표준 작성 스타일**로 권장한다.

---

# 104. 완전한 JE 예제

다음은 현재 JE 1.1 구현의 주요 요소를 함께 사용하는 예제다.

```text
[ JE : O]

AUTHOR = "JE Example"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

SECTION .text {
    CODE {
        main:
            # 카운터
            MOV R0, 5

        loop:
            PRINT R0
            DEC R0
            JNZ loop

            # 문자열 생성
            STR R0, "JE 1.1"
            MOV R4, R0

            PRINTS R4

            # 문자열 해제
            MOV R0, R4
            CALL IMPORT.free

            EXIT 0
    }
}
```

---

# 105. 최종 문법 요약

## 105.1 매직

정확히:

```text
[ JE : O]
```

파일의 첫 바이트부터 시작해야 한다.

---

## 105.2 권장 헤더

```text
AUTHOR = "..."
FORMAT = "JE"
VERSION = 1
ARCH = X86_64
```

---

## 105.3 시스템 블록

```text
SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}
```

현재 실행기는 `ENTRY` 값을 실제 시작 위치로 자동 사용하지 않는다.

---

## 105.4 코드 블록

```text
CODE {
    main:
        ...
}
```

현재 구현에서는 `main:`을 첫 실행 위치에 배치하는 것을 권장한다.

---

## 105.5 레이블

```text
label:
```

예:

```text
loop:
```

---

## 105.6 레지스터

```text
R0
R1
R2
R3
R4
R5
R6
R7
```

---

## 105.7 숫자

10진수:

```text
123
```

16진수:

```text
0x7B
```

---

## 105.8 문자열

```text
"Hello"
```

이스케이프:

```text
\n
\r
\t
\"
\\
```

---

## 105.9 주석

```text
# comment
```

또는:

```text
MOV R0, 1 # comment
```

문자열 내부의 `#`는 문자열이다.

---

## 105.10 기본 명령

```text
NOP
MOV
ADD
SUB
XOR
AND
OR
SHL
SHR
CMP
INC
DEC
PUSH
POP
CALL
RET
JMP
JZ
JNZ
EXIT
STR
PRINT
PRINTS
```

---

## 105.11 메모리

```text
MOV R0, 64
CALL IMPORT.malloc
```

반환:

```text
R0 = pointer
```

해제:

```text
CALL IMPORT.free
```

---

## 105.12 memcpy

```text
R0 = destination
R1 = source
R2 = length
```

호출:

```text
CALL IMPORT.memcpy
```

---

## 105.13 memset

```text
R0 = destination
R1 = byte
R2 = length
```

호출:

```text
CALL IMPORT.memset
```

---

## 105.14 파일 열기

```text
STR R0, "file.bin"
MOV R4, R0

STR R0, "rb"
MOV R5, R0

MOV R0, R4
MOV R1, R5
CALL IMPORT.fopen
```

---

## 105.15 파일 읽기

```text
MOV R0, buffer
MOV R1, 1
MOV R2, 64
MOV R3, file
CALL IMPORT.fread
```

---

## 105.16 파일 쓰기

```text
MOV R0, buffer
MOV R1, 1
MOV R2, 64
MOV R3, file
CALL IMPORT.fwrite
```

---

## 105.17 파일 닫기

```text
MOV R0, file
CALL IMPORT.fclose
```

---

## 105.18 실행

```bat
je run program.jxj
```

또는:

```bat
je program.jxj
```

---

## 105.19 검증

```bat
je validate program.jxj
```

---

## 105.20 빌드

```bat
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc
cmake --build build --config Release --parallel
```

---

# 결론

현재 **JE 1.1-MinGW-fixed**의 핵심 형식은 다음 한 덩어리로 이해하면 된다.

```text
[ JE : O]

AUTHOR = "프로그램 이름"
FORMAT = "JE"
VERSION = 1
ARCH = X86_64

SYSTEM {
    ENTRY = main
    SUBSYSTEM = CONSOLE
}

CODE {
    main:
        MOV R0, 10
        ADD R0, 5
        PRINT R0
        EXIT 0
}
```

여기서 반드시 기억해야 하는 핵심은 다음과 같다.

```text
[ JE : O]
```

은 **문자 그대로 정확해야 한다.

```text
VERSION = 1
```

현재 구현에서 숫자로 작성한다.

```text
ARCH = X86_64
```

Windows x64용이다.

```text
CODE {
```

은 실제 실행 코드를 시작한다.

```text
main:
```

은 현재 구현에서는 CODE의 첫 실행 위치에 두는 것이 안전하다.

```text
R0 ~ R7
```

이 현재 레지스터 집합이다.

```text
CALL IMPORT.malloc
CALL IMPORT.free
CALL IMPORT.memcpy
CALL IMPORT.memset
CALL IMPORT.fopen
CALL IMPORT.fread
CALL IMPORT.fwrite
CALL IMPORT.fclose
CALL IMPORT.fseek
CALL IMPORT.fflush
```

가 현재 JE 1.1의 주요 host runtime ABI다.

JE의 일반 코드 들여쓰기는 현재 구현상 강제되지 않지만, 다음 형태를 **공식적인 권장 스타일**로 사용한다.

```text
CODE {
    main:
        MOV R0, 10
        PRINT R0
        EXIT 0
}
```

즉 **매직은 엄격하고, 일반 코드 공백은 유연하지만, 가이드에서는 일관된 들여쓰기와 공백 스타일을 사용한다.**

또한 현재 JE 1.1은 **PE와 유사한 텍스트 실행 파일 포맷 + 제한된 C runtime ABI + PE↔JE 변환 인프라**까지 구현된 상태이며, 아직 **임의 DLL 함수의 일반적인 동적 심볼 링크 시스템**까지 구현된 것은 아니다.
