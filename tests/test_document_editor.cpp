#include "amigaguide/document_editor.h"
#include "amigaguide/parser.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

amigaguide::Document parse(const std::string& source)
{
    amigaguide::Document document;
    amigaguide::ParseError error;
    check(amigaguide::Parser{}.parse(source, document, &error), "test source should parse");
    return document;
}
} // namespace

int main()
{
    const std::string original =
        "@database Test Guide\n\n"
        "@node Main \"Main Page\"\n"
        "@keywords old keywords\n"
        "@prev Previous\n"
        "@help \"Help Page\"\n"
        "Preserve this text.\n"
        "@{Keep LINK Keep}\n"
        "@endnode\n\n"
        "@node Keep \"Keep Page\"\n"
        "Keep this node.\n"
        "@endnode\n";

    auto document = parse(original);
    std::string error;

    amigaguide::DocumentEditor edit(document);
    check(edit.set_node_title(0, "Changed Main", &error), "title change succeeds");
    check(document.source().find("@node Main \"Changed Main\"") != std::string::npos, "title changes declaration");
    check(document.source().find("Preserve this text.") != std::string::npos, "title preserves body");

    document = parse(document.source());
    amigaguide::DocumentEditor metadata(document);
    check(metadata.set_node_property(0, amigaguide::NodeProperty::Keywords, "new keywords", &error), "keywords update succeeds");
    check(document.source().find("@keywords new keywords") != std::string::npos, "keywords updated");

    document = parse(document.source());
    amigaguide::DocumentEditor next(document);
    check(next.set_node_property(0, amigaguide::NodeProperty::Next, "NextPage", &error), "missing property insertion succeeds");
    check(document.source().find("@next \"NextPage\"") != std::string::npos, "missing property inserted");

    document = parse(document.source());
    amigaguide::DocumentEditor help(document);
    check(help.set_node_property(0, amigaguide::NodeProperty::Help, "New Help", &error), "quoted property update succeeds");
    check(document.source().find("@help \"New Help\"") != std::string::npos, "help updated");

    document = parse(document.source());
    amigaguide::DocumentEditor prev(document);
    check(prev.set_node_property(0, amigaguide::NodeProperty::Prev, "", &error), "property removal succeeds");
    check(document.source().find("@prev Previous") == std::string::npos, "property removed");

    document = parse(document.source());
    amigaguide::DocumentEditor tab(document);
    check(tab.set_node_property(0, amigaguide::NodeProperty::TabWidth, "8", &error), "tab width insertion succeeds");
    check(document.source().find("@tab 8") != std::string::npos, "tab width inserted");

    document = parse(document.source());
    amigaguide::DocumentEditor flags(document);
    check(flags.set_node_flag(0, amigaguide::NodeFlag::SmartWrap, true, &error), "flag insertion succeeds");
    check(document.source().find("@smartwrap") != std::string::npos, "flag inserted");

    document = parse(document.source());
    amigaguide::DocumentEditor flags_off(document);
    check(flags_off.set_node_flag(0, amigaguide::NodeFlag::SmartWrap, false, &error), "flag removal succeeds");
    check(document.source().find("@smartwrap") == std::string::npos, "flag removed");

    document = parse(document.source());
    amigaguide::DocumentEditor edit_after(document);
    check(edit_after.rename_node(1, "Retained", &error), "rename succeeds");
    check(document.source().find("@node Retained \"Keep Page\"") != std::string::npos, "rename changes node name");
    check(document.source().find("@{Keep LINK Keep}") != std::string::npos, "rename does not rewrite links");

    document = parse(document.source());
    amigaguide::DocumentEditor add(document);
    check(add.add_node("NewNode", "New Node", &error), "add succeeds");
    auto reparsed = parse(document.source());
    check(reparsed.nodes().size() == 3, "added node reparses");

    amigaguide::DocumentEditor remove(reparsed);
    check(remove.remove_node(1, &error), "remove succeeds");
    auto after = parse(reparsed.source());
    check(after.nodes().size() == 2, "removed node is gone");
    check(after.find_node("Main") != nullptr, "main remains");
    check(after.find_node("NewNode") != nullptr, "new node remains");
    check(after.source().find("Keep this node.") == std::string::npos, "removed body is gone");
    check(after.source().find("Preserve this text.") != std::string::npos, "surrounding content remains");

    check(!remove.remove_node(99, &error), "invalid removal rejected");
    check(!remove.rename_node(99, "Nope", &error), "invalid rename rejected");
    check(!remove.add_node("Main", "Duplicate", &error), "duplicate name rejected");

    std::cout << "All document editor tests passed.\n";
    return 0;
}
