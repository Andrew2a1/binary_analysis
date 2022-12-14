#include "DisasmUtils.h"

#include <fmt/format.h>

void print_insn(const cs_insn *ins)
{
    fmt::print("{:016x}: ", ins->address);
    std::string bytes;
    for (size_t j = 0; j < ins->size; ++j)
    {
        bytes += fmt::format("{:02x} ", ins->bytes[j]);
    }
    fmt::print("{:32s} {} {}\n", bytes, ins->mnemonic, ins->op_str);
}

bool is_flow_group(uint8_t g) { return (g == CS_GRP_JUMP) || (g == CS_GRP_CALL) || (g == CS_GRP_RET) || (g == CS_GRP_IRET); }

bool is_flow_insn(const cs_insn *ins)
{
    for (size_t i = 0; i < ins->detail->groups_count; ++i)
    {
        if (is_flow_group(ins->detail->groups[i]))
        {
            return true;
        }
    }
    return false;
}

bool is_unconditional_flow_insn(const cs_insn *ins)
{
    switch (ins->id)
    {
        case X86_INS_JMP:
        case X86_INS_LJMP:
        case X86_INS_RET:
        case X86_INS_RETF:
        case X86_INS_RETFQ:
            return true;
    }
    return false;
}

uint64_t get_insn_immediate_target(const cs_insn *ins)
{
    for (size_t i = 0; i < ins->detail->groups_count; ++i)
    {
        if (is_flow_group(ins->detail->groups[i]))
        {
            for (size_t j = 0; j < ins->detail->x86.op_count; ++j)
            {
                const cs_x86_op &cs_op = ins->detail->x86.operands[j];
                if (cs_op.type == X86_OP_IMM)
                {
                    return cs_op.imm;
                }
            }
        }
    }
    return 0;
}