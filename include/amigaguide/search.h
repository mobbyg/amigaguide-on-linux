#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "amigaguide/document.h"

namespace amigaguide {

struct SearchMatch {
    std::size_t node_index = 0;
    std::size_t source_offset = 0;
    std::size_t length = 0;
};

class SearchEngine {
public:
    std::vector<SearchMatch> find(const Document& document,
                                  const std::string& query,
                                  bool case_sensitive = false) const;
};

} // namespace amigaguide
