#include "amigaguide/search.h"

#include <algorithm>
#include <cctype>
#include <string_view>

namespace amigaguide {
namespace {

char fold(char c)
{
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

bool equal_at(std::string_view text, std::size_t offset,
              std::string_view query, bool case_sensitive)
{
    if (offset + query.size() > text.size()) return false;
    for (std::size_t i = 0; i < query.size(); ++i) {
        const char lhs = text[offset + i];
        const char rhs = query[i];
        if (case_sensitive ? lhs != rhs : fold(lhs) != fold(rhs)) return false;
    }
    return true;
}

bool is_command_line(std::string_view line)
{
    while (!line.empty() && (line.front() == ' ' || line.front() == '\t' ||
                              line.front() == '\r')) {
        line.remove_prefix(1);
    }
    return !line.empty() && line.front() == '@';
}

} // namespace

std::vector<SearchMatch> SearchEngine::find(const Document& document,
                                            const std::string& query,
                                            bool case_sensitive) const
{
    std::vector<SearchMatch> matches;
    if (query.empty()) return matches;

    const auto& source = document.source();
    const auto& nodes = document.nodes();

    for (std::size_t node_index = 0; node_index < nodes.size(); ++node_index) {
        const auto& node = nodes[node_index];
        if (node.source_begin >= source.size()) continue;

        std::size_t begin = source.find('\n', node.source_begin);
        if (begin == std::string::npos) continue;
        ++begin;
        const std::size_t end = std::min(node.source_end, source.size());

        std::size_t line_begin = begin;
        while (line_begin < end) {
            const auto newline = source.find('\n', line_begin);
            const std::size_t line_end =
                newline == std::string::npos ? end : std::min(newline, end);
            const std::string_view line(source.data() + line_begin,
                                        line_end - line_begin);

            // Search visible node text, not structural @commands. Inline
            // formatting commands remain searchable because their labels are
            // part of the visible text.
            if (!is_command_line(line)) {
                for (std::size_t pos = 0; pos + query.size() <= line.size(); ++pos) {
                    if (equal_at(line, pos, query, case_sensitive)) {
                        matches.push_back({node_index, line_begin + pos, query.size()});
                    }
                }
            }

            if (newline == std::string::npos || newline >= end) break;
            line_begin = newline + 1;
        }
    }

    return matches;
}

} // namespace amigaguide
