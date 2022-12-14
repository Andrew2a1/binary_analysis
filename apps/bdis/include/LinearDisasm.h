#pragma once

#include <cstdint>
#include <string>

#include "CapstoneWrapper.h"
#include "Disasm.h"

class LinearDisasm : public Disasm
{
public:
    explicit LinearDisasm(const CapstoneWrapper &capstone) : Disasm(capstone) {}
    void disasm(const std::string &filename, const std::string &section_name = ".text") override;

private:
    std::pair<InsnPtr, size_t> disassemble_data_block(const uint8_t *data, size_t data_size, uint64_t address, size_t count = 0) const;
};
