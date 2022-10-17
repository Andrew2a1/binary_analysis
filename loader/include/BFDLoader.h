#pragma once

#include <string>

#include "Binary.h"
#include "BinaryLoader.h"

class BFDLoader : public BinaryLoader
{
public:
    Binary load_binary(const std::string &file_name) override;
};
