Get-ChildItem -Path . -Directory -Recurse |
    foreach {
        cd $_.FullName
        &clang-format -i -style=file *.c *.h *.cpp *.hpp
    }