@echo off
cd /d %~dp0
echo off
rem Generate Vsual Studio 2019 project.
cmake -G"Visual Studio 16 2019" -T"ClangCl" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B out/build
cd out/build
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
MSBuild.exe ALL_BUILD.vcxproj -property:Configuration=Debug