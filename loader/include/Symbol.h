#pragma once

#include <cstdint>
#include <string>

enum class SymbolType
{
    Unknown,
    Function,
    Data
};

enum class SymbolBindType
{
    Unknown,
    Local,
    Global,
    Weak
};

struct Symbol
{
    SymbolType type = SymbolType::Unknown;
    SymbolBindType bind = SymbolBindType::Unknown;
    std::string name;
    uint64_t addr = 0;
};

std::string demangle_symbol_name(const std::string &name);

namespace std
{
std::string to_string(SymbolType symbol_type);
std::string to_string(SymbolBindType symbol_type);
}  // namespace std