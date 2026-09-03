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
        "Preserve this text.\n"
        "@{Keep LINK Keep}\n"
        "@endnode\n\n"
        "@node Keep \"Keep Page\"\n"
        "Keep this node.\n"
        "@endnode\n";

    auto document = parse(original);
    amigaguide::DocumentEditor edit(document);
    std::string error;

    check(edit.set_node_title(0, "Changed Main", &error), "title change succeeds");
    check(document.source().find("@node Main \"Changed Main\"") != std::string::npos, "title changes declaration");
    check(document.source().find("Preserve this text.") != std::string::npos, "title change preserves body");

    document = parse(document.source());
    amigaguide::DocumentEditor edit_after_title(document);
    check(edit_after_title.rename_node(1, "Retained", &error), "rename succeeds");
    check(document.source().find("@node Retained \"Keep Page\"") != std::string::npos, "rename changes node name");
    check(document.source().find("@{Keep LINK Keep}") != std::string::npos, "rename does not rewrite links");

    document = parse(document.source());
    amigaguide::DocumentEditor edit_after_rename(document);
    check(edit_after_rename.add_node("NewNode", "New Node", &error), "add succeeds");
    check(document.source().find("@node NewNode \"New Node\"\n\n@endnode\n") != std::string::npos, "new node uses existing declaration form");
    auto reparsed = parse(document.source());
    check(reparsed.nodes().size() == 3, "added node reparses");

    amigaguide::DocumentEditor remove(reparsed);
    check(remove.remove_node(1, &error), "remove succeeds");
    auto after_remove = parse(reparsed.source());
    check(after_remove.nodes().size() == 2, "removed node is gone");
    check(after_remove.find_node("Main") != nullptr, "main node remains");
    check(after_remove.find_node("NewNode") != nullptr, "new node remains");
    check(after_remove.source().find("Keep this node.") == std::string::npos, "removed body is gone");
    check(after_remove.source().find("Preserve this text.") != std::string::npos, "surrounding content remains");

    check(!remove.remove_node(99, &error), "invalid removal is rejected");
    check(!remove.rename_node(99, "Nope", &error), "invalid rename is rejected");
    check(!remove.add_node("Main", "Duplicate", &error), "duplicate name is rejected");

    std::cout << "All document editor tests passed.\n";
    return 0;
}
