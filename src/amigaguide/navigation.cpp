#include "amigaguide/navigation.h"

namespace amigaguide {

void NavigationHistory::clear()
{
    entries_.clear();
    index_ = -1;
}

void NavigationHistory::visit(const std::string& destination)
{
    if (destination.empty()) return;

    if (index_ >= 0 && index_ < static_cast<int>(entries_.size()) &&
        entries_[index_] == destination) {
        return;
    }

    if (index_ + 1 < static_cast<int>(entries_.size())) {
        entries_.erase(entries_.begin() + index_ + 1, entries_.end());
    }

    entries_.push_back(destination);
    index_ = static_cast<int>(entries_.size()) - 1;
}

bool NavigationHistory::back()
{
    if (!can_back()) return false;
    --index_;
    return true;
}

bool NavigationHistory::forward()
{
    if (!can_forward()) return false;
    ++index_;
    return true;
}

bool NavigationHistory::can_back() const
{
    return index_ > 0;
}

bool NavigationHistory::can_forward() const
{
    return index_ >= 0 && index_ + 1 < static_cast<int>(entries_.size());
}

bool NavigationHistory::empty() const
{
    return entries_.empty();
}

const std::string& NavigationHistory::current() const
{
    static const std::string empty_destination;
    if (index_ < 0 || index_ >= static_cast<int>(entries_.size())) return empty_destination;
    return entries_[index_];
}

} // namespace amigaguide
