#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "amigaguide/document.h"

namespace amigaguide {

enum class ValidationSeverity {
    Error,
    Warning
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Error;
    std::size_t line = 0;
    std::size_t column = 0;
    std::string message;
    std::string node;
};

struct ValidationResult {
    std::vector<ValidationIssue> issues;

    bool ok() const noexcept;
    std::size_t error_count() const noexcept;
    std::size_t warning_count() const noexcept;
};

class Validator {
public:
    ValidationResult validate(const std::string& source, const Document* parsed = nullptr) const;
};

} // namespace amigaguide
