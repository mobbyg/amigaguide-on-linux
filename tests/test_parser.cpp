#include "amigaguide/parser.h"
#include "amigaguide/renderer.h"
#include "amigaguide/search.h"

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
        "@font Topaz.font 8\n"
        "@node main Main Node\n"
        "@title \"Main Page\"\n"
        "@font Courier.font 12\n"
        "@tab 4\n"
        "@wordwrap\n"
        "Hello @{B}world@{UB}.\n"
        "Hello again.\n"
        "@{\"Second page\" LINK \"second\"}\n"
        "@next \"second\"\n"
        "@endnode\n"
        "@node \"second\" \"Second Node\"\n"
        "@proportional\n"
        "Second text.\n"
        "WORLD appears here too.\n"
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
    if (!check(main_node->title == "Main Page", "@TITLE should override the node title")) return 1;
    if (!check(main_node->font == "Courier.font 12", "node font should be parsed")) return 1;
    if (!check(main_node->tab_width == 4, "node tab width should be parsed")) return 1;
    if (!check(main_node->word_wrap, "node word wrapping should be enabled")) return 1;
    if (!check(second_node->proportional, "@PROPORTIONAL should be parsed")) return 1;
    if (!check(main_node->next == "second", "main node should link to second")) return 1;
    if (!check(second_node->prev == "main", "second node should link back to main")) return 1;
    if (!check(document.metadata().name == "TestGuide", "database name should be parsed")) return 1;
    if (!check(document.metadata().author == "Rich", "author should be parsed")) return 1;
    if (!check(document.metadata().font == "Topaz.font 8", "global font should be parsed")) return 1;

    const auto html = amigaguide::render_node_html(document, *main_node);
    if (!check(html.find("font-weight:bold") != std::string::npos, "bold formatting should render")) return 1;
    if (!check(html.find("href=\"node:second\"") != std::string::npos, "node link should render")) return 1;
    if (!check(html.find("Second page") != std::string::npos, "link label should render")) return 1;

    amigaguide::SearchEngine search;
    const auto world_matches = search.find(document, "world");
    if (!check(world_matches.size() == 2, "case-insensitive search should find both world matches")) return 1;
    if (!check(world_matches[0].node_index == 0 && world_matches[1].node_index == 1,
               "search results should be returned in document order")) return 1;

    const auto exact_matches = search.find(document, "WORLD", true);
    if (!check(exact_matches.size() == 1 && exact_matches[0].node_index == 1,
               "case-sensitive search should find only the exact match")) return 1;
    if (!check(search.find(document, "missing").empty(), "missing search should return no matches")) return 1;
    if (!check(search.find(document, "").empty(), "empty search should return no matches")) return 1;

    amigaguide::Document malformed;
    const std::string malformed_guide =
        "@database Broken\n"
        "@node main\n"
        "Missing end node\n";
    if (!check(!parser.parse(malformed_guide, malformed, &error), "unterminated node should be rejected")) return 1;

    return 0;
}
