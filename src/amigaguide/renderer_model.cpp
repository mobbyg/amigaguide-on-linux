#include "amigaguide/renderer.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <string_view>

namespace amigaguide {
namespace {

std::string html_escape(std::string_view s)
{
    std::string out;
    for (char c : s) {
        switch (c) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        case '"': out += "&quot;"; break;
        default: out += c; break;
        }
    }
    return out;
}

std::string lower(std::string_view s)
{
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

std::string trim(std::string_view s)
{
    std::size_t first = 0;
    std::size_t last = s.size();
    while (first < last && std::isspace(static_cast<unsigned char>(s[first]))) ++first;
    while (last > first && std::isspace(static_cast<unsigned char>(s[last - 1]))) --last;
    return std::string(s.substr(first, last - first));
}

std::string first_token(std::string_view s)
{
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    const auto p = s.find_first_of(" \t\r\n");
    return std::string(s.substr(0, p));
}

bool quoted(std::string_view s, std::string& value, std::size_t& end)
{
    if (s.empty() || s.front() != '"') return false;

    std::string result;
    bool escaped = false;
    for (std::size_t i = 1; i < s.size(); ++i) {
        const char c = s[i];
        if (escaped) {
            result += c;
            escaped = false;
            continue;
        }
        if (c == '\\') {
            escaped = true;
            continue;
        }
        if (c == '"') {
            value = result;
            end = i + 1;
            return true;
        }
        result += c;
    }
    return false;
}

std::string css_color(std::string_view value)
{
    const auto color = lower(trim(value));
    if (color == "black" || color == "white" || color == "red" ||
        color == "green" || color == "blue" || color == "yellow" ||
        color == "cyan" || color == "magenta" || color == "gray" ||
        color == "grey" || color == "orange" || color == "purple") {
        return color;
    }
    return {};
}

struct Style {
    bool bold = false;
    bool italic = false;
    bool underline = false;
    std::string fg;
    std::string bg;
    std::string align = "left";
    int indent = 0;
};

// Text formatting (bold, italic, colours, links) belongs on inline spans.
// Paragraph/line layout (justification and indentation) belongs on a block.
// Keeping those two levels separate is important: QTextDocument/WebKit-style
// HTML ignores text-align and margins on an ordinary inline span.
std::string inline_style_css(const Style& style)
{
    std::ostringstream css;
    if (style.bold) css << "font-weight:bold;";
    if (style.italic) css << "font-style:italic;";
    if (style.underline) css << "text-decoration:underline;";
    if (!style.fg.empty()) css << "color:" << style.fg << ';';
    if (!style.bg.empty()) css << "background-color:" << style.bg << ';';
    return css.str();
}

std::string block_style_css(const Style& style)
{
    std::ostringstream css;
    css << "text-align:" << style.align << ';';
    if (style.indent > 0) {
        // AmigaGuide expresses LINDENT in spaces.  ch gives a stable,
        // character-oriented approximation in a proportional Qt font while
        // remaining much closer to the original intent than pixels/em.
        css << "padding-left:" << style.indent << "ch;";
    }
    return css.str();
}

void append_inline(std::string& out, const Style& style, std::string_view text)
{
    if (text.empty()) return;

    const auto css = inline_style_css(style);
    if (css.empty()) {
        out += html_escape(text);
    } else {
        out += "<span style=\"" + css + "\">" + html_escape(text) + "</span>";
    }
}

std::string node_body(const Document& document, const Node& node)
{
    const auto& source = document.source();
    if (node.source_begin >= source.size()) return {};

    auto begin = source.find('\n', node.source_begin);
    if (begin == std::string::npos) return {};
    ++begin;

    const auto end = std::min(node.source_end, source.size());
    auto end_node = source.rfind("@endnode", end);
    if (end_node != std::string::npos && end_node >= begin) end_node = end_node;
    else end_node = end;

    return source.substr(begin, end_node - begin);
}

// Render one physical source line.  The returned value is a block so that
// justification and indentation are applied to the complete line rather than
// to an inline span.  Formatting attributes remain inline inside that block.
std::string render_line(std::string_view line)
{
    Style style;
    Style block_style = style;
    bool has_visible_text = false;
    std::string content;
    std::string pending;

    auto flush = [&] {
        if (!pending.empty()) {
            if (!has_visible_text) {
                block_style = style;
                has_visible_text = true;
            }
            append_inline(content, style, pending);
            pending.clear();
        }
    };

    for (std::size_t i = 0; i < line.size();) {
        if (line[i] == '\\' && i + 1 < line.size() &&
            (line[i + 1] == '@' || line[i + 1] == '\\')) {
            pending += line[i + 1];
            i += 2;
            continue;
        }

        if (line[i] != '@' || i + 1 >= line.size() || line[i + 1] != '{') {
            pending += line[i++];
            continue;
        }

        const auto close = line.find('}', i + 2);
        if (close == std::string_view::npos) {
            pending += line[i++];
            continue;
        }

        const auto raw = trim(line.substr(i + 2, close - i - 2));
        const auto command = lower(first_token(raw));
        flush();

        if (command == "b") style.bold = true;
        else if (command == "ub") style.bold = false;
        else if (command == "i") style.italic = true;
        else if (command == "ui") style.italic = false;
        else if (command == "u") style.underline = true;
        else if (command == "uu") style.underline = false;
        else if (command == "jleft") {
            style.align = "left";
            if (!has_visible_text) block_style.align = style.align;
        } else if (command == "jcenter") {
            style.align = "center";
            if (!has_visible_text) block_style.align = style.align;
        } else if (command == "jright") {
            style.align = "right";
            if (!has_visible_text) block_style.align = style.align;
        } else if (command == "lindent") {
            try {
                const auto argument = std::string_view(raw).substr(command.size());
                style.indent = std::max(0, std::stoi(first_token(argument)));
            } catch (...) {
                style.indent = 0;
            }
            if (!has_visible_text) block_style.indent = style.indent;
        } else if (command == "fg") {
            style.fg = css_color(std::string_view(raw).substr(command.size()));
        } else if (command == "bg") {
            style.bg = css_color(std::string_view(raw).substr(command.size()));
        } else if (command == "par") {
            content += "<br><br>";
        } else if (command == "line") {
            content += "<br>";
        } else if (command == "tab") {
            content += "&nbsp;&nbsp;&nbsp;&nbsp;";
        } else if (!raw.empty() && raw.front() == '"') {
            std::string label;
            std::size_t used = 0;
            if (quoted(raw, label, used)) {
                auto rest = trim(std::string_view(raw).substr(used));
                const auto action = lower(first_token(rest));
                if (action == "link") {
                    rest = trim(std::string_view(rest).substr(4));
                    std::string target;
                    std::size_t used_target = 0;
                    if (quoted(rest, target, used_target)) {
                        content += "<a href=\"node:" + html_escape(target) + "\">" +
                                   html_escape(label) + "</a>";
                    } else {
                        target = first_token(rest);
                        if (!target.empty()) {
                            content += "<a href=\"node:" + html_escape(target) + "\">" +
                                       html_escape(label) + "</a>";
                        } else {
                            pending += label;
                        }
                    }
                } else {
                    // Do not execute CLOSE/RX/RXS/SYSTEM/etc.  Preserve the
                    // visible label as ordinary text in the Linux reader.
                    pending += label;
                }
            } else {
                pending += "@{" + raw + "}";
            }
        } else {
            // Unknown attributes are kept visible rather than silently lost.
            pending += "@{" + raw + "}";
        }

        i = close + 1;
    }

    flush();

    // A line containing only a layout command is still represented as an
    // empty block.  This preserves its vertical position without exposing the
    // command itself to the user.
    return "<div class=\"guide-line\" style=\"" + block_style_css(block_style) + "\">" +
           content + "</div>";
}

} // namespace

std::string render_node_html(const Document& document, const Node& node)
{
    const auto body = node_body(document, node);
    std::string html = "<div class=\"guide-node\"><h1 id=\"" +
                       html_escape(node.name) + "\">" +
                       html_escape(node.title.empty() ? node.name : node.title) +
                       "</h1><div class=\"guide-body\">";

    std::size_t pos = 0;
    while (pos < body.size()) {
        const auto newline = body.find('\n', pos);
        const auto end = newline == std::string::npos ? body.size() : newline;
        const std::string_view line(body.data() + pos, end - pos);
        const auto trimmed = trim(line);

        if (!trimmed.empty() && trimmed.front() == '@') {
            const auto command = lower(first_token(std::string_view(trimmed).substr(1)));
            if (command == "next" || command == "prev" || command == "help" ||
                command == "toc" || command == "index" || command == "keywords" ||
                command == "font" || command == "width" || command == "height" ||
                command == "wordwrap" || command == "smartwrap" || command == "endnode") {
                pos = newline == std::string::npos ? body.size() : newline + 1;
                continue;
            }
        }

        html += render_line(line);
        pos = newline == std::string::npos ? body.size() : newline + 1;
    }

    html += "</div></div>";
    return html;
}

} // namespace amigaguide
