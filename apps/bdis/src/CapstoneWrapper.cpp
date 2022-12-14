#include "CapstoneWrapper.h"

#include <functional>
#include <stdexcept>

CapstoneWrapper::CapstoneWrapper()
{
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle) != CS_ERR_OK)
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