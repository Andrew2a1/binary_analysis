#pragma once

#include <string>

#include "Binary.h"

class BinaryLoader
{
public:
    virtual Binary load_binary(const std::string &file_name) = 0;
};
