#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace amigaguide {

struct Node {
    std::string name;
    std::string title;
    std::string keywords;
    std::string prev;
    std::string next;
    std::string help;
    std::string toc;
    std::string index;
    std::string font;
    std::string on_open;
    std::string on_close;
    int tab_width = 0;
    bool word_wrap = true;
    bool smart_wrap = false;
    bool proportional = false;
    std::size_t source_begin = 0;
    std::size_t source_end = 0;
};

struct DocumentMetadata {
    std::string name;
    std::string author;
    std::string version;
    std::string copyright;
    std::string font;
    std::string help;
    std::string toc;
    std::string index;
    std::string word_delimiter;
    int width = 0;
    int height = 0;
    int tab_width = 0;
    bool word_wrap = true;
    bool smart_wrap = false;
};

class Document {
public:
    Document() = default;

    const DocumentMetadata& metadata() const noexcept { return metadata_; }
    const std::vector<Node>& nodes() const noexcept { return nodes_; }
    const Node* find_node(std::string_view name) const noexcept;

    void set_source(std::string source) { source_ = std::move(source); }
    const std::string& source() const noexcept { return source_; }
    std::string& source() noexcept { return source_; }
    DocumentMetadata& metadata() noexcept { return metadata_; }
    std::vector<Node>& nodes() noexcept { return nodes_; }

private:
    DocumentMetadata metadata_;
    std::vector<Node> nodes_;
    std::string source_;
};

} // namespace amigaguide
