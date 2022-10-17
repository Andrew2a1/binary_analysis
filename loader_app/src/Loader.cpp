#include <BFDLoader.h>
#include <Binary.h>
#include <fmt/core.h>

#include <algorithm>
#include <iostream>

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <binary> [section]" << std::endl;
        return 1;
    }

    BFDLoader loader;
    Binary bin = loader.load_binary(argv[1]);

    fmt::print("Loaded binary '{}' {}/{} ({} bits) entry@{:#016x}\n", bin.filename, bin.type_name, bin.arch_name, bin.bits, bin.entry);
    for (const auto &section : bin.sections)
    {
        fmt::print("  {:#016x} {:<8} {:<20} {}\n", section.vma, section.size, section.name, section.type == SectionType::Code ? "CODE" : "DATA");
    }

    if (argc == 3)
    {
        const std::string section_name = argv[2];
        const auto &section = bin.get_section(section_name);
        const uint8_t *raw_data = section.bytes.data();
        for (int n = 0; n < static_cast<int>(section.bytes.size()); ++n)
        {
            if (!(n % 32))
            {
                std::cout << "\n";
            }
            fmt::print("{:02x} ", raw_data[n]);
        }
    }

    if (!bin.symbols.empty())
    {
        std::cout << "\nSymbols:\n";
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
            fmt::print("  {:#016x} {:<5} {:<8} {}\n", symbol.addr, symbol_type_name, symbol_bind_name, symbol.name);
        }
    }

    return 0;
}
