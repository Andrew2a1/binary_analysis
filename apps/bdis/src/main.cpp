#include <BFDLoader.h>
#include <Binary.h>
#include <fmt/format.h>

#include <CLI/CLI.hpp>
#include <iostream>
#include <map>
#include <queue>
#include <stdexcept>

#include "CapstoneWrapper.h"
#include "Disasm.h"
#include "LinearDisasm.h"
#include "RecursiveDisasm.h"

std::unique_ptr<Disasm> create_disassembler(const CapstoneWrapper &capstone, bool recursive)
{
    if (recursive)
    {
        return std::unique_ptr<Disasm>(new RecursiveDisasm(capstone));
    }
    return std::unique_ptr<Disasm>(new LinearDisasm(capstone));
}

int main(int argc, char *argv[])
{
    std::filesystem::path input_filename;
    std::string section_name = ".text";
    bool use_recursive = false;

    CLI::App app("bdis (binary disassembler) is a program for linear and recursive disassemble of an executable file.", "bdis");
    app.add_option("filename", input_filename, "Input executable to disassemble")->required()->check(CLI::ExistingFile);
    app.add_option("section", section_name, "Section name to disassemble");
    app.add_flag("-r,--recursive", use_recursive, "Use recursive mode of disassembler");
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

    try
    {
        BFDLoader loader;
        Binary binary = loader.load_binary(input_filename);

        CapstoneWrapper capstone(binary.arch, binary.bits);
        const auto disasm = create_disassembler(capstone, use_recursive);
        disasm->disasm(binary, section_name);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
