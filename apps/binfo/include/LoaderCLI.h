#pragma once

#include <tabulate/table.hpp>
#include <vector>

#include "Binary.h"
#include "Section.h"

class LoaderCLI
{
private:
    Binary bin;

public:
    explicit LoaderCLI(const std::string &binary_name);

    void show_sections() const;
    void show_section_data(const std::string &section_name, int data_bytes_per_row = 16) const;
    void show_symbols(bool demangle = false) const;

private:
    void format_table_single_header(tabulate::Table &table, int row_count) const;
};