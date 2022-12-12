#include "CapstoneWrapper.h"

CapstoneWrapper::CapstoneWrapper()
{
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle) != CS_ERR_OK)
    {
        throw std::runtime_error("Failed to initialize Capstone");
    }
}

CapstoneWrapper::~CapstoneWrapper() { cs_close(&capstone_handle); }
csh CapstoneWrapper::handle() const { return capstone_handle; }

std::pair<std::unique_ptr<cs_insn, std::function<void(cs_insn *inst)>>, size_t> CapstoneWrapper::disasm(const uint8_t *data, size_t data_size, uint64_t address,
                                                                                                        size_t count) const
{
    cs_insn *instructions;
    size_t number_of_instructions = cs_disasm(capstone_handle, data, data_size, address, count, &instructions);

    if (number_of_instructions == 0)
    {
        throw std::runtime_error("Disassembly error: " + strerror());
    }

    const auto instr_deleter = [=](cs_insn *inst) { cs_free(inst, number_of_instructions); };
    return std::make_pair(std::unique_ptr<cs_insn, decltype(instr_deleter)>{instructions, instr_deleter}, number_of_instructions);
}

std::string CapstoneWrapper::strerror() const { return cs_strerror(cs_errno(capstone_handle)); }