#pragma once

#include <capstone/capstone.h>

#include <functional>
#include <memory>
#include <utility>

using InsnPtr = std::unique_ptr<cs_insn, std::function<void(cs_insn *inst)>>;

class CapstoneWrapper
{
    csh capstone_handle;

public:
    CapstoneWrapper();
    ~CapstoneWrapper();

    std::pair<InsnPtr, size_t> disasm(const uint8_t *data, size_t data_size, uint64_t address, size_t count = 0) const;

    csh handle() const;
    std::string strerror() const;
    InsnPtr create_insn() const;
};
