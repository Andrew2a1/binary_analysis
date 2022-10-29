#include <fmt/core.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <tabulate/table.hpp>

#include "LoaderCLI.h"

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <binary> [section]" << std::endl;
        return 1;
    }

    const std::string binary_name = argv[1];
    LoaderCLI loader_cli(binary_name);

    loader_cli.show_sections();
    std::cout << std::endl;

    loader_cli.show_symbols();
    std::cout << std::endl;

    if (argc == 3)
    {
        const std::string section_name = argv[2];
        loader_cli.show_section_data(section_name);
    }

    return 0;
}
