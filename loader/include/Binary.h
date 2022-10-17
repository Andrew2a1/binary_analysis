#pragma once

#include <string>
#include <vector>

#include "Section.h"
#include "Symbol.h"

enum class BinaryType
{
    Auto,
    ELF,
    PE
};

enum class BinaryArch
{
    None,
    X86
};

struct Binary
{
    std::string filename;
    std::string type_name;
    std::string arch_name;

    BinaryType type = BinaryType::Auto;
    BinaryArch arch = BinaryArch::None;

    unsigned bits = 0;
    uint64_t entry = 0;

    std::vector<Section> sections;
    std::vector<Symbol> symbols;

    const Section &get_section(const std::string &section_name) const;
    bool section_exists(const std::string &section_name) const;
};
