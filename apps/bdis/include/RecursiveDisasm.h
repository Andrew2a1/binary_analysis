#pragma once

#include <Section.h>

#include <cstdint>
#include <list>
#include <map>
#include <string>

#include "CapstoneWrapper.h"
#include "Disasm.h"

class RecursiveDisasm : public Disasm
{
public:
    explicit RecursiveDisasm(const CapstoneWrapper &capstone) : Disasm(capstone) {}
    void disasm(const Binary &binary, const std::string &section_name = ".text") override;

private:
    std::list<uint64_t> process_instruction(const InsnPtr &instruction, const Section &section, uint64_t address, std::map<uint64_t, bool> &seen) const;
};
