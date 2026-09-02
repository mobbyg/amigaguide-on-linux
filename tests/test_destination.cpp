#include "amigaguide/destination.h"
#include "amigaguide/destination_resolver.h"

#include <cassert>
#include <iostream>

using amigaguide::Destination;
using amigaguide::DestinationType;
using amigaguide::LocalDestinationResolver;
using amigaguide::ResolutionKind;

int main()
{
    {
        const auto d = Destination::parse("Main");
        assert(d.valid());
        assert(d.type() == DestinationType::Node);
        assert(d.value() == "Main");
        assert(d.uri() == "node:Main");
    }
    {
        const auto d = Destination::parse("node:/Main");
        assert(d.type() == DestinationType::Node);
        assert(d.value() == "Main");
    }
    {
        const auto d = Destination::parse("FILE:///tmp/example.guide");
        assert(d.type() == DestinationType::File);
        assert(d.value() == "FILE:///tmp/example.guide");
    }
    {
        const auto d = Destination::parse("https://example.com/foo.guide");
        assert(d.type() == DestinationType::Https);
    }
    {
        const auto d = Destination::parse("ag://gov.us.constitution");
        assert(d.type() == DestinationType::Ag);
    }
    {
        const auto d = Destination::parse("system:delete");
        assert(!d.valid());
    }
    {
        LocalDestinationResolver resolver;
        assert(resolver.resolve(Destination::node("Main")).kind == ResolutionKind::InternalNode);
        assert(resolver.resolve(Destination::parse("file:///tmp/a.guide")).kind == ResolutionKind::LocalFile);
        assert(resolver.resolve(Destination::parse("https://example.com/a.guide")).kind == ResolutionKind::RemoteHttp);
        assert(resolver.resolve(Destination::parse("ag://example")).kind == ResolutionKind::LibraryDocument);
    }

    std::cout << "Destination tests passed\n";
    return 0;
}
