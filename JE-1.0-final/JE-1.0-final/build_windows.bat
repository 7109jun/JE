@echo off
setlocal
where cmake >nul 2>&1 || (echo CMake was not found.& exit /b 1)
where gcc >nul 2>&1 || (echo GCC was not found. Install MinGW-w64 or add it to PATH.& exit /b 1)
cmake -S . -B build-windows -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build build-windows --config Release || exit /b 1
ctest --test-dir build-windows --output-on-failure || exit /b 1
echo.
echo Build completed: build-windows\je.exe
endlocal
