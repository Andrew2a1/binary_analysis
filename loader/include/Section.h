#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Binary;

enum class SectionType
{
    None,
    Code,
    Data
};

struct Section
{
    Binary *binary = nullptr;
    std::string name;
    SectionType type = SectionType::None;
    uint64_t vma = 0;
    uint64_t size = 0;
    std::vector<uint8_t> bytes;
};
