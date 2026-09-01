#include "amigaguide/document.h"

#include <algorithm>
#include <cctype>

namespace amigaguide {

const Node* Document::find_node(std::string_view name) const noexcept
{
    auto equals = [name](const Node& node) {
        if (node.name.size() != name.size()) return false;
        for (std::size_t i = 0; i < name.size(); ++i) {
            const auto a = static_cast<unsigned char>(node.name[i]);
            const auto b = static_cast<unsigned char>(name[i]);
            if (std::tolower(a) != std::tolower(b)) return false;
        }
        return true;
    };

    const auto it = std::find_if(nodes_.begin(), nodes_.end(), equals);
    return it == nodes_.end() ? nullptr : &*it;
}

} // namespace amigaguide
