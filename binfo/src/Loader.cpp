#include <fmt/core.h>

#include <CLI/CLI.hpp>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <tabulate/table.hpp>

#include "LoaderCLI.h"

int main(int argc, char const *argv[])
{
    std::string input_filename;
    std::vector<std::string> section_names;

    bool show_sections = false;
    bool show_symbols = false;
    bool demangle = false;

    CLI::App app("binfo (binary info) is a simple program for obtaining information about ELF or PE binary file.", "binfo");
    app.add_flag("-s,--sections", show_sections, "Show sections");
    app.add_flag("-S,--symbols", show_symbols, "Show symbols");
    app.add_flag("-d,--demangle", demangle, "Demangle symbols");
    app.add_option("filename", input_filename, "Input filename")->required();
    app.add_option("sections", section_names, "List of sections to show");

    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e)
    {
        return app.exit(e);
    }

    LoaderCLI loader_cli(input_filename);

    if (show_sections)
    {
        loader_cli.show_sections();
        std::cout << std::endl;
    }

    if (show_symbols)
    {
        loader_cli.show_symbols(demangle);
        std::cout << std::endl;
    }

    for (const auto &section_name : section_names)
    {
        loader_cli.show_section_data(section_name);
        std::cout << std::endl;
    }

    return 0;
}
