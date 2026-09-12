cmake_minimum_required(VERSION 3.15)
project(TE_TextExecutable VERSION 1.0 LANGUAGES CXX)

# C++ 표준
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# 출력 디렉토리
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ===== TE Parser 라이브러리 =====
add_library(te_parser
    te_parser.cpp
    te_parser.h
)

target_include_directories(te_parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ===== 메인 실행 파일 =====
add_executable(te_demo
    main.cpp
)

target_link_libraries(te_demo PRIVATE
    te_parser
)

# ===== 설치 =====
install(TARGETS te_demo
    RUNTIME DESTINATION bin
)

install(TARGETS te_parser
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES te_parser.h
    DESTINATION include/te
)

# ===== 테스트 (필요시) =====
enable_testing()

# 샘플 파일 생성 (설치 후)
install(FILES
    examples/hello_world.yac
    examples/calculator.yac
    DESTINATION share/te/examples
    OPTIONAL
)
