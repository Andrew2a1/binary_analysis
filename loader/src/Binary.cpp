#include "Binary.h"

#include <algorithm>
#include <stdexcept>
#include <string>

#include "Section.h"

const Section &Binary::get_section(const std::string &section_name) const
{
    const auto found_section = std::find_if(sections.cbegin(), sections.cend(), [&section_name](const Section &section) { return section.name == section_name; });
    if (found_section == sections.cend())
    {
        throw std::invalid_argument("Section with name: " + section_name + " doesn't exists");
    }
    return *found_section;
}

bool Binary::section_exists(const std::string &section_name) const
{
    return std::find_if(sections.cbegin(), sections.cend(), [&section_name](const Section &section) { return section.name == section_name; }) != sections.cend();
}