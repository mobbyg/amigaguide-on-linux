#include "amigaguide/document_editor.h"

#include <algorithm>
#include <cctype>

namespace amigaguide {

namespace {

struct Token {
    std::size_t begin = 0;
    std::size_t end = 0;
    bool quoted = false;
};

bool next_token(std::string_view value, std::size_t& cursor, Token& token)
{
    while (cursor < value.size() && std::isspace(static_cast<unsigned char>(value[cursor]))) ++cursor;
    if (cursor >= value.size()) return false;

    token.begin = cursor;
    token.quoted = value[cursor] == '"';
    if (token.quoted) {
        ++cursor;
        while (cursor < value.size() && value[cursor] != '"') ++cursor;
        if (cursor < value.size()) ++cursor;
        token.end = cursor;
        return true;
    }

    while (cursor < value.size() && !std::isspace(static_cast<unsigned char>(value[cursor]))) ++cursor;
    token.end = cursor;
    return true;
}

std::string quote(std::string_view value)
{
    std::string result = "\"";
    for (const char c : value) {
        if (c == '"') result += "\\\"";
        else result += c;
    }
    result += '"';
    return result;
}

bool is_title_command(std::string_view line, Token& argument)
{
    std::size_t cursor = 0;
    Token command;
    if (!next_token(line, cursor, command)) return false;
    if (line.substr(command.begin, command.end - command.begin) != "@title") return false;
    return next_token(line, cursor, argument);
}

} // namespace

void DocumentEditor::set_error(std::string* error, std::string message)
{
    if (error) *error = std::move(message);
}

bool DocumentEditor::replace_source(std::size_t begin, std::size_t end, std::string_view replacement, std::string* error)
{
    if (begin > end || end > document_.source().size()) {
        set_error(error, "invalid source range");
        return false;
    }
    document_.source().replace(begin, end - begin, replacement.data(), replacement.size());
    return true;
}

bool DocumentEditor::add_node(std::string name, std::string title, std::string* error)
{
    if (name.empty()) {
        set_error(error, "node name cannot be empty");
        return false;
    }
    if (document_.find_node(name)) {
        set_error(error, "a node with that name already exists");
        return false;
    }

    std::string block;
    if (!document_.source().empty() && document_.source().back() != '\n') block += '\n';
    block += "@node " + name + " " + quote(title) + "\n\n@endnode\n";
    document_.source().append(block);
    return true;
}

bool DocumentEditor::rename_node(std::size_t index, std::string name, std::string* error)
{
    if (index >= document_.nodes().size()) {
        set_error(error, "node index is out of range");
        return false;
    }
    if (name.empty()) {
        set_error(error, "node name cannot be empty");
        return false;
    }
    const auto& node = document_.nodes()[index];
    const Node* existing = document_.find_node(name);
    if (existing && existing != &node) {
        set_error(error, "a node with that name already exists");
        return false;
    }

    const auto line_end = document_.source().find('\n', node.source_begin);
    const auto header_end = line_end == std::string::npos ? document_.source().size() : line_end;
    const std::string_view line(document_.source().data() + node.source_begin, header_end - node.source_begin);
    std::size_t cursor = 0;
    Token command;
    Token name_token;
    if (!next_token(line, cursor, command) || !next_token(line, cursor, name_token)) {
        set_error(error, "could not locate node declaration");
        return false;
    }

    const std::string replacement = name_token.quoted ? quote(name) : name;
    return replace_source(node.source_begin + name_token.begin,
                          node.source_begin + name_token.end,
                          replacement,
                          error);
}

bool DocumentEditor::set_node_title(std::size_t index, std::string title, std::string* error)
{
    if (index >= document_.nodes().size()) {
        set_error(error, "node index is out of range");
        return false;
    }

    const auto& node = document_.nodes()[index];
    const std::size_t block_end = node.source_end;
    std::size_t line_begin = node.source_begin;
    while (line_begin < block_end) {
        const auto line_end = document_.source().find('\n', line_begin);
        const auto end = line_end == std::string::npos ? block_end : std::min(line_end, block_end);
        const std::string_view line(document_.source().data() + line_begin, end - line_begin);
        Token argument;
        if (is_title_command(line, argument)) {
            return replace_source(line_begin + argument.begin,
                                  line_begin + argument.end,
                                  quote(title),
                                  error);
        }
        if (line_end == std::string::npos || line_end >= block_end) break;
        line_begin = line_end + 1;
    }

    const auto header_end = document_.source().find('\n', node.source_begin);
    const auto end = header_end == std::string::npos ? document_.source().size() : header_end;
    const std::string_view line(document_.source().data() + node.source_begin, end - node.source_begin);
    std::size_t cursor = 0;
    Token command;
    Token name_token;
    if (!next_token(line, cursor, command) || !next_token(line, cursor, name_token)) {
        set_error(error, "could not locate node declaration");
        return false;
    }

    Token title_token;
    if (next_token(line, cursor, title_token)) {
        return replace_source(node.source_begin + title_token.begin,
                              node.source_begin + title_token.end,
                              quote(title),
                              error);
    }

    return replace_source(end, end, std::string(" ") + quote(title), error);
}

bool DocumentEditor::remove_node(std::size_t index, std::string* error)
{
    if (index >= document_.nodes().size()) {
        set_error(error, "node index is out of range");
        return false;
    }
    const auto& node = document_.nodes()[index];
    if (node.source_end <= node.source_begin || node.source_end > document_.source().size()) {
        set_error(error, "could not locate complete node source");
        return false;
    }
    return replace_source(node.source_begin, node.source_end, {}, error);
}

} // namespace amigaguide
