/* Original code from: https://practicalbinaryanalysis.com/ */

#include "ElfInject.h"

#include <fcntl.h>
#include <gelf.h>
#include <getopt.h>
#include <libelf.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ABITAG_NAME ".note.ABI-tag"
#define SHSTRTAB_NAME ".shstrtab"

#include <CLI/CLI.hpp>
#include <filesystem>
#include <fstream>

#include "ElfFile.h"

typedef struct
{
    size_t pidx;               /* index of program header to overwrite */
    GElf_Phdr phdr;            /* program header to overwrite */
    size_t sidx;               /* index of section header to overwrite */
    Elf_Scn *scn;              /* section to overwrite */
    GElf_Shdr shdr;            /* section header to overwrite */
    off_t shstroff;            /* offset to section name to overwrite */
    std::vector<uint8_t> code; /* code to inject */
    long entry;                /* code buffer offset to entry point (-1 for none) */
    off_t off;                 /* file offset to injected code */
    size_t secaddr;            /* section address for injected code */
    std::string secname;       /* section name for injected code */
} inject_data_t;

struct ElfData
{
    const ElfFile &elf_file;
    GElf_Ehdr ehdr; /* executable header */
};

off_t write_code(int elf_fd, const std::vector<uint8_t> &code_data)
{
    off_t off = lseek(elf_fd, 0, SEEK_END);
    size_t n = write(elf_fd, code_data.data(), code_data.size());
    if (n != code_data.size())
    {
        throw std::runtime_error("Failed to inject code bytes");
    }
    return off;
}

void write_ehdr(ElfData &elf)
{
    if (!gelf_update_ehdr(elf.elf_file.elf(), &elf.ehdr))
    {
        throw std::runtime_error("Failed to update executable header");
    }

    void *ehdr_buf;
    size_t ehdr_size;
    if (elf.elf_file.get_bits() == 32)
    {
        ehdr_buf = elf32_getehdr(elf.elf_file.elf());
        ehdr_size = sizeof(Elf32_Ehdr);
    }
    else
    {
        ehdr_buf = elf64_getehdr(elf.elf_file.elf());
        ehdr_size = sizeof(Elf64_Ehdr);
    }

    if (!ehdr_buf)
    {
        throw std::runtime_error("Failed to get executable header");
    }

    lseek(elf.elf_file.fd(), 0, SEEK_SET);
    size_t n = write(elf.elf_file.fd(), ehdr_buf, ehdr_size);
    if (n != ehdr_size)
    {
        throw std::runtime_error("Failed to write executable header");
    }
}

void write_phdr(ElfData &elf, size_t pidx, GElf_Phdr *phdr)
{
    if (!gelf_update_phdr(elf.elf_file.elf(), pidx, phdr))
    {
        throw std::runtime_error("Failed to update program header");
    }

    void *phdr_buf = NULL;
    size_t phdr_size;
    if (elf.elf_file.get_bits() == 32)
    {
        Elf32_Phdr *phdr_list32 = elf32_getphdr(elf.elf_file.elf());
        if (phdr_list32)
        {
            phdr_buf = &phdr_list32[pidx];
            phdr_size = sizeof(Elf32_Phdr);
        }
    }
    else
    {
        Elf64_Phdr *phdr_list64 = elf64_getphdr(elf.elf_file.elf());
        if (phdr_list64)
        {
            phdr_buf = &phdr_list64[pidx];
            phdr_size = sizeof(Elf64_Phdr);
        }
    }

    if (!phdr_buf)
    {
        throw std::runtime_error("Failed to get program header");
    }

    lseek(elf.elf_file.fd(), elf.ehdr.e_phoff + pidx * elf.ehdr.e_phentsize, SEEK_SET);
    size_t n = write(elf.elf_file.fd(), phdr_buf, phdr_size);
    if (n != phdr_size)
    {
        throw std::runtime_error("Failed to write program header");
    }
}

void write_shdr(ElfData &elf, Elf_Scn *scn, GElf_Shdr *shdr, size_t sidx)
{
    if (!gelf_update_shdr(scn, shdr))
    {
        throw std::runtime_error("Failed to update section header");
    }

    void *shdr_buf;
    size_t shdr_size;
    if (elf.elf_file.get_bits() == 32)
    {
        shdr_buf = elf32_getshdr(scn);
        shdr_size = sizeof(Elf32_Shdr);
    }
    else
    {
        shdr_buf = elf64_getshdr(scn);
        shdr_size = sizeof(Elf64_Shdr);
    }

    if (!shdr_buf)
    {
        throw std::runtime_error("Failed to get section header");
    }

    lseek(elf.elf_file.fd(), elf.ehdr.e_shoff + sidx * elf.ehdr.e_shentsize, SEEK_SET);
    size_t n = write(elf.elf_file.fd(), shdr_buf, shdr_size);
    if (n != shdr_size)
    {
        throw std::runtime_error("Failed to write section header");
    }
}

