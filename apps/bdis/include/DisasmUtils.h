#pragma once

#include <capstone/capstone.h>

#include <cstdint>

void print_insn(const cs_insn *ins);
uint64_t get_insn_immediate_target(const cs_insn *ins);

bool is_flow_group(uint8_t group);
bool is_flow_insn(const cs_insn *ins);
bool is_unconditional_flow_insn(const cs_insn *ins);