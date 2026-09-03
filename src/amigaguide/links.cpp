#include "amigaguide/links.h"

#include <algorithm>
#include <cctype>
#include <string_view>

namespace amigaguide {
namespace {

std::string trim(std::string_view value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.remove_prefix(1);
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) value.remove_suffix(1);
    return std::string(value);
}

std::string lower(std::string_view value)
{
    std::string out(value);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

bool quoted(std::string_view value, std::string& result, std::size_t& used)
{
    if (value.empty() || value.front() != '"') return false;
    result.clear();
    bool escaped = false;
    for (std::size_t i = 1; i < value.size(); ++i) {
        const char c = value[i];
        if (escaped) {
            result += c;
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '"') {
            used = i + 1;
            return true;
        } else {
            result += c;
        }
    }
    return false;
}

std::string first_token(std::string_view value)
{
    value.remove_prefix(std::min(value.find_first_not_of(" \t\r\n"), value.size()));
    if (value.empty()) return {};
    if (value.front() == '"') {
        std::string result;
        std::size_t used = 0;
        if (quoted(value, result, used)) return result;
        return std::string(value.substr(1));
    }
    const auto end = value.find_first_of(" \t\r\n");
    return std::string(value.substr(0, end));
}

void line_location(const std::string& source, std::size_t offset, std::size_t& line, std::size_t& column)
{
    line = 1;
    column = 1;
    for (std::size_t i = 0; i < offset; ++i) {
        if (source[i] == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
    }
}

std::string current_node(const std::string& source, std::size_t offset)
{
    std::size_t pos = 0;
    std::string node;
    while (pos < offset) {
        const auto end = source.find('\n', pos);
        const auto line_end = end == std::string::npos ? source.size() : end;
        const std::string_view line(source.data() + pos, line_end - pos);
        const auto trimmed = trim(line);
        if (trimmed.size() >= 5 && trimmed[0] == '@' &&
            lower(trimmed.substr(1, 4)) == "node" &&
            (trimmed.size() == 5 || std::isspace(static_cast<unsigned char>(trimmed[5])))) {
            node = first_token(trimmed.substr(5));
        } else if (trimmed.size() >= 8 && trimmed[0] == '@' &&
                   lower(trimmed.substr(1, 7)) == "endnode" &&
                   (trimmed.size() == 8 || std::isspace(static_cast<unsigned char>(trimmed[8])))) {
            node.clear();
        }
        pos = end == std::string::npos ? source.size() : end + 1;
    }
    return node;
}

void add_link(const std::string& source, std::vector<Link>& links, std::size_t offset,
              std::string label, std::string target, bool legacy)
{
    if (label.empty() || target.empty()) return;
    Link link;
    link.label = std::move(label);
    link.target = std::move(target);
    link.source_node = current_node(source, offset);
    link.source_offset = offset;
    line_location(source, offset, link.line, link.column);
    link.legacy = legacy;
    links.push_back(std::move(link));
}

} // namespace

std::vector<Link> find_links(const std::string& source)
{
    std::vector<Link> links;
    std::size_t line_start = 0;
    while (line_start < source.size()) {
        const auto newline = source.find('\n', line_start);
        const auto line_end = newline == std::string::npos ? source.size() : newline;
        const std::string_view line(source.data() + line_start, line_end - line_start);

        for (std::size_t i = 0; i + 2 < line.size(); ++i) {
            if (line[i] != '@' || line[i + 1] != '{') continue;
            const auto close = line.find('}', i + 2);
            if (close == std::string_view::npos) break;
            const auto raw = trim(line.substr(i + 2, close - i - 2));

            std::string label;
            std::size_t used = 0;
            if (!quoted(raw, label, used)) {
                const auto pipe = raw.find('|');
                if (pipe != std::string_view::npos) {
                    const auto left = trim(raw.substr(0, pipe));
                    auto rest = trim(raw.substr(pipe + 1));
                    if (quoted(rest, label, used)) {
                        rest = trim(rest.substr(used));
                        if (lower(first_token(rest)) == "link") {
                            rest = trim(rest.substr(4));
                            const auto target = first_token(rest);
                            if (!target.empty()) add_link(source, links, line_start + i, label,
                                                          target, true);
                        }
                    } else if (!left.empty()) {
                        // Legacy syntax requires a quoted label; leave other pipe attributes alone.
                    }
                }
                i = close;
                continue;
            }

            auto rest = trim(raw.substr(used));
            if (lower(first_token(rest)) != "link") {
                i = close;
                continue;
            }
            rest = trim(rest.substr(4));
            const auto target = first_token(rest);
            if (!target.empty()) add_link(source, links, line_start + i, label, target, false);
            i = close;
        }

        if (newline == std::string::npos) break;
        line_start = newline + 1;
    }
    return links;
}

} // namespace amigaguide
