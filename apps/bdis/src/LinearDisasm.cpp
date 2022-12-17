#include "LinearDisasm.h"

#include <BFDLoader.h>
#include <Binary.h>
#include <fmt/format.h>

#include <cstdint>
#include <string>

#include "CapstoneWrapper.h"
#include "DisasmUtils.h"

void LinearDisasm::disasm(const Binary &binary, const std::string &section_name)
{
    const auto &file_section = binary.get_section(section_name);
    const auto [instructions, number_of_instructions] = disassemble_data_block(file_section.bytes.data(), file_section.bytes.size(), file_section.vma);

    for (size_t i = 0; i < number_of_instructions; i++)
    {
        print_insn(&instructions.get()[i]);
    }
}

std::pair<InsnPtr, size_t> LinearDisasm::disassemble_data_block(const uint8_t *data, size_t data_size, uint64_t address, size_t count) const
{
    cs_insn *instructions;
    size_t number_of_instructions = cs_disasm(capstone_handle().handle(), data, data_size, address, count, &instructions);

    if (number_of_instructions == 0)
    {
        throw std::runtime_error("Disassembly error: " + capstone_handle().strerror());
    }

    const auto deleter = [=](cs_insn *inst) { cs_free(inst, number_of_instructions); };
    return std::make_pair(std::unique_ptr<cs_insn, decltype(deleter)>{instructions, deleter}, number_of_instructions);
}
