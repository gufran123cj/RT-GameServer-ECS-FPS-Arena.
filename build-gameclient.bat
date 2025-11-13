@echo off
echo ========================================
echo Top-Down 2D Game Client Build Script
echo ========================================
echo.

REM MinGW path kontrol
set MINGW_PATH=D:\MinGW\bin
if not exist "%MINGW_PATH%\g++.exe" (
    echo HATA: g++ bulunamadi!
    echo MinGW path'ini kontrol edin: %MINGW_PATH%
    pause
    exit /b 1
)

REM Raylib kontrol
if not exist "raylib\include\raylib.h" (
    echo.
    echo UYARI: Raylib bulunamadi!
    echo.
    echo Raylib kurulumu icin:
    echo 1. https://github.com/raysan5/raylib/releases adresinden indirin
    echo 2. raylib.h dosyasini raylib\include\ klasorune koyun
    echo 3. Veya RAYLIB_SETUP.md dosyasini okuyun
    echo.
    pause
    exit /b 1
)

echo Game Client derleme basladi...
echo.

REM Eğer GameClient.exe çalışıyorsa kapat
taskkill /F /IM GameClient.exe >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo GameClient.exe kapatildi.
    timeout /t 1 /nobreak >nul
)

REM Game client'ı derle
REM Not: Raylib static library gerekli (libraylib.a)
REM Eğer header-only kullanıyorsanız, RAYLIB_STANDALONE define edin
"%MINGW_PATH%\g++.exe" ^
    -std=c++17 ^
    -O3 ^
    -Wall ^
    -Wextra ^
    -D_MINGW_EXTENSION=__extension__ ^
    -D__USE_MINGW_ANSI_STDIO=1 ^
    -U__STRICT_ANSI__ ^
    -fpermissive ^
    -Wno-unknown-pragmas ^
    -DRAYLIB_STANDALONE ^
    -I. ^
    -Iinclude ^
    -Iecs ^
    -Inet ^
    -Iphysics ^
    -Imatchmaker ^
    -Ianti-cheat-lite ^
    -Icomponents ^
    -Iraylib\include ^
    -Ildtk ^
    -Iassets ^
    -Iinclude\json ^
    src\GameClient.cpp ^
    net\Socket.cpp ^
    ldtk\LDtkParser.cpp ^
    assets\AssetManager.cpp ^
    -o GameClient.exe ^
    -Lraylib\lib ^
    -lws2_32 ^
    -lraylib ^
    -lopengl32 ^
    -lgdi32 ^
    -lwinmm

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Game Client derleme BASARILI!
    echo ========================================
    echo.
    echo Calistirmak icin: GameClient.exe
    echo Veya: GameClient.exe 127.0.0.1 7777
    echo.
) else (
    echo.
    echo ========================================
    echo Game Client derleme BASARISIZ!
    echo ========================================
    echo.
    echo Not: Raylib library gerekli!
    echo RAYLIB_SETUP.md dosyasini okuyun.
    echo.
)

pause

