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

    int bytes_per_row = 16;

    CLI::App app("binfo (binary info) is a simple program for obtaining information about ELF or PE binary file.", "binfo");
    app.add_flag("-s,--sections", show_sections, "Show sections");
    app.add_flag("-S,--symbols", show_symbols, "Show symbols");
    app.add_flag("-d,--demangle", demangle, "Demangle symbols");
    app.add_option("-b,--bytes_per_row", bytes_per_row, "Number of bytes per row for section data")->check(CLI::Range(1, 128));
    app.add_option("filename", input_filename, "Input filename")->required()->check(CLI::ExistingFile);
    app.add_option("sections", section_names, "List of sections to show");

    app.validate_optional_arguments();
    app.validate_positionals();

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
        loader_cli.show_section_data(section_name, bytes_per_row);
        std::cout << std::endl;
    }

    return 0;
}
