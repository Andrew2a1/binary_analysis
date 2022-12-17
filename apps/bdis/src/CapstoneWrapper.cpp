#include "CapstoneWrapper.h"

#include <functional>
#include <map>
#include <stdexcept>

CapstoneWrapper::CapstoneWrapper(const BinaryArch &arch, int bits)
{
    static const std::map<BinaryArch, cs_arch> bin_arch_capstone_arch = {{BinaryArch::X86, CS_ARCH_X86}};
    static const std::map<int, cs_mode> bits_capstone_mode = {{16, CS_MODE_16}, {32, CS_MODE_32}, {64, CS_MODE_64}};

    if (cs_open(bin_arch_capstone_arch.at(arch), bits_capstone_mode.at(bits), &capstone_handle) != CS_ERR_OK)
    {
        throw std::runtime_error("Failed to initialize Capstone");
    }
}

CapstoneWrapper::~CapstoneWrapper() { cs_close(&capstone_handle); }
csh CapstoneWrapper::handle() const { return capstone_handle; }

InsnPtr CapstoneWrapper::create_insn() const
{
    cs_insn *ins = cs_malloc(capstone_handle);
    const auto deleter = [=](cs_insn *inst) { cs_free(inst, 1); };
    return std::unique_ptr<cs_insn, decltype(deleter)>{ins, deleter};
}

std::string CapstoneWrapper::strerror() const { return cs_strerror(cs_errno(capstone_handle)); }