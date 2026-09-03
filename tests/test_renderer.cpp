#include "amigaguide/parser.h"
#include "amigaguide/renderer.h"

#include <cassert>
#include <string>

using amigaguide::Document;
using amigaguide::Parser;
using amigaguide::render_node_html;

namespace {

std::string render(const std::string& body)
{
    const std::string source =
        "@database Test\n"
        "@node Main Main\n" + body +
        "@endnode\n";

    Document document;
    Parser parser;
    amigaguide::ParseError error;
    assert(parser.parse(source, document, &error));
    assert(!document.nodes().empty());
    return render_node_html(document, document.nodes().front());
}

} // namespace

int main()
{
    {
        const auto html = render("@{\"Preamble\" LINK \"Preamble\"}\n");
        assert(html.find("<a href=\"node:Preamble\">Preamble</a>") != std::string::npos);
        assert(html.find("@{\"Preamble\" LINK \"Preamble\"}") == std::string::npos);
    }

    {
        const auto html = render("@{Preamble|\"Preamble\" LINK Preamble}\n");
        assert(html.find("<a href=\"node:Preamble\">Preamble</a>") != std::string::npos);
        assert(html.find("@{Preamble|\"Preamble\" LINK Preamble}") == std::string::npos);
    }

    {
        const auto html = render("@{Article I|\"Article I\" LINK Article_I}\n");
        assert(html.find("<a href=\"node:Article_I\">Article I</a>") != std::string::npos);
        assert(html.find("@{Article I|\"Article I\" LINK Article_I}") == std::string::npos);
    }

    return 0;
}
