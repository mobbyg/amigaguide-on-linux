#pragma once

#include "amigaguide/destination.h"

namespace amigaguide {

enum class ResolutionKind
{
    Invalid,
    InternalNode,
    LocalFile,
    RemoteHttp,
    LibraryDocument
};

struct Resolution
{
    ResolutionKind kind = ResolutionKind::Invalid;
    std::string value;

    bool resolved() const { return kind != ResolutionKind::Invalid; }
};

class DestinationResolver
{
public:
    virtual ~DestinationResolver() = default;
    virtual Resolution resolve(const Destination& destination) const = 0;
};

class LocalDestinationResolver final : public DestinationResolver
{
public:
    Resolution resolve(const Destination& destination) const override;
};

} // namespace amigaguide
