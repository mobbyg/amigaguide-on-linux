#include "amigaguide/parser.h"
#include "amigaguide/renderer.h"

#include <iostream>
#include <string>

namespace {

bool check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "TEST FAILURE: " << message << '\n';
        return false;
    }
    return true;
}

} // namespace

int main()
{
    const std::string guide =
        "@database TestGuide\n"
        "@author Rich\n"
        "@node main Main Node\n"
        "Hello @{B}world@{UB}.\n"
        "@{\"Second page\" LINK \"second\"}\n"
        "@next second\n"
        "@endnode\n"
        "@node second \"Second Node\"\n"
        "Second text.\n"
        "@prev main\n"
        "@endnode\n";

    amigaguide::Document document;
    amigaguide::ParseError error;
    amigaguide::Parser parser;

    if (!check(parser.parse(guide, document, &error), "parser should accept the test guide")) return 1;
    if (!check(document.nodes().size() == 2, "guide should contain two nodes")) return 1;
    if (!check(document.find_node("MAIN") != nullptr, "node lookup should be case-insensitive")) return 1;

    const auto* main_node = document.find_node("main");
    const auto* second_node = document.find_node("SECOND");
    if (!check(main_node != nullptr, "main node should exist")) return 1;
    if (!check(second_node != nullptr, "second node should exist")) return 1;
    if (!check(main_node->next == "second", "main node should link to second")) return 1;
    if (!check(second_node->prev == "main", "second node should link back to main")) return 1;
    if (!check(document.metadata().name == "TestGuide", "database name should be parsed")) return 1;
    if (!check(document.metadata().author == "Rich", "author should be parsed")) return 1;

    const auto html = amigaguide::render_node_html(document, *main_node);
    if (!check(html.find("font-weight:bold") != std::string::npos, "bold formatting should render")) return 1;
    if (!check(html.find("href=\"node:second\"") != std::string::npos, "node link should render")) return 1;
    if (!check(html.find("Second page") != std::string::npos, "link label should render")) return 1;

    return 0;
}
