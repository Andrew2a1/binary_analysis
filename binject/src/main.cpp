#include <CLI/CLI.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "ElfInject.h"

std::vector<uint8_t> load_file_data(const std::filesystem::path &filename)
{
    std::ifstream input_file(filename, std::ios::in | std::ios::binary);

    if (input_file.bad())
    {
        throw std::runtime_error("Failed to open: " + filename.string());
    }

    input_file.seekg(0, std::ios::end);
    size_t data_size = input_file.tellg();
    input_file.seekg(0, std::ios::beg);

    std::vector<uint8_t> file_data;
    file_data.resize(data_size);
    input_file.read(reinterpret_cast<char *>(file_data.data()), data_size);
    return file_data;
}

int main(int argc, char *argv[])
{
    std::filesystem::path input_filename, inject_filename;
    std::string injected_section_name;
    int addr;
    long entry = -1;

    CLI::App app("binject (binary inject) is a program for code injection into elf file.", "binject");
    app.add_option("filename", input_filename, "Input filename to inject code to")->required()->check(CLI::ExistingFile);
    app.add_option("inject", inject_filename, "File with code to inject")->required()->check(CLI::ExistingFile);
    app.add_option("name", injected_section_name, "Injected section name")->required();
    app.add_option("addr", addr, "Section base address")->required();
    app.add_option("entry", entry, "Section base address");
    app.validate_positionals();

    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e)
    {
        return app.exit(e);
    }

    ElfFile elf_file(input_filename);
    InjectInfo inject_info{injected_section_name, load_file_data(inject_filename), addr, entry};

    try
    {
        inject_code(inject_info, elf_file);
    }
    catch (std::exception &err)
    {
        std::cerr << "Error: " << err.what() << std::endl;
        return 1;
    }

    return 0;
}