void reorder_shdrs(ElfData &elf, inject_data_t *inject)
{
    GElf_Shdr shdr;
    Elf_Scn *scn = elf_getscn(elf.elf_file.elf(), inject->sidx - 1);
    if (scn && !gelf_getshdr(scn, &shdr))
    {
        throw std::runtime_error("Failed to get section header");
    }

    int direction = 0;
    if (scn && shdr.sh_addr > inject->shdr.sh_addr)
    {
        /* Injected section header must be moved left */
        direction = -1;
    }

    scn = elf_getscn(elf.elf_file.elf(), inject->sidx + 1);
    if (scn && !gelf_getshdr(scn, &shdr))
    {
        throw std::runtime_error("Failed to get section header");
    }

    if (scn && shdr.sh_addr < inject->shdr.sh_addr)
    {
        /* Injected section header must be moved right */
        direction = 1;
    }

    if (direction == 0)
    {
        /* Section headers are already in order */
        return;
    }

    /* Order section headers by increasing address */
    int skip = 0;
    for (scn = elf_getscn(elf.elf_file.elf(), inject->sidx + direction); scn != NULL; scn = elf_getscn(elf.elf_file.elf(), inject->sidx + direction + skip))
    {
        if (!gelf_getshdr(scn, &shdr))
        {
            throw std::runtime_error("Failed to get section header");
        }

        if ((direction < 0 && shdr.sh_addr <= inject->shdr.sh_addr) || (direction > 0 && shdr.sh_addr >= inject->shdr.sh_addr))
        {
            /* The order is okay from this point on */
            break;
        }

        /* Only reorder code section headers */
        if (shdr.sh_type != SHT_PROGBITS)
        {
            skip += direction;
            continue;
        }

        /* Swap the injected shdr with its neighbor PROGBITS header */
        write_shdr(elf, scn, &inject->shdr, elf_ndxscn(scn));
        write_shdr(elf, inject->scn, &shdr, inject->sidx);

        inject->sidx += direction + skip;
        inject->scn = elf_getscn(elf.elf_file.elf(), inject->sidx);
        skip = 0;
    }
}

void write_secname(const ElfData &elf, inject_data_t *inject)
{
    lseek(elf.elf_file.fd(), inject->shstroff, SEEK_SET);
    size_t n = write(elf.elf_file.fd(), inject->secname.c_str(), inject->secname.size());
    if (n != inject->secname.size())
    {
        throw std::runtime_error("Failed to write section name");
    }

    n = strlen(ABITAG_NAME) - inject->secname.size();
    while (n > 0)
    {
        if (!write(elf.elf_file.fd(), "\0", 1))
        {
            throw std::runtime_error("Failed to write section name");
        }
        n--;
    }
}

void find_rewritable_segment(const ElfData &elf, inject_data_t *inject)
{
    int ret;
    size_t i, n;

    ret = elf_getphdrnum(elf.elf_file.elf(), &n);
    if (ret != 0)
    {
        throw std::runtime_error("Cannot find any program headers");
    }

    for (i = 0; i < n; i++)
    {
        if (!gelf_getphdr(elf.elf_file.elf(), i, &inject->phdr))
        {
            throw std::runtime_error("Failed to get program header");
        }

        switch (inject->phdr.p_type)
        {
            case PT_NOTE:
                inject->pidx = i;
                return;
            default:
                break;
        }
    }

    throw std::runtime_error("Cannot find segment to rewrite");
}

void rewrite_code_segment(ElfData &elf, inject_data_t *inject)
{
    inject->phdr.p_type = PT_LOAD;               /* type */
    inject->phdr.p_offset = inject->off;         /* file offset to start of segment */
    inject->phdr.p_vaddr = inject->secaddr;      /* virtual address to load segment at */
    inject->phdr.p_paddr = inject->secaddr;      /* physical address to load segment at */
    inject->phdr.p_filesz = inject->code.size(); /* byte size in file */
    inject->phdr.p_memsz = inject->code.size();  /* byte size in memory */
    inject->phdr.p_flags = PF_R | PF_X;          /* flags */
    inject->phdr.p_align = 0x1000;               /* alignment in memory and file */
    write_phdr(elf, inject->pidx, &inject->phdr);
}

