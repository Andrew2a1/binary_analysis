#pragma once

#include <Binary.h>
#include <capstone/capstone.h>

#include <functional>
#include <memory>
#include <utility>

using InsnPtr = std::unique_ptr<cs_insn, std::function<void(cs_insn *inst)>>;

class CapstoneWrapper
{
    csh capstone_handle;

public:
    explicit CapstoneWrapper(const BinaryArch &arch = BinaryArch::X86, int bits = 64);
    ~CapstoneWrapper();

    csh handle() const;
    std::string strerror() const;
    InsnPtr create_insn() const;
};
