#include "amigaguide/destination.h"

#include <algorithm>
#include <cctype>

namespace amigaguide {
namespace {

std::string lower_copy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

bool has_prefix_ci(const std::string& value, const char* prefix)
{
    const std::string p(prefix);
    if (value.size() < p.size()) return false;
    return lower_copy(value.substr(0, p.size())) == p;
}

} // namespace

Destination::Destination(DestinationType type, std::string value, std::string uri)
    : type_(type), value_(std::move(value)), uri_(std::move(uri))
{
}

Destination Destination::node(std::string name)
{
    while (!name.empty() && name.front() == '/') name.erase(name.begin());
    if (name.empty()) return {};
    return Destination(DestinationType::Node, name, "node:" + name);
}

Destination Destination::parse(const std::string& input)
{
    if (input.empty()) return {};

    if (has_prefix_ci(input, "node:")) {
        return node(input.substr(5));
    }
    if (has_prefix_ci(input, "file:")) {
        return Destination(DestinationType::File, input, input);
    }
    if (has_prefix_ci(input, "http:")) {
        return Destination(DestinationType::Http, input, input);
    }
    if (has_prefix_ci(input, "https:")) {
        return Destination(DestinationType::Https, input, input);
    }
    if (has_prefix_ci(input, "ag:")) {
        return Destination(DestinationType::Ag, input, input);
    }

    // Existing AmigaGuide links may name a node directly.
    if (input.find(':') == std::string::npos) return node(input);

    // Unknown schemes are deliberately rejected rather than treated as
    // executable commands or opaque destinations.
    return {};
}

} // namespace amigaguide
