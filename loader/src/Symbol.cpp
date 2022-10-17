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