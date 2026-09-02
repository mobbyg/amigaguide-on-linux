#include "amigaguide/parser.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace amigaguide {

namespace {

std::string lower(std::string_view value)
{
    std::string out(value);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

std::string unquote(std::string value)
{
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value.erase(value.begin());
        value.pop_back();
    }
    return value;
}

// Read the first whitespace-delimited argument, honoring quoted arguments.
// AmigaGuide uses quotes when an option contains spaces.
std::string first_token(std::string_view value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.remove_prefix(1);
    if (value.empty()) return {};

    if (value.front() == '"') {
        const auto end = value.find('"', 1);
        if (end == std::string_view::npos) return std::string(value.substr(1));
        return std::string(value.substr(1, end - 1));
    }

    const auto p = value.find_first_of(" \t\r\n");
    return std::string(value.substr(0, p));
}

} // namespace

std::string Parser::trim(std::string_view value)
{
    std::size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) ++first;
    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) --last;
    return std::string(value.substr(first, last - first));
}

bool Parser::command(std::string_view line, std::string& name, std::string& args)
{
    if (line.empty() || line.front() != '@') return false;
    line.remove_prefix(1);
    const auto p = line.find_first_of(" \t\r\n");
    name = lower(line.substr(0, p));
    args = p == std::string_view::npos ? std::string{} : trim(line.substr(p + 1));
    return !name.empty();
}

bool Parser::node_header(std::string_view args, Node& node)
{
    while (!args.empty() && std::isspace(static_cast<unsigned char>(args.front()))) args.remove_prefix(1);
    if (args.empty()) return false;

    node.name = first_token(args);
    if (node.name.empty()) return false;

    // The title is the second argument and may be quoted.
    std::size_t consumed = 0;
    if (args.front() == '"') {
        const auto end = args.find('"', 1);
        if (end == std::string_view::npos) return true;
        consumed = end + 1;
    } else {
        const auto p = args.find_first_of(" \t");
        consumed = p == std::string_view::npos ? args.size() : p;
    }

    std::string_view rest = args.substr(consumed);
    while (!rest.empty() && std::isspace(static_cast<unsigned char>(rest.front()))) rest.remove_prefix(1);
    if (!rest.empty()) node.title = first_token(rest);
    return true;
}

bool Parser::parse(std::string source, Document& document, ParseError* error) const
{
    document = Document{};
    document.set_source(std::move(source));

    std::size_t line_number = 0;
    std::size_t offset = 0;
    Node* current = nullptr;

    std::istringstream input(document.source());
    std::string line;
    while (std::getline(input, line)) {
        ++line_number;
        const std::size_t line_begin = offset;
        offset += line.size();
        if (offset < document.source().size() && document.source()[offset] == '\n') ++offset;

        std::string name;
        std::string args;
        if (!command(line, name, args)) continue;

        if (name == "node") {
            if (current) {
                if (error) *error = {line_number, "@NODE encountered before @ENDNODE"};
                return false;
            }
            document.nodes().push_back(Node{});
            current = &document.nodes().back();
            current->source_begin = line_begin;
            if (!node_header(args, *current)) {
                if (error) *error = {line_number, "@NODE has no node name"};
                return false;
            }
            continue;
        }

        if (name == "endnode") {
            if (!current) {
                if (error) *error = {line_number, "@ENDNODE without an active @NODE"};
                return false;
            }
            current->source_end = offset;
            current = nullptr;
            continue;
        }

        if (name == "database" || name == "amigaguide") {
            document.metadata().name = first_token(args);
            continue;
        }
        if (name == "author") { document.metadata().author = args; continue; }
        if (name == "version" || name == "$ver:") { document.metadata().version = args; continue; }
        if (name == "copyright" || name == "(c)") { document.metadata().copyright = args; continue; }
        if (name == "font") {
            if (current) current->font = args;
            else document.metadata().font = args;
            continue;
        }
        if (name == "title" && current) { current->title = first_token(args); continue; }
        if (name == "help") {
            if (current) current->help = first_token(args); else document.metadata().help = first_token(args);
            continue;
        }
        if (name == "toc") {
            if (current) current->toc = first_token(args); else document.metadata().toc = first_token(args);
            continue;
        }
        if (name == "index") {
            if (current) current->index = first_token(args); else document.metadata().index = first_token(args);
            continue;
        }
        if (name == "keywords" && current) { current->keywords = args; continue; }
        if (name == "next" && current) { current->next = first_token(args); continue; }
        if (name == "prev" && current) { current->prev = first_token(args); continue; }
        if (name == "onopen") {
            if (current) current->on_open = args;
            continue;
        }
        if (name == "onclose") {
            if (current) current->on_close = args;
            continue;
        }
        if (name == "width") {
            try { document.metadata().width = std::stoi(first_token(args)); } catch (...) {}
            continue;
        }
        if (name == "height") {
            try { document.metadata().height = std::stoi(first_token(args)); } catch (...) {}
            continue;
        }
        if (name == "tab") {
            try {
                const int width = std::stoi(first_token(args));
                if (current) current->tab_width = width;
                else document.metadata().tab_width = width;
            } catch (...) {}
            continue;
        }
        if (name == "wordwrap") {
            if (current) current->word_wrap = true;
            else document.metadata().word_wrap = true;
            continue;
        }
        if (name == "smartwrap") {
            if (current) current->smart_wrap = true;
            else document.metadata().smart_wrap = true;
            continue;
        }
        if (name == "proportional" && current) {
            current->proportional = true;
            continue;
        }
    }

    if (current) {
        if (error) *error = {line_number, "document ended before @ENDNODE"};
        return false;
    }

    return true;
}

} // namespace amigaguide
