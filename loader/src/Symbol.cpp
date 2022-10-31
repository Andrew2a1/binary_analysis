#include "Symbol.h"

#include <cxxabi.h>

#include <string>

std::string demangle_symbol_name(const std::string &name)
{
    int status;
    char *ret = abi::__cxa_demangle(name.c_str(), 0, 0, &status);

    if (ret != nullptr)
    {
        const std::string after_demangle{ret};
        free(ret);
        if (status == 0)
        {
            return after_demangle;
        }
    }

    return name;
}

std::string std::to_string(SymbolType symbol_type)
{
    switch (symbol_type)
    {
        case SymbolType::Data:
            return "DATA";
        case SymbolType::Function:
            return "FUNC";
        case SymbolType::Unknown:
        default:
            return "UNKNOWN";
    }
}

std::string std::to_string(SymbolBindType symbol_bind_type)
{
    switch (symbol_bind_type)
    {
        case SymbolBindType::Local:
            return "LOCAL";
        case SymbolBindType::Global:
            return "GLOBAL";
        case SymbolBindType::Weak:
            return "WEAK";
        default:
            return "UNKNOWN";
    }
}