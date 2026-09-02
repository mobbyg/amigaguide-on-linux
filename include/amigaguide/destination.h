#pragma once

#include <string>

namespace amigaguide {

enum class DestinationType
{
    Invalid,
    Node,
    File,
    Http,
    Https,
    Ag
};

class Destination final
{
public:
    static Destination node(std::string name);
    static Destination parse(const std::string& value);

    DestinationType type() const { return type_; }
    const std::string& value() const { return value_; }
    const std::string& uri() const { return uri_; }
    bool valid() const { return type_ != DestinationType::Invalid; }

private:
    Destination(DestinationType type, std::string value, std::string uri);

    DestinationType type_ = DestinationType::Invalid;
    std::string value_;
    std::string uri_;
};

} // namespace amigaguide
