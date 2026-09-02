#include "amigaguide/destination_resolver.h"

namespace amigaguide {

Resolution LocalDestinationResolver::resolve(const Destination& destination) const
{
    switch (destination.type()) {
    case DestinationType::Node:
        return {ResolutionKind::InternalNode, destination.value()};
    case DestinationType::File:
        return {ResolutionKind::LocalFile, destination.value()};
    case DestinationType::Http:
    case DestinationType::Https:
        return {ResolutionKind::RemoteHttp, destination.value()};
    case DestinationType::Ag:
        return {ResolutionKind::LibraryDocument, destination.value()};
    case DestinationType::Invalid:
        return {};
    }
    return {};
}

} // namespace amigaguide
