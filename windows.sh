#!/bin/bash

VCPKG=$(pwd)/../vcpkg/
CMAKE=${VCPKG}/downloads/tools/cmake-3.14.0-windows/cmake-3.14.0-win32-x86/bin/cmake.exe
TOOLCHAIN=${VCPKG}/scripts/buildsystems/vcpkg.cmake

cd build/
${CMAKE} -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} -DWIN32=on -G "Visual Studio 15 2017 Win64" ../control/