void rewrite_code_section(ElfData &elf, inject_data_t *inject)
{
    Elf_Scn *scn;
    GElf_Shdr shdr;
    size_t shstrndx;

    if (elf_getshdrstrndx(elf.elf_file.elf(), &shstrndx) < 0)
    {
        throw std::runtime_error("Failed to get string table section index");
    }

    scn = NULL;
    while ((scn = elf_nextscn(elf.elf_file.elf(), scn)))
    {
        if (!gelf_getshdr(scn, &shdr))
        {
            throw std::runtime_error("Failed to get section header");
        }
        char *s = elf_strptr(elf.elf_file.elf(), shstrndx, shdr.sh_name);
        if (!s)
        {
            throw std::runtime_error("Failed to get section name");
        }

        if (!strcmp(s, ABITAG_NAME))
        {
            shdr.sh_name = shdr.sh_name;               /* offset into string table */
            shdr.sh_type = SHT_PROGBITS;               /* type */
            shdr.sh_flags = SHF_ALLOC | SHF_EXECINSTR; /* flags */
            shdr.sh_addr = inject->secaddr;            /* address to load section at */
            shdr.sh_offset = inject->off;              /* file offset to start of section */
            shdr.sh_size = inject->code.size();        /* size in bytes */
            shdr.sh_link = 0;                          /* not used for code section */
            shdr.sh_info = 0;                          /* not used for code section */
            shdr.sh_addralign = 16;                    /* memory alignment */
            shdr.sh_entsize = 0;                       /* not used for code section */

            inject->sidx = elf_ndxscn(scn);
            inject->scn = scn;

            memcpy(&inject->shdr, &shdr, sizeof(shdr));
            write_shdr(elf, scn, &shdr, elf_ndxscn(scn));
            reorder_shdrs(elf, inject);
            break;
        }
    }
    if (!scn)
    {
        throw std::runtime_error("Cannot find section to rewrite");
    }
}

void rewrite_section_name(const ElfData &elf, inject_data_t *inject)
{
    Elf_Scn *scn;
    GElf_Shdr shdr;
    size_t shstrndx, stroff, strbase;

    if (inject->secname.size() > strlen(ABITAG_NAME))
    {
        throw std::runtime_error("Section name too long");
    }

    if (elf_getshdrstrndx(elf.elf_file.elf(), &shstrndx) < 0)
    {
        throw std::runtime_error("Failed to get string table section index");
    }

    stroff = 0;
    strbase = 0;
    scn = NULL;
    while ((scn = elf_nextscn(elf.elf_file.elf(), scn)))
    {
        if (!gelf_getshdr(scn, &shdr))
        {
            throw std::runtime_error("Failed to get section header");
        }
        char *s = elf_strptr(elf.elf_file.elf(), shstrndx, shdr.sh_name);
        if (!s)
        {
            throw std::runtime_error("Failed to get section name");
        }

        if (!strcmp(s, ABITAG_NAME))
        {
            stroff = shdr.sh_name; /* offset into shstrtab */
        }
        else if (!strcmp(s, SHSTRTAB_NAME))
        {
            strbase = shdr.sh_offset; /* offset to start of shstrtab */
        }
    }

    if (stroff == 0)
    {
        throw std::runtime_error("Cannot find shstrtab entry for injected section");
    }
    else if (strbase == 0)
    {
        throw std::runtime_error("Cannot find shstrtab");
    }

    inject->shstroff = strbase + stroff;
    write_secname(elf, inject);
}

void rewrite_entry_point(ElfData &elf, inject_data_t *inject)
{
    elf.ehdr.e_entry = inject->phdr.p_vaddr + inject->entry;
    write_ehdr(elf);
}

void inject_code(const InjectInfo &inject_info, const ElfFile &elf_file)
{
    ElfData elf{elf_file, GElf_Ehdr()};

    inject_data_t inject;
    inject.code = inject_info.data_to_inject;
    inject.entry = inject_info.entry;
    inject.secname = inject_info.section_name;
    inject.secaddr = inject_info.section_address;

    if (elf.elf_file.kind() != ELF_K_ELF)
    {
        throw std::runtime_error("Not an ELF executable");
    }

    elf.ehdr = elf.elf_file.get_ehdr();

    /* Find a rewritable program header */
    find_rewritable_segment(elf, &inject);

    /* Write the injected code to the binary */
    inject.off = write_code(elf.elf_file.fd(), inject.code);

    /* Align code address so it's congruent to the file offset modulo 4096 */
    size_t n = (inject.off % 4096) - (inject.secaddr % 4096);
    inject.secaddr += n;

    /* Rewrite a section for the injected code */
    rewrite_code_section(elf, &inject);
    rewrite_section_name(elf, &inject);

    /* Rewrite a segment for the added code section */
    rewrite_code_segment(elf, &inject);

    if (inject.entry >= 0)
    {
        rewrite_entry_point(elf, &inject);
    }
}
