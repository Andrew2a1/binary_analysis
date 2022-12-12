#include "LoaderCLI.h"

#include <BFDLoader.h>
#include <Symbol.h>
#include <fmt/core.h>

bool is_text_chr(char chr) { return chr > 31 && chr < 127; }

LoaderCLI::LoaderCLI(const std::string &binary_name)
{
    BFDLoader bfd_loader;
    bin = bfd_loader.load_binary(binary_name);
    fmt::print("Binary file: '{}' {}/{} ({} bits) entry@{:#016x}\n\n", bin.filename, bin.type_name, bin.arch_name, bin.bits, bin.entry);
}

void LoaderCLI::format_table_single_header(tabulate::Table &table, int row_count) const
{
    if (row_count > 2)
    {
        table.format().border_top("").border_bottom("").border_left("|").border_right("|").corner("");
        table[0].format().border_top("-").border_bottom("-").border_left("|").border_right("|").corner("+");
        table[1].format().border_top("-").border_bottom("-").border_left("|").border_right("|").corner("+");
        table[row_count - 1].format().border_top("").border_bottom("-").border_left("|").border_right("|").corner_bottom_left("+").corner_bottom_right("+");
    }
}

void LoaderCLI::show_sections() const
{
    tabulate::Table file_sections;
    file_sections.add_row({"Addr", "Size", "Name", "Type"});

    for (const auto &section : bin.sections)
    {
        const std::string section_type = section.type == SectionType::Code ? "CODE" : "DATA";
        file_sections.add_row({fmt::format("{:#016x}", section.vma), std::to_string(section.size), section.name, section_type});
    }

    format_table_single_header(file_sections, bin.sections.size() + 1);

    fmt::print("Sections:\n");
    std::cout << file_sections << std::endl;
}

void LoaderCLI::show_section_data(const std::string &section_name, int data_bytes_per_row) const
{
    const Section &section = bin.get_section(section_name);
    const uint8_t *raw_data = section.bytes.data();

    std::string row_data;
    std::string row_text;
    row_data.reserve(data_bytes_per_row * 3);
    row_text.reserve(data_bytes_per_row);

    tabulate::Table contents;
    contents.add_row({"Addr", "Bytes", "Text"});
    int row_count = 1;

    for (int n = 0; n < static_cast<int>(section.bytes.size()); ++n)
    {
        row_data += fmt::format("{:02x} ", raw_data[n]);
        row_text += is_text_chr(raw_data[n]) ? raw_data[n] : '.';

        if (n % data_bytes_per_row == data_bytes_per_row - 1)
        {
            row_count += 1;
            contents.add_row({fmt::format("{:#016x}", section.vma + n - data_bytes_per_row + 1), row_data, row_text});
            row_data.clear();
            row_text.clear();
        }
    }

    if (!row_data.empty())
    {
        row_count += 1;
        contents.add_row({fmt::format("{:#016x}", section.vma + section.bytes.size() - data_bytes_per_row + 1), row_data, row_text});
    }

    format_table_single_header(contents, row_count);
    fmt::print("Contents of section '{}':\n", section.name);
    std::cout << contents << std::endl;
}

void LoaderCLI::show_symbols(bool demangle) const
{
    if (bin.symbols.empty())
    {
        std::cout << "No symbols found." << std::endl;
        return;
    }

    tabulate::Table symbols_table;
    symbols_table.add_row({"Addr", "Type", "Bind", "Name"});

    for (const auto &symbol : bin.symbols)
    {
        const std::string symbol_name = demangle ? demangle_symbol_name(symbol_name) : symbol.name;
        auto &row = symbols_table.add_row({fmt::format("{:#016x}", symbol.addr), std::to_string(symbol.type), std::to_string(symbol.bind), symbol_name});
        row[0][3].format().width(80);
    }

    fmt::print("Symbols:\n");
    std::cout << symbols_table << std::endl;
}