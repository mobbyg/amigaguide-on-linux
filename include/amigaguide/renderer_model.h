#pragma once

#include <string>
#include <vector>

namespace amigaguide {

enum class TextStyle : unsigned char {
    Normal,
    Bold,
    Italic,
    Underline
};

struct Span {
    std::string text;
    TextStyle style = TextStyle::Normal;
    int foreground = -1;
    int background = -1;
    bool link = false;
    std::string target;
};

struct Paragraph {
    std::vector<Span> spans;
    int justification = 0; // -1 left, 0 default, 1 right, 2 center
    int left_indent = 0;
    int first_line_indent = 0;
};

} // namespace amigaguide
