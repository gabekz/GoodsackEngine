<# 
 # Copyright (c) 2022-2023, Gabriel Kutuzov
 # SPDX-License-Identifier: MIT
-------------------------------------------------------------------------------
.TITLE
build.ps1
-------------------------------------------------------------------------------
#>

$run_demo = ($args[0] -eq "--demo");
$run_tests = ($args[0] -eq "--test");

# check cmake version
try {
    & 'cmake.exe' --version
} catch {
    Write-Warning "Missing cmake.exe in system path"
    Exit
}

# check msbuild version
try {
    & 'MSBuild.exe' -v
} catch {
    Write-Warning "Missing MSBuild.exe in system path"
    Exit
}

# run cmake with vcpkg
& 'cmake.exe' -S . -B build/ `
    -G "Visual Studio 17 2022" `
    -DCMAKE_TOOLCHAIN_FILE="D:/Projects/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake"

# msbuild
& 'MSBuild.exe' .\build\GoodsackEngine.sln

if($run_demo) {
    & .\build\output\bin\Debug\demo.exe
}
elseif($run_tests) {
    & .\build\output\bin\Debug\GoodsackEngine_Test.exe
}