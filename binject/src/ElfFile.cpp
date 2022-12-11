#include "ElfFile.h"

ElfFile::ElfFile(const std::string &filename) : filename(filename)
{
    file_descriptor = open(filename.c_str(), O_RDWR);

    if (file_descriptor < 0)
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        close(file_descriptor);
        throw std::runtime_error("Failed to initialize libelf");
    }

    elf_file = elf_begin(file_descriptor, ELF_C_READ, NULL);
    if (!elf_file)
    {
        close(file_descriptor);
        throw std::runtime_error("Failed to open ELF file: " + filename);
    }

    this->bits = read_bits();
}

ElfFile::~ElfFile()
{
    elf_end(elf_file);
    close(file_descriptor);
}

int ElfFile::read_bits() const
{
    switch (get_class())
    {
        case ELFCLASS32:
            return 32;
        case ELFCLASS64:
            return 64;
    }
    throw std::runtime_error("Unknown ELF class");
}

GElf_Ehdr ElfFile::get_ehdr() const
{
    GElf_Ehdr ehdr;
    if (!gelf_getehdr(elf_file, &ehdr))
    {
        throw std::runtime_error("Failed to get executable header");
    }
    return ehdr;
}
