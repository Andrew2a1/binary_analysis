#pragma once

#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <unistd.h>

#include <stdexcept>
#include <string>

class ElfFile
{
private:
    const std::string filename;
    Elf *elf_file;
    int file_descriptor;
    int bits;

public:
    explicit ElfFile(const std::string &filename);
    ~ElfFile();

    ElfFile(ElfFile &&) = default;
    ElfFile(const ElfFile &) = delete;
    ElfFile &operator=(const ElfFile &) = delete;

    Elf *elf() const { return elf_file; }
    int fd() const { return file_descriptor; }
    const std::string &name() const { return filename; }

    Elf_Kind kind() const { return elf_kind(elf_file); }
    int get_class() const { return gelf_getclass(elf_file); }

    int get_bits() const { return bits; }
    GElf_Ehdr get_ehdr() const;

private:
    int read_bits() const;
};