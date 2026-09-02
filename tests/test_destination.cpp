#include "amigaguide/destination.h"
#include "amigaguide/destination_resolver.h"

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

    return 0;
}
