@echo off
cd /d %~dp0
echo off
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
rem Generate Vsual Studio 2019 project.
cmake -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B out/build
cd out/build
ninja
