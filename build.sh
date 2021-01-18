#!/bin/bash
cd `dirname $0`
# build in unix like system?
# cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B out/build
# build macOS-Universal
# cmake -DCMAKE_OSX_ARCHITECTURES=arm64;x86_64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B out/build
# build macOS-arm64
# cmake -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B out/build
# build macOS-x86_64
cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B out/build
ln -s out/build/compile_commands.json ./
cd out/build
make
