#include "BFDLoader.h"

#include <bfd.h>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "Binary.h"

static bfd *open_bfd(const std::string &file_name)
{
    static bool bfd_initialized = false;
    bfd *bfd_handle = nullptr;

    if (!bfd_initialized)
    {
        bfd_init();
        bfd_initialized = true;
    }

    bfd_handle = bfd_openr(file_name.c_str(), nullptr);
    if (!bfd_handle)
    {
        throw std::runtime_error("Failed to open file: " + file_name);
    }

    if (!bfd_check_format(bfd_handle, bfd_object))
    {
        throw std::invalid_argument("File " + file_name + " does not look like executable. " + bfd_errmsg(bfd_get_error()));
    }

    if (bfd_get_flavour(bfd_handle) == bfd_target_unknown_flavour)
    {
        throw std::invalid_argument("Unrecognised format for file:  " + file_name + ". " + bfd_errmsg(bfd_get_error()));
    }

    return bfd_handle;
}

static void load_from_symtab(asymbol **bfd_symtab, int nsyms, Binary &binary)
{
    for (int i = 0; i < nsyms; ++i)
    {
        const bool is_function = bfd_symtab[i]->flags & BSF_FUNCTION;
        const bool is_weak = bfd_symtab[i]->flags & BSF_WEAK;
        const bool is_local = bfd_symtab[i]->flags & BSF_LOCAL;
        const bool is_global = bfd_symtab[i]->flags & BSF_GLOBAL;

        SymbolBindType bind_type = SymbolBindType::Unknown;
        if (is_weak)
        {
            bind_type = SymbolBindType::Weak;
        }
        else if (is_local)
        {
            bind_type = SymbolBindType::Local;
        }
        else if (is_global)
        {
            bind_type = SymbolBindType::Global;
        }

        const SymbolType symbol_type = is_function ? SymbolType::Function : SymbolType::Data;
        const Symbol symbol{symbol_type, bind_type, bfd_symtab[i]->name, bfd_asymbol_value(bfd_symtab[i])};

        if (!is_weak)
        {
            auto weak_symbol = std::find_if(binary.symbols.begin(), binary.symbols.end(),
                                            [&](const Symbol &s) { return s.name == symbol.name && s.bind == SymbolBindType::Weak; });
            if (weak_symbol != binary.symbols.end())
            {
                *weak_symbol = symbol;
                continue;
            }
        }

        binary.symbols.push_back(symbol);
    }
}

static void load_symbols(bfd *bfd_handle, Binary &binary)
{
    int n = bfd_get_symtab_upper_bound(bfd_handle);
    if (n <= 0)
    {
        return;
    }

    asymbol **bfd_symtab = new asymbol *[n];
    int nsyms = bfd_canonicalize_symtab(bfd_handle, bfd_symtab);
    load_from_symtab(bfd_symtab, nsyms, binary);
    delete[] bfd_symtab;
}

static void load_dynsym(bfd *bfd_handle, Binary &binary)
{
    int n = bfd_get_dynamic_symtab_upper_bound(bfd_handle);
    if (n <= 0)
    {
        return;
    }

    asymbol **bfd_symtab = new asymbol *[n];
    int nsyms = bfd_canonicalize_dynamic_symtab(bfd_handle, bfd_symtab);
    load_from_symtab(bfd_symtab, nsyms, binary);
    delete[] bfd_symtab;
}

static void load_sections(bfd *bfd_handle, Binary &binary)
{
    for (asection *bfd_section = bfd_handle->sections; bfd_section; bfd_section = bfd_section->next)
    {
        const auto flags = bfd_section->flags;
        SectionType section_type = SectionType::None;
        if (flags & SEC_CODE)
        {
            section_type = SectionType::Code;
        }
        else if (flags & SEC_DATA)
        {
            section_type = SectionType::Data;
        }
        else
        {
            continue;
        }

        const char *section_name = bfd_section_name(bfd_section);
        const std::string name = (section_name != NULL) ? section_name : "<unnamed>";
        const auto size = bfd_section_size(bfd_section);

        std::vector<uint8_t> bytes;
        bytes.resize(size);

        if (!bfd_get_section_contents(bfd_handle, bfd_section, bytes.data(), 0, size))
        {
            throw std::runtime_error("Failed to read section: " + name + ". " + bfd_errmsg(bfd_get_error()));
        }

        const Section section{&binary, name, section_type, bfd_section_vma(bfd_section), size, bytes};
        binary.sections.push_back(section);
    }
}

Binary BFDLoader::load_binary(const std::string &file_name)
{
    bfd *bfd_handle = open_bfd(file_name);

    Binary binary;
    binary.filename = file_name;
    binary.entry = bfd_get_start_address(bfd_handle);
    binary.type_name = bfd_handle->xvec->name;

    switch (bfd_handle->xvec->flavour)
    {
        case bfd_target_elf_flavour:
            binary.type = BinaryType::ELF;
            break;
        case bfd_target_coff_flavour:
            binary.type = BinaryType::PE;
            break;
        default:
            bfd_close(bfd_handle);
            throw std::invalid_argument("Unsupported binary type: " + std::string(bfd_handle->xvec->name));
    }

    const bfd_arch_info_type *bfd_info = bfd_get_arch_info(bfd_handle);
    binary.arch_name = bfd_info->printable_name;
    switch (bfd_info->mach)
    {
        case bfd_mach_i386_i386:
            binary.arch = BinaryArch::X86;
            binary.bits = 32;
            break;
        case bfd_mach_x86_64:
            binary.arch = BinaryArch::X86;
            binary.bits = 64;
            break;
        default:
            bfd_close(bfd_handle);
            throw std::invalid_argument("Unsupported architecture: " + std::string(bfd_info->printable_name));
    }

    load_symbols(bfd_handle, binary);
    load_dynsym(bfd_handle, binary);

    try
    {
        load_sections(bfd_handle, binary);
    }
    catch (std::runtime_error &)
    {
        bfd_close(bfd_handle);
        throw;
    }

    bfd_close(bfd_handle);
    return binary;
}
