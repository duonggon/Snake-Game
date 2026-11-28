@echo off
chcp 65001 >nul
set "PATH=C:\msys64\ucrt64\bin;%PATH%"
echo ============================================================================
echo   BIEN DICH VA KIEM THU MODULE THANH VIEN 2 - SNAKE ^& APPLE SYSTEM
echo   Sinh vien: Dao Duy Khoi (24137037)
echo ============================================================================

set "GPP_PATH=C:\msys64\ucrt64\bin\g++.exe"
if not exist "%GPP_PATH%" (
    set "GPP_PATH=g++"
)

echo [1/2] Dang bien dich voi C++17 (Static Link)...
"%GPP_PATH%" -std=c++17 -Wall -Wextra -O2 -static -static-libgcc -static-libstdc++ ^
    main.cpp ^
    Player/Snake.cpp ^
    Player/Apple.cpp ^
    Player/NormalApple.cpp ^
    Player/HealthApple.cpp ^
    Player/EnergyApple.cpp ^
    Player/AppleSpawner.cpp ^
    -o SnakeBreaker_Test.exe

if %ERRORLEVEL% NEQ 0 (
    echo [LOI] Bien dich that bai!
    pause
    exit /b 1
)

echo [2/2] Bien dich thanh cong: SnakeBreaker_Test.exe
echo.
if "%1"=="--test" (
    SnakeBreaker_Test.exe --test
) else (
    SnakeBreaker_Test.exe
)
