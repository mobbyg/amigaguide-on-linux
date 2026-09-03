#include "amigaguide/links.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}
}

int main()
{
    const std::string source =
        "@node Main\n"
        "Read @{\"the preamble\" LINK Preamble}.\n"
        "Legacy: @{Preamble|\"old link\" LINK Preamble}.\n"
        "External: @{\"website\" LINK \"https://example.com\"}.\n"
        "@endnode\n"
        "@node Preamble\n"
        "@endnode\n";

    const auto links = amigaguide::find_links(source);
    check(links.size() == 3, "three links found");

    check(links[0].label == "the preamble", "canonical label");
    check(links[0].target == "Preamble", "canonical target");
    check(links[0].source_node == "Main", "canonical source node");
    check(!links[0].legacy, "canonical syntax marked non-legacy");
    check(links[0].line == 2, "canonical line");

    check(links[1].label == "old link", "legacy label");
    check(links[1].target == "Preamble", "legacy target");
    check(links[1].source_node == "Main", "legacy source node");
    check(links[1].legacy, "legacy syntax detected");

    check(links[2].label == "website", "external label");
    check(links[2].target == "https://example.com", "external target preserved");

    const auto broken = amigaguide::find_links(
        "@node Main\nBroken: @{\"missing\" LINK Missing}\n@endnode\n");
    check(broken.size() == 1, "broken link is still inspectable");
    check(broken[0].target == "Missing", "broken target preserved");
    check(broken[0].source_offset == 19, "source offset recorded");
    check(broken[0].column == 9, "source column recorded");

    std::cout << "PASS\n";
    return 0;
}
