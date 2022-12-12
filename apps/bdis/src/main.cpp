#include <BFDLoader.h>
#include <Binary.h>
#include <fmt/format.h>

#include <CLI/CLI.hpp>
#include <iostream>
#include <stdexcept>

#include "CapstoneWrapper.h"

void disasm(const std::string &filename, const std::string &section)
{
    BFDLoader loader;
    Binary executable = loader.load_binary(filename);
    const auto &text_section = executable.get_section(section);

    CapstoneWrapper capstone;
    const auto [instructions, number_of_instructions] = capstone.disasm(text_section.bytes.data(), text_section.bytes.size(), text_section.vma);

    for (size_t i = 0; i < number_of_instructions; i++)
    {
        const auto &instr = instructions.get()[i];
        fmt::print("{:016x}: ", instr.address);

        std::string data_line;
        for (size_t j = 0; j < instr.size; ++j)
        {
            data_line += fmt::format("{:02x} ", instr.bytes[j]);
        }

        fmt::print("{:48s} {} {}\n", data_line, instr.mnemonic, instr.op_str);
    }
}

int main(int argc, char *argv[])
{
    std::filesystem::path input_filename;
    std::string section = ".text";
    bool use_recursive = false;

    CLI::App app("bdis (binary disassembler) is a program for linear and recursive disassemble of an executable file.", "bdis");
    app.add_option("filename", input_filename, "Input executable to disassemble")->required()->check(CLI::ExistingFile);
    app.add_option("section", section, "Section name to disassemble");
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
        disasm(input_filename, section);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
