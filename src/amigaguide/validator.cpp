#include "amigaguide/validator.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "amigaguide/parser.h"

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

std::string trim(std::string_view value)
{
    std::size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) ++first;
    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) --last;
    return std::string(value.substr(first, last - first));
}

std::string first_token(std::string_view value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.remove_prefix(1);
    if (value.empty()) return {};
    if (value.front() == '"') {
        const auto end = value.find('"', 1);
        return std::string(value.substr(1, end == std::string_view::npos ? value.size() - 1 : end - 1));
    }
    const auto end = value.find_first_of(" \t\r\n");
    return std::string(value.substr(0, end));
}

bool has_scheme_or_path(std::string_view target)
{
    if (target.find("://") != std::string_view::npos) return true;
    if (target.rfind("ag:", 0) == 0) return true;
    return target.find_first_of("/\\") != std::string_view::npos ||
           target.find_first_of(".") != std::string_view::npos;
}

void add_issue(ValidationResult& result, ValidationSeverity severity, std::size_t line,
               std::size_t column, std::string message, std::string node = {})
{
    result.issues.push_back({severity, line, column, std::move(message), std::move(node)});
}

std::string current_node_for_offset(const Document& document, std::size_t offset)
{
    for (const auto& node : document.nodes()) {
        if (offset >= node.source_begin && offset < node.source_end) return node.name;
    }
    return {};
}

bool parse_link(std::string_view raw, std::string& target)
{
    raw = std::string_view(raw.data(), raw.size());
    const auto pipe = raw.find('|');
    if (pipe != std::string_view::npos) {
        auto rest = trim(raw.substr(pipe + 1));
        if (rest.empty() || rest.front() != '"') return false;
        const auto label_end = rest.find('"', 1);
        if (label_end == std::string_view::npos) return false;
        rest = trim(rest.substr(label_end + 1));
        if (lower(first_token(rest)) != "link") return false;
        rest = trim(rest.substr(4));
        target = first_token(rest);
        if (!target.empty() && target.front() == '"' && target.back() == '"' && target.size() > 1)
            target = target.substr(1, target.size() - 2);
        return !target.empty();
    }

    if (raw.empty() || raw.front() != '"') return false;
    const auto label_end = raw.find('"', 1);
    if (label_end == std::string_view::npos) return false;
    auto rest = trim(raw.substr(label_end + 1));
    if (lower(first_token(rest)) != "link") return false;
    rest = trim(rest.substr(4));
    target = first_token(rest);
    if (!target.empty() && target.front() == '"' && target.back() == '"' && target.size() > 1)
        target = target.substr(1, target.size() - 2);
    return !target.empty();
}

const std::unordered_set<std::string>& known_commands()
{
    static const std::unordered_set<std::string> commands = {
        "database", "amigaguide", "author", "version", "$ver:", "copyright", "(c)",
        "font", "help", "toc", "index", "worddelimiter", "width", "height", "tab",
        "wordwrap", "smartwrap", "node", "endnode", "title", "keywords", "next", "prev",
        "onopen", "onclose", "b", "ub", "i", "ui", "u", "uu", "jleft", "jcenter",
        "jright", "lindent", "fg", "bg", "par", "line", "tab", "system", "rx", "rxs",
        "close", "link"
    };
    return commands;
}

} // namespace

bool ValidationResult::ok() const noexcept
{
    return error_count() == 0;
}

std::size_t ValidationResult::error_count() const noexcept
{
    return static_cast<std::size_t>(std::count_if(issues.begin(), issues.end(), [](const ValidationIssue& issue) {
        return issue.severity == ValidationSeverity::Error;
    }));
}

std::size_t ValidationResult::warning_count() const noexcept
{
    return static_cast<std::size_t>(std::count_if(issues.begin(), issues.end(), [](const ValidationIssue& issue) {
        return issue.severity == ValidationSeverity::Warning;
    }));
}

ValidationResult Validator::validate(const std::string& source, const Document* parsed) const
{
    ValidationResult result;
    Document local_document;
    ParseError parse_error;
    const Document* document = parsed;
    if (!document) {
        if (Parser{}.parse(source, local_document, &parse_error)) {
            document = &local_document;
        } else {
            add_issue(result, ValidationSeverity::Error, parse_error.line, 1, parse_error.message);
        }
    }

    if (document) {
        std::unordered_map<std::string, std::size_t> node_lines;
        for (const auto& node : document->nodes()) {
            const auto key = lower(node.name);
            if (const auto found = node_lines.find(key); found != node_lines.end()) {
                std::size_t line = 1;
                for (std::size_t i = 0, end = node.source_begin; i < end && i < source.size(); ++i)
                    if (source[i] == '\n') ++line;
                add_issue(result, ValidationSeverity::Error, line, 1,
                          "duplicate node name: " + node.name, node.name);
            } else {
                node_lines.emplace(key, 0);
            }
        }

        std::unordered_set<std::string> node_names;
        for (const auto& node : document->nodes()) node_names.insert(lower(node.name));

        std::size_t offset = 0;
        std::size_t line_number = 0;
        std::istringstream input(source);
        std::string line;
        while (std::getline(input, line)) {
            ++line_number;
            const auto line_begin = offset;
            offset += line.size();
            if (offset < source.size() && source[offset] == '\n') ++offset;

            std::string current_node = current_node_for_offset(*document, line_begin);
            for (std::size_t i = 0; i + 1 < line.size(); ++i) {
                if (line[i] != '@' || line[i + 1] != '{') continue;
                const auto close = line.find('}', i + 2);
                if (close == std::string::npos) {
                    add_issue(result, ValidationSeverity::Error, line_number, i + 1,
                              "unterminated @{...} attribute", current_node);
                    break;
                }
                std::string target;
                const auto raw = trim(std::string_view(line).substr(i + 2, close - i - 2));
                if (parse_link(raw, target) && !has_scheme_or_path(target) &&
                    node_names.find(lower(target)) == node_names.end()) {
                    add_issue(result, ValidationSeverity::Error, line_number, i + 1,
                              "link target does not exist: " + target, current_node);
                }
                i = close;
            }

            const auto trimmed = trim(line);
            if (trimmed.empty() || trimmed.front() != '@' || trimmed.rfind("@{", 0) == 0) continue;
            std::size_t p = 1;
            while (p < trimmed.size() && !std::isspace(static_cast<unsigned char>(trimmed[p]))) ++p;
            const auto command = lower(std::string_view(trimmed).substr(1, p - 1));
            if (known_commands().find(command) == known_commands().end()) {
                add_issue(result, ValidationSeverity::Warning, line_number, 1,
                          "unknown or unsupported command: @" + command, current_node);
            }
        }
    }

    return result;
}

} // namespace amigaguide
