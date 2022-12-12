#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct InjectInfo
{
    std::string section_name;
    const std::vector<uint8_t> data_to_inject;
    int section_address;
    long entry = -1;
};
