#pragma once

#include <string>
#include <string_view>

#include "amigaguide/document.h"

namespace amigaguide {

struct ParseError {
    std::size_t line = 0;
    std::string message;
};

class Parser {
public:
    bool parse(std::string source, Document& document, ParseError* error = nullptr) const;

private:
    static std::string trim(std::string_view value);
    static bool command(std::string_view line, std::string& name, std::string& args);
    static bool node_header(std::string_view args, Node& node);
};

} // namespace amigaguide
