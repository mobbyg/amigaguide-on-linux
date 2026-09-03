#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace amigaguide {

struct Link {
    std::string label;
    std::string target;
    std::string source_node;
    std::size_t source_offset = 0;
    std::size_t line = 0;
    std::size_t column = 0;
    bool legacy = false;
};

std::vector<Link> find_links(const std::string& source);

} // namespace amigaguide
