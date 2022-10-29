#include "LoaderCLI.h"

#include <BFDLoader.h>
#include <Symbol.h>
#include <fmt/core.h>

LoaderCLI::LoaderCLI(const std::string &binary_name)
{
    BFDLoader bfd_loader;
    bin = bfd_loader.load_binary(binary_name);
    fmt::print("Loaded binary '{}' {}/{} ({} bits) entry@{:#016x}\n\n", bin.filename, bin.type_name, bin.arch_name, bin.bits, bin.entry);
}

void LoaderCLI::format_table_single_header(tabulate::Table &table, int row_count) const
{
    table.format().border_top("").border_bottom("").border_left("|").border_right("|").corner("");
    table[0].format().border_top("-").border_bottom("-").border_left("|").border_right("|").corner("+");

    if (row_count > 1)
    {
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

void LoaderCLI::show_section_data(const std::string &section_name) const
{
    constexpr int BYTES_PER_ROW = 16;
    const Section &section = bin.get_section(section_name);
    const uint8_t *raw_data = section.bytes.data();

    std::string row_data;
    std::string row_text;
    row_data.reserve(BYTES_PER_ROW * 3);
    row_text.reserve(BYTES_PER_ROW);

    tabulate::Table contents;
    contents.add_row({"Addr", "Bytes", "Text"});
    int row_count = 1;

    for (int n = 0; n < static_cast<int>(section.bytes.size()); ++n)
    {
        row_data += fmt::format("{:02x} ", raw_data[n]);

        if (isalnum(raw_data[n]))
        {
            row_text += fmt::format("{:c}", raw_data[n]);
        }
        else
        {
            row_text += ".";
        }

        if (n % BYTES_PER_ROW == BYTES_PER_ROW - 1)
        {
            row_count += 1;
            contents.add_row({fmt::format("{:#016x}", section.vma + n - BYTES_PER_ROW + 1), row_data, row_text});
            row_data.clear();
            row_text.clear();
        }
    }

    format_table_single_header(contents, row_count);
    fmt::print("Contents of section '{}':\n", section.name);
    std::cout << contents << std::endl;
}

void LoaderCLI::show_symbols() const
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
        const std::string symbol_type_name = (symbol.type == SymbolType::Function) ? "FUNC" : "DATA";
        std::string symbol_bind_name;
        switch (symbol.bind)
        {
            case SymbolBindType::Local:
                symbol_bind_name = "LOCAL";
                break;
            case SymbolBindType::Global:
                symbol_bind_name = "GLOBAL";
                break;
            case SymbolBindType::Weak:
                symbol_bind_name = "WEAK";
                break;
            default:
                symbol_bind_name = "UNKNOWN";
                break;
        }
        symbols_table.add_row({fmt::format("{:#016x}", symbol.addr), symbol_type_name, symbol_bind_name, symbol.name});
    }

    format_table_single_header(symbols_table, bin.symbols.size() + 1);
    fmt::print("Symbols:\n");
    std::cout << symbols_table << std::endl;
}