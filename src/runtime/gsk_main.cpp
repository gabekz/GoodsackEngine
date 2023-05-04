#include <runtime/gsk_runtime.hpp>

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int
main(int argc, char *argv[])
{
    gsk_runtime_setup(argc, argv);
    gsk_runtime_loop();

    std::cout << "Current path is " << fs::current_path() << '\n'; // (1)
    fs::current_path(fs::temp_directory_path());                   // (3)
    std::cout << "Current path is " << fs::current_path() << '\n';
}