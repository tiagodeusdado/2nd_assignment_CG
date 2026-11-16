@echo off
setlocal
echo =======================================================
echo Compiling UBI CG Assignment 2 - Aircraft Takeoff Sim...
echo =======================================================

rem Close any previous running instance so Windows doesn't lock the executable
taskkill /f /im flight_sim.exe >nul 2>&1
taskkill /f /im flight_sim_takeoff.exe >nul 2>&1

set CXX=C:\msys64\ucrt64\bin\g++.exe
if not exist "%CXX%" (
    set CXX=g++
)

set CXXFLAGS=-std=c++17 -O2 -DGLEW_STATIC
set INCLUDES=-I"deps/glfw-3.3.8.bin.WIN64/include" -I"deps/glew-2.2.0/include" -I"deps/glm" -I"src"
set SOURCES=src/main.cpp src/Shader.cpp src/Model.cpp src/FlightSimulator.cpp src/SceneExtras.cpp deps/glew-2.2.0/src/glew.c
set LIBS=-L"deps/glfw-3.3.8.bin.WIN64/lib-mingw-w64" -lglfw3 -lopengl32 -lgdi32 -luser32 -lshell32

if not exist bin (
    mkdir bin
)

echo Invoking compiler (%CXX%)...
%CXX% %CXXFLAGS% %INCLUDES% %SOURCES% %LIBS% -o bin/flight_sim.exe

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Build failed! Check compiler output above.
    pause
    exit /b %errorlevel%
) else (
    echo.
    echo [SUCCESS] Build succeeded! Executable created at bin\flight_sim.exe
    echo.
    echo Running simulation...
    bin\flight_sim.exe
)
endlocal
