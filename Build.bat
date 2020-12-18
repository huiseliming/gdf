@echo off
cd /d %~dp0
echo off
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" x64
rem Generate Vsual Studio 2019 project.
cmake -G "Ninja" -S . -B out/build
pause