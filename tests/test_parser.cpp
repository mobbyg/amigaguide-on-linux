#include "amigaguide/destination.h"
#include "amigaguide/destination_resolver.h"
#include "amigaguide/navigation.h"
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

    amigaguide::NavigationHistory history;
    if (!check(history.empty(), "new navigation history should be empty")) return 1;
    if (!check(!history.back(), "Back should fail on empty history")) return 1;
    history.visit("main");
    history.visit("second");
    history.visit("third");
    if (!check(history.current() == "third", "current destination should be latest visit")) return 1;
    if (!check(history.can_back() && !history.can_forward(), "history should initially only go back")) return 1;
    history.visit("third");
    if (!check(!history.can_forward(), "revisiting current destination should not add history")) return 1;
    if (!check(history.back() && history.current() == "second", "Back should move to previous destination")) return 1;
    if (!check(history.forward() && history.current() == "third", "Forward should restore destination")) return 1;
    if (!check(history.back() && history.current() == "second", "Back should work again")) return 1;
    history.visit("new");
    if (!check(history.current() == "new", "new visit should become current")) return 1;
    if (!check(!history.can_forward(), "new visit should discard forward history")) return 1;
    history.clear();
    if (!check(history.empty() && !history.can_back() && !history.can_forward(),
               "clear should reset navigation history")) return 1;

    const auto node = amigaguide::Destination::parse("Main");
    if (!check(node.valid() && node.type() == amigaguide::DestinationType::Node,
               "bare link should become a node destination")) return 1;
    if (!check(node.uri() == "node:Main", "node destination should have a canonical URI")) return 1;
    if (!check(amigaguide::Destination::parse("node:/Main").value() == "Main",
               "node URI should normalize a leading slash")) return 1;
    if (!check(amigaguide::Destination::parse("file:///tmp/example.guide").type() == amigaguide::DestinationType::File,
               "file URI should be recognized")) return 1;
    if (!check(amigaguide::Destination::parse("https://example.com/foo.guide").type() == amigaguide::DestinationType::Https,
               "HTTPS URI should be recognized")) return 1;
    if (!check(amigaguide::Destination::parse("ag://gov.us.constitution").type() == amigaguide::DestinationType::Ag,
               "AG URI should be recognized")) return 1;
    if (!check(!amigaguide::Destination::parse("system:delete").valid(),
               "unknown executable schemes should be rejected")) return 1;

    amigaguide::LocalDestinationResolver resolver;
    if (!check(resolver.resolve(node).kind == amigaguide::ResolutionKind::InternalNode,
               "node destinations should resolve internally")) return 1;
    if (!check(resolver.resolve(amigaguide::Destination::parse("file:///tmp/a.guide")).kind == amigaguide::ResolutionKind::LocalFile,
               "file destinations should resolve as local files")) return 1;
    if (!check(resolver.resolve(amigaguide::Destination::parse("https://example.com/a.guide")).kind == amigaguide::ResolutionKind::RemoteHttp,
               "HTTP destinations should be classified as remote")) return 1;
    if (!check(resolver.resolve(amigaguide::Destination::parse("ag://example")).kind == amigaguide::ResolutionKind::LibraryDocument,
               "AG destinations should be classified for the library")) return 1;

    amigaguide::Document malformed;
    const std::string malformed_guide =
        "@database Broken\n"
        "@node main\n"
        "Missing end node\n";
    if (!check(!parser.parse(malformed_guide, malformed, &error), "unterminated node should be rejected")) return 1;

    return 0;
}
