#pragma once

#include "CapstoneWrapper.h"

class Disasm
{
private:
    const CapstoneWrapper &capstone;

public:
    explicit Disasm(const CapstoneWrapper &capstone) : capstone(capstone) {}
    virtual ~Disasm() = default;

    virtual void disasm(const Binary &binary, const std::string &section_name = ".text") = 0;
    const CapstoneWrapper &capstone_handle() const { return capstone; }
};