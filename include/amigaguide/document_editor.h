#pragma once

#include "amigaguide/document.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace amigaguide {

class DocumentEditor {
public:
    explicit DocumentEditor(Document& document) : document_(document) {}

    bool add_node(std::string name, std::string title, std::string* error = nullptr);
    bool rename_node(std::size_t index, std::string name, std::string* error = nullptr);
    bool set_node_title(std::size_t index, std::string title, std::string* error = nullptr);
    bool remove_node(std::size_t index, std::string* error = nullptr);

private:
    bool replace_source(std::size_t begin, std::size_t end, std::string_view replacement, std::string* error);
    static void set_error(std::string* error, std::string message);

    Document& document_;
};

} // namespace amigaguide
