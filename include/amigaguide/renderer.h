#pragma once

#include <string>
#include <string_view>

#include "amigaguide/document.h"

namespace amigaguide {

// Convert the contents of one parsed node to a small, self-contained HTML
// fragment suitable for display in Qt's QTextBrowser/QTextDocument.
//
// This deliberately does not execute AmigaDOS, ARexx, or other action links.
// Such commands are represented as unsupported text for the Linux reader.
std::string render_node_html(const Document& document, const Node& node);

} // namespace amigaguide
