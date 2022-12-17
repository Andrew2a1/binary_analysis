#include "RecursiveDisasm.h"

#include <BFDLoader.h>
#include <Binary.h>
#include <fmt/format.h>

#include <list>
#include <map>

#include "DisasmUtils.h"

void RecursiveDisasm::disasm(const Binary &binary, const std::string &section_name)
{
    const auto &section = binary.get_section(section_name);

    std::list<uint64_t> instr;
    std::map<uint64_t, bool> seen;

    cs_option(capstone_handle().handle(), CS_OPT_DETAIL, CS_OPT_ON);

    if (section.contains(binary.entry))
    {
        instr.push_back(binary.entry);
        fmt::print("Entry point: {:016x}\n", binary.entry);
    }

    const auto current_instr = capstone_handle().create_insn();
    while (!instr.empty())
    {
        uint64_t addr = instr.front();
        instr.pop_front();

        if (!seen[addr])
        {
            const auto discovered_targets = process_instruction(current_instr, section, addr, seen);
            instr.insert(instr.begin(), std::make_move_iterator(discovered_targets.begin()), std::make_move_iterator(discovered_targets.end()));
        }
    }
}

std::list<uint64_t> RecursiveDisasm::process_instruction(const InsnPtr &instruction, const Section &section, uint64_t address,
                                                         std::map<uint64_t, bool> &seen) const
{
    std::list<uint64_t> discovered_targets;

    const uint64_t offset = address - section.vma;
    const uint8_t *code = &section.bytes[offset];
    uint64_t data_size = section.size - offset;

    while (cs_disasm_iter(capstone_handle().handle(), &code, &data_size, &address, instruction.get()))
    {
        if (instruction->id == X86_INS_INVALID || instruction->size == 0) break;
        seen[instruction->address] = true;
        print_insn(instruction.get());

        if (is_flow_insn(instruction.get()))
        {
            uint64_t target = get_insn_immediate_target(instruction.get());
            if (target && !seen[target] && section.contains(target))
            {
                discovered_targets.push_back(target);
                fmt::print("-> new target: {:016x}\n", target);
            }
            if (is_unconditional_flow_insn(instruction.get()))
            {
                break;
            }
        }
        else if (instruction->id == X86_INS_HLT)
        {
            break;
        }
    }

    return discovered_targets;
}
