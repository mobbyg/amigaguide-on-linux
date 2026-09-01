#include "amigaguide/parser.h"

#include <cassert>
#include <string>

int main()
{
    const std::string guide =
        "@database TestGuide\n"
        "@author Rich\n"
        "@node main Main Node\n"
        "Hello world.\n"
        "@next second\n"
        "@endnode\n"
        "@node second \"Second Node\"\n"
        "Second text.\n"
        "@prev main\n"
        "@endnode\n";

    amigaguide::Document document;
    amigaguide::ParseError error;
    amigaguide::Parser parser;
    assert(parser.parse(guide, document, &error));
    assert(document.nodes().size() == 2);
    assert(document.find_node("MAIN") != nullptr);
    assert(document.find_node("main")->next == "second");
    assert(document.find_node("SECOND")->prev == "main");
    assert(document.metadata().name == "TestGuide");
    assert(document.metadata().author == "Rich");
    return 0;
}
