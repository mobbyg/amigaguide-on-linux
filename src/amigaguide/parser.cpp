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

std::string first_token(std::string_view value)
{
    const auto p = value.find_first_of(" \t\r\n");
    return std::string(value.substr(0, p));
}

std::string unquote(std::string value)
{
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value.erase(value.begin());
        value.pop_back();
    }
    return value;
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
    args = std::string_view(args.data(), args.size());
    while (!args.empty() && std::isspace(static_cast<unsigned char>(args.front()))) args.remove_prefix(1);
    if (args.empty()) return false;

    const auto p = args.find_first_of(" \t");
    node.name = unquote(std::string(args.substr(0, p)));
    if (p != std::string_view::npos) {
        std::string rest = trim(args.substr(p + 1));
        // AmigaGuide permits a title as the second quoted argument. Keep the
        // remainder intact for now; later parsing can expose additional attrs.
        if (!rest.empty() && rest.front() == '"') {
            const auto q = rest.find('"', 1);
            if (q != std::string::npos) node.title = rest.substr(1, q - 1);
        }
    }
    return !node.name.empty();
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
            if (current) current->source_end = offset;
            current = nullptr;
            continue;
        }

        if (name == "database" || name == "amigaguide") {
            document.metadata().name = unquote(first_token(args));
            continue;
        }
        if (name == "author") { document.metadata().author = args; continue; }
        if (name == "version") { document.metadata().version = args; continue; }
        if (name == "copyright" || name == "(c)") { document.metadata().copyright = args; continue; }
        if (name == "font") {
            if (current) current->title = current->title; else document.metadata().font = args;
            continue;
        }
        if (name == "help") {
            if (current) current->help = unquote(first_token(args)); else document.metadata().help = unquote(first_token(args));
            continue;
        }
        if (name == "toc") {
            if (current) current->toc = unquote(first_token(args)); else document.metadata().toc = unquote(first_token(args));
            continue;
        }
        if (name == "index") {
            if (current) current->index = unquote(first_token(args)); else document.metadata().index = unquote(first_token(args));
            continue;
        }
        if (name == "keywords" && current) { current->keywords = args; continue; }
        if (name == "next" && current) { current->next = unquote(first_token(args)); continue; }
        if (name == "prev" && current) { current->prev = unquote(first_token(args)); continue; }
        if (name == "width") { try { document.metadata().width = std::stoi(first_token(args)); } catch (...) {} continue; }
        if (name == "height") { try { document.metadata().height = std::stoi(first_token(args)); } catch (...) {} continue; }
        if (name == "tab") { try { document.metadata().tab_width = std::stoi(first_token(args)); } catch (...) {} continue; }
        if (name == "wordwrap") { document.metadata().word_wrap = true; continue; }
        if (name == "smartwrap") { document.metadata().smart_wrap = true; continue; }
    }

    if (current) current->source_end = document.source().size();
    return true;
}

} // namespace amigaguide
