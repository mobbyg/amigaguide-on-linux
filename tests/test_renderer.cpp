#include "amigaguide/renderer.h"

#include <iostream>
#include <string>

using amigaguide::Document;
using amigaguide::Node;
using amigaguide::render_node_html;

namespace {

std::string render(const std::string& body)
{
    Document document;
    document.set_source(
        "@database Test\n"
        "@node Main Main\n" + body +
        "@endnode\n");

    Node node;
    node.name = "Main";
    node.title = "Main";
    node.source_begin = document.source().find("@node Main Main");
    node.source_end = document.source().size();
    document.nodes().push_back(node);

    return render_node_html(document, document.nodes().front());
}

bool contains(const std::string& html, const std::string& expected)
{
    if (html.find(expected) != std::string::npos) return true;
    std::cerr << "Expected output not found: " << expected << "\n";
    std::cerr << "Actual HTML: " << html << "\n";
    return false;
}

bool not_contains(const std::string& html, const std::string& unexpected)
{
    if (html.find(unexpected) == std::string::npos) return true;
    std::cerr << "Unexpected output found: " << unexpected << "\n";
    std::cerr << "Actual HTML: " << html << "\n";
    return false;
}

} // namespace

int main()
{
    bool ok = true;

    {
        const auto html = render("@{\"Preamble\" LINK \"Preamble\"}\n");
        ok &= contains(html, "<a href=\"node:Preamble\">Preamble</a>");
        ok &= not_contains(html, "@{\"Preamble\" LINK \"Preamble\"}");
    }

    {
        const auto html = render("@{Preamble|\"Preamble\" LINK Preamble}\n");
        ok &= contains(html, "<a href=\"node:Preamble\">Preamble</a>");
        ok &= not_contains(html, "@{Preamble|\"Preamble\" LINK Preamble}");
    }

    {
        const auto html = render("@{Article I|\"Article I\" LINK Article_I}\n");
        ok &= contains(html, "<a href=\"node:Article_I\">Article I</a>");
        ok &= not_contains(html, "@{Article I|\"Article I\" LINK Article_I}");
    }

    return ok ? 0 : 1;
}
