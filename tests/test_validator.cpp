#include "amigaguide/parser.h"
#include "amigaguide/validator.h"

#include <iostream>
#include <string>

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}
}

int main()
{
    const std::string valid =
        "@database Test\n"
        "@node Main\n"
        "@title Main\n"
        "See @{\"Other\" LINK Other}.\n"
        "@endnode\n"
        "@node Other\n"
        "@title Other\n"
        "Back @{Main|\"Main\" LINK Main}.\n"
        "@endnode\n";

    amigaguide::Validator validator;
    auto result = validator.validate(valid);
    check(result.ok(), "valid guide has no errors");
    check(result.warning_count() == 0, "valid guide has no warnings");

    const std::string duplicate =
        "@node Main\n@endnode\n"
        "@node main\n@endnode\n";
    result = validator.validate(duplicate);
    check(result.error_count() == 1, "duplicate node is detected");
    check(result.issues[0].line == 3, "duplicate node reports its line");

    const std::string broken_link =
        "@node Main\n"
        "Missing @{\"Missing\" LINK Missing}.\n"
        "@endnode\n";
    result = validator.validate(broken_link);
    check(result.error_count() == 1, "broken local link is detected");
    check(result.issues[0].message.find("Missing") != std::string::npos, "broken link names its target");
    check(result.issues[0].node == "Main", "broken link identifies its node");

    const std::string malformed =
        "@node Main\n"
        "text\n";
    result = validator.validate(malformed);
    check(result.error_count() == 1, "missing endnode is detected");
    check(result.issues[0].line == 2, "malformed node reports parser line");

    const std::string unknown =
        "@node Main\n"
        "@bogus value\n"
        "@endnode\n";
    result = validator.validate(unknown);
    check(result.ok(), "unknown command is a warning rather than an error");
    check(result.warning_count() == 1, "unknown command warning is reported");

    const std::string unterminated_attribute =
        "@node Main\n"
        "Broken @{\"link\" LINK Main\n"
        "@endnode\n";
    result = validator.validate(unterminated_attribute);
    check(result.error_count() == 1, "unterminated attribute is detected");
    check(result.issues[0].line == 2, "unterminated attribute reports its line");

    return failures == 0 ? 0 : 1;
}
