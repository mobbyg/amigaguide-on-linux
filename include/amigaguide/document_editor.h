#pragma once

#include "amigaguide/document.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace amigaguide {

enum class DocumentProperty {
    Name, Author, Version, Copyright, Font, Help, Toc, Index, WordDelimiter, Width, Height, TabWidth
};

enum class DocumentFlag { WordWrap, SmartWrap };
enum class NodeProperty { Keywords, Prev, Next, Help, Toc, Index, Font, TabWidth };
enum class NodeFlag { WordWrap, SmartWrap, Proportional };

class DocumentEditor {
public:
    explicit DocumentEditor(Document& document) : document_(document) {}
    bool set_document_property(DocumentProperty property, std::string value, std::string* error = nullptr);
    bool set_document_flag(DocumentFlag flag, bool enabled, std::string* error = nullptr);
    bool add_node(std::string name, std::string title, std::string* error = nullptr);
    bool rename_node(std::size_t index, std::string name, std::string* error = nullptr);
    bool set_node_title(std::size_t index, std::string title, std::string* error = nullptr);
    bool set_node_property(std::size_t index, NodeProperty property, std::string value, std::string* error = nullptr);
    bool set_node_flag(std::size_t index, NodeFlag flag, bool enabled, std::string* error = nullptr);
    bool remove_node(std::size_t index, std::string* error = nullptr);
private:
    bool replace_source(std::size_t begin, std::size_t end, std::string_view replacement, std::string* error);
    static void set_error(std::string* error, std::string message);
    Document& document_;
};

} // namespace amigaguide
