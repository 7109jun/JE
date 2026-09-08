@echo off
setlocal
set "CMAKE_GENERATOR=MinGW Makefiles"

where cmake >nul 2>&1 || (echo CMake was not found.& exit /b 1)
where gcc >nul 2>&1 || (echo MinGW-w64 GCC was not found. Add MinGW-w64 bin to PATH.& exit /b 1)
where mingw32-make >nul 2>&1 || (echo mingw32-make was not found. Install MinGW-w64 and add its bin directory to PATH.& exit /b 1)

gcc --version
mingw32-make --version

echo.
echo Configuring JE with MinGW-w64 GCC...
cmake -S . -B build-windows -G "%CMAKE_GENERATOR%" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc || exit /b 1

echo.
echo Building JE...
cmake --build build-windows --parallel || exit /b 1

echo.
echo Running tests...
ctest --test-dir build-windows --output-on-failure || exit /b 1

echo.
echo MinGW-w64 build completed: build-windows\je.exe
endlocal
