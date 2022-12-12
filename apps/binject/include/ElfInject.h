#pragma once

#include "ElfFile.h"
#include "InjectInfo.h"

void inject_code(const InjectInfo &inject_info, const ElfFile &elf_file);
