#pragma once

#include <string>
#include <vector>

namespace amigaguide {

// Browser-style navigation history for document destinations.
// Search operations deliberately do not belong here: finding text is not
// document navigation and should not create Back/Forward entries.
class NavigationHistory final
{
public:
    void clear();

    // Visit a destination. If the destination is already current, no new
    // history entry is created. Visiting after going Back discards Forward
    // history, matching normal browser behavior.
    void visit(const std::string& destination);

    bool back();
    bool forward();

    bool can_back() const;
    bool can_forward() const;
    bool empty() const;
    const std::string& current() const;

private:
    std::vector<std::string> entries_;
    int index_ = -1;
};

} // namespace amigaguide
